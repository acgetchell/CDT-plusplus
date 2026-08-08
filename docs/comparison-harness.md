# Offline C++/Rust comparison harness

`cdt-compare` is a local orchestration and analysis boundary around independent
CDT++ and `causal-triangulations` producers. Raw process artifacts are
canonical. Python does not calculate expected topology, action values, move
legality, or acceptance decisions.

## Producer contract

Each producer writes exactly one UTF-8 JSON object to standard output and may
write diagnostics to standard error. The committed CDT++
`cdt-reference-raw-v1` document is validated directly against
[`result-v1.schema.json`](https://github.com/acgetchell/CDT-plusplus/blob/main/reference/schema/result-v1.schema.json).
Every bundle copies that canonical result, and the harness checks the live
CDT++ payload against it before using the live C++ payload as the reference for
the Rust comparison. When live C++ output includes transition observations,
the harness first checks them against the committed transitions in the copied
protocol, so matching C++ and Rust drift cannot replace the #94 oracle.

A language-neutral producer may use `schema: "cdt-comparison-result-v1"`. Its
`implementation` object must contain nonempty `name`, `version`, and `revision`
fields and may add language-specific provenance. Its `states` and `actions`
arrays are validated with the state and action definitions from the #94 result
schema. It may also provide a `transitions` array containing the #94 transition
id, move, before/proposed/committed state ids, site, parameters, proposal and
reverse probabilities, action delta, acceptance probability and variate,
decision, and committed state.

When both producers include transition observations, the harness compares the
protocol-anchored live C++ observations with Rust, using exact transition
fields and the named action and probability tolerances. When neither includes
them, the before/proposed state payloads are still compared and `summary.json`
classifies the missing observation stream as unsupported. One-sided transition
coverage is a comparison failure.

The harness launches commands directly, without a shell. Use an executable
path or a command discoverable through `PATH`; add each argument separately:

```bash
uv run --no-sync cdt-compare run \
  --cpp out/build/reference/tests/CDT_reference_fixture \
  --rust /absolute/path/to/causal-triangulations-fixture \
  --rust-arg=--protocol \
  --rust-arg='{protocol}' \
  --rust-cwd /absolute/path/to/causal-triangulations \
  --output-directory out/comparisons/run-1
```

Arguments support `{protocol}`, `{manifest}`, and `{result_schema}`
placeholders. Both commands also receive the same absolute input paths through
`CDT_COMPARISON_PROTOCOL`, `CDT_COMPARISON_REFERENCE_MANIFEST`, and
`CDT_COMPARISON_RESULT_SCHEMA`. Only basic process/runtime variables are
inherited; credentials and hosted-service configuration are not passed.

The shorter repository command uses the default CDT++ fixture executable:

```bash
just comparison-run /absolute/path/to/causal-triangulations-fixture out/comparisons/run-1
```

## Artifact layout

Every run requires a nonexistent output directory. It is assembled in a
sibling staging directory and atomically published with this layout only after
analysis and manifest creation finish:

```text
run-1/
├── inputs/
│   ├── fixture-v1.schema.json
│   ├── cpp-reference.json
│   ├── run-manifest-v1.schema.json
│   ├── protocol.json
│   ├── reference-manifest.json
│   └── result-v1.schema.json
├── raw/
│   ├── cpp/
│   │   ├── process.json
│   │   ├── stderr.txt
│   │   └── stdout.txt
│   └── rust/
│       ├── process.json
│       ├── stderr.txt
│       └── stdout.txt
├── manifest.json
└── summary.json
```

`process.json` records the resolved command, working directory, environment
contract, executable path/size/SHA-256, exit status, timeout state, and
executable duration. `manifest.json` records copied-input provenance, host and
Python identity, SHA-256 and size for every retained input/raw artifact,
separate per-executable timing, analysis time, and total orchestration time.
Executable time is never combined with Python analysis time in a performance
claim.

Producer launch, timeout, exit, and result-schema failures are normal comparison
outcomes, so their complete raw evidence is still published. A harness-internal
failure removes its staging directory and leaves the requested output path
available for a corrected retry.

`summary.json` is deterministic and concise. It records implementation
metadata, exact and numerical check counts, the complete #94 classification
policy, transition coverage, status, and failures. Each failure identifies the
Rust implementation, fixture, field, comparison rule or named tolerance,
expected C++ value, observed Rust value, and retained Rust stdout artifact.
CDT++ is the reference side of the comparison, not presumed scientific truth;
a discrepancy remains an investigation target.

## Offline reproduction

Reanalysis first requires the exact canonical input/raw artifact inventory,
rejecting missing, unexpected, duplicate, escaping, or noncanonical paths. It
then verifies every retained size and SHA-256 before rebuilding the summary. It
never launches either implementation:

```bash
just comparison-analyze out/comparisons/run-1
```

The direct equivalent is:

```bash
uv run --no-sync cdt-compare analyze out/comparisons/run-1
```

Preserve the entire directory. Copying only `summary.json` discards the raw
evidence needed to reproduce or investigate the result.

## Scientific boundary

The harness performs schema validation, command construction, artifact
retention, declared exact comparisons, named absolute-plus-relative numerical
comparisons, classification, and rendering. It never imports either
implementation and does not derive scientific expected values. The #94
protocol remains the authority for canonical ordering, quantity-specific
tolerances, deterministic fixtures, implementation-specific fields, and
unsupported claims.
