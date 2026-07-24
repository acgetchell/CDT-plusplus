# Reproducible random runs

CDT++ owns random state at the simulation boundary. The root `cdt::Random`
engine records a 64-bit seed and derives named PCG streams for independent
subsystems:

- stream `0` generates the initial triangulation;
- stream `1` selects and constructs Metropolis-Hastings transitions.

Pass `--seed SEED` to `cdt` or `initialize` to replay the random inputs to a
run. Without that option, the command obtains operating-system entropy once
and prints the effective seed. The validated runtime configuration retains that
effective seed rather than the pre-parse default. Checkpoint and final payload
filenames include `seed-SEED`; simulation checkpoints also include
`pass-PASS`. Filename timestamps use UTC and replace the colon separators that
are invalid in Windows filenames.

```console
./out/build/reference/src/cdt -s -n640 -t4 -a0.6 -k1.1 -l0.1 -p10 --seed 92
./out/build/reference/src/initialize -s -n640 -t4 --seed 92
```

`--threads` is orthogonal to RNG ownership. It defaults to 1 and records the
maximum eligible Delaunay concurrency as `parallel.max_threads` in persisted
metadata. Parallel scheduling consumes no random draws; changing the thread
limit can nevertheless select another valid CGAL topology for cospherical
input, so the fresh-topology replay limitations below still apply.

Distributions are short-lived operations applied to a caller-owned engine.
Algorithms do not acquire entropy per sample, and tests use fixed seeds. The
repository-owned Semgrep rules prevent new direct `std::random_device` or PCG
engine construction outside `Random.hpp`. The supported spherical CGAL point
generator receives a single seed derived from its caller-owned initialization
stream rather than using CGAL's hidden default generator.

Stream consumption is sequential and defined at the subsystem boundary. The
initialization stream supplies the one seed used to construct CGAL's spherical
point generator. The generated vertices retain the existing exact spherical
placement and CGAL range-insertion path; reproducibility does not perturb their
radii or change foliation repair policy. For each Metropolis attempt, the
transition stream selects a
move type, draws the acceptance variate, and then performs the selected raw-site
selection and any candidate shuffles. A pass captures the current simplex count
before its first attempt. Failed candidate construction is an explicit
self-transition and does not retry a different site. The output metadata
records an FNV-1a fingerprint of the ordered move/outcome trace—candidate
failure, acceptance, or Metropolis-Hastings rejection—and its transition count.
The fingerprint is a compact replay diagnostic, not a cryptographic proof.

The reproducibility guarantee is exact for PCG draws and the ordered,
pre-CGAL initialization point sequence on the recorded supported toolchain.
Given an identical starting manifold, the complete Metropolis transition
sequence and counters also replay exactly. Canonical proposal ordering only
maps random draws to the same uniformly selected candidate; it does not change
the candidate set or proposal probabilities.

A freshly constructed triangulation is deliberately not promised to have
identical topology or a matching post-repair vertex set, even in separate
processes using the same toolchain. Points on each spherical layer are
cospherical, so CGAL may choose a different valid tetrahedralization when
resolving geometric ties. Foliation repair operates on that tetrahedralization
and may consequently remove different vertices. CDT++ does not perturb point
radii, replace CGAL's range insertion, or change foliation repair policy solely
to force fresh-topology replay.

The placement fingerprint describes the finite vertices that survive CGAL
construction and foliation repair. Along with the topology fingerprint and
actual counts, it makes output-state differences visible; it is not a
fingerprint of the pre-CGAL generated point sequence. Exact generation replay
is tested directly before insertion.

## Persistence contract

Each new stochastic `.off` payload has a neighboring `.off.meta` text
manifest. The manifest records:

- the root seed, PCG engine, and named stream identifiers;
- the explicit limits on fresh-topology and transition replay;
- requested and actual topology dimensions and counts;
- foliation parameters and, for simulations, the action parameters and pass
  cadence;
- the maximum requested Delaunay thread count;
- completed passes and the transition-trace fingerprint;
- a canonical placement fingerprint derived from sorted finite vertices and
  their timeslices;
- a canonical topology fingerprint derived from sorted vertices, causal
  metadata, and finite cells;
