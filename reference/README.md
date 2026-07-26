# CDT++ reference fixture package

This directory is the bounded behavior-level oracle published for issue #94
and for the Rust `causal-triangulations` implementation. CDT++ is an
independent implementation under comparison, not presumed ground truth. A
discrepancy is an investigation target until the protocol or one of the
implementations explains it.

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

The committed macOS records contain a `-dirty` revision because they document
the issue #94 implementation while it is under review. Regenerate them from
the resulting clean commit before the archival v1.0.0 tag; the archival check
will fail until that handoff is complete.

## Scope limits

The package does not claim exact cross-language Monte Carlo trajectories,
exact fresh topology for nested cospherical CGAL inputs, a general performance
study, or a calibrated phase-distribution equivalence result. The spherical
population value is explicitly a monotone construction heuristic; randomized
post-repair simplex counts are implementation-specific. The rigorous
quadratic tetrahedron bound is retained only for safety and preflight
reasoning.
