# Reproducible random runs

CDT++ owns random state at the simulation boundary. The root `cdt::Random`
engine records a 64-bit seed and derives named PCG streams for independent
subsystems:

- stream `0` generates the initial triangulation;
- stream `1` selects and constructs Metropolis-Hastings transitions.

Pass `--seed SEED` to `cdt` or `initialize` to replay the random inputs to a
run. Without that option, the command obtains operating-system entropy once
and prints the effective seed. Checkpoint and final OFF filenames also include
`seed-SEED`; simulation checkpoints include `pass-PASS`. The seed is metadata
in the filename rather than extra OFF content so files remain readable by
standard CGAL triangulation parsers.

```console
./out/build/reference/src/cdt -s -n640 -t4 -a0.6 -k1.1 -l0.1 -p10 --seed 92
./out/build/reference/src/initialize -s -n640 -t4 --seed 92
```

Distributions are short-lived operations applied to a caller-owned engine.
Algorithms do not acquire entropy per sample, and tests use fixed seeds. The
repository-owned Semgrep rules prevent new direct `std::random_device` or PCG
engine construction outside `Random.hpp`. The supported spherical CGAL point
generator receives a single seed derived from its caller-owned initialization
stream rather than using CGAL's hidden default generator.

The reproducibility guarantee is exact for PCG draws and generated
initialization points. Given the same starting manifold, the complete
Metropolis transition sequence and counters also replay exactly. A freshly
constructed triangulation is not promised to have byte-identical topology
across CGAL versions, platforms, or builds: points on the same spherical layer
are cospherical, so CGAL may choose a different valid tetrahedralization when
resolving geometric ties.

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
machine-dependent; replay and distribution boundaries remain correctness tests
in `just ci`.
