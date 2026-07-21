#!/usr/bin/env python3
"""Generate a per-fixture Semgrep config from fixture annotations."""

import argparse
import re
import sys
from pathlib import Path
from typing import TYPE_CHECKING

if TYPE_CHECKING:
    from collections.abc import Sequence

UTF8 = "utf-8"
RULE_ANNOTATION_RE = re.compile(r"(?:ruleid|ok):\s*([^\n]+)")
RULE_SPLIT_RE = re.compile(r"(?m)^  - id: ")


def annotated_rule_ids(fixture_text: str) -> list[str]:
    """Return unique CDT++ Semgrep rule IDs referenced by one fixture."""
    rule_ids: list[str] = []
    for match in RULE_ANNOTATION_RE.finditer(fixture_text):
        for raw_rule_id in match.group(1).split(","):
            rule_id = raw_rule_id.strip()
            if rule_id.startswith("cdt.") and rule_id not in rule_ids:
                rule_ids.append(rule_id)
    return rule_ids


def config_rule_chunks(config_text: str) -> dict[str, str]:
    """Return YAML chunks keyed by Semgrep rule ID."""
    chunks: dict[str, str] = {}
    for chunk in RULE_SPLIT_RE.split(config_text)[1:]:
        lines = chunk.splitlines()
        if not lines:
            continue
        rule_id = lines[0].strip()
        chunks[rule_id] = f"  - id: {chunk}"
    return chunks


def build_fixture_config(fixture_path: Path, source_config_path: Path) -> str:
    """Build the minimal Semgrep config needed to test one fixture."""
    annotation_ids = annotated_rule_ids(fixture_path.read_text(encoding=UTF8))
    rule_chunks = config_rule_chunks(source_config_path.read_text(encoding=UTF8))
    missing_rule_ids = [rule_id for rule_id in annotation_ids if rule_id not in rule_chunks]
    if missing_rule_ids:
        missing_rules = ", ".join(missing_rule_ids)
        msg = f"missing Semgrep rules for fixture {fixture_path}: {missing_rules}"
        raise ValueError(msg)
    return "rules:\n" + "".join(rule_chunks[rule_id] for rule_id in annotation_ids)


def parse_args(argv: "Sequence[str]") -> argparse.Namespace:
    """Parse command-line arguments."""
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("fixture", type=Path)
    parser.add_argument("source_config", type=Path)
    parser.add_argument("output_config", type=Path)
    return parser.parse_args(argv)


def main(argv: "Sequence[str] | None" = None) -> int:
    """Write the selected rules beside the mirrored fixture path."""
    args = parse_args(sys.argv[1:] if argv is None else argv)
    try:
        config = build_fixture_config(args.fixture, args.source_config)
        args.output_config.write_text(config, encoding=UTF8)
    except ValueError as exc:
        print(exc, file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
