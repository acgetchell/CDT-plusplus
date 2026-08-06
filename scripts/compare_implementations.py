"""Run and reanalyze local CDT++/Rust reference comparisons."""

import argparse
import json
import os
import platform
import shutil
import subprocess
import sys
import time
from dataclasses import dataclass, field
from datetime import UTC, datetime
from pathlib import Path
from typing import TYPE_CHECKING, Any, cast

from jsonschema import Draft202012Validator
from jsonschema.exceptions import SchemaError

from scripts.experiment_artifacts import OutputDirectoryExistsError, artifact_record, sha256, staged_run_directory, write_json
from scripts.validate_reference_fixtures import load_json, validate_document

if TYPE_CHECKING:
    from collections.abc import Mapping, Sequence


def _repository_root() -> Path:
    """Find the source checkout for editable and wheel-based invocations."""
    source_root = Path(__file__).resolve().parents[1]
    if (source_root / "reference" / "schema" / "result-v1.schema.json").is_file():
        return source_root
    working_root = Path.cwd()
    if (working_root / "reference" / "schema" / "result-v1.schema.json").is_file():
        return working_root
    return source_root


ROOT = _repository_root()
DEFAULT_PROTOCOL = ROOT / "reference" / "fixtures" / "v1" / "protocol.json"
DEFAULT_FIXTURE_SCHEMA = ROOT / "reference" / "schema" / "fixture-v1.schema.json"
DEFAULT_RESULT_SCHEMA = ROOT / "reference" / "schema" / "result-v1.schema.json"
DEFAULT_REFERENCE_RESULT = ROOT / "reference" / "raw" / "v1" / "cpp-reference.json"
DEFAULT_REFERENCE_MANIFEST = ROOT / "reference" / "manifests" / "v1" / "macos-arm64.json"
DEFAULT_MANIFEST_SCHEMA = ROOT / "reference" / "schema" / "run-manifest-v1.schema.json"
DEFAULT_CPP_EXECUTABLE = ROOT / "out" / "build" / "reference" / "tests" / "CDT_reference_fixture"
INPUT_PATHS = {
    "protocol": Path("inputs/protocol.json"),
    "fixture_schema": Path("inputs/fixture-v1.schema.json"),
    "result_schema": Path("inputs/result-v1.schema.json"),
    "reference_result": Path("inputs/cpp-reference.json"),
    "reference_manifest": Path("inputs/reference-manifest.json"),
    "manifest_schema": Path("inputs/run-manifest-v1.schema.json"),
}
EXPECTED_ARTIFACT_PATHS = frozenset(path.as_posix() for path in INPUT_PATHS.values()) | frozenset(
    f"raw/{label}/{filename}" for label in ("cpp", "rust") for filename in ("process.json", "stderr.txt", "stdout.txt")
)
PASSTHROUGH_ENVIRONMENT = (
    "DYLD_LIBRARY_PATH",
    "LANG",
    "LC_ALL",
    "LD_LIBRARY_PATH",
    "PATH",
    "SystemRoot",
    "TEMP",
    "TMP",
    "TMPDIR",
    "WINDIR",
)


@dataclass(frozen=True)
class ProcessSpec:
    """One explicitly configured implementation command."""

    label: str
    executable: str
    arguments: tuple[str, ...]
    cwd: Path


@dataclass
class ComparisonRecorder:
    """Accumulate compact comparison counts and actionable failures."""

    implementation: str = "rust"
    exact: int = 0
    numerical: int = 0
    failures: list[dict[str, object]] = field(default_factory=list)

    def compare_exact(
        self,
        fixture: str,
        field_name: str,
        expected: object,
        observed: object,
        raw_artifact: str,
    ) -> None:
        """Record one protocol-designated exact comparison."""
        self.exact += 1
        if expected != observed:
            self.failures.append(
                _failure(
                    fixture,
                    field_name,
                    {"kind": "exact"},
                    expected,
                    observed,
                    raw_artifact,
                    self.implementation,
                )
            )

    def compare_numerical(  # noqa: PLR0913, PLR0917 - One finding carries the complete failure contract.
        self,
        fixture: str,
        field_name: str,
        expected: object,
        observed: object,
        tolerance_name: str,
        tolerance: Mapping[str, object],
        raw_artifact: str,
    ) -> None:
        """Record one named absolute-plus-relative comparison."""
        self.numerical += 1
        matches = False
        try:
            expected_number = _as_float(expected)
            observed_number = _as_float(observed)
            absolute = _as_float(tolerance["absolute"])
            relative = _as_float(tolerance["relative"])
            matches = abs(observed_number - expected_number) <= absolute + relative * abs(expected_number)
        except KeyError, TypeError, ValueError:
            pass
        if not matches:
            self.failures.append(
                _failure(
                    fixture,
                    field_name,
                    {
                        "absolute": tolerance.get("absolute"),
                        "kind": "numerical",
                        "name": tolerance_name,
                        "relative": tolerance.get("relative"),
                    },
                    expected,
                    observed,
                    raw_artifact,
                    self.implementation,
                )
            )

    def record_failure(  # noqa: PLR0913, PLR0917 - Producer failures use the same complete failure contract.
        self,
        implementation: str,
        fixture: str,
        field_name: str,
        rule: str,
        observed: object,
        raw_artifact: str,
    ) -> None:
        """Record a producer or schema failure before field comparison."""
        self.failures.append(
            _failure(
                fixture,
                field_name,
                {"kind": rule},
                "valid structured result",
                observed,
                raw_artifact,
                implementation,
            )
        )


