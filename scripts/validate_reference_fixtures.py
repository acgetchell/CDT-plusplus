"""Validate the versioned CDT++ cross-language reference package."""

from __future__ import annotations

import argparse
import hashlib
import itertools
import json
import math
import re
import subprocess
import sys
from pathlib import Path
from typing import Any

from jsonschema import Draft202012Validator, FormatChecker
from jsonschema.exceptions import SchemaError, ValidationError

ROOT = Path(__file__).resolve().parents[1]
REFERENCE = ROOT / "reference"


def reject_nonstandard_number(value: str) -> None:
    """Reject NaN and infinities, which are not JSON numbers."""
    message = f"nonstandard JSON number {value!r}"
    raise ValueError(message)


def load_json(path: Path) -> dict[str, Any]:
    """Load one JSON object or raise a path-specific validation error."""
    try:
        value = json.loads(path.read_text(encoding="utf-8"), parse_constant=reject_nonstandard_number)
    except (OSError, json.JSONDecodeError, ValueError) as error:
        message = f"{path}: invalid JSON: {error}"
        raise ValueError(message) from error
    if not isinstance(value, dict):
        message = f"{path}: top-level JSON value must be an object"
        raise TypeError(message)
    return value


def validate_document(document: dict[str, Any], schema: dict[str, Any], path: Path) -> None:
    """Apply one complete JSON Schema Draft 2020-12 contract."""
    try:
        Draft202012Validator.check_schema(schema)
        Draft202012Validator(schema, format_checker=FormatChecker()).validate(document)
    except SchemaError as error:
        location = ".".join(str(part) for part in error.absolute_schema_path)
        message = f"{path}: invalid JSON schema at {location or '<root>'}: {error.message}"
        raise ValueError(message) from error
    except ValidationError as error:
        location = ".".join(str(part) for part in error.absolute_path)
        message = f"{path}: schema violation at {location or '<root>'}: {error.message}"
        raise ValueError(message) from error


def is_close(actual: float, expected: float, tolerance: dict[str, Any]) -> bool:
    """Compare one numerical quantity with named absolute and relative parts."""
    absolute = float(tolerance["absolute"])
    relative = float(tolerance["relative"])
    return abs(actual - expected) <= absolute + relative * abs(expected)


def expected_cell_type(times: list[int]) -> str:
    """Classify one tetrahedron independently from production cell metadata."""
    minimum = min(times)
    maximum = max(times)
    if maximum - minimum != 1:
        return "acausal"
    return {3: "3-1", 2: "2-2", 1: "1-3"}.get(times.count(minimum), "unclassified")


def validate_entity_ids(entities: list[dict[str, Any]], prefix: str, state_id: str) -> None:
    """Require dense canonical entity identifiers in array order."""
    actual = [entity["id"] for entity in entities]
    expected = [f"{prefix}{index:02d}" for index in range(len(entities))]
    if actual != expected:
        message = f"{state_id}: noncanonical {prefix!r} identifiers"
        raise ValueError(message)


