# CDT++ reference fixture package

This directory is the bounded behavior-level oracle published for issue #94
and for the Rust `causal-triangulations` implementation. CDT++ is an
independent implementation under comparison, not presumed ground truth. A
discrepancy is an investigation target until the protocol or one of the
implementations explains it.

## Regression-oracle scope

The principal reason to preserve this implementation is its causality-filtering Delaunay construction path in
[`Foliated_triangulation.hpp`](https://github.com/acgetchell/CDT-plusplus/blob/main/include/Foliated_triangulation.hpp).
`find_invalid_timevalue_cells` classifies cells from stored vertex time labels, `has_valid_timevalues` provides the
predicate, `find_bad_vertex` selects a vertex responsible for an acausal local configuration, and `fix_timevalues`
removes offending vertices through CGAL so the cavity is retriangulated until the foliation contract is satisfied.

The deterministic doctest scenario **"Detecting and fixing problems with vertices and cells"** in
[`Foliated_triangulation_test.cpp`](https://github.com/acgetchell/CDT-plusplus/blob/main/tests/Foliated_triangulation_test.cpp)
exercises this path with fixed points and time labels. Its inputs, detected bad vertex, final initialization state,
cell counts, and causal classification are the first comparison fixture for `causal-triangulations`; exact Monte
Carlo trajectories are not required to match.

The package extends that construction case with the complete move set, action values, Metropolis-Hastings decisions,
persistence records, and one bounded end-to-end run.

After building, run the construction fixture directly with:

```console
./out/build/reference/tests/CDT_unit_tests \
  --test-case='*Detecting and fixing problems with vertices and cells*'
```

## Package layout

- `schema/fixture-v1.schema.json` defines the comparison protocol.
- `schema/result-v1.schema.json` defines canonical C++ topology and action
  output.
- `schema/run-manifest-v1.schema.json` defines run and artifact provenance.
- `fixtures/v1/protocol.json` declares ordering, units, tolerances, exact
  fields, allowed divergence, move sites, action parameters, proposal
  probabilities, acceptance variates, persistence checks, and the bounded run.
- `raw/v1/cpp-reference.json` is direct output from the C++ fixture executable,
  before Python or notebook processing.
- `raw/v1/persistence-v1.off` and its `.off.meta` sidecar are the smallest
  committed persistence round trip.
- `raw/v1/end-to-end.txt` is the bounded one-pass C++ run.
- `raw/v1/scaling-threads-*.txt` preserves the matched #88 one-, two-, and
  four-thread records, including every raw sample.
- `manifests/v1/` records source, toolchain, build, host, thread limits, seeds,
  command lines, and SHA-256 artifact checksums.

Run the complete offline package validator with:

```console
just reference-check
```

It applies the complete JSON Schema Draft 2020-12 contracts, then checks
canonical identifiers and ordering,
incidence, reciprocal adjacency, causal edge and simplex classification,
f-vectors and Euler relations, all five move sites and deltas, raw proposal
domains, independent closed-form actions and Metropolis-Hastings probabilities,
deterministic decisions, persistence size and FNV-1a integrity, bounded-run
command provenance and declared f-vector band, matched scaling parameters, raw
sample counts, and manifest SHA-256 values.

## Rust consumption

Read `fixtures/v1/protocol.json` first. Entity arrays are canonical:

1. vertices sort by `(time, x, y, z)`;
2. edges, facets, and cells sort by their sorted vertex-id tuple;
3. dense `vNN`, `eNN`, `fNN`, and `cNN` identifiers follow array order; and
4. adjacent cell ids sort lexicographically.

Compare topology, incidence, adjacency, foliation labels, simplex and edge
types, f-vectors, move sites, integer deltas, and accept/reject decisions
exactly. Compare coordinates, actions, deltas, and probabilities with the
named quantity-specific absolute-plus-relative tolerance. Do not apply one
repository-wide percentage.

Transition fixtures provide the raw proposal site and acceptance variate.
They therefore test proposal preparation, action delta, Hastings factor, and
commit/reject behavior without requiring the C++ and Rust implementations to
share an RNG engine, allocation order, or container iteration order.

### Initial-triangulation interchange

This complete command writes the same manifested pair consumed by the second
command:

```console
just initialize -s -n640 -t4 -o --seed 92
just load /path/to/generated-file.off -a0.6 -k1.1 -l0.1 -p1000 --seed 93
```

1. the `.off` payload contains CGAL's native triangulation stream followed by
   CDT++'s versioned causal-data trailer; and
2. the neighboring `.off.meta` manifest records the artifact role, foliation
   parameters, seed and stream provenance, canonical fingerprints, payload
   checksum, and producer toolchain.

Despite the suffix, this is not generic mesh OFF. Plain geometry cannot carry
the vertex time labels, causal cell types, artifact role, or stochastic
provenance required to reconstruct a CDT state. The pair is the archival
interchange boundary and remains coupled to the pinned CDT++/CGAL persistence
contract documented in
[`docs/reproducibility.md`](../docs/reproducibility.md).

The successor workflow is:

```text
just initialize -s -n640 -t4 -o --seed 92
        |
        +--> initial.off + initial.off.meta
                 |                       |
                 +--> just load PATH      +--> causal-triangulations importer
                         -a0.6 -k1.1              |
                         -l0.1 -p1000             +--> native Rust triangulation
                         --seed 93
```

The `causal-triangulations` side should own that importer: validate the complete
pair at its input boundary, reconstruct its native invariant-bearing state, and
then use its own serialization. CDT++ should not add a second lossy converter
or pretend that the CGAL payload alone is portable. Direct Rust import is a
downstream compatibility direction, not a capability claimed by the CDT++
v1.0.0 release.

For generating many separately seeded random starting states, invoke
`just initialize` with a different seed in a dedicated directory for each run
and preserve every payload/manifest pair together. The seed replays pre-CGAL
random inputs; the persisted pair, not the seed alone, identifies the exact
post-repair topology to import or evolve.

CDT++ checkpoints serve a different purpose from this interchange boundary.
`just resume CHECKPOINT.off` can continue the identical CDT++ Markov
chain after an interrupted Slurm/HPC job because the checkpoint sidecar records
mutable PCG state and cumulative transition accounting. That restart contract
is deliberately locked to the recorded CDT++ source revision and producer
toolchain. A successor importer should consume `initial-triangulation`
artifacts for independent evolution, not depend on CDT++'s private checkpoint
engine state.

## Local comparison harness

The repository's [`cdt-compare`](../docs/comparison-harness.md) command copies
this protocol, the canonical C++ result, the v1 result schema, and a selected
run manifest into a local bundle before launching independent C++ and Rust
producers. It anchors the live C++ payload to the committed result, retains both
raw process records, anchors any live C++ transition observations to this
protocol, and applies only the exact and named numerical rules declared here.
`just comparison-analyze PATH` requires the complete canonical artifact
inventory, verifies every retained digest, and reproduces the machine-readable
summary without rerunning either implementation.

After building CDT++ and a compatible Rust fixture producer, run one bounded
comparison and retain it locally:

```console
just comparison-run /absolute/path/to/causal-triangulations-fixture out/comparisons/run-1
```

Reproduce `summary.json` entirely from the stored raw artifacts, without
running either executable:

```console
just comparison-analyze out/comparisons/run-1
```

The bundle under `out/comparisons/run-1` is published atomically only after
analysis and manifest creation finish. Preserve its `inputs/`, `raw/`,
`manifest.json`, and `summary.json` together. Python validates the artifact
inventory and its digests, validates schemas, constructs commands, classifies
comparisons, and renders a small text table; it does not implement topology,
action, move-legality, or acceptance rules. See the
[comparison-harness contract](../docs/comparison-harness.md) for producer
configuration, placeholders, artifact layout, failure records, and the C++
reference/Rust result boundary.

### Local initialization sweep

The retained `cdt-optimize-initialize` command is also entirely local and
dependency-free. It writes one directory per parameter pair under
`out/experiments/initialize`, including configuration JSON, raw stdout, a
tab-separated volume profile, metrics, artifact digests, and source/executable
provenance. Each invocation requires a nonexistent output path. Seed `92` is
the default; use `--seed` and `--output-directory` for another replayable
record. Fresh CGAL triangulations remain subject to the
[reproducibility contract](../docs/reproducibility.md).

Inspect its command line with:

```console
uv run cdt-optimize-initialize --help
```

## Regeneration

`reference-fixtures` is a quick diagnostic that builds and prints only
`cpp-reference.json`:

```console
just reference-fixtures
```

The archival workflow is:

```console
just reference-regenerate
```

It refuses a dirty worktree, builds the sequential and parallel configurations,
and regenerates all four artifact families before publishing them:

- the canonical topology/action JSON from `CDT_reference_fixture`;
- the bounded run transcript from `cdt`;
- the OFF payload and metadata sidecar from `initialize`; and
- the matched one-, two-, and four-thread records from `CDT_cgal_benchmark`.

The generator updates both manifests, records the exact producer command for
every file under `raw/v1/`, and refreshes all SHA-256 values. The recipe then
runs the offline validator and `reference-archive-check`, which requires every
raw record and manifest to name the same clean Git commit.
The v1 manifest templates are macOS-arm64-specific, so regeneration refuses a
different host instead of publishing records under a misleading platform name.

Regeneration is a review operation. Explain every exact-field change. A
randomized CGAL f-vector, wall-clock sample, or benchmark checksum change is
diagnostic data, not automatically a defect; deterministic minimal fixtures
and transition results must remain exact.

The comprehensive `just ci` gate additionally rebuilds the C++ fixture
executable and compares its canonical states and actions with the committed raw
record. It reuses the offline validation performed by `just check`; the
generated-only step compares exact topology and metadata while applying the
protocol’s named tolerances to coordinates and action values. Host, compiler,
and source-revision provenance are intentionally excluded from that equality
check.

The committed macOS records retain the `-dirty` revision captured during the
review for issue #94. Their scientific payload remains covered by
`just reference-check` and `just reference-generated-check`, but they are not
the final archival provenance record. [Issue #97](https://github.com/acgetchell/CDT-plusplus/issues/97)
must first commit the release metadata, run `just reference-regenerate` from
that exact clean producer commit, and then review and commit the regenerated
raw artifacts and manifests. The final merged release state is tagged; its
files name the preceding producer commit because an artifact commit cannot
record its own hash. Both `just tag` and `just tag-check` enforce the
provenance-only archival check, which intentionally fails until that handoff
is complete.

## Scope limits

The package does not claim exact cross-language Monte Carlo trajectories,
exact fresh topology for nested cospherical CGAL inputs, a general performance
study, or a calibrated phase-distribution equivalence result. The spherical
population value is explicitly a monotone construction heuristic; randomized
post-repair simplex counts are implementation-specific. The rigorous
quadratic tetrahedron bound is retained only for safety and preflight
reasoning.