def _failure(  # noqa: PLR0913, PLR0917 - The serialized finding keeps these fields explicit.
    fixture: str,
    field_name: str,
    rule: Mapping[str, object],
    expected: object,
    observed: object,
    raw_artifact: str,
    implementation: str = "rust",
) -> dict[str, object]:
    """Build one stable, machine-readable implementation discrepancy."""
    return {
        "expected": expected,
        "field": field_name,
        "fixture": fixture,
        "implementation": implementation,
        "observed": observed,
        "raw_artifact": raw_artifact,
        "rule": dict(rule),
    }


def _as_float(value: object) -> float:
    """Narrow one schema-checked JSON number for static analysis."""
    if not isinstance(value, int | float | str):
        message = f"not a numeric JSON value: {value!r}"
        raise TypeError(message)
    return float(value)


def _utc_now() -> str:
    """Return a canonical UTC timestamp for run provenance."""
    return datetime.now(UTC).replace(microsecond=0).isoformat().replace("+00:00", "Z")


def _positive_timeout(value: str) -> float:
    """Parse a finite positive subprocess timeout."""
    try:
        parsed = float(value)
    except ValueError as error:
        message = "timeout must be a positive number"
        raise argparse.ArgumentTypeError(message) from error
    if not 0 < parsed < float("inf"):
        message = "timeout must be a positive finite number"
        raise argparse.ArgumentTypeError(message)
    return parsed


def _resolve_executable(command: str, cwd: Path) -> Path:
    """Resolve one command without invoking a shell or accepting a directory."""
    candidate = Path(command)
    if candidate.is_absolute() or candidate.parent != Path():
        resolved = candidate if candidate.is_absolute() else cwd / candidate
    else:
        found = shutil.which(command)
        if found is None:
            message = f"executable not found: {command}"
            raise ValueError(message)
        resolved = Path(found)
    resolved = resolved.resolve()
    if not resolved.is_file():
        message = f"executable is not a file: {resolved}"
        raise ValueError(message)
    if os.name != "nt" and not os.access(resolved, os.X_OK):
        message = f"executable is not executable: {resolved}"
        raise ValueError(message)
    return resolved


def _expanded_arguments(arguments: Sequence[str], bundle: Path) -> list[str]:
    """Substitute only documented comparison input placeholders."""
    replacements = {
        "{manifest}": str((bundle / INPUT_PATHS["reference_manifest"]).resolve()),
        "{protocol}": str((bundle / INPUT_PATHS["protocol"]).resolve()),
        "{result_schema}": str((bundle / INPUT_PATHS["result_schema"]).resolve()),
    }
    expanded: list[str] = []
    for argument in arguments:
        expanded_argument = argument
        for placeholder, value in replacements.items():
            expanded_argument = expanded_argument.replace(placeholder, value)
        expanded.append(expanded_argument)
    return expanded


def _command_environment(bundle: Path) -> dict[str, str]:
    """Build a credential-free environment shared by both implementations."""
    environment = {name: os.environ[name] for name in PASSTHROUGH_ENVIRONMENT if name in os.environ}
    environment.update(
        {
            "CDT_COMPARISON_PROTOCOL": str((bundle / INPUT_PATHS["protocol"]).resolve()),
            "CDT_COMPARISON_REFERENCE_MANIFEST": str((bundle / INPUT_PATHS["reference_manifest"]).resolve()),
            "CDT_COMPARISON_RESULT_SCHEMA": str((bundle / INPUT_PATHS["result_schema"]).resolve()),
        }
    )
    return environment


def _bytes(value: bytes | str | None) -> bytes:
    """Normalize TimeoutExpired payloads to exact bytes."""
    if value is None:
        return b""
    if isinstance(value, bytes):
        return value
    return value.encode()