def validate_state(state: dict[str, Any]) -> None:  # noqa: C901, PLR0912, PLR0915
    """Validate ordering, incidence, adjacency, classification, and f-vector."""
    state_id = str(state["id"])
    vertices = state["vertices"]
    edges = state["edges"]
    facets = state["facets"]
    cells = state["cells"]
    validate_entity_ids(vertices, "v", state_id)
    validate_entity_ids(edges, "e", state_id)
    validate_entity_ids(facets, "f", state_id)
    validate_entity_ids(cells, "c", state_id)

    f_vector = [len(vertices), len(edges), len(facets), len(cells)]
    if state["f_vector"] != f_vector:
        message = f"{state_id}: f-vector does not match entity arrays"
        raise ValueError(message)
    if f_vector[0] - f_vector[1] + f_vector[2] - f_vector[3] != 1:
        message = f"{state_id}: finite complex does not have Euler characteristic 1"
        raise ValueError(message)

    vertex_order = [(vertex["time"], *vertex["position"]) for vertex in vertices]
    if vertex_order != sorted(vertex_order):
        message = f"{state_id}: vertices are not in canonical order"
        raise ValueError(message)
    vertex_by_id = {vertex["id"]: vertex for vertex in vertices}
    observed_times = [int(vertex["time"]) for vertex in vertices]
    if state["time_bounds"] != [min(observed_times), max(observed_times)]:
        message = f"{state_id}: time bounds do not match vertices"
        raise ValueError(message)

    edge_keys: list[list[str]] = []
    for edge in edges:
        endpoints = edge["vertices"]
        if endpoints != sorted(endpoints) or any(endpoint not in vertex_by_id for endpoint in endpoints):
            message = f"{state_id}: invalid edge incidence"
            raise ValueError(message)
        times = [vertex_by_id[endpoint]["time"] for endpoint in endpoints]
        expected_type = "spacelike" if times[0] == times[1] else "timelike"
        if edge["type"] != expected_type:
            message = f"{state_id}: edge {edge['id']} has incorrect causal type"
            raise ValueError(message)
        edge_keys.append(endpoints)
    if edge_keys != sorted(edge_keys) or len(edge_keys) != len({tuple(key) for key in edge_keys}):
        message = f"{state_id}: edges are not unique and canonical"
        raise ValueError(message)

    facet_keys: list[list[str]] = []
    for facet in facets:
        incident = facet["vertices"]
        if incident != sorted(incident) or any(vertex not in vertex_by_id for vertex in incident):
            message = f"{state_id}: invalid facet incidence"
            raise ValueError(message)
        times = [vertex_by_id[vertex]["time"] for vertex in incident]
        spacelike = len(set(times)) == 1
        if facet["spacelike"] is not spacelike:
            message = f"{state_id}: facet {facet['id']} has incorrect causal type"
            raise ValueError(message)
        if spacelike and facet.get("time") != times[0]:
            message = f"{state_id}: facet {facet['id']} has incorrect time"
            raise ValueError(message)
        facet_keys.append(incident)
    if facet_keys != sorted(facet_keys) or len(facet_keys) != len({tuple(key) for key in facet_keys}):
        message = f"{state_id}: facets are not unique and canonical"
        raise ValueError(message)

    cell_keys: list[list[str]] = []
    for cell in cells:
        incident = cell["vertices"]
        if incident != sorted(incident) or len(incident) != len(set(incident)) or any(vertex not in vertex_by_id for vertex in incident):
            message = f"{state_id}: invalid cell incidence"
            raise ValueError(message)
        cell_keys.append(incident)
    if cell_keys != sorted(cell_keys) or len(cell_keys) != len({tuple(key) for key in cell_keys}):
        message = f"{state_id}: cells are not unique and canonical"
        raise ValueError(message)

    expected_edges = sorted({pair for cell in cell_keys for pair in itertools.combinations(cell, 2)})
    if [tuple(key) for key in edge_keys] != expected_edges:
        message = f"{state_id}: edge set does not equal cell incidence"
        raise ValueError(message)
    expected_facets = sorted({triple for cell in cell_keys for triple in itertools.combinations(cell, 3)})
    if [tuple(key) for key in facet_keys] != expected_facets:
        message = f"{state_id}: facet set does not equal cell incidence"
        raise ValueError(message)

    for index, cell in enumerate(cells):
        incident = cell["vertices"]
        times = [int(vertex_by_id[vertex]["time"]) for vertex in incident]
        if cell["type"] != expected_cell_type(times):
            message = f"{state_id}: cell {cell['id']} has incorrect simplex type"
            raise ValueError(message)
        expected_adjacency = [cells[other]["id"] for other in range(len(cells)) if other != index and len(set(incident) & set(cells[other]["vertices"])) == 3]
        if cell["adjacent_cells"] != expected_adjacency:
            message = f"{state_id}: cell {cell['id']} has incorrect adjacency"
            raise ValueError(message)


def generalized_action(counts: dict[str, Any], parameters: dict[str, Any]) -> float:
    """Evaluate the published generalized action independently with libm."""
    n1 = float(counts["n1_timelike"])
    n31 = float(counts["n3_31_13"])
    n22 = float(counts["n3_22"])
    alpha = float(parameters["alpha"])
    coupling = float(parameters["k"])
    cosmological = float(parameters["lambda"])
    root_alpha = math.sqrt(alpha)
    denominator = 4.0 * alpha + 1.0
    three_one = (
        -3.0 * coupling * math.asinh(1.0 / (math.sqrt(3.0) * math.sqrt(denominator)))
        - 3.0 * coupling * root_alpha * math.acos((2.0 * alpha + 1.0) / denominator)
        - cosmological * math.sqrt(3.0 * alpha + 1.0) / 12.0
    )
    two_two = (
        2.0 * coupling * math.asinh(2.0 * math.sqrt(2.0) * math.sqrt(2.0 * alpha + 1.0) / denominator)
        - 4.0 * coupling * root_alpha * math.acos(-1.0 / denominator)
        - cosmological * math.sqrt(4.0 * alpha + 2.0) / 12.0
    )
    return 2.0 * math.pi * coupling * root_alpha * n1 + n31 * three_one + n22 * two_two


def validate_actions(raw: dict[str, Any], protocol: dict[str, Any]) -> None:
    """Compare raw C++ actions with independent closed-form calculations."""
    tolerance = protocol["tolerances"]["regge_action_closed_form"]
    for action in raw["actions"]:
        counts = action["counts"]
        parameters = action["parameters"]
        n1 = float(counts["n1_timelike"])
        n31 = float(counts["n3_31_13"])
        n22 = float(counts["n3_22"])
        coupling = float(parameters["k"])
        cosmological = float(parameters["lambda"])
        if action["id"] == "alpha-minus-one-imaginary-coefficient":
            expected = -2.0 * math.pi * coupling * n1 + n31 * (2.673 * coupling + 0.118 * cosmological) + n22 * (7.386 * coupling + 0.118 * cosmological)
        elif action["id"] == "alpha-one-rounded":
            expected = 2.0 * math.pi * coupling * n1 + n31 * (-3.548 * coupling - 0.167 * cosmological) + n22 * (-5.355 * coupling - 0.204 * cosmological)
        elif action["id"] == "alpha-generalized":
            expected = generalized_action(counts, parameters)
        else:
            message = f"unknown action fixture {action['id']!r}"
            raise ValueError(message)
        if not is_close(float(action["value"]), expected, tolerance):
            message = f"{action['id']}: raw action does not match independent oracle"
            raise ValueError(message)


