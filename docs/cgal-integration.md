# CGAL 6.2 integration contract

This document records the supported CGAL boundary for CDT++ 1.0. It is the
audit record for issue #102 and complements the
[C++ API boundary](api-boundary.md), the
[ergodic-move contract](ergodic-moves.md), and the
[reproducibility contract](reproducibility.md).

## Supported dependency and compiler contract

The repository-owned vcpkg baseline in `vcpkg.json` is the dependency source
of truth. CMake requires exactly CGAL 6.2 and treats deprecated declarations as
errors by default. The baseline currently resolves CGAL 6.2 with Boost 1.91.0,
GMP 6.3.0 revision 4, MPFR 4.2.2 revision 1, and oneTBB 2023.0.0. Those
transitive versions are a resolution record for the pinned baseline, not
independent version promises.

The supported compiler floor is the intersection of CDT++'s C++23 library
requirements and CGAL 6.2's tested platforms:

| Compiler | Minimum accepted by CMake |
| --- | --- |
| GCC | 13.3 |
| upstream Clang | 22.0 |
| AppleClang | 15.0 (Xcode 15) |
| MSVC | 19.34 (Visual Studio 2022 17.4) |

CI exercises newer representatives of this matrix. A successful local
configuration demonstrates only that local compiler and platform; it does not
substitute for the other CI cells.

`ENABLE_DEPRECATION_ERRORS=ON` adds `/we4996` with MSVC and
`-Werror=deprecated-declarations` elsewhere. `CGAL_DONT_OVERRIDE_CMAKE_FLAGS`
keeps warning, sanitizer, and optimization policy under repository control.
CDT++ does not disable CGAL triangulation assertions or postconditions.

## Production usage inventory

| Surface | Classification | CGAL 6.2 decision |
| --- | --- | --- |
| `Triangulation_traits.hpp` kernel and info bases | Modernized | Retain EPICK; compose the cell info base over `Delaunay_triangulation_cell_base_3`; select the TDS concurrency tag explicitly; publish canonical Delaunay handle, facet, and edge types. |
| `Delaunay_state` construction and ownership | Modernized | Retain bulk point/info range insertion; reject duplicate geometric points with ambiguous labels; own the parallel lock grid with the triangulation and detach it before returning an unowned triangulation. |
| `FoliatedTriangulation` traversal and caches | Modernized | Use finite handle and simplex ranges; rebuild all derived handle caches at the owning mutation boundary and expose an opt-in diagnostic comparison. |
| Causality repair and vertex removal | Retained | Keep CGAL range removal and cavity retriangulation; every published replacement is constructed on a private copy and rebuilds caches before swap. |
| Point lookup | Retained | The current zero-overhead adapter adds optional absence handling and is used at stable point-value mutation boundaries. |
| Checked Delaunay and TDS flips | Retained | CGAL establishes combinatorial/geometric flippability; typed applicable moves independently establish CDT causal admissibility. |
| Persistence and fingerprints | Modernized | Use finite handle ranges while preserving canonical sorting. Native CGAL stream data remains version-coupled and is not claimed as a stable interchange format. |
| Spherical point generation | Retained | A caller-owned CDT random stream seeds `CGAL::Random`; the generated sequence and the limits of cospherical topology replay are documented separately. |
| CGAL timers and visualization | Removed from production | Production code uses standard timing and has no CGAL timer dependency. Qt visualization remains separately owned by issue #98. |
| Periodic 3D and d-dimensional torus prototypes | Excluded | These remain under `cdt::experimental`, outside supported-header compilation and production dependency decisions. They are archival source, not supported CGAL 6.2 APIs. |

No production path uses a deprecated CGAL triangulation API. Canonical
`Delaunay::Vertex_handle`, `Cell_handle`, `Facet`, and `Edge` types are retained
where they are the clearest zero-overhead representation; CDT++ does not add a
wrapper solely to hide CGAL.

## Kernel and exact-arithmetic policy

Production triangulation uses
`CGAL::Exact_predicates_inexact_constructions_kernel` (EPICK). CDT++ relies on
exact orientation and sphere-side predicates for Delaunay insertion, point
location, and checked geometric flips. Coordinates, centroids, distances, and
other constructed values remain floating-point approximations.

That split is intentional. The scientific state is the causal combinatorial
triangulation plus time and cell metadata; it is not a claim that every
constructed coordinate is exact. The integration suite includes an almost
coplanar orientation fixture and a co-spherical insertion fixture before any
future kernel change. There is no reproduced construction defect that
justifies the runtime, memory, persistence, and semantic cost of moving to an
exact-constructions kernel.

GMP and MPFR are retained. CGAL's `Gmpzf` is part of the existing exact-number
surface, and CDT action and Metropolis calculations directly use
`CGAL::Gmpfr` at 256-bit precision. The pinned CGAL port supplies and links
GMP/MPFR through `CGAL::CGAL`. Boost.Multiprecision is present through CGAL,
but replacing these production types would be a numerical and persistence
migration, not a dependency-only cleanup. No such migration is proposed
without adversarial equivalence tests and compile-time, runtime, and peak-memory
measurements from the benchmark protocol below.

## Metadata, insertion, and duplicate points

Vertices store one `Int_precision` time value and cells store one
`Int_precision` causal cell type through CGAL's info-bearing bases. The cell
base is layered over the Delaunay-specific base so it satisfies the current
cell concept, including the circumcenter operation.

Construction inserts the point/info pairs as one range, allowing CGAL's
spatially sorted bulk path while preserving pair association. The number of
inserted vertices must equal the number of input pairs. Two labels for one
geometric point would otherwise collapse to one vertex with ambiguous
metadata, so that input is rejected rather than normalized. Time values
arriving from wider unsigned types are range-checked before conversion.