def _run_process(spec: ProcessSpec, bundle: Path, timeout_seconds: float) -> dict[str, object]:
    """Run one implementation and retain all process-boundary evidence."""
    raw_directory = bundle / "raw" / spec.label
    raw_directory.mkdir(parents=True)
    stdout_path = raw_directory / "stdout.txt"
    stderr_path = raw_directory / "stderr.txt"
    started = time.perf_counter_ns()
    exit_status: int | None = None
    timed_out = False
    launch_error: str | None = None
    stdout = b""
    stderr = b""
    executable: Path | None = None
    command: list[str] = [spec.executable, *spec.arguments]
    environment = _command_environment(bundle)
    try:
        executable = _resolve_executable(spec.executable, spec.cwd)
        command = [str(executable), *_expanded_arguments(spec.arguments, bundle)]
        completed = subprocess.run(  # noqa: S603
            command,
            cwd=spec.cwd,
            env=environment,
            check=False,
            capture_output=True,
            timeout=timeout_seconds,
        )
        exit_status = completed.returncode
        stdout = completed.stdout
        stderr = completed.stderr
    except subprocess.TimeoutExpired as error:
        timed_out = True
        stdout = _bytes(error.stdout)
        stderr = _bytes(error.stderr)
        launch_error = f"command timed out after {timeout_seconds:g} seconds"
    except (OSError, ValueError) as error:
        launch_error = str(error)
    duration_ns = time.perf_counter_ns() - started
    stdout_path.write_bytes(stdout)
    stderr_path.write_bytes(stderr)
    executable_record: dict[str, object] | None = None
    if executable is not None:
        executable_record = {
            "bytes": executable.stat().st_size,
            "path": str(executable),
            "sha256": sha256(executable),
        }
    record: dict[str, object] = {
        "command": command,
        "cwd": str(spec.cwd.resolve()),
        "duration_ns": duration_ns,
        "environment": {
            **{name: value for name, value in environment.items() if not name.startswith("CDT_COMPARISON_")},
            "CDT_COMPARISON_PROTOCOL": INPUT_PATHS["protocol"].as_posix(),
            "CDT_COMPARISON_REFERENCE_MANIFEST": INPUT_PATHS["reference_manifest"].as_posix(),
            "CDT_COMPARISON_RESULT_SCHEMA": INPUT_PATHS["result_schema"].as_posix(),
        },
        "executable": executable_record,
        "exit_status": exit_status,
        "label": spec.label,
        "launch_error": launch_error,
        "stderr": f"raw/{spec.label}/stderr.txt",
        "stdout": f"raw/{spec.label}/stdout.txt",
        "timed_out": timed_out,
    }
    write_json(raw_directory / "process.json", record)
    return record


def _generic_result_schema(
    result_schema: Mapping[str, Any],
    fixture_schema: Mapping[str, Any],
) -> dict[str, Any]:
    """Reuse #94 domain definitions with language-neutral provenance."""
    definitions = dict(result_schema["$defs"])
    definitions["site"] = fixture_schema["$defs"]["site"]
    definitions["transitionObservation"] = fixture_schema["$defs"]["transition"]
    return {
        "$schema": "https://json-schema.org/draft/2020-12/schema",
        "type": "object",
        "additionalProperties": False,
        "required": ["schema", "implementation", "states", "actions"],
        "properties": {
            "schema": {"enum": ["cdt-reference-raw-v1", "cdt-comparison-result-v1"]},
            "implementation": {
                "type": "object",
                "required": ["name", "version", "revision"],
                "properties": {
                    "name": {"$ref": "#/$defs/nonemptyString"},
                    "version": {"$ref": "#/$defs/nonemptyString"},
                    "revision": {"$ref": "#/$defs/nonemptyString"},
                },
                "additionalProperties": True,
            },
            "states": {
                "type": "array",
                "minItems": 1,
                "items": {"$ref": "#/$defs/state"},
            },
            "actions": {
                "type": "array",
                "minItems": 1,
                "items": {"$ref": "#/$defs/action"},
            },
            "transitions": {
                "type": "array",
                "items": {"$ref": "#/$defs/transitionObservation"},
            },
        },
        "$defs": definitions,
    }