def validate_causality_filter(raw: dict[str, Any], protocol: dict[str, Any]) -> None:
    """Check the explicit acausal input and its failure-atomic repair result."""
    states = {state["id"]: state for state in raw["states"]}
    fixture = next(item for item in protocol["state_fixtures"] if item["id"] == "causality-filter")
    before = states[fixture["input_state"]]
    after = states[fixture["expected_state"]]
    removed = fixture["removed_vertex"]

    def vertex_value(vertex: dict[str, Any]) -> tuple[tuple[float, ...], int]:
        return tuple(float(value) for value in vertex["position"]), int(vertex["time"])

    removed_value = (
        tuple(float(value) for value in removed["position"]),
        int(removed["time"]),
    )
    before_values = [vertex_value(vertex) for vertex in before["vertices"]]
    after_values = [vertex_value(vertex) for vertex in after["vertices"]]
    if before_values.count(removed_value) != 1:
        message = "causality-filter input does not contain the unique declared bad vertex"
        raise ValueError(message)
    if after_values != [value for value in before_values if value != removed_value]:
        message = "causality-filter output does not remove exactly the declared bad vertex"
        raise ValueError(message)
    if not any(cell["type"] == "acausal" for cell in before["cells"]):
        message = "causality-filter input does not expose an acausal cell"
        raise ValueError(message)
    if any(cell["type"] == "acausal" for cell in after["cells"]):
        message = "causality-filter output still contains an acausal cell"
        raise ValueError(message)
    actual_delta = [right - left for left, right in zip(before["f_vector"], after["f_vector"], strict=True)]
    if actual_delta != [-1, -3, -3, -1]:
        message = "causality-filter output has an unexpected exact f-vector delta"
        raise ValueError(message)


REVERSE_MOVE = {
    "2-3": "3-2",
    "3-2": "2-3",
    "2-6": "6-2",
    "6-2": "2-6",
    "4-4": "4-4",
}


def entity_by_id(state: dict[str, Any], collection: str, entity_id: str) -> dict[str, Any]:
    """Resolve one canonical entity or reject a stale protocol identifier."""
    entity = next((item for item in state[collection] if item["id"] == entity_id), None)
    if entity is None:
        message = f"{state['id']}: unknown {collection[:-1]} id {entity_id!r}"
        raise ValueError(message)
    return entity


def proposal_site_count(state: dict[str, Any], move: str) -> int:
    """Derive the raw proposal-domain size independently from canonical state."""
    if move == "2-3":
        return sum(cell["type"] == "2-2" for cell in state["cells"])
    if move == "3-2":
        return sum(edge["type"] == "timelike" for edge in state["edges"])
    if move == "2-6":
        return sum(cell["type"] == "1-3" for cell in state["cells"])
    if move == "6-2":
        return len(state["vertices"])
    if move == "4-4":
        return sum(edge["type"] == "spacelike" for edge in state["edges"])
    message = f"unknown move type {move!r}"
    raise ValueError(message)


def proposal_probability(state: dict[str, Any], move: str) -> float:
    """Return the declared uniform-move, uniform-raw-site probability."""
    site_count = proposal_site_count(state, move)
    if site_count <= 0:
        message = f"{state['id']}: {move} has an empty raw proposal domain"
        raise ValueError(message)
    return 1.0 / (5.0 * site_count)


def validate_cell_site(transition: dict[str, Any], before: dict[str, Any]) -> None:
    """Validate a cell-based proposal site."""
    move = transition["move"]
    site = transition["site"]
    cell = entity_by_id(before, "cells", str(site["entity"]))
    expected_type = "2-2" if move == "2-3" else "1-3"
    if cell["type"] != expected_type or site["vertices"] != cell["vertices"]:
        message = f"{transition['id']}: declared cell site does not match raw topology"
        raise ValueError(message)
    if move == "2-6":
        facet = entity_by_id(before, "facets", str(site["shared_spacelike_facet"]))
        if not facet["spacelike"] or not set(facet["vertices"]).issubset(cell["vertices"]):
            message = f"{transition['id']}: shared facet is not spacelike incidence of the proposed cell"
            raise ValueError(message)


