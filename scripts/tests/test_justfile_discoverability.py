"""Regression tests for the public Just recipe surface."""

import json
import re
import shutil
import subprocess
import unittest
from collections import defaultdict
from pathlib import Path
from typing import Any, cast

import yaml

REPO_ROOT = Path(__file__).resolve().parents[2]
WORKFLOW_VERSION_LOOKUP = re.compile(r"\bjust --evaluate ([a-z][a-z0-9_-]*)")
UV_INVOCATION = re.compile(r"(?<![\w-])uvx?(?=\s)")


def _run_just(*args: str) -> subprocess.CompletedProcess[str]:
    """Run the repository's installed Just executable without a shell."""
    executable = shutil.which("just")
    if executable is None:
        message = "Just is required to validate the repository command surface."
        raise RuntimeError(message)
    return subprocess.run(  # noqa: S603 - executable is resolved; arguments are fixed by these tests.
        [executable, *args],
        cwd=REPO_ROOT,
        check=True,
        capture_output=True,
        encoding="utf-8",
    )


def _just_document() -> dict[str, Any]:
    """Return the parsed Justfile metadata from the pinned executable."""
    result = _run_just("--dump", "--dump-format", "json")
    return cast("dict[str, Any]", json.loads(result.stdout))


def _just_recipes() -> dict[str, dict[str, Any]]:
    """Return recipe metadata keyed by recipe name."""
    return cast("dict[str, dict[str, Any]]", _just_document()["recipes"])


def _workflow_document(filename: str) -> dict[str, Any]:
    """Return one parsed GitHub Actions workflow."""
    path = REPO_ROOT / ".github" / "workflows" / filename
    return cast("dict[str, Any]", yaml.safe_load(path.read_text(encoding="utf-8")))


def _dependency_names(recipe: dict[str, Any]) -> set[str]:
    """Return the direct recipe dependencies recorded by Just."""
    return {dependency["recipe"] for dependency in recipe["dependencies"]}


def _body_fragments(value: object) -> list[str]:
    """Return every literal string fragment from a parsed recipe body value."""
    if isinstance(value, str):
        return [value]
    if isinstance(value, list):
        return [fragment for item in value for fragment in _body_fragments(item)]
    return []


def _recipe_invokes_uv(recipe: dict[str, Any]) -> bool:
    """Return whether a parsed recipe body directly invokes uv or uvx."""
    return any(UV_INVOCATION.search("".join(_body_fragments(line))) for line in recipe["body"])


def _recipe_reaches(recipes: dict[str, dict[str, Any]], recipe_name: str, target: str) -> bool:
    """Return whether a recipe reaches a target through its dependency graph."""
    pending = list(_dependency_names(recipes[recipe_name]))
    visited: set[str] = set()
    while pending:
        dependency = pending.pop()
        if dependency == target:
            return True
        if dependency in visited:
            continue
        visited.add(dependency)
        pending.extend(_dependency_names(recipes[dependency]))
    return False