def _load_result(
    bundle: Path,
    label: str,
    result_schema: dict[str, Any],
    fixture_schema: dict[str, Any],
    recorder: ComparisonRecorder,
) -> dict[str, Any] | None:
    """Parse and schema-check one retained producer result."""
    process_path = bundle / "raw" / label / "process.json"
    stdout_path = bundle / "raw" / label / "stdout.txt"
    raw_artifact = stdout_path.relative_to(bundle).as_posix()
    try:
        process = load_json(process_path)
    except (OSError, TypeError, ValueError) as error:
        recorder.record_failure(label, "process", "process.json", "artifact", str(error), raw_artifact)
        return None
    if process.get("exit_status") != 0:
        observed = process.get("launch_error") or f"exit status {process.get('exit_status')}"
        recorder.record_failure(label, "process", "exit_status", "process", observed, raw_artifact)
        return None
    try:
        payload = stdout_path.read_text(encoding="utf-8")
        result = json.loads(payload, parse_constant=_reject_nonstandard_number)
        if not isinstance(result, dict):
            message = "top-level JSON value must be an object"
            raise TypeError(message)
        schema = result_schema if result.get("schema") == "cdt-reference-raw-v1" else _generic_result_schema(result_schema, fixture_schema)
        validate_document(result, schema, stdout_path)
    except (json.JSONDecodeError, OSError, TypeError, UnicodeDecodeError, ValueError) as error:
        recorder.record_failure(label, "result", "schema", "schema", str(error), raw_artifact)
        return None
    return result


def _reject_nonstandard_number(value: str) -> None:
    """Reject NaN and infinities in producer JSON."""
    message = f"nonstandard JSON number {value!r}"
    raise ValueError(message)


def _compare_state(
    expected: Mapping[str, Any],
    observed: Mapping[str, Any],
    protocol: Mapping[str, Any],
    recorder: ComparisonRecorder,
    raw_artifact: str,
) -> None:
    """Compare one state without deriving scientific expectations in Python."""
    fixture = f"state:{expected.get('id', '<unknown>')}"
    for name in ("id", "coordinate_units", "time_units", "time_bounds", "f_vector"):
        recorder.compare_exact(fixture, name, expected.get(name), observed.get(name), raw_artifact)
    expected_vertices = expected.get("vertices", [])
    observed_vertices = observed.get("vertices", [])
    recorder.compare_exact(fixture, "vertices.length", len(expected_vertices), len(observed_vertices), raw_artifact)
    coordinate_tolerance = protocol["tolerances"]["coordinates"]
    for index, (left, right) in enumerate(zip(expected_vertices, observed_vertices, strict=False)):
        recorder.compare_exact(fixture, f"vertices[{index}].id", left.get("id"), right.get("id"), raw_artifact)
        recorder.compare_exact(fixture, f"vertices[{index}].time", left.get("time"), right.get("time"), raw_artifact)
        left_position = left.get("position", [])
        right_position = right.get("position", [])
        recorder.compare_exact(fixture, f"vertices[{index}].position.length", len(left_position), len(right_position), raw_artifact)
        for coordinate, (expected_value, observed_value) in enumerate(zip(left_position, right_position, strict=False)):
            recorder.compare_numerical(
                fixture,
                f"vertices[{index}].position[{coordinate}]",
                expected_value,
                observed_value,
                "coordinates",
                coordinate_tolerance,
                raw_artifact,
            )
    for collection in ("edges", "facets", "cells"):
        left_items = expected.get(collection, [])
        right_items = observed.get(collection, [])
        recorder.compare_exact(fixture, f"{collection}.length", len(left_items), len(right_items), raw_artifact)
        for index, (left, right) in enumerate(zip(left_items, right_items, strict=False)):
            recorder.compare_exact(fixture, f"{collection}[{index}]", left, right, raw_artifact)


def _compare_action(
    expected: Mapping[str, Any],
    observed: Mapping[str, Any],
    protocol: Mapping[str, Any],
    recorder: ComparisonRecorder,
    raw_artifact: str,
) -> None:
    """Compare one action fixture using only the declared #94 tolerance."""
    fixture = f"action:{expected.get('id', '<unknown>')}"
    for name in ("id", "counts", "parameters"):
        recorder.compare_exact(fixture, name, expected.get(name), observed.get(name), raw_artifact)
    tolerance = protocol["tolerances"]["regge_action_closed_form"]
    recorder.compare_numerical(
        fixture,
        "value",
        expected.get("value"),
        observed.get("value"),
        "regge_action_closed_form",
        tolerance,
        raw_artifact,
    )