def validate_edge_site(transition: dict[str, Any], before: dict[str, Any], proposed: dict[str, Any]) -> None:
    """Validate an edge-based proposal site."""
    move = transition["move"]
    site = transition["site"]
    edge = entity_by_id(before, "edges", str(site["entity"]))
    expected_type = "timelike" if move == "3-2" else "spacelike"
    if edge["type"] != expected_type or site["vertices"] != edge["vertices"]:
        message = f"{transition['id']}: declared edge site does not match raw topology"
        raise ValueError(message)
    if move == "4-4":
        reverse_vertices = site["reverse_vertices"]
        if not any(candidate["vertices"] == reverse_vertices and candidate["type"] == "spacelike" for candidate in proposed["edges"]):
            message = f"{transition['id']}: proposed state does not contain the declared reverse edge"
            raise ValueError(message)


def validate_vertex_site(transition: dict[str, Any], before: dict[str, Any]) -> None:
    """Validate a degree-five vertex proposal site."""
    site = transition["site"]
    vertex_id = str(site["entity"])
    vertex = entity_by_id(before, "vertices", vertex_id)
    if site["vertices"] != [vertex["id"]]:
        message = f"{transition['id']}: declared vertex site does not match raw topology"
        raise ValueError(message)
    degree = sum(vertex_id in edge["vertices"] for edge in before["edges"])
    if degree != 5:
        message = f"{transition['id']}: removal site has degree {degree}, expected 5"
        raise ValueError(message)


def validate_transition_site(transition: dict[str, Any], before: dict[str, Any], proposed: dict[str, Any]) -> None:
    """Bind a declared move site to the exact raw topology it names."""
    move = str(transition["move"])
    expected_kind = {
        "2-3": "2-2-cell",
        "3-2": "timelike-edge",
        "2-6": "1-3-cell",
        "6-2": "degree-five-vertex",
        "4-4": "spacelike-edge",
    }[move]
    if transition["site"]["kind"] != expected_kind:
        message = f"{transition['id']}: site kind does not match move {move}"
        raise ValueError(message)
    if move in {"2-3", "2-6"}:
        validate_cell_site(transition, before)
    elif move in {"3-2", "4-4"}:
        validate_edge_site(transition, before, proposed)
    else:
        validate_vertex_site(transition, before)


def state_action(state: dict[str, Any], parameters: dict[str, Any]) -> float:
    """Derive the action from raw causal counts rather than protocol deltas."""
    counts = {
        "n1_timelike": sum(edge["type"] == "timelike" for edge in state["edges"]),
        "n3_31_13": sum(cell["type"] in {"3-1", "1-3"} for cell in state["cells"]),
        "n3_22": sum(cell["type"] == "2-2" for cell in state["cells"]),
    }
    return generalized_action(counts, parameters)


def validate_protocol(protocol: dict[str, Any], raw: dict[str, Any]) -> None:
    """Validate exact move deltas and independent Metropolis decisions."""
    states = {state["id"]: state for state in raw["states"]}
    expected_delta = {
        "2-3": [0, 1, 2, 1],
        "3-2": [0, -1, -2, -1],
        "2-6": [1, 5, 8, 4],
        "6-2": [-1, -5, -8, -4],
        "4-4": [0, 0, 0, 0],
    }
    probability_tolerance = protocol["tolerances"]["acceptance_probability"]
    observed_moves: set[str] = set()
    for transition in protocol["transitions"]:
        move = transition["move"]
        observed_moves.add(move)
        before = states[transition["before"]]
        proposed = states[transition["proposed"]]
        validate_transition_site(transition, before, proposed)
        actual_delta = [right - left for left, right in zip(before["f_vector"], proposed["f_vector"], strict=True)]
        if actual_delta != expected_delta[move]:
            message = f"{transition['id']}: incorrect exact f-vector delta"
            raise ValueError(message)
        expected_forward = proposal_probability(before, move)
        expected_reverse = proposal_probability(proposed, REVERSE_MOVE[move])
        if not is_close(float(transition["proposal_probability"]), expected_forward, probability_tolerance):
            message = f"{transition['id']}: forward proposal probability does not match the raw proposal domain"
            raise ValueError(message)
        if not is_close(float(transition["reverse_probability"]), expected_reverse, probability_tolerance):
            message = f"{transition['id']}: reverse proposal probability does not match the raw proposal domain"
            raise ValueError(message)
        expected_action_delta = state_action(before, transition["parameters"]) - state_action(proposed, transition["parameters"])
        action_tolerance = protocol["tolerances"]["regge_action_closed_form"]
        if not is_close(float(transition["action_delta"]), expected_action_delta, action_tolerance):
            message = f"{transition['id']}: action delta does not match independently derived raw-state actions"
            raise ValueError(message)
        expected_probability = min(
            1.0,
            expected_reverse / expected_forward * math.exp(expected_action_delta),
        )
        if not is_close(float(transition["acceptance_probability"]), expected_probability, probability_tolerance):
            message = f"{transition['id']}: incorrect Metropolis-Hastings probability"
            raise ValueError(message)
        accepted = float(transition["acceptance_variate"]) <= expected_probability
        if transition["accepted"] is not accepted:
            message = f"{transition['id']}: accept/reject decision is inconsistent"
            raise ValueError(message)
        committed = transition["proposed"] if accepted else transition["before"]
        if transition["committed"] != committed:
            message = f"{transition['id']}: committed state is inconsistent"
            raise ValueError(message)
    if observed_moves != set(expected_delta):
        message = "transition protocol does not cover every supported 3D move"
        raise ValueError(message)