- the CDT++ version, compiler, build configuration, standard library,
  operating system, architecture, C++ standard, and CGAL version;
- the payload byte count and FNV-1a corruption checksum.

The triangulation remains a CGAL-readable payload; provenance is in the sidecar
rather than prepended to the CGAL stream. Because CGAL's native triangulation
stream omits `info()` fields, CDT++ appends a versioned, canonically ordered
causal-data trailer that preserves every finite vertex timeslice and cell type.
Legacy streams without this trailer remain topology-readable. Before
publication, CDT++ serializes with round-trip floating-point precision to a
temporary file, flushes and closes it, parses the complete CGAL stream and
causal trailer, rejects any other trailing data, validates its triangulation
data structure, and requires the parsed dimension, incidence counts, causal
metadata, and canonical topology fingerprint to match the source. It similarly
writes and reparses the complete typed manifest. Payload-derived dimensions,
incidence counts, time bounds, placement fingerprint, and topology fingerprint
are derived from the serialized state rather than trusted from caller-supplied
metadata. The manifest is published first and the payload second using
same-directory atomic replacement (`rename` on POSIX and `MoveFileExW` on
Windows). If a process is interrupted between the two replacements, their
checksum mismatch is detectable rather than silently pairing a payload with
stale provenance.

Reads of a manifested payload verify the size and checksum before parsing, then
repeat the complete-input and causal-metadata checks and compare every
payload-derived manifest field with the parsed state. Evolved CDT states are
abstract causal triangulations and need not retain the Euclidean Delaunay
empty-sphere property after Pachner moves, so integrity validation requires a
valid CGAL triangulation data structure without imposing Delaunayhood. Legacy
files without a manifest remain readable, but naturally have no checksum or
provenance guarantee; legacy CGAL streams also lack the causal `info()` data
that older CDT++ versions never serialized.
Malformed manifests, truncated payloads, trailing input, invalid topology, and
manifest/payload mismatches fail with filesystem diagnostics. FNV-1a protects
against accidental truncation or corruption; it does not authenticate files
against deliberate modification.

Checkpoints are snapshots only. CDT++ does not currently expose a resume CLI,
and a checkpoint does not serialize mutable PCG engine state or enough runtime
state to continue the identical stream. The manifest explicitly records
`resume_supported=false`. The manifest also records
`fresh_topology_replay_supported=false` and
`transition_replay_requires_identical_start=true`. Starting a second CLI run
from the recorded seed and configuration replays the stochastic inputs, but it
does not override CGAL's non-unique cospherical tetrahedralization. Exact
transition replay is conditional on supplying an identical starting manifold;
it is not checkpoint resume.

Payload parsing is supported for the repository's declared build matrix and
pinned dependency set. Exact PCG prefixes replay on the same supported
toolchain; transition traces replay when the starting manifold is identical.
The manifest makes fresh-construction and cross-toolchain differences
diagnosable, but it does not promise matching fresh topology, post-repair
placement, counts, or payload bytes. CGAL may also serialize the same abstract
topology in a different handle iteration order, so equality of a persisted
state is defined by the canonical topology fingerprint and reproducibility
fields rather than raw payload byte order.

## Parallel stream policy

`cdt::Random` is not internally synchronized. One mutable engine belongs to
one sequential run or one thread. Before parallel stochastic work is enabled,
each worker must receive `root.split(worker_stream)` with a unique, stable
stream identifier; engines must not be shared concurrently. PCG's stream
selector gives independently parameterized sequences while retaining the root
seed needed for replay. See the repository's
[PCG reference](../REFERENCES.md#pcg-random-number-generators).

## Move-selection performance check

Issue #105 removes the old entropy-per-draw behavior from move-heavy paths.
The benchmark retains that behavior only as a labeled baseline and compares it
with one run-owned PCG stream:

```console
just benchmark-rng 10000
```

The diagnostic reports both durations and their ratio. It is intentionally not
a pass/fail CI test because operating-system entropy latency and runner load are
machine-dependent; RNG-prefix, pre-CGAL point-generation, identical-start
transition replay, and distribution boundaries remain correctness tests in
`just ci`.