def _compare_transitions(
    expected_result: Mapping[str, Any],
    observed_result: Mapping[str, Any],
    protocol: Mapping[str, Any],
    recorder: ComparisonRecorder,
    raw_artifact: str,
) -> str:
    """Compare optional producer observations for #94 deterministic transitions."""
    expected = expected_result.get("transitions")
    observed = observed_result.get("transitions")
    if expected is None and observed is None:
        return "state-payload-only"
    recorder.compare_exact("transitions", "presence", expected is not None, observed is not None, raw_artifact)
    if not isinstance(expected, list) or not isinstance(observed, list):
        return "incomplete"
    recorder.compare_exact("transitions", "length", len(expected), len(observed), raw_artifact)
    action_tolerance = protocol["tolerances"]["regge_action_closed_form"]
    probability_tolerance = protocol["tolerances"]["acceptance_probability"]
    for left_value, right_value in zip(expected, observed, strict=False):
        left = cast("dict[str, Any]", left_value)
        right = cast("dict[str, Any]", right_value)
        fixture = f"transition:{left.get('id', '<unknown>')}"
        for name in ("id", "move", "before", "proposed", "site", "parameters", "acceptance_variate", "accepted", "committed"):
            recorder.compare_exact(fixture, name, left.get(name), right.get(name), raw_artifact)
        recorder.compare_numerical(
            fixture,
            "action_delta",
            left.get("action_delta"),
            right.get("action_delta"),
            "regge_action_closed_form",
            action_tolerance,
            raw_artifact,
        )
        for name in ("proposal_probability", "reverse_probability", "acceptance_probability"):
            recorder.compare_numerical(
                fixture,
                name,
                left.get(name),
                right.get(name),
                "acceptance_probability",
                probability_tolerance,
                raw_artifact,
            )
    return "observed"


def _compare_results(  # noqa: PLR0913 - The optional transition phase shares one comparison context.
    cpp: Mapping[str, Any],
    rust: Mapping[str, Any],
    protocol: Mapping[str, Any],
    recorder: ComparisonRecorder,
    raw_artifact: str = "raw/rust/stdout.txt",
    *,
    compare_transitions: bool = True,
) -> str:
    """Compare the two scientific payloads according to the #94 policy."""
    cpp_states = cpp.get("states", [])
    rust_states = rust.get("states", [])
    recorder.compare_exact("result", "states.length", len(cpp_states), len(rust_states), raw_artifact)
    recorder.compare_exact(
        "result",
        "states.order",
        [item.get("id") for item in cpp_states],
        [item.get("id") for item in rust_states],
        raw_artifact,
    )
    for expected, observed in zip(cpp_states, rust_states, strict=False):
        _compare_state(expected, observed, protocol, recorder, raw_artifact)

    cpp_actions = cpp.get("actions", [])
    rust_actions = rust.get("actions", [])
    recorder.compare_exact("result", "actions.length", len(cpp_actions), len(rust_actions), raw_artifact)
    recorder.compare_exact(
        "result",
        "actions.order",
        [item.get("id") for item in cpp_actions],
        [item.get("id") for item in rust_actions],
        raw_artifact,
    )
    for expected, observed in zip(cpp_actions, rust_actions, strict=False):
        _compare_action(expected, observed, protocol, recorder, raw_artifact)
    if compare_transitions:
        return _compare_transitions(cpp, rust, protocol, recorder, raw_artifact)
    return "not-compared"