def file_digest(path: Path) -> str:
    """Return a lowercase SHA-256 digest for one artifact."""
    return hashlib.sha256(path.read_bytes()).hexdigest()


def validate_manifest(manifest: dict[str, Any]) -> None:
    """Check that every manifested local artifact exists and matches SHA-256."""
    artifact_paths = {artifact["path"] for artifact in manifest["artifacts"]}
    command_ids = [command["id"] for command in manifest["commands"]]
    if len(command_ids) != len(set(command_ids)):
        message = "manifest producer command ids must be unique"
        raise ValueError(message)
    command_artifacts = [artifact for command in manifest["commands"] for artifact in command["artifacts"]]
    if len(command_artifacts) != len(set(command_artifacts)):
        message = "manifest maps one generated artifact to multiple producer commands"
        raise ValueError(message)
    if not set(command_artifacts).issubset(artifact_paths):
        message = "manifest producer command names an undeclared artifact"
        raise ValueError(message)
    generated_artifacts = {path for path in artifact_paths if path.startswith("reference/raw/")}
    if set(command_artifacts) != generated_artifacts:
        message = "manifest does not record exactly one producer for every raw artifact"
        raise ValueError(message)

    for artifact in manifest["artifacts"]:
        path = (ROOT / artifact["path"]).resolve()
        if not path.is_relative_to(REFERENCE.resolve()):
            message = f"manifest artifact escapes the reference directory: {path}"
            raise ValueError(message)
        if not path.is_file():
            message = f"manifest artifact does not exist: {path}"
            raise ValueError(message)
        if file_digest(path) != artifact["sha256"]:
            message = f"manifest artifact checksum mismatch: {path}"
            raise ValueError(message)


def validate_command_provenance(
    manifests: list[dict[str, Any]],
    protocol: dict[str, Any],
) -> None:
    """Require exact producer argv for every raw artifact family."""
    command_entries = [command for manifest in manifests for command in manifest["commands"]]
    commands = {command["id"]: command["command_line"] for command in command_entries}
    if len(commands) != len(command_entries):
        message = "producer command ids must be unique across manifests"
        raise ValueError(message)
    expected = {
        "canonical-fixture": [
            "out/build/reference/tests/CDT_reference_fixture",
        ],
        "bounded-end-to-end": protocol["bounded_run"]["command"],
        "persistence-roundtrip": [
            "out/build/reference/src/initialize",
            "-s",
            "-n2",
            "-t2",
            "-o",
            "--seed",
            "92",
            "--threads",
            "1",
        ],
        **{
            f"scaling-threads-{threads}": [
                "out/build/parallel/tests/CDT_cgal_benchmark",
                "640",
                "5",
                "50",
                str(threads),
                "1",
            ]
            for threads in (1, 2, 4)
        },
    }
    if commands != expected:
        message = "manifest producer commands do not match the reference protocol"
        raise ValueError(message)


def fnv1a64(payload: bytes) -> str:
    """Return the lowercase 64-bit FNV-1a spelling used by persistence."""
    value = 14695981039346656037
    for byte in payload:
        value ^= byte
        value = value * 1099511628211 & 0xFFFFFFFFFFFFFFFF
    return f"{value:016x}"


def validate_persistence() -> None:
    """Verify the committed payload against its raw C++ sidecar."""
    payload_path = REFERENCE / "raw" / "v1" / "persistence-v1.off"
    metadata_path = REFERENCE / "raw" / "v1" / "persistence-v1.off.meta"
    payload = payload_path.read_bytes()
    metadata = dict(line.split("=", maxsplit=1) for line in metadata_path.read_text(encoding="utf-8").splitlines() if "=" in line)
    if int(metadata["payload.size"]) != len(payload):
        message = "persistence payload size does not match its sidecar"
        raise ValueError(message)
    if metadata["payload.fnv1a64"] != fnv1a64(payload):
        message = "persistence payload FNV-1a digest does not match its sidecar"
        raise ValueError(message)
    if metadata["placement.fnv1a64"] == metadata["topology.fnv1a64"]:
        message = "placement and topology fingerprints must be separately derived"
        raise ValueError(message)


def command_option(command: list[str], short_name: str, long_name: str | None = None) -> str:
    """Read one required option from either joined or separated CLI spelling."""
    names = (short_name,) if long_name is None else (short_name, long_name)
    for index, token in enumerate(command):
        for name in names:
            if token == name:
                if index + 1 >= len(command):
                    message = f"command option {name!r} has no value"
                    raise ValueError(message)
                return command[index + 1]
            if token.startswith(name) and token != name:
                return token[len(name) :]
    message = f"command does not declare required option {short_name!r}"
    raise ValueError(message)


