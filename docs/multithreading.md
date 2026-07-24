# Multithreaded CGAL contract

CDT++ retains one bounded multithreaded capability for the archived C++
reference: CGAL may use oneTBB for bulk 3D Delaunay insertion and range
removal. The deterministic `reference` configuration remains the canonical
single-threaded correctness oracle. Enabling the parallel configuration does
not create a task runtime, parallelize Pachner moves, or make a
`FoliatedTriangulation` safe for concurrent member calls.

## Build and test

Run the supported opt-in configuration from a clean checkout with:

```console
just build-parallel
```

This bootstraps the pinned vcpkg baseline, configures the `parallel` CMake
preset in `out/build/parallel`, builds every production and test target, and
runs the `parallel-smoke` preset. The complete ordinary suite is compiled with
`CGAL::Parallel_tag`; a separate launcher adds lock ownership, contention,
failure-atomicity, transfer, and replayable stress coverage.

The equivalent CMake surface, when the pinned vcpkg toolchain is already
available, is:

```console
cmake --preset parallel
cmake --build --preset parallel
ctest --preset parallel-smoke
```

The supported release matrix is the same C++23 matrix declared by the root
CMake configuration: Linux with GCC 13.3 or newer or Clang 22 or newer, macOS
with AppleClang 15 or newer, and Windows with MSVC 19.34 or newer. The primary
CI workflow runs `build-parallel` with Ubuntu GCC, Ubuntu Clang, macOS
AppleClang, and Windows MSVC. The Linux AddressSanitizer workflow supplies the
additional parallel sanitizer cell. Configuration fails when the selected
CGAL package does not provide `CGAL::TBB_support`; a compiler version alone is
not evidence that the dependency boundary is available.

The stress scenario reports its replay inputs through doctest captures.
Override them to reproduce or expand a run:

```console
CDT_PARALLEL_TEST_SEED=88 \
CDT_PARALLEL_TEST_THREADS=4 \
CDT_PARALLEL_TEST_ITERATIONS=16 \
./out/build/parallel/tests/CDT_parallel_triangulation_test
```

## Runtime thread limit

Both triangulation-producing commands expose the same resource contract:

```console
./out/build/parallel/src/cdt ... --threads 4
./out/build/parallel/src/initialize ... --threads 4
```

`--threads N` sets oneTBB's maximum allowed parallelism for the lifetime of
the command, and each command reports and persists the selected value. The
default is 1 so an unqualified invocation remains single-threaded. Values
less than 1 are invalid. The `reference` build accepts only 1; larger values
require a `parallel` build. The value is a cap rather than a promise that
CGAL will schedule that many workers, especially for small triangulations.

The scoped oneTBB control does not broaden the supported execution boundary:
only the eligible CGAL operations below can consume worker threads.

## Supported execution boundary

| Operation | Execution and ownership contract |
| --- | --- |
| Point/info range insertion | CGAL may schedule internal oneTBB work. The unpublished `Delaunay_state` owns the lock grid for the complete call. |
| Vertex-handle range removal | CGAL may schedule internal oneTBB work. No cached handle, iterator, or circulator is retained across removal. |
| Foliation repair | Classification is sequential. The invalid-vertex batch is removed through the supported CGAL range operation before caches are built. |
| Wrapper construction and caches | Sequential. A complete private triangulation is classified before publication. |
| Pachner moves and Metropolis-Hastings | Sequential copy/validate/swap transactions. Parallel builds preserve the same admissibility, action, probability, and counter contracts. |
| Persistence and snapshots | Sequential. Snapshots detach the non-owning lock pointer; persisted state is validated before atomic publication. |
| Concurrent wrapper access | Unsupported. Callers must externally serialize access to one `FoliatedTriangulation` or `Manifold`. Independent objects do not share topology or RNG state. |

Every parallel triangulation has exactly one lock-grid owner. Copies allocate
and bind a new grid. Moves and swaps transfer the triangulation and owner
together. A handle belongs only to the triangulation that produced it; using a
handle after mutation or with a copy is unsupported.

There is no public cancellation API. A supported CGAL range call runs to
completion inside its owning scope. Construction and repair happen on
unpublished state, so an exception destroys the complete candidate rather than
publishing partial topology. Published scientific mutations continue to use
private-copy validation followed by a non-throwing swap.

`cdt::Random` remains thread-confined. The current parallel operations consume
no random draws inside worker tasks. Any future stochastic worker must receive
a unique, stable `root.split(worker_stream)` engine; sharing one mutable engine
is a data race and is unsupported.

## Determinism and correctness

The `reference` preset uses `CGAL::Sequential_tag` and remains the
step-by-step oracle. The `parallel` preset must preserve TDS validity,
foliation and causal classification, geometry counts, action values, move
semantics, cache invariants, and persistence round trips. Those contracts are
exercised by running the complete unit and integration suite under the
parallel TDS.

oneTBB scheduling and CGAL's valid choices for cospherical input may change
fresh topology and the resulting Monte Carlo trajectory. Parallel runs
therefore promise invariant and distributional equivalence, not byte-identical
topology or trajectories.

## Sanitizers

The Linux `asan` preset enables the parallel TDS and exercises the parallel
contract under AddressSanitizer and UndefinedBehaviorSanitizer. The pinned
CGAL/oneTBB dependency build is not an authoritative ThreadSanitizer
combination, so CMake rejects `ENABLE_PARALLEL_TRIANGULATION=ON` together with
`ENABLE_SANITIZER_THREAD=ON`. The `tsan` preset continues to instrument the
repository-owned sequential path. This unsupported cell is explicit rather
than being reported as parallel TSan evidence.

## Matched scaling record

Correctness gates must pass before recording performance. Run the same
parallel binary at one and increasing thread limits:

```console
just benchmark-cgal-parallel "1 2 4 8" 640 5 50 1
```

The arguments are thread counts, requested simplices, measured repetitions,
queued moves, and discarded warm-up repetitions. Each record includes the
source revision and dirty marker, compiler and standard library, platform and
processor, dependency versions, logical and active thread counts, seed,
fixture and parameters, checksum, and every raw nanosecond sample. Run
`just benchmark-cgal` with the same workload for the canonical sequential-TDS
latency.

Compare records only on the same machine, build profile, dependency baseline,
fixture, parameters, warm-up policy, and measurement protocol. The
`cdt-cgal-benchmark-v1` key/value record is the raw C++ input for issue #94's
language-neutral comparison manifest; #94 owns cross-language tables and
plots. A correctness failure invalidates the associated timing record.