def analyze_bundle(bundle: Path) -> dict[str, object]:
    """Rebuild a deterministic summary from retained local artifacts."""
    protocol_path = bundle / INPUT_PATHS["protocol"]
    result_schema_path = bundle / INPUT_PATHS["result_schema"]
    protocol = load_json(protocol_path)
    fixture_schema = load_json(bundle / INPUT_PATHS["fixture_schema"])
    result_schema = load_json(result_schema_path)
    reference_result_path = bundle / INPUT_PATHS["reference_result"]
    reference_result = load_json(reference_result_path)
    reference_manifest = load_json(bundle / INPUT_PATHS["reference_manifest"])
    manifest_schema = load_json(bundle / INPUT_PATHS["manifest_schema"])
    validate_document(protocol, fixture_schema, protocol_path)
    validate_document(reference_result, result_schema, reference_result_path)
    validate_document(reference_manifest, manifest_schema, bundle / INPUT_PATHS["reference_manifest"])

    recorder = ComparisonRecorder()
    cpp = _load_result(bundle, "cpp", result_schema, fixture_schema, recorder)
    rust = _load_result(bundle, "rust", result_schema, fixture_schema, recorder)
    transition_coverage = "unavailable"
    if cpp is not None:
        recorder.implementation = "cpp"
        _compare_results(
            reference_result,
            cpp,
            protocol,
            recorder,
            "raw/cpp/stdout.txt",
            compare_transitions=False,
        )
        if cpp.get("transitions") is not None:
            _compare_transitions(
                {"transitions": protocol["transitions"]},
                cpp,
                protocol,
                recorder,
                "raw/cpp/stdout.txt",
            )
    if rust is not None:
        recorder.implementation = "rust"
        rust_reference = cpp if cpp is not None else reference_result
        transition_coverage = _compare_results(
            rust_reference,
            rust,
            protocol,
            recorder,
            compare_transitions=cpp is not None,
        )
    policy = protocol["comparison_policy"]
    unsupported = list(policy["unsupported"])
    if transition_coverage == "state-payload-only":
        unsupported.append("producer transition observations are absent; only the exact before/proposed state payloads are compared")
    implementations = {
        label: result.get("implementation") if result is not None else None for label, result in (("reference", reference_result), ("cpp", cpp), ("rust", rust))
    }
    summary: dict[str, object] = {
        "classifications": {
            "exact": policy["exact"],
            "implementation_specific": policy["implementation_specific"],
            "numerical": policy["numerical"],
            "unsupported": unsupported,
        },
        "counts": {
            "exact": recorder.exact,
            "failed": len(recorder.failures),
            "implementation_specific": len(policy["implementation_specific"]),
            "numerical": recorder.numerical,
            "unsupported": len(unsupported),
        },
        "failures": recorder.failures,
        "fixture_set": protocol["fixture_set"],
        "implementations": implementations,
        "schema": "cdt-comparison-summary-v1",
        "status": "passed" if not recorder.failures else "failed",
        "transition_coverage": transition_coverage,
    }
    return summary


def _copy_inputs(bundle: Path, sources: Mapping[str, Path]) -> dict[str, dict[str, object]]:
    """Copy every schema/protocol input needed for offline reanalysis."""
    records: dict[str, dict[str, object]] = {}
    for name, source in sources.items():
        resolved = source.resolve()
        if not resolved.is_file():
            message = f"comparison input does not exist: {resolved}"
            raise ValueError(message)
        destination = bundle / INPUT_PATHS[name]
        destination.parent.mkdir(parents=True, exist_ok=True)
        shutil.copyfile(resolved, destination)
        records[name] = {
            "original_path": str(resolved),
            **artifact_record(destination, bundle),
        }
    return records


def _validate_input_sources(sources: Mapping[str, Path]) -> None:
    """Reject incomplete or invalid #94 inputs before launching either producer."""
    missing = [source.resolve() for source in sources.values() if not source.resolve().is_file()]
    if missing:
        message = f"comparison input does not exist: {missing[0]}"
        raise ValueError(message)
    fixture_schema = load_json(sources["fixture_schema"])
    result_schema = load_json(sources["result_schema"])
    manifest_schema = load_json(sources["manifest_schema"])
    protocol = load_json(sources["protocol"])
    reference_result = load_json(sources["reference_result"])
    reference_manifest = load_json(sources["reference_manifest"])
    try:
        for schema in (fixture_schema, result_schema, manifest_schema, _generic_result_schema(result_schema, fixture_schema)):
            Draft202012Validator.check_schema(schema)
    except SchemaError as error:
        message = f"invalid comparison schema: {error.message}"
        raise ValueError(message) from error
    validate_document(protocol, fixture_schema, sources["protocol"])
    validate_document(reference_result, result_schema, sources["reference_result"])
    validate_document(reference_manifest, manifest_schema, sources["reference_manifest"])


def _artifact_records(bundle: Path) -> list[dict[str, object]]:
    """Hash retained inputs and raw process evidence for the run manifest."""
    paths = sorted((*bundle.glob("inputs/*"), *bundle.glob("raw/*/*")))
    return [artifact_record(path, bundle) for path in paths if path.is_file()]


def _validated_artifact_path(bundle_root: Path, value: object, index: int) -> tuple[str, Path]:
    """Parse one canonical artifact path contained by its comparison bundle."""
    if not isinstance(value, str) or not value:
        message = f"stored comparison artifact record {index} has an invalid path"
        raise ValueError(message)
    relative_path = Path(value)
    if relative_path.is_absolute() or ".." in relative_path.parts or relative_path.as_posix() != value:
        message = f"stored comparison artifact path is not a canonical relative path: {value!r}"
        raise ValueError(message)
    path = bundle_root / relative_path
    try:
        path.resolve().relative_to(bundle_root)
    except (OSError, RuntimeError, ValueError) as error:
        message = f"stored comparison artifact escapes its bundle: {value!r}"
        raise ValueError(message) from error
    return value, path