def output_number(output: str, label: str) -> float:
    """Parse one scalar from the bounded-run output."""
    match = re.search(rf"^{re.escape(label)}:\s+([^\s]+)$", output, flags=re.MULTILINE)
    if match is None:
        message = f"bounded run does not report {label!r}"
        raise ValueError(message)
    return float(match.group(1))


def validate_bounded_run_command(bounded_run: dict[str, Any], output: str, path: Path) -> None:
    """Match every scientific CLI value to its retained textual record."""
    command = bounded_run["command"]
    expected_text = (
        f"Number of desired simplices: {command_option(command, '-n', '--simplices')}",
        f"Number of desired timeslices: {command_option(command, '-t', '--timeslices')}",
        f"Number of passes: {command_option(command, '-p', '--passes')}",
        f"Checkpoint every {command_option(command, '-c', '--checkpoint')} passes.",
        f"Effective random seed: {command_option(command, '--seed')}",
        f"Maximum Delaunay threads: {command_option(command, '--threads')}",
    )
    for text in expected_text:
        if text not in output:
            message = f"{path}: output contradicts declared command field {text!r}"
            raise ValueError(message)

    expected_parameters = {
        "Alpha": float(command_option(command, "-a", "--alpha")),
        "K": float(command_option(command, "-k")),
        "Lambda": float(command_option(command, "-l", "--lambda")),
    }
    for label, expected in expected_parameters.items():
        if output_number(output, label) != expected:
            message = f"{path}: output {label!r} contradicts the declared command"
            raise ValueError(message)


def bounded_run_final_f_vector(output: str) -> list[int]:
    """Return the final, rather than initial, reported manifold counts."""
    matches = re.findall(r"Manifold has (\d+) vertices and (\d+) edges and (\d+) faces and (\d+) simplices\.", output)
    if not matches:
        message = "bounded run does not report a final manifold f-vector"
        raise ValueError(message)
    return [int(value) for value in matches[-1]]


def validate_end_to_end(protocol: dict[str, Any], path: Path | None = None) -> None:
    """Check bounded-run command provenance, bands, and accounting identities."""
    if path is None:
        path = REFERENCE / "raw" / "v1" / "end-to-end.txt"
    output = path.read_text(encoding="utf-8")
    bounded_run = protocol["bounded_run"]
    command = bounded_run["command"]
    validate_bounded_run_command(bounded_run, output, path)
    required = (
        f"Effective random seed: {command_option(command, '--seed')} (stream 1).",
        "There were 7 proposed moves with 0 accepted moves and 7 rejected moves.",
        "There were 7 candidate construction attempts with 0 successful candidates and 7 failed candidates.",
    )
    for text in required:
        if text not in output:
            message = f"{path}: missing bounded-run evidence {text!r}"
            raise ValueError(message)
    f_vector = bounded_run_final_f_vector(output)
    n0, n1, n2, n3 = f_vector
    if n0 - n1 + n2 - n3 != 1:
        message = "bounded run final manifold violates the Euler relation"
        raise ValueError(message)
    band = bounded_run["bands"]["randomized_cgal_f_vector"]
    if any(value < minimum or value > maximum for value, minimum, maximum in zip(f_vector, band["minimum"], band["maximum"], strict=True)):
        message = f"bounded run final f-vector {f_vector} lies outside its declared band"
        raise ValueError(message)


def parse_key_value_record(path: Path) -> dict[str, str]:
    """Read one raw benchmark record without changing or normalizing it."""
    return dict(line.split("=", maxsplit=1) for line in path.read_text(encoding="utf-8").splitlines() if "=" in line)


def validate_scaling_records() -> None:
    """Verify #88 records share one matched protocol and retain raw samples."""
    records = [parse_key_value_record(REFERENCE / "raw" / "v1" / f"scaling-threads-{threads}.txt") for threads in (1, 2, 4)]
    matched_keys = (
        "record.schema",
        "implementation.revision",
        "build.compiler_id",
        "build.compiler_version",
        "build.configuration",
        "build.system",
        "build.processor",
        "dependency.cgal_version",
        "dependency.tbb_version",
        "fixture.id",
        "random.seed",
        "requested_simplices",
        "timeslices",
        "generated_points",
        "warmups",
        "repetitions",
        "moves_per_repetition",
        "sample.unit",
    )
    for key in matched_keys:
        if len({record[key] for record in records}) != 1:
            message = f"scaling records do not match on {key!r}"
            raise ValueError(message)
    for threads, record in zip((1, 2, 4), records, strict=True):
        if int(record["requested_threads"]) != threads or int(record["active_threads"]) != threads:
            message = f"scaling record does not enforce the requested {threads}-thread limit"
            raise ValueError(message)
        repetitions = int(record["repetitions"])
        for key, value in record.items():
            if key.endswith("_ns_samples") and len(value.split(",")) != repetitions:
                message = f"{key}: raw sample count does not match repetitions"
                raise ValueError(message)