Co-spherical inputs can have more than one mathematically valid Delaunay
triangulation. CDT++ tests validity and point/label preservation but does not
promise identical fresh topology across toolchains; see
[Reproducible random runs](reproducibility.md).

## Mutation and lifetime rules

CGAL documents that every triangulation modification invalidates iterators.
Checked TDS flips preserve vertex handles and invalidate cell handles only for
the affected cells. Removal destroys the selected vertex and its incident
cells while retriangulating the cavity. CDT++ deliberately applies the
following stricter owning rule:

| Operation | CDT++ lifetime action |
| --- | --- |
| Bulk insertion or repair insertion | Construct unpublished state, then build every vertex, cell, facet, edge, and classification cache. |
| Vertex or range removal | Mutate a private triangulation; do not retain any cached simplex or circulator across the call. |
| Facet or edge flip | Resolve point-value locators immediately before mutation; retain no affected cell handle after the checked flip. |
| Copy | Treat every handle in the copy as belonging to the copy; rebuild wrapper caches from its canonical triangulation. |
| Move or swap | Transfer the lock owner, triangulation, caches, and scalar bounds together. |
| Public snapshot | Return an owning triangulation with no borrowed lock-grid pointer; snapshot handles cannot mutate the source wrapper. |
| Published replacement | Rebuild derived caches and validate TDS, foliation, metadata, and tracked counts before a non-throwing swap; the full derived-cache comparison remains opt-in. |

The underlying mutable triangulation accessor is an internal CGAL algorithm
boundary. Public callers receive a read-only view or an owning snapshot, so
they cannot silently mutate canonical topology while leaving geometry or
handle caches stale.

## Checked flips and CDT admissibility

CGAL's checked `flip()` operations remain the mutation primitive where the
geometric or combinatorial precondition can still fail. Low-level TDS
operations are used only behind move-specific applicable values that prove the
local CDT topology, foliation, simplex-type, and metadata contract.

A successful CGAL flip is therefore necessary but not sufficient for a CDT
move. Mutations occur on private copies, postconditions are checked, and only a
valid replacement is published. The detailed per-move proof and invalidation
record is in [2+1D CDT ergodic move audit](ergodic-moves.md).

## Sequential and TBB-backed configurations

The reference build sets `ENABLE_PARALLEL_TRIANGULATION=OFF` and instantiates
the TDS with `CGAL::Sequential_tag`. Enabling the option:

1. requires oneTBB;
2. imports CGAL's current `CGAL::TBB_support` target;
3. links `TBB::tbb`, `TBB::tbbmalloc`, and the platform thread target through
   that CGAL target;
4. defines CGAL's `CGAL_LINKED_WITH_TBB` capability; and
5. instantiates the TDS with `CGAL::Parallel_tag`.

`CDT_ENABLE_PARALLEL_TRIANGULATION` records repository policy.
`CGAL_LINKED_WITH_TBB` remains a capability supplied by the linked CGAL target;
the traits header rejects a parallel policy without that capability. Merely
including TBB headers or setting the CGAL macro manually is unsupported.

Each parallel `Delaunay_state` owns the lock grid referenced by its
triangulation. Copies allocate and bind their own grid; moves transfer the
owner and pointer together. Returned snapshots are detached and operate
sequentially unless a new owner explicitly attaches a compatible grid. The
focused parallel test covers insertion, point/info association, copy, move,
lock-zone refusal without mutation, range removal, wrapper transfer, and
post-donor lifetime. Issue #88 owns scaling and any broader multithreaded
simulation policy.

## Reproducible performance baseline

The benchmark is diagnostic rather than a pass/fail test:

```console
just benchmark-cgal 640 5 50
```

The arguments are requested simplices, repetitions, and queued moves per
repetition. Input generation uses seed `102`. The report includes CGAL version,
TDS mode, generated and surviving topology counts, a checksum, and
minimum/median/maximum nanoseconds for:

- bulk point/info insertion;
- foliation repair;
- wrapper and cache construction;
- point lookup;
- triangulation snapshot copying;
- range vertex removal; and
- a representative five-move queued workload.

The seed fixes the generated input and move stream, not CGAL's choice among
valid co-spherical tetrahedralizations or the resulting foliation repair.
Topology counts and the checksum are comparison diagnostics, not strict
fresh-construction replay identities; see
[Reproducible random runs](reproducibility.md).

Compare builds only on the same machine, build type, compiler, dependency
baseline, and TDS mode. Record the commit, complete command, and raw output.
Use `/usr/bin/time -l` on macOS or `/usr/bin/time -v` on Linux around the
benchmark for peak resident memory. Time a clean target build separately when
evaluating compile-time changes. A kernel, arithmetic-backend, or parallelism
change must also compare topology counts, checksum, adversarial tests, and
simulation observables; wall-clock time alone is not acceptance evidence.

## Upgrade checklist

For a future CGAL upgrade:

1. update the pinned vcpkg baseline and the exact CMake version together;
2. review CGAL release notes, the 3D Triangulations deprecated list, TDS
   concepts, supported compilers, and the `CGAL::TBB_support` target;
3. record the resolved Boost, GMP, MPFR, and TBB versions;
4. build supported headers and API consumers with deprecations as errors;
5. run sequential and parallel insertion/removal and lock-ownership tests;
6. run near-degenerate, co-spherical, persistence, repair, and complete
   deterministic move suites;
7. regenerate the benchmark record on the same comparison host; and
8. update this inventory for every retained, modernized, replaced, removed, or
   newly experimental CGAL surface.

Do not change the kernel, exact backend, persistence representation, or
parallel simulation policy as an incidental part of a version bump.