def _validated_artifact_record(bundle_root: Path, raw_record: object, index: int) -> tuple[str, Path, int, str]:
    """Parse one manifest artifact record into trusted values."""
    if not isinstance(raw_record, dict) or set(raw_record) != {"bytes", "path", "sha256"}:
        message = f"stored comparison artifact record {index} has an invalid shape"
        raise ValueError(message)
    record = cast("dict[str, object]", raw_record)
    relative_value, path = _validated_artifact_path(bundle_root, record["path"], index)
    byte_count = record["bytes"]
    if not isinstance(byte_count, int) or isinstance(byte_count, bool) or byte_count < 0:
        message = f"stored comparison artifact {relative_value!r} has an invalid byte count"
        raise ValueError(message)
    digest = record["sha256"]
    if not isinstance(digest, str) or len(digest) != 64 or any(character not in "0123456789abcdef" for character in digest):
        message = f"stored comparison artifact {relative_value!r} has an invalid SHA-256 digest"
        raise ValueError(message)
    return relative_value, path, byte_count, digest


def _validated_artifact_records(bundle: Path, manifest: Mapping[str, Any]) -> dict[str, tuple[Path, int, str]]:
    """Parse one complete, contained canonical artifact inventory."""
    raw_records = manifest.get("artifacts")
    if not isinstance(raw_records, list):
        message = "stored comparison manifest artifacts must be an array"
        raise TypeError(message)

    bundle_root = bundle.resolve()
    records: dict[str, tuple[Path, int, str]] = {}
    for index, raw_record in enumerate(raw_records):
        relative_value, path, byte_count, digest = _validated_artifact_record(bundle_root, raw_record, index)
        if relative_value in records:
            message = f"stored comparison artifact path is duplicated: {relative_value!r}"
            raise ValueError(message)
        records[relative_value] = (path, byte_count, digest)

    actual_paths = frozenset(records)
    if actual_paths != EXPECTED_ARTIFACT_PATHS:
        missing = sorted(EXPECTED_ARTIFACT_PATHS - actual_paths)
        unexpected = sorted(actual_paths - EXPECTED_ARTIFACT_PATHS)
        message = f"stored comparison artifact inventory is incomplete or unexpected: missing={missing!r}, unexpected={unexpected!r}"
        raise ValueError(message)
    return records


def _verify_bundle_artifacts(bundle: Path, manifest: Mapping[str, Any]) -> None:
    """Reject altered or missing canonical inputs and raw outputs."""
    for relative_path, (path, byte_count, digest) in _validated_artifact_records(bundle, manifest).items():
        if path.is_symlink() or not path.is_file():
            message = f"stored comparison artifact is missing or not a regular file: {relative_path}"
            raise ValueError(message)
        if path.stat().st_size != byte_count or sha256(path) != digest:
            message = f"stored comparison artifact digest does not match: {relative_path}"
            raise ValueError(message)


def run_comparison(
    output_directory: Path,
    cpp: ProcessSpec,
    rust: ProcessSpec,
    input_sources: Mapping[str, Path],
    timeout_seconds: float,
) -> dict[str, object]:
    """Launch both implementations and publish one complete local bundle."""
    output = output_directory.resolve()
    if os.path.lexists(output):
        message = f"Output directory already exists: {output}; choose a new --output-directory."
        raise OutputDirectoryExistsError(message)
    if (cpp.label, rust.label) != ("cpp", "rust"):
        message = "comparison process labels must be 'cpp' and 'rust'"
        raise ValueError(message)
    _validate_input_sources(input_sources)
    with staged_run_directory(output) as staged_output:
        started_at = _utc_now()
        started_ns = time.perf_counter_ns()
        inputs = _copy_inputs(staged_output, input_sources)
        processes = {spec.label: _run_process(spec, staged_output, timeout_seconds) for spec in (cpp, rust)}
        analysis_started_ns = time.perf_counter_ns()
        summary = analyze_bundle(staged_output)
        analysis_duration_ns = time.perf_counter_ns() - analysis_started_ns
        write_json(staged_output / "summary.json", summary)
        manifest: dict[str, object] = {
            "artifacts": _artifact_records(staged_output),
            "completed_at_utc": _utc_now(),
            "host": {
                "architecture": platform.machine(),
                "logical_threads": os.cpu_count(),
                "operating_system": platform.system(),
                "python": platform.python_version(),
            },
            "inputs": inputs,
            "processes": processes,
            "run_id": output.name,
            "schema": "cdt-comparison-run-v1",
            "started_at_utc": started_at,
            "status": summary["status"],
            "timing": {
                "analysis_ns": analysis_duration_ns,
                "process_ns": {label: record["duration_ns"] for label, record in processes.items()},
                "total_ns": time.perf_counter_ns() - started_ns,
            },
        }
        write_json(staged_output / "manifest.json", manifest)
    return summary