def reference_revisions(
    raw: dict[str, Any],
    manifests: list[dict[str, Any]],
) -> set[str]:
    """Collect every source revision recorded by the reference package."""
    return {
        str(raw["implementation"]["revision"]),
        *(str(manifest["implementation"]["source_revision"]) for manifest in manifests),
        *(parse_key_value_record(REFERENCE / "raw" / "v1" / f"scaling-threads-{threads}.txt")["implementation.revision"] for threads in (1, 2, 4)),
    }


def validate_provenance_consistency(
    raw: dict[str, Any],
    manifests: list[dict[str, Any]],
) -> str:
    """Require every artifact family to name the same source revision."""
    revisions = reference_revisions(raw, manifests)
    if len(revisions) != 1:
        message = "reference artifacts do not share one source revision"
        raise ValueError(message)
    return revisions.pop()


def validate_clean_provenance(
    raw: dict[str, Any],
    manifests: list[dict[str, Any]],
) -> None:
    """Require archival artifacts to identify one clean source commit."""
    revision = validate_provenance_consistency(raw, manifests)
    if revision.endswith("-dirty") or re.fullmatch(r"[0-9a-f]{40}", revision) is None:
        message = "archival reference artifacts must name one clean Git commit"
        raise ValueError(message)


def parse_json_object(payload: str, source: str) -> dict[str, Any]:
    """Parse a generated JSON object without publishing an intermediate file."""
    try:
        value = json.loads(payload, parse_constant=reject_nonstandard_number)
    except (json.JSONDecodeError, ValueError) as error:
        message = f"{source}: invalid JSON: {error}"
        raise ValueError(message) from error
    if not isinstance(value, dict):
        message = f"{source}: top-level JSON value must be an object"
        raise TypeError(message)
    return value


def compare_generated_vertex(
    generated: dict[str, Any],
    committed: dict[str, Any],
    tolerance: dict[str, Any],
) -> None:
    """Compare one vertex's exact metadata and tolerant coordinates."""
    generated_exact = {key: value for key, value in generated.items() if key != "position"}
    committed_exact = {key: value for key, value in committed.items() if key != "position"}
    if generated_exact != committed_exact:
        message = f"generated reference vertex {committed['id']!r} differs in exact metadata"
        raise ValueError(message)
    for coordinate, expected in zip(generated["position"], committed["position"], strict=True):
        if not is_close(float(coordinate), float(expected), tolerance):
            message = f"generated reference vertex {committed['id']!r} differs outside the coordinate tolerance"
            raise ValueError(message)


def compare_generated_state(
    generated: dict[str, Any],
    committed: dict[str, Any],
    tolerance: dict[str, Any],
) -> None:
    """Compare one state's exact topology and tolerant coordinates."""
    generated_exact = {key: value for key, value in generated.items() if key != "vertices"}
    committed_exact = {key: value for key, value in committed.items() if key != "vertices"}
    if generated_exact != committed_exact:
        message = f"generated reference state {committed['id']!r} differs in exact topology or metadata"
        raise ValueError(message)
    generated_vertices = generated["vertices"]
    committed_vertices = committed["vertices"]
    if len(generated_vertices) != len(committed_vertices):
        message = f"generated reference state {committed['id']!r} differs in vertex count"
        raise ValueError(message)
    for generated_vertex, committed_vertex in zip(generated_vertices, committed_vertices, strict=True):
        compare_generated_vertex(generated_vertex, committed_vertex, tolerance)


def compare_generated_action(
    generated: dict[str, Any],
    committed: dict[str, Any],
    tolerance: dict[str, Any],
) -> None:
    """Compare one action's exact inputs and tolerant numerical result."""
    generated_exact = {key: value for key, value in generated.items() if key != "value"}
    committed_exact = {key: value for key, value in committed.items() if key != "value"}
    if generated_exact != committed_exact:
        message = f"generated reference action {committed['id']!r} differs in exact inputs"
        raise ValueError(message)
    if not is_close(float(generated["value"]), float(committed["value"]), tolerance):
        message = f"generated reference action {committed['id']!r} differs outside the action tolerance"
        raise ValueError(message)


def compare_generated_scientific_payload(
    generated: dict[str, Any],
    committed: dict[str, Any],
    protocol: dict[str, Any],
) -> None:
    """Compare exact topology and protocol-designated numerical quantities."""
    generated_states = generated["states"]
    committed_states = committed["states"]
    if len(generated_states) != len(committed_states):
        message = "generated reference states differ in count"
        raise ValueError(message)
    coordinate_tolerance = protocol["tolerances"]["coordinates"]
    for generated_state, committed_state in zip(generated_states, committed_states, strict=True):
        compare_generated_state(generated_state, committed_state, coordinate_tolerance)

    generated_actions = generated["actions"]
    committed_actions = committed["actions"]
    if len(generated_actions) != len(committed_actions):
        message = "generated reference actions differ in count"
        raise ValueError(message)
    action_tolerance = protocol["tolerances"]["regge_action_closed_form"]
    for generated_action, committed_action in zip(generated_actions, committed_actions, strict=True):
        compare_generated_action(generated_action, committed_action, action_tolerance)


