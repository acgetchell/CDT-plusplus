# GMP/MPFR-free arithmetic evaluation

This document is the decision record for issue
[#140](https://github.com/acgetchell/CDT-plusplus/issues/140). It evaluates
whether CDT++ can remove GMP and MPFR without weakening exact geometric
predicates or the numerical range used by the action and Metropolis-Hastings
calculation. It does not perform the production migration.

## Decision

Boost-native arithmetic is technically viable on the evaluated arm64 macOS
configuration, but defer the production migration until
[microsoft/vcpkg#53269](https://github.com/microsoft/vcpkg/issues/53269) (or an
equivalent upstream feature) lets the repository request CGAL without GMP and
MPFR. The eventual separately scoped migration would:

- keep `CGAL::Exact_predicates_inexact_constructions_kernel` (EPICK), but use
  CGAL's `BOOST_BACKEND` for its exact predicate fallback;
- replace the 256-bit `CGAL::Gmpfr` action and acceptance layer with a fixed
  256-bit binary `boost::multiprecision::cpp_bin_float` value; and
- remove the unused `CGAL::Gmpzf` compatibility surface.

Do not adopt the geometry-only hybrid. It passes the local geometry gates, but
MPFR would remain in the action layer, so GMP and MPFR would still be built and
the dependency problem would not change.

The local evidence supports the upstream vcpkg request and retaining Boost as
the preferred candidate. Boost action arithmetic is slower in isolation, but
the measured cost model adds about 0.217 seconds to the 8.5-second seeded
Metropolis fixture. The evaluated exact-predicate workload is effectively
neutral, and the local correctness gates pass. Clean-cache dependency timing,
the supported compiler/platform matrix, and sanitizer coverage remain migration
gates; this evaluation branch does not change the production implementation or
manifest.

## Required contracts

### Geometry

CDT++ needs exact signs for EPICK orientation and sphere-side predicates. It
does not need exact constructed coordinates. CGAL's
[`TriangulationTraits_3`](https://doc.cgal.org/latest/Triangulation_3/classTriangulationTraits__3.html)
and
[`DelaunayTriangulationTraits_3`](https://doc.cgal.org/latest/Triangulation_3/classDelaunayTriangulationTraits__3.html)
identify those predicate boundaries. CGAL 6.2 supports either GMP/MPFR or
Boost.Multiprecision for exact number types, as documented in its
[third-party dependency policy](https://doc.cgal.org/latest/Manual/thirdparty.html).

For the Boost configuration, CGAL selects `boost::multiprecision::cpp_int` and
`cpp_rational` for the exact fallback. The required result is the same exact
predicate sign, not an identical tetrahedralization for degenerate
co-spherical input. CGAL can choose among multiple valid Delaunay
representatives, as already recorded in the
[CGAL integration contract](cgal-integration.md#metadata-insertion-and-duplicate-points).

### Action and Metropolis-Hastings arithmetic

The candidate action value has:

- 256 binary significand bits;
- round-to-nearest arithmetic with ties to even;
- `sqrt`, `asinh`, `acos`, and `exp` over the production domains;
- enough exponent range to keep `exp(-1'000'000)` positive; and
- enough precision to distinguish action values that collapse to one
  `double`.

The evaluated type is:

```cpp
boost::multiprecision::number<
    boost::multiprecision::backends::cpp_bin_float<
        256,
        boost::multiprecision::backends::digit_base_2,
        void,
        std::int32_t>,
    boost::multiprecision::et_off>
```

Boost documents `cpp_bin_float` as an all-C++,
[header-only binary floating-point backend](https://www.boost.org/doc/libs/latest/libs/multiprecision/doc/html/boost_multiprecision/tut/floats.html).
The evaluation does not claim that Boost transcendental functions are
correctly rounded. Instead, the production MPFR calculation is retained as an
independent 256-bit oracle and the candidate must remain within `1e-70`
relative error on adversarial fixtures.

### Packaging

The vcpkg `cgal` port at baseline
`9e593bb18ea69cc5095e012465dcd675a822ed0d` unconditionally depends on `gmp`
and `mpfr`, even though CGAL itself supports the Boost backend. Merely setting
`CGAL_CMAKE_EXACT_NT_BACKEND=BOOST_BACKEND` therefore does not remove either
native dependency. The optional-default-feature request is tracked upstream as
[microsoft/vcpkg#53269](https://github.com/microsoft/vcpkg/issues/53269).

The evaluation-only port shape moves GMP and MPFR into a default `gmp` feature.
The default preserves current behavior; a Boost-only consumer uses:

```json
{
  "name": "cgal",
  "default-features": false
}
```

When the feature is absent, a vcpkg CMake wrapper sets
`CGAL_DISABLE_GMP=ON` and `CGAL_CMAKE_EXACT_NT_BACKEND=BOOST_BACKEND`. The
candidate port and standalone consumer live under
[`tests/cgal_no_gmp_probe`](https://github.com/acgetchell/CDT-plusplus/tree/main/tests/cgal_no_gmp_probe). They are a one-time
evaluation fixture, not a production overlay or root-CI dependency contract;
shipping a downstream fork would recreate the maintenance burden this work is
meant to remove.

## Production inventory

| Surface | Actual requirement | Migration action |
| --- | --- | --- |
| `Triangulation_traits.hpp` | EPICK exact predicate signs | Keep EPICK and select CGAL `BOOST_BACKEND`. |
| `Mpfr_value.hpp` | 256-bit action and acceptance values | Preserve its value-returning operations over fixed 256-bit `cpp_bin_float`; rename the namespace and documentation so MPFR is no longer promised. |
| `S3Action.hpp` | `sqrt`, `asinh`, `acos`, constants, and sub-double distinctions | Retain formulas and validation; compare the replacement against the MPFR oracle before deleting the oracle from production. |
| `Metropolis.hpp` | Exact proposal ratios, `exp`, positive tiny probabilities, and a comparison with the random draw | Replace direct `mpfr_cmp` access with backend-neutral value comparison. |
| `Settings.hpp` and `Utilities.hpp` | No production need for `CGAL::Gmpzf` | Remove the `Gmpzf` alias, `gmpzf_to_double()`, and their test-only compatibility fixture. |
| `vcpkg.json` | Direct Boost.Multiprecision use after migration | Add `boost-multiprecision`; request `cgal` with `default-features: false`. |

The `Gmpzf` alias is used only by `gmpzf_to_double()` and its unit test. It is
not part of triangulation predicates, action evaluation, Metropolis-Hastings,
persistence, or the CLI.

## Alternatives

| Candidate | Geometry correctness | Action range and precision | Removes GMP/MPFR | Representative cost | Decision |
| --- | --- | --- | --- | --- | --- |
| Current CGAL default plus MPFR action | Pass | Pass | No | Baseline | Retain while migration is deferred. |
| CGAL Boost predicates plus MPFR action | Pass | Pass | No | Geometry-neutral | Reject: it keeps the dependency burden. |
| CGAL Boost predicates plus native Boost 256-bit action | Pass locally | Pass locally against MPFR oracle | Yes, with an upstream vcpkg feature | About 2.6% modeled Metropolis cost | Preferred candidate; defer pending upstream support and the remaining migration gates. |
| Built-in floating types for the action | Geometry unaffected | Fail: sub-double deltas and sub-`long double` probabilities collapse | Yes | Fast but incorrect | Reject. |

## Evaluation protocol and provenance

The measurements below used baseline commit
`3d554296e97fd7198b0a9790b94d9230c06ff7bd` plus the issue #140 evaluation
changes committed alongside this document. The raw benchmark therefore reports
that baseline revision with a dirty suffix. The environment was AppleClang
21.0.0, C++23, Release mode, CGAL 6.2, Boost 1.91, GMP 6.3.0, and MPFR 4.2.2 on
arm64 macOS. Geometry comparisons were sequential and used one thread. These
figures are decision-spike diagnostics rather than archival benchmark
artifacts; an archival comparison must run from a clean migration revision.

The principal commands are:

```console
cmake -S . -B out/build/default-geometry \
  -DVCPKG_MANIFEST_MODE=OFF
cmake -S . -B out/build/boost-geometry \
  -DVCPKG_MANIFEST_MODE=OFF \
  -DCGAL_CMAKE_EXACT_NT_BACKEND=BOOST_BACKEND

ctest --test-dir out/build/boost-geometry --output-on-failure
out/build/boost-geometry/tests/CDT_cgal_benchmark 640 9 50 1 2
out/build/boost-geometry/tests/CDT_arithmetic_backend_benchmark 1000 7
```

The evaluation-only no-GMP package probe uses the candidate overlay and
manifest directly:

```console
cmake -S tests/cgal_no_gmp_probe -B <build-directory> \
  -DCMAKE_TOOLCHAIN_FILE=<vcpkg>/scripts/buildsystems/vcpkg.cmake \
  -DVCPKG_OVERLAY_PORTS=$PWD/tests/cgal_no_gmp_probe/overlay-ports/cgal
cmake --build <build-directory>
<build-directory>/cgal_no_gmp_probe
```

Set `VCPKG_BINARY_SOURCES=clear` when measuring source builds so restored
binary packages do not make unlike runs appear comparable. A sandbox-denied
binary-cache submission delayed the local clean package run after all packages
had installed, so that wall time is deliberately excluded from the decision.
The probe is deliberately not wired into root CI while it depends on a
repository-owned copy of the vcpkg port; upstream feature support is the durable
CI boundary.

## Correctness results

- All 128 tests passed with CGAL's `BOOST_BACKEND`, including the
  almost-coplanar orientation, exact co-spherical insertion, action,
  Metropolis-Hastings, persistence, and scientific-reference suites.
- The default and Boost geometry builds emitted byte-identical canonical
  `cdt-reference-raw-v1` fixtures.
- Across four action fixtures, the maximum relative difference from the
  256-bit MPFR oracle was `1.411303947664e-76`, below the `1e-70` gate.
- The representative complete acceptance probability and the positive
  `exp(-1'000'000)` fixture are compared against the MPFR oracle with true
  relative error, including values below one. Their measured relative errors
  were zero and `9.313999580314e-73`, respectively, below the `1e-70` gate.
- Both backends preserved the deliberately sub-double action delta, including
  its MPFR-oracle sign and magnitude with zero relative error at the reported
  precision, and a positive `exp(-1'000'000)` that converts to zero when
  represented as `long double`.
- The standalone vcpkg probe installed CGAL and Boost.Multiprecision without
  `gmp` or `mpfr`, passed the adversarial predicate checks, and linked only
  `libc++` and `libSystem` on macOS.

Fresh generated topology counts are not an equality gate. The benchmark uses
nested co-spherical point sets, and valid topology varied even between repeated
runs of one backend. Invariants, point preservation, exact predicate fixtures,
and canonical deterministic outputs are the correctness evidence.

## Performance results

These are matched local diagnostics, not portable performance promises. The
geometry, build, binary, and resident-set rows compare the production MPFR
action layer under two CGAL predicate backends. They do not measure the final
all-Boost production binary.

| Measurement | Default CGAL + MPFR action | Boost-predicate + MPFR-action hybrid | Difference |
| --- | ---: | ---: | ---: |
| Clean project-owned target build | 92.76 s | 101.09 s | +9.0% wall time, one observation |
| No-op incremental build | 0.04 s | 0.04 s | None observed |
| Production `cdt` binary (hybrid geometry only) | 1,572,784 bytes | 1,596,032 bytes | +1.48% |
| CGAL bulk insertion median | 2.396 ms | 2.625 ms | +9.5% |
| Foliation repair median | 7.476 ms | 8.129 ms | +8.7% |
| Cache rebuild median | 2.537 ms | 2.426 ms | -4.4% |
| Point lookup median | 0.646 ms | 0.576 ms | -10.9% |
| Vertex removal median | 0.856 ms | 0.845 ms | -1.3% |
| Fifty-move workload median | 323.363 ms | 316.860 ms | -2.0% |
| CGAL benchmark peak resident set | 9.31 MB | 9.50 MB | +2.1% |

The per-operation arithmetic benchmark reported:

| Operation | MPFR median | Boost median | Ratio |
| --- | ---: | ---: | ---: |
| Generalized action | 21.309 us | 127.531 us | 5.98x |
| Full acceptance probability | 44.309 us | 260.618 us | 5.88x |

The seeded end-to-end fixture attempted 3,428 moves in 8.5 seconds. Candidate
construction succeeded 1,003 times, which is when the acceptance calculation
runs. Applying the measured `216.309 us` per-success difference gives a
`0.217 s`, or approximately 2.6%, modeled increase. This is intentionally
identified as a model; the migration must rerun the complete benchmark with
the production Boost implementation before its review is complete.

## Migration and rollback gates

The production migration is acceptable only if it:

1. uses an upstream vcpkg feature to request Boost.Multiprecision directly and
   CGAL without GMP/MPFR, without a repository-owned production overlay;
2. removes all production `CGAL::Gmpfr`, MPFR C API, and `CGAL::Gmpzf` use;
3. retains the MPFR-oracle fixtures during review or replaces them with
   versioned expected values generated by the existing harness;
4. measures clean-cache dependency and project builds, then passes the full
   sequential, parallel, sanitizer, and supported-platform CI matrix;
5. reruns the canonical reference comparison and representative Metropolis
   benchmark; and
6. documents the new fixed precision and rounding contract in the public API.

Rollback is a single backend-and-manifest change if any supported platform
fails a correctness gate or if the measured full simulation regression is
materially larger than the approximately 2.6% model. A kernel downgrade,
floating-only predicates, or early conversion of acceptance probabilities to
built-in floating point is not an acceptable rollback.