def reanalyze_comparison(bundle: Path) -> dict[str, object]:
    """Verify and reproduce a summary without launching an implementation."""
    resolved = bundle.resolve()
    manifest = load_json(resolved / "manifest.json")
    if manifest.get("schema") != "cdt-comparison-run-v1":
        message = f"{resolved / 'manifest.json'}: unsupported comparison manifest schema"
        raise ValueError(message)
    _verify_bundle_artifacts(resolved, manifest)
    summary = analyze_bundle(resolved)
    write_json(resolved / "summary.json", summary)
    return summary


def _print_summary(summary: Mapping[str, Any], output: Path) -> None:
    """Render one small offline comparison table."""
    counts = summary["counts"]
    print(f"Comparison {summary['status']}: {output}")
    print("rule                     checks/classifications")
    print(f"exact                    {counts['exact']}")
    print(f"numerical                {counts['numerical']}")
    print(f"implementation-specific  {counts['implementation_specific']}")
    print(f"unsupported              {counts['unsupported']}")
    print(f"failed                   {counts['failed']}")


def _parser() -> argparse.ArgumentParser:
    """Build the installed comparison command parser."""
    parser = argparse.ArgumentParser(description=__doc__)
    subparsers = parser.add_subparsers(dest="subcommand", required=True)
    run = subparsers.add_parser("run", help="launch both implementations and retain a local comparison bundle")
    run.add_argument("--cpp", default=str(DEFAULT_CPP_EXECUTABLE), help="CDT++ fixture executable or command name")
    run.add_argument("--cpp-arg", action="append", default=[], help="one CDT++ argument; supports {protocol}, {manifest}, and {result_schema}")
    run.add_argument("--cpp-cwd", type=Path, default=ROOT, help="CDT++ command working directory")
    run.add_argument("--rust", required=True, help="causal-triangulations fixture executable or command name")
    run.add_argument("--rust-arg", action="append", default=[], help="one Rust argument; supports {protocol}, {manifest}, and {result_schema}")
    run.add_argument("--rust-cwd", type=Path, default=ROOT, help="Rust command working directory")
    run.add_argument("--output-directory", type=Path, required=True, help="new canonical comparison bundle directory")
    run.add_argument("--protocol", type=Path, default=DEFAULT_PROTOCOL)
    run.add_argument("--fixture-schema", type=Path, default=DEFAULT_FIXTURE_SCHEMA)
    run.add_argument("--result-schema", type=Path, default=DEFAULT_RESULT_SCHEMA)
    run.add_argument("--reference-result", type=Path, default=DEFAULT_REFERENCE_RESULT)
    run.add_argument("--reference-manifest", type=Path, default=DEFAULT_REFERENCE_MANIFEST)
    run.add_argument("--manifest-schema", type=Path, default=DEFAULT_MANIFEST_SCHEMA)
    run.add_argument("--timeout-seconds", type=_positive_timeout, default=60.0)
    analyze = subparsers.add_parser("analyze", help="rebuild summary.json from a stored comparison bundle")
    analyze.add_argument("bundle", type=Path)
    return parser


def main(argv: Sequence[str] | None = None) -> int:
    """Run the local comparison CLI."""
    arguments = _parser().parse_args(sys.argv[1:] if argv is None else argv)
    try:
        if arguments.subcommand == "analyze":
            summary = reanalyze_comparison(arguments.bundle)
            output = arguments.bundle.resolve()
        else:
            sources = {
                "protocol": arguments.protocol,
                "fixture_schema": arguments.fixture_schema,
                "result_schema": arguments.result_schema,
                "reference_result": arguments.reference_result,
                "reference_manifest": arguments.reference_manifest,
                "manifest_schema": arguments.manifest_schema,
            }
            summary = run_comparison(
                arguments.output_directory,
                ProcessSpec("cpp", arguments.cpp, tuple(arguments.cpp_arg), arguments.cpp_cwd.resolve()),
                ProcessSpec("rust", arguments.rust, tuple(arguments.rust_arg), arguments.rust_cwd.resolve()),
                sources,
                arguments.timeout_seconds,
            )
            output = arguments.output_directory.resolve()
    except (OSError, OutputDirectoryExistsError, TypeError, ValueError, subprocess.SubprocessError) as error:
        print(f"error: {error}", file=sys.stderr)
        return 2
    _print_summary(summary, output)
    return 0 if summary["status"] == "passed" else 1


if __name__ == "__main__":
    raise SystemExit(main())