def validate_generated_fixture(
    fixture_binary: Path,
    committed: dict[str, Any],
    result_schema: dict[str, Any],
    protocol: dict[str, Any],
) -> None:
    """Compare generated scientific fields while ignoring host provenance."""
    binary = fixture_binary if fixture_binary.is_absolute() else ROOT / fixture_binary
    if not binary.is_file():
        message = f"reference fixture binary does not exist: {binary}"
        raise ValueError(message)
    completed = subprocess.run(  # noqa: S603
        [str(binary)],
        cwd=ROOT,
        check=False,
        capture_output=True,
        text=True,
        timeout=60,
    )
    if completed.returncode != 0:
        message = f"reference fixture generator failed with exit {completed.returncode}: {completed.stderr.strip()}"
        raise RuntimeError(message)
    generated = parse_json_object(completed.stdout, str(binary))
    validate_document(generated, result_schema, binary)
    compare_generated_scientific_payload(generated, committed, protocol)


def main(  # noqa: C901
    fixture_binary: Path | None = None,
    *,
    generated_only: bool = False,
    provenance_only: bool = False,
    require_clean_provenance: bool = False,
) -> int:
    """Validate schemas, protocol data, raw results, and provenance."""
    fixture_path = REFERENCE / "fixtures" / "v1" / "protocol.json"
    result_path = REFERENCE / "raw" / "v1" / "cpp-reference.json"
    result_schema = load_json(REFERENCE / "schema" / "result-v1.schema.json")
    protocol = load_json(fixture_path)
    raw = load_json(result_path)
    if generated_only:
        if fixture_binary is None:
            message = "--generated-only requires --fixture-binary"
            raise ValueError(message)
        validate_generated_fixture(fixture_binary, raw, result_schema, protocol)
        print("Generated reference fixture matches the committed scientific payload.")
        return 0

    manifest_paths = sorted((REFERENCE / "manifests" / "v1").glob("*.json"))
    manifests = [load_json(path) for path in manifest_paths]
    if provenance_only:
        validate_clean_provenance(raw, manifests)
        print("Reference artifacts identify one clean source commit.")
        return 0

    fixture_schema = load_json(REFERENCE / "schema" / "fixture-v1.schema.json")
    manifest_schema = load_json(REFERENCE / "schema" / "run-manifest-v1.schema.json")

    validate_document(protocol, fixture_schema, fixture_path)
    validate_document(raw, result_schema, result_path)
    for path, manifest in zip(manifest_paths, manifests, strict=True):
        validate_document(manifest, manifest_schema, path)
    for schema in (fixture_schema, result_schema, manifest_schema):
        if schema.get("$schema") != "https://json-schema.org/draft/2020-12/schema":
            message = "reference schemas must declare JSON Schema Draft 2020-12"
            raise ValueError(message)

    state_ids = [state["id"] for state in raw["states"]]
    if len(state_ids) != len(set(state_ids)):
        message = "raw result contains duplicate state ids"
        raise ValueError(message)
    for state in raw["states"]:
        validate_state(state)
    validate_causality_filter(raw, protocol)
    validate_actions(raw, protocol)
    validate_protocol(protocol, raw)
    for manifest in manifests:
        validate_manifest(manifest)
    validate_command_provenance(manifests, protocol)
    validate_provenance_consistency(raw, manifests)
    validate_persistence()
    validate_end_to_end(protocol)
    validate_scaling_records()
    if require_clean_provenance:
        validate_clean_provenance(raw, manifests)
    if fixture_binary is not None:
        validate_generated_fixture(fixture_binary, raw, result_schema, protocol)
    print("Reference fixture package is valid.")
    return 0


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--fixture-binary",
        type=Path,
        help="run this C++ generator and compare its canonical states and actions",
    )
    parser.add_argument(
        "--generated-only",
        action="store_true",
        help="skip offline package checks and compare only generated scientific data",
    )
    parser.add_argument(
        "--provenance-only",
        action="store_true",
        help="skip offline package checks and require one clean source revision",
    )
    parser.add_argument(
        "--require-clean-provenance",
        action="store_true",
        help="reject dirty or inconsistent source revisions for archival use",
    )
    arguments = parser.parse_args()
    try:
        raise SystemExit(
            main(
                arguments.fixture_binary,
                generated_only=arguments.generated_only,
                provenance_only=arguments.provenance_only,
                require_clean_provenance=arguments.require_clean_provenance,
            )
        )
    except (OSError, RuntimeError, TypeError, ValueError, subprocess.SubprocessError) as error:
        print(f"error: {error}", file=sys.stderr)
        raise SystemExit(1) from None
