# C++ API boundary

CDT++ 1.0 uses `cdt` as the sole root namespace for project-owned C++
declarations. Component namespaces describe a domain, while any nested
`detail` namespace is an implementation contract and is not supported for
downstream callers. The namespace is a maintenance signal rather than an
access-control mechanism.

The public headers do not contain namespace-wide `using` directives. The only
project macro intended for callers' preprocessing environment is
`CDT_PRETTY_FUNCTION`, which normalizes compiler function-name spellings and is
project-prefixed because macros cannot be namespaced. Include guards and the
platform-required `NOMINMAX` definition are preprocessing exceptions rather
than C++ API declarations. The formatter specializations remain in `fmt`, as
required by that library's customization contract.

## Header classification

| Header | Supported surface | Internal, customization, or experimental surface |
| --- | --- | --- |
| `Apply_move.hpp` | `cdt::apply_move` | None |
| `Ergodic_moves_3.hpp` | `cdt::ergodic_moves` aliases plus `null_move`, `do_*_move`, and `propose_*_move` | Every declaration in `cdt::ergodic_moves::detail`, including applicable-move preparation/execution, raw CGAL flips, cavity recognition, collection helpers, `bistellar_flip`, and `check_move` |
| `Foliated_triangulation.hpp` | Root CGAL interop aliases and `Cell_type`; construction, inspection, repair, and `FoliatedTriangulation` declarations in `cdt::foliated_triangulations` | Generic constraints and repair limits in `cdt::detail`; the component declarations are the supported advanced triangulation API |
| `Formatters.hpp` | `fmt::formatter<CGAL::Point_3<...>>` | Supported external-library customization point |
| `Geometry.hpp` | `cdt::Geometry` and `cdt::Geometry_3` | None |
| `Manifold.hpp` | `cdt::manifolds::make_causal_vertices`, `Manifold`, and `Manifold_3` | None |
| `Metropolis.hpp` | `cdt::MoveStrategy` Metropolis specialization and `cdt::Metropolis_3` | Private members and nested types are implementation details |
| `Move_always.hpp` | `cdt::MoveStrategy` move-always specialization and `cdt::MoveAlways_3` | Private members are implementation details |
| `Move_command.hpp` | `cdt::MoveCommand` | Private queue and counter types are implementation details |
| `Move_outcome.hpp` | `cdt::ergodic_moves::MoveFailure`, `MoveError`, `MoveResult`, `MoveOutcome`, and `outcome_from` | `format_as` is the supported `fmt`/`spdlog` customization hook for `MoveError` |
| `Move_strategy.hpp` | `cdt::Strategies` and `cdt::MoveStrategy` | None |
| `Move_tracker.hpp` | Move enumeration, counters, conversion, and sampling in `cdt::move_tracker` | None |
| `Mpfr_value.hpp` | Scoped MPFR value and operations in `cdt::mpfr_values` | None |
| `Periodic_3_complex.hpp` | None | Legacy prototype in `cdt::experimental::periodic_complex`; unsupported and retained only for archival source access |
| `Periodic_3_triangulations.hpp` | None | Legacy prototype in `cdt::experimental::periodic_triangulations`; unsupported and retained only for archival source access |
| `Random.hpp` | Random engine, seed/stream types, and named streams in `cdt` | None |
| `Runtime_config.hpp` | Validated configuration values and factories in `cdt::runtime_config` | Parsing helpers in `cdt::runtime_config::detail` |
| `S3Action.hpp` | Physical parameters and action functions in `cdt::s3_action` | None |
| `Settings.hpp` | Scalar types and named constants in `cdt` | Project-prefixed preprocessing exception described above |
| `Torus_d.hpp` | None | Legacy prototype in `cdt::experimental::torus`; unsupported and retained only for archival source access |
| `Triangulation_traits.hpp` | None | CGAL assembly traits in `cdt::detail`; supported aliases are published by `Foliated_triangulation.hpp` |
| `Utilities.hpp` | `cdt::topology_type` and persistence, reproducibility, random-distribution, logging, and conversion operations in `cdt::utilities` | Serialization and parsing machinery in `cdt::utilities::detail` |

Tests that name `detail` are deliberate white-box tests for mutation
failure-atomicity and malformed-handle rejection. Test access does not promote
those declarations into the supported API.

The supported-header compile contract intentionally excludes `experimental`.
The CGAL 6.2 audit confirmed that the periodic and d-dimensional prototypes are
not production dependencies and do not meet the supported compile contract.
Their presence under `include/` preserves archival source access, not a promise
that they are usable downstream. See the
[CGAL integration contract](cgal-integration.md).

## Compatibility policy

This namespace normalization is the source-compatibility boundary for the
final 1.0 release. Repository production code, tests, and documented examples
were the only callers found for the pre-1.0 global and unrelated top-level
namespaces. Compatibility aliases would reintroduce precisely those global
names and would leave an indefinite archival burden, so no deprecated aliases
are provided. Downstream source must qualify project names through `cdt`.

The project publishes no stable binary ABI: its library surface is templates
and inline headers, and the CMake build produces executables rather than a
versioned binary library. `BUILD_SHARED_LIBS` alone does not establish an ABI
promise. The supported contract is C++23 source compatibility within the 1.0
surface documented here; `detail` and `experimental` declarations may change
without compatibility shims.
