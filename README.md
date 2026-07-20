# CDT-plusplus

**Quantize spacetime on your laptop.**

[![License](https://badgen.net/github/license/acgetchell/CDT-plusplus)](https://github.com/acgetchell/CDT-plusplus/blob/main/LICENSE.md)
[![CI](https://github.com/acgetchell/CDT-plusplus/actions/workflows/ci.yml/badge.svg)](https://github.com/acgetchell/CDT-plusplus/actions/workflows/ci.yml)
[![Documentation](https://github.com/acgetchell/CDT-plusplus/actions/workflows/doxygen.yml/badge.svg)](https://www.adamgetchell.org/CDT-plusplus/)
[![codecov](https://codecov.io/gh/acgetchell/CDT-plusplus/branch/main/graph/badge.svg)](https://codecov.io/gh/acgetchell/CDT-plusplus)

![Small foliated Delaunay triangulation](docs/images/S3-7-27528-I1-R1.png "7 timeslices 27528 simplices")

## Maintenance status

CDT++ is being prepared for one final C++23 release, v1.0.0, after which this repository will be archived. It is
maintained as an independent scientific reference and regression oracle for
[causal-triangulations](https://github.com/acgetchell/causal-triangulations), the supported Rust successor. New C++
work is limited to correctness, reproducibility, cross-implementation validation, the complete supported 2+1D move
set, and the final release contract tracked by [issue #90](https://github.com/acgetchell/CDT-plusplus/issues/90).

## Table of contents

- [CDT-plusplus](#cdt-plusplus)
  - [Maintenance status](#maintenance-status)
  - [Introduction](#introduction)
    - [Regression-oracle scope](#regression-oracle-scope)
  - [Roadmap](#roadmap)
  - [Quickstart](#quickstart)
    - [Current reference-suite status](#current-reference-suite-status)
  - [Setup](#setup)
    - [Prerequisites](#prerequisites)
    - [Developer workflow](#developer-workflow)
    - [vcpkg maintenance](#vcpkg-maintenance)
  - [Build](#build)
    - [Project Layout](#project-layout)
    - [Run](#run)
  - [Usage](#usage)
  - [Documentation](#documentation)
  - [Testing](#testing)
    - [Static Analysis](#static-analysis)
    - [Sanitizers](#sanitizers)
  - [Optimizing Parameters](#optimizing-parameters)
  - [Visualization](#visualization)
  - [Contributing](#contributing)
  - [Issues](#issues)

## Introduction

For an introduction to [Causal Dynamical Triangulations](https://github.com/acgetchell/CDT-plusplus/wiki),
including the foundations and recent results, please see the [wiki](https://github.com/acgetchell/CDT-plusplus/wiki).

[Causal Dynamical Triangulations][CDT] in [C++] uses the
[Computational Geometry Algorithms Library][CGAL], [Boost], and [TBB].
Arbitrary-precision numbers and functions are by [MPFR] and [GMP].
[Melissa E. O'Neill's Permuted Congruential Generators][PCG] library provides high-quality RNGs that pass L'Ecuyer's
[TestU01] statistical tests.
[doctest] provides [BDD]/[TDD].
[vcpkg] provides library management and building.
[Doxygen] provides automated document generation.
[{fmt}] provides a safe and fast alternative to `iostream`.
[spdlog] provides fast, multithreaded logging.
[CometML] provides experiment tracking for the optional Python workflows.

### Regression-oracle scope

The principal reason to preserve this implementation is its causality-filtering Delaunay construction path in
[`include/Foliated_triangulation.hpp`](include/Foliated_triangulation.hpp). `check_timevalues` classifies cells from
stored vertex time labels, `find_bad_vertex` selects a vertex responsible for an acausal local configuration, and
`fix_timevalues` removes offending vertices through CGAL so the cavity is retriangulated until the foliation contract
is satisfied.

The deterministic doctest scenario **"Detecting and fixing problems with vertices and cells"** in
[`tests/Foliated_triangulation_test.cpp`](tests/Foliated_triangulation_test.cpp) exercises this path with fixed points
and time labels. Its inputs, detected bad vertex, final initialization state, cell counts, and causal classification
are the first comparison fixture for `causal-triangulations`; exact Monte Carlo trajectories are not required to
match.

After building, run that fixture directly with:

```bash
./out/build/reference/tests/CDT_unit_tests \
  --test-case='*Detecting and fixing problems with vertices and cells*'
```

## Roadmap

- [x] Cross-platform support on Linux, macOS (x64 & arm64), and Windows
- [x] Cross-compiler support on gcc, clang, and MSVC
- [x] Develop with [literate programming] using [Doxygen]
- [x] [Efficient Pure Functional Programming in C++ Using Move Semantics][functional]
- [x] Test using [CTest]
- [x] Develop using Behavior-driven development ([BDD]) with [doctest]
- [x] Continuous integration by [GitHub Actions] on the leading edge
- [x] 3D Simplex
- [x] 3D Spherical triangulation
- [x] 2+1 foliation
- [x] S3 Bulk action
- [x] 3D Ergodic moves
- [x] High-quality Random Number Generation with M.E. O'Neill's [PCG] library
- [ ] Restore optional parallel triangulation with [TBB] ([#74](https://github.com/acgetchell/CDT-plusplus/issues/74))
- [x] Automated code analysis with [CodeQL]
- [x] Build/debug with [Visual Studio 2022]
- [x] Use [{fmt}] library (instead of `iostream`)
- [x] 3D Metropolis algorithm
- [x] Multithreaded logging with [spdlog]
- [ ] Restore optional visualization with [Qt] ([#98](https://github.com/acgetchell/CDT-plusplus/issues/98))
- [ ] Initialize two masses
- [ ] The shortest path algorithm
- [ ] Einstein tensor
- [ ] Complete test coverage
- [ ] Complete documentation
- [ ] Quantize Spacetime!

## Quickstart

From a fresh checkout, the primary supported headless build, dependency bootstrap, and test path is:

```bash
just build
```

The `build` recipe uses the pkgx launcher on Unix when pkgx is available, then delegates to `scripts/build.sh`. Its
first run creates an ignored `.cache/vcpkg` checkout at the exact `builtin-baseline` recorded in `vcpkg.json`,
bootstraps vcpkg, installs the manifest dependencies, builds in `out/build/reference`, and runs the supported CTest
smoke suite. The first dependency build can take several minutes; subsequent runs reuse both vcpkg dependencies and
CMake/Ninja outputs, so an unchanged build is a no-op apart from configuration and tests.

The optional [pkgx](https://pkgx.sh/) entry point supplies the complete Unix developer-tool environment ephemerally
and invokes the same build contract directly, without requiring Just:

```bash
./scripts/pkgx-build.sh
```

pkgx does not install CGAL or any other project library; those remain owned by the pinned vcpkg manifest. The
underlying `./scripts/build.sh` and `scripts\build.bat` entry points remain available for troubleshooting and native
Windows development.

### Current reference-suite status

With the pinned baseline, the reference configuration and build succeed on macOS with AppleClang. `build.sh` runs
eleven supported smoke tests on Unix: nine lightweight CLI integration tests and two focused doctest suites. The known
failing `initialize` scenario is excluded from Windows smoke runs, while the other initialization cases remain enabled.
The complete registered suite additionally runs the full unit-test executable and a focused utilities registration;
the focused registrations remain available for quick iteration and are labeled as full-suite duplicates for sanitizer
runs.

## Setup

This project uses [CMake]+[Ninja] to build C++23 sources and [vcpkg] manifest mode to manage C++ libraries. macOS with
AppleClang is the primary v1.0.0 restoration target; the remaining compiler and platform matrix will be recorded as
it is verified.

### Prerequisites

The smallest pkgx-assisted host setup is:

- Xcode Command Line Tools on macOS, or a C++23 compiler and base build environment on Linux
- pkgx
- Just when invoking the recipes directly; `scripts/pkgx-build.sh` does not require it
- Python 3.12 and uv when checking or running the Python support scripts

The pkgx launcher supplies Git, Bash, CMake, Ninja, Python, M4, Autoconf, Autoconf Archive, Automake, GNU
Libtool, Texinfo, and pkg-config. If pkgx is not installed, provide these tools conventionally through a package
manager such as [Homebrew] or apt:

- Git
- Bash
- build-essential (Linux only)
- m4
- automake
- autoconf
- autoconf-archive
- libtool (macOS) or libtool-bin (Linux)
- pkg-config
- texinfo
- ninja (macOS) or ninja-build (Linux)

The build does not require a pre-existing personal vcpkg checkout, a fork, a submodule, Docker, or a hosted
development environment.

### Developer workflow

The repository-root [Justfile](Justfile) provides the same small command vocabulary used by the related projects:

```bash
just check                 # Fast, non-mutating local checks
just fix                   # Format C++/Python source and the Justfile
just clang-tidy            # Analyze C++ with LLVM 22
just sanitize asan         # Build and exercise one Linux sanitizer preset
just build                 # Bootstrap, configure, build, and smoke-test
just run --help            # Build as needed and run cdt with forwarded arguments
just ci                    # Comprehensive pre-commit/pre-push validation
just update-actions        # Update and repin Actions with pinact, then validate
just python-sync           # Install the locked Ruff and ty development environment
just python-check          # Check Python formatting, lint, and types
just python-fix            # Apply safe Ruff fixes and formatting
```

`check` covers repository-wide C++ formatting, Python formatting/lint/type checks, YAML, GitHub Actions syntax and
security, whitespace, and CMake preset parsing. `ci` adds the pinact policy check and the supported build/test
contract. Install the developer tools with
Homebrew, use equivalent system packages, or let pkgx supply them ephemerally; pkgx remains optional. For example:

```bash
pkgx +just.systems +git-scm.org +cmake.org +ninja-build.org +python.org +llvm.org@22 \
  +yamllint +actionlint +zizmor just check
```

[pinact](https://github.com/suzuki-shunsuke/pinact) uses [`.pinact.yaml`](.pinact.yaml) to retain immutable action
SHAs, readable release comments, and a seven-day release cooldown. `just update-actions` uses an installed pinact or
a pkgx-provided Go fallback, then requires `yamllint`, `actionlint`, and `zizmor` to pass.

### vcpkg maintenance

`vcpkg.json` is the dependency source of truth. Its `builtin-baseline` pins the official
[`microsoft/vcpkg`](https://github.com/microsoft/vcpkg) registry commit used locally and in CI. The repository-local
`.cache/vcpkg` checkout is disposable tool/cache infrastructure and must not be edited or committed.

To update dependencies intentionally, bootstrap the current checkout, run the vcpkg baseline updater, review the
manifest diff, and then rerun the complete build:

```bash
./scripts/bootstrap-vcpkg.sh
export VCPKG_ROOT="$PWD/.cache/vcpkg"
"$VCPKG_ROOT/vcpkg" x-update-baseline
./scripts/build.sh
```

CI uses `lukka/run-vcpkg`, which derives the vcpkg checkout commit from the same manifest baseline and supplies a
binary cache. No separately maintained repository variable is required.

## Build

Run `just build` from the repository root. It delegates to `./scripts/build.sh`, which can itself be run from any
working directory for troubleshooting. If `VCPKG_ROOT` already names the clean official checkout at the manifest
baseline, the script respects it; otherwise it uses the pinned disposable checkout described above. The script
invokes the `reference` configure and build presets followed by the `reference-smoke` test preset; products and tests
are isolated under `out/build/reference`. Windows uses the same presets through `scripts\build.bat`, while
`scripts\fast-build.bat` configures the same reference tree and builds only the primary `cdt` target. All entry points
preserve a compatible CMake cache and refresh it only when the selected vcpkg toolchain path changes.

### Project Layout

The project is similar to [PitchFork Layout], as follows:

- .github - GitHub specific settings
- out/build/reference - Ephemeral supported headless build directory
- cmake - Cmake configurations
- docs - Documentation
- external - Includes submodules of external projects (none so far, all using [vcpkg])
- include - Header files
- scripts - Build, test, and run scripts
- src - Source files
- tests - Unit tests

### Run

The supported build produces `cdt` and `initialize` in `out/build/reference/src`. Run the primary `cdt`
executable through Just and pass its arguments after the recipe name:

```bash
just run --help
```

For troubleshooting, the equivalent direct command is `./out/build/reference/src/cdt --help`.

Use `--no-output` for batch, debugging, or scripted runs that should print results without writing checkpoint or final
triangulation files:

```bash
just run -s -n256 -t4 -a0.6 -k1.1 -l0.1 -p10 -c10 --no-output
```

- `cdt-viewer` is currently disabled and will be restored as an opt-in v1.0.0 target by
  [#98](https://github.com/acgetchell/CDT-plusplus/issues/98)
- `initialize` is used by [CometML] to run [parameter optimization](#optimizing-parameters)

## Usage

CDT-plusplus uses [program_options] to parse options from the help message, and so
understands long or short argument formats, provided the short argument given
is an unambiguous match to a longer one. The help message should be instructive:

```text
./out/build/reference/src/cdt --help
Causal Dynamical Triangulations in C++ using CGAL.

Copyright (c) 2013-2026 Adam Getchell

A program that generates d-dimensional triangulated spacetimes
with a defined causal structure and evolves them according
to the Metropolis algorithm. Specify the number of passes to control
how much evolution is desired. Each pass attempts a number of ergodic
moves equal to the number of simplices in the simulation.

Usage:./cdt (--spherical | --toroidal) -n SIMPLICES -t TIMESLICES
            [-d DIM]
            [--init INITIAL RADIUS]
            [--foliate FOLIATION SPACING]
            [--no-output]
            -k K
            --alpha ALPHA
            --lambda LAMBDA
            [-p PASSES]
            [-c CHECKPOINT]

Optional arguments are in square brackets.

Examples:
./cdt --spherical -n 32000 -t 11 --alpha 0.6 -k 1.1 --lambda 0.1 --passes 1000
./cdt -s -n32000 -t11 -a.6 -k1.1 -l.1 -p1000

Options:
  -h [ --help ]                 Show this message
  -v [ --version ]              Show program version
  -s [ --spherical ]            Spherical topology
  -e [ --toroidal ]             Toroidal topology
  -n [ --simplices ] arg        Approximate number of simplices
  -t [ --timeslices ] arg       Number of timeslices
  -d [ --dimensions ] arg (=3)  Dimensionality
  -i [ --init ] arg (=1)        Initial radius
  -f [ --foliate ] arg (=1)     Foliation spacing
  --no-output                   Do not write checkpoint or final triangulation
                                files
  -a [ --alpha ] arg            Negative squared geodesic length of 1-d
                                timelike edges
  -k [ --k ] arg                K = 1/(8*pi*G_newton)
  -l [ --lambda ] arg           K * Cosmological constant
  -p [ --passes ] arg (=100)    Number of passes
  -c [ --checkpoint ] arg (=10) Checkpoint every n passes
```

The dimensionality of the spacetime is such that each slice of spacetime is
`d-1`-dimensional, so setting `d=3` generates two spacelike dimensions and one
timelike dimension, with a defined global time foliation. A
`d`-dimensional simplex will have some `d-1` sub-simplices that are purely
spacelike (all on the same timeslice) as well as some that are timelike
(span two timeslices). In [CDT] we actually care more about the timelike
links (in 2+1 spacetime), and the timelike faces (in 3+1 spacetime).

## Documentation

Online documentation is at <https://adamgetchell.org/CDT-plusplus/>.

If you have [Doxygen] installed you can generate the same information
locally using the configuration file in `docs/Doxyfile` by simply typing at the top
level directory ([Doxygen] will recursively search):

```bash
doxygen ./docs/Doxyfile
```

This will generate a `docs/html/` directory containing
documentation generated from CDT++ source files. `USE_MATHJAX` has been enabled
in [Doxyfile] so that the LaTeX formulae can be rendered in the HTML
documentation using [MathJax]. `HAVE_DOT` is set to **YES** which allows
various graphs to be autogenerated by [Doxygen] using [GraphViz].
If you do not have GraphViz installed, set this option to **NO**
(along with `UML_LOOK`).

## Testing

Run `just build`; it delegates to `./scripts/build.sh`, builds the test target, and executes the eleven supported smoke
tests. These include focused doctest runs for the Boost.Compat `function_ref` migration and the causal-foliation
construction code used as a regression oracle. Run `just ci` for the complete local validation gate.

The doctest executable can also be run directly:

```bash
./out/build/reference/tests/CDT_unit_tests
```

To rerun the supported smoke suite without rebuilding:

```bash
ctest --preset reference-smoke
```

To run every currently registered test, including the full unit-test executable and focused duplicate registrations,
bypass the smoke filter explicitly:

```bash
ctest --test-dir out/build/reference --output-on-failure
```

In addition to the command line output, you can see detailed results in the
`out/build/reference/Testing` directory generated by CTest.

### Static Analysis

Python 3.12 is selected by [`.python-version`](.python-version), uv locks the environment in `uv.lock`, Ruff owns
Python formatting and linting, and ty owns static type checking. Run `just python-sync` once and then use
`just python-check` or `just python-fix`; both commands are also part of the repository-wide validation recipes.

This project follows the [CppCore Guidelines][guidelines] as enforced by [ClangTidy]. The repository pins LLVM 22;
run Clang-Tidy through its Just recipe:

```bash
just clang-tidy
```

(Or use your favorite linter plugin for your editor/IDE.)

### Sanitizers

[AddressSanitizer] + [UndefinedBehaviorSanitizer], [LeakSanitizer], [MemorySanitizer], and [ThreadSanitizer] share the
repository-owned Linux driver and CMake presets. Run one locally with `just sanitize asan`, `just sanitize lsan`,
`just sanitize msan`, or `just sanitize tsan`; the GitHub Actions workflows invoke the same commands. MemorySanitizer
remains experimental because third-party dependencies are not instrumented.

## Optimizing Parameters

[CometML] is used to record [Experiments] conducted by `src/optimize-initialize.py`; `src/test.py` is the existing
TensorFlow MNIST experiment. These optional, heavyweight dependencies are kept out of the normal lint environment.
Synchronize them from the same uv lockfile when working on the experiment scripts:

```bash
just python-sync-experiments
uv run --locked --group experiments python src/optimize-initialize.py
```

Set `COMET_API_KEY` before starting an online experiment. The experiment results are then available in Comet.
Migration of these legacy scripts to Python 3.14, PyTorch, and the current Comet API is tracked by
[#104](https://github.com/acgetchell/CDT-plusplus/issues/104).

## Visualization

The Qt-based `cdt-viewer` target is currently disabled. Its opt-in dependency feature, deterministic smoke fixture,
supported desktop matrix, and v1.0.0 restoration are tracked by
[#98](https://github.com/acgetchell/CDT-plusplus/issues/98); Qt and Eigen are intentionally absent from the default
headless build until that work is complete.

## Contributing

Please see [CONTRIBUTING.md] and our [CODE_OF_CONDUCT.md].

Your code should pass Continuous Integration:

- `just fix` for safe automatic formatting with the repository's [.clang-format]

- `just clang-tidy` to analyze C++ with the pinned LLVM 22 toolchain

- `just check` for fast, non-mutating source, YAML, workflow, and CMake validation

- `just ci` for the supported build and smoke-test contract before pushing

The slower sanitizer workflows remain available through [GitHub Actions] and repository commands when relevant to a
change:

- [AddressSanitizer], [UndefinedBehaviorSanitizer], [LeakSanitizer], [MemorySanitizer], and [ThreadSanitizer]

Valgrind is intentionally unsupported: on x86/AMD64 its floating-point emulation does not honor the directed
rounding required by CGAL's interval predicates. Disabling CGAL's rounding check would make geometric results
untrustworthy, so memory diagnostics use the supported sanitizer workflows instead.

Optional:

- [ClangTidy] on all changed files

## Issues

- [vcpkg]'s version of [date] has an unfixed bug [#23637] which produces `use-of-uninitialized-value` in [MemorySanitizer].
- [vcpkg] has issues with `fontconfig:arm64-osx` [#40623].
- [vcpkg] fails to build [gmp] on Linux (which breaks CI) [#45336]

[#45336]: https://github.com/microsoft/vcpkg/issues/45336
[#40623]: https://github.com/microsoft/vcpkg/issues/40623
[#23637]: https://github.com/microsoft/vcpkg/issues/23637
[CDT]: https://arxiv.org/abs/hep-th/0105267
[CGAL]: https://www.cgal.org
[CMake]: https://www.cmake.org
[doctest]: https://github.com/doctest/doctest
[guidelines]: https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines
[CTest]: https://gitlab.kitware.com/cmake/community/wikis/doc/ctest/Testing-With-CTest
[literate programming]: http://www.literateprogramming.com
[Doxygen]: http://www.doxygen.org
[Homebrew]: https://brew.sh
[Ninja]: https://ninja-build.org
[program_options]: https://www.boost.org/doc/libs/1_85_0/doc/html/program_options/tutorial.html
[Mathjax]: https://www.mathjax.org
[GraphViz]: https://www.graphviz.org
[MPFR]: https://www.mpfr.org
[GMP]: https://gmplib.org
[functional]: https://blog.knatten.org/2012/11/02/efficient-pure-functional-programming-in-c-using-move-semantics/
[TBB]: https://www.threadingbuildingblocks.org
[Doxyfile]: https://github.com/acgetchell/CDT-plusplus/blob/main/docs/Doxyfile
[Boost]: https://www.boost.org
[ClangTidy]: https://clang.llvm.org/extra/clang-tidy/
[date]: https://howardhinnant.github.io/date/date.html
[BDD]: https://en.wikipedia.org/wiki/Behavior-driven_development
[TDD]: https://en.wikipedia.org/wiki/Test-driven_development
[CometML]: https://www.comet.ml/
[Experiments]: https://www.comet.ml/acgetchell/cdt-plusplus
[vcpkg]: https://github.com/Microsoft/vcpkg
[C++]: https://isocpp.org/
[Pitchfork Layout]: https://api.csswg.org/bikeshed/?force=1&url=https://raw.githubusercontent.com/vector-of-bool/pitchfork/develop/data/spec.bs#tld.docs
[PCG]: http://www.pcg-random.org/paper.html
[TestU01]: http://simul.iro.umontreal.ca/testu01/tu01.html
[CONTRIBUTING.md]: https://github.com/acgetchell/CDT-plusplus/blob/main/.github/CONTRIBUTING.md
[CODE_OF_CONDUCT.md]: https://github.com/acgetchell/CDT-plusplus/blob/main/.github/CODE_OF_CONDUCT.md
[GitHub Actions]: https://github.com/features/actions
[CodeQL]: https://codeql.github.com/
[Visual Studio 2022]: https://visualstudio.microsoft.com/vs/
[{fmt}]: https://github.com/fmtlib/fmt
[AddressSanitizer]: https://github.com/google/sanitizers/wiki/AddressSanitizer
[LeakSanitizer]: https://github.com/google/sanitizers/wiki/AddressSanitizerLeakSanitizer
[ThreadSanitizer]: https://github.com/google/sanitizers/wiki/ThreadSanitizerCppManual
[MemorySanitizer]: https://github.com/google/sanitizers/wiki/MemorySanitizer
[UndefinedBehaviorSanitizer]: https://clang.llvm.org/docs/UndefinedBehaviorSanitizer.html
[.clang-format]: https://github.com/acgetchell/CDT-plusplus/blob/main/.clang-format
[spdlog]: https://github.com/gabime/spdlog
[Qt]: https://www.qt.io