class JustfileDiscoverabilityTests(unittest.TestCase):
    """Keep the maintainer-facing Just command layer coherent and discoverable."""

    def test_bare_just_shows_the_grouped_recipe_reference(self) -> None:
        """Invoking Just without a recipe must not start validation or a build."""
        result = _run_just()

        self.assertTrue(result.stdout.startswith("Available recipes:\n"))
        self.assertIn("[workflows]", result.stdout)
        self.assertIn("check", result.stdout)
        self.assertNotIn("Checks complete.", result.stdout)

    def test_default_gates_include_core_checks(self) -> None:
        """Default validation includes the lightweight Python and reference surfaces."""
        recipes = _just_recipes()
        check_dependencies = _dependency_names(recipes["check"])
        ci_dependencies = _dependency_names(recipes["ci"])

        self.assertTrue(
            {
                "_justfile-check",
                "python-check",
                "reference-check",
                "release-check",
                "semgrep",
                "semgrep-test",
                "spell-check",
                "viewer-check",
            }
            <= check_dependencies
        )
        self.assertTrue({"check", "python-package-check", "reference-generated-check"} <= ci_dependencies)
        self.assertNotIn("python-experiment-check", recipes)
        self.assertNotIn("_sync-python-experiments", recipes)

    def test_public_recipes_have_one_group_and_a_description(self) -> None:
        """Every listed recipe should explain its purpose in one stable section."""
        for name, recipe in _just_recipes().items():
            if recipe["private"]:
                continue
            groups = [attribute["group"] for attribute in recipe["attributes"] if isinstance(attribute, dict) and "group" in attribute]
            with self.subTest(recipe=name):
                self.assertTrue(recipe["doc"], f"public recipe {name!r} has no description")
                self.assertEqual(len(groups), 1, f"public recipe {name!r} has groups {groups!r}")

    def test_public_recipes_do_not_duplicate_exact_behavior(self) -> None:
        """Public names should not expose byte-for-byte duplicate implementations."""
        signatures: defaultdict[str, list[str]] = defaultdict(list)
        for name, recipe in _just_recipes().items():
            if recipe["private"]:
                continue
            signature = json.dumps(
                {
                    "body": recipe["body"],
                    "dependencies": recipe["dependencies"],
                    "parameters": recipe["parameters"],
                },
                sort_keys=True,
            )
            signatures[signature].append(name)

        duplicates = [names for names in signatures.values() if len(names) > 1]
        self.assertEqual(duplicates, [])

    def test_uv_backed_recipes_reuse_pinned_guards(self) -> None:
        """Every uv consumer should reach the repository's exact-version guard."""
        recipes = _just_recipes()
        for name in ("_sync-python-dev", "_zizmor"):
            with self.subTest(recipe=name):
                self.assertIn("_ensure-uv", _dependency_names(recipes[name]))

        uv_consumers = {name for name, recipe in recipes.items() if name != "_ensure-uv" and _recipe_invokes_uv(recipe)}
        self.assertTrue(uv_consumers)
        for name in sorted(uv_consumers):
            with self.subTest(recipe=name):
                self.assertTrue(
                    _recipe_reaches(recipes, name, "_ensure-uv"),
                    f"uv-backed recipe {name!r} does not reach _ensure-uv",
                )

    def test_workflow_tool_lookups_resolve_from_just(self) -> None:
        """GitHub Actions should resolve shared tool pins through Just assignments."""
        workflow_directory = REPO_ROOT / ".github" / "workflows"
        workflow_paths = sorted((*workflow_directory.glob("*.yml"), *workflow_directory.glob("*.yaml")))
        workflow_text = "\n".join(path.read_text(encoding="utf-8") for path in workflow_paths)
        assignment_names = set(_just_document()["assignments"])
        lookup_names = sorted(set(WORKFLOW_VERSION_LOOKUP.findall(workflow_text)))

        self.assertTrue(lookup_names)
        for name in lookup_names:
            with self.subTest(assignment=name):
                self.assertIn(name, assignment_names)
                self.assertTrue(_run_just("--evaluate", name).stdout.strip())

    def test_python_patch_version_has_one_source_of_truth(self) -> None:
        """Local and workflow Python selection must resolve the same exact patch."""
        version_file = (REPO_ROOT / ".python-version").read_text(encoding="utf-8").strip()

        self.assertRegex(version_file, r"^[0-9]+[.][0-9]+[.][0-9]+$")
        self.assertEqual(_run_just("--evaluate", "python_version").stdout.strip(), version_file)

    def test_deterministic_release_workflows_gate_pull_requests(self) -> None:
        """Coverage and generated documentation must validate before merge."""
        expectations = (
            ("codecov-upload.yml", "codecov", "CodeCov"),
            ("doxygen.yml", "validate", "docs"),
        )
        for filename, job_id, stable_name in expectations:
            with self.subTest(workflow=filename):
                workflow = _workflow_document(filename)
                self.assertEqual(workflow["on"]["pull_request"]["branches"], ["main"])
                self.assertEqual(workflow["jobs"][job_id]["name"], stable_name)

    def test_macos_matrix_runs_the_opt_in_viewer_contract(self) -> None:
        """The archival viewer stays opt-in locally but is exercised on its supported CI host."""
        workflow = _workflow_document("ci.yml")
        platform = workflow["jobs"]["platform"]
        matrix = platform["strategy"]["matrix"]["include"]
        viewer_cells = [cell["name"] for cell in matrix if cell["run_viewer_contract"]]
        viewer_steps = [step for step in platform["steps"] if step.get("name") == "Run the opt-in macOS viewer contract"]

        self.assertEqual(viewer_cells, ["macOS AppleClang"])
        self.assertEqual(len(viewer_steps), 1)
        self.assertEqual(viewer_steps[0]["if"], "matrix.run_viewer_contract")
        self.assertEqual(viewer_steps[0]["run"], "just viewer-build")


if __name__ == "__main__":
    unittest.main()
