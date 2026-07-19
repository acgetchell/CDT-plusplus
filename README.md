# CDT-plusplus
**Quantize spacetime on your laptop.**

[![CI](https://github.com/acgetchell/CDT-plusplus/actions/workflows/ci.yml/badge.svg)](https://github.com/acgetchell/CDT-plusplus/actions/workflows/ci.yml)
[![codecov](https://codecov.io/gh/acgetchell/CDT-plusplus/branch/main/graph/badge.svg)](https://codecov.io/gh/acgetchell/CDT-plusplus)
[![cpp-linter](https://github.com/acgetchell/CDT-plusplus/actions/workflows/cpp-linter.yml/badge.svg)](https://github.com/acgetchell/CDT-plusplus/actions/workflows/cpp-linter.yml)

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
[PVS-Studio] provides commercial-grade static analysis.
[CometML] provides machine learning for model building.

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
- [x] Multithreading via [TBB]
- [x] Automated code analysis with [LGTM]
- [x] Build/debug with [Visual Studio 2019]
- [x] Use [{fmt}] library (instead of `iostream`)
- [x] Static code analysis with [PVS-Studio]
- [x] 3D Metropolis algorithm
- [x] Multithreaded logging with [spdlog]
- [ ] Restore optional visualization with [Qt] ([#98](https://github.com/acgetchell/CDT-plusplus/issues/98))
- [ ] Output via [HDF5]
- [ ] 4D Simplex
- [ ] 4D Spherical triangulation
- [ ] 3+1 foliation
- [ ] S4 Bulk action
- [ ] 4D Ergodic moves
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
smoke suite. The first dependency build can take several minutes; subsequent runs use vcpkg's binary cache.

If Just is not installed globally, the optional [pkgx](https://pkgx.sh/) entry point supplies it and the complete
Unix developer-tool environment ephemerally before invoking the same recipe and build contract:

```bash
./scripts/pkgx-build.sh
```

pkgx does not install CGAL or any other project library; those remain owned by the pinned vcpkg manifest. The
underlying `./scripts/build.sh` and `scripts\build.bat` entry points remain available for troubleshooting and native
Windows development.

### Current reference-suite status

With the pinned baseline, the reference configuration and build succeed on macOS with AppleClang. `build.sh` runs
ten supported smoke tests: eight lightweight CLI integration tests and two focused doctest suites. The full
`reference` test preset additionally runs the `cdt-opt` simulation, which can nondeterministically stall during a
Metropolis pass, and the unit-test executable, which remains red in eight test cases tracing to the existing 4,4
move/bistellar-flip path: the direct move application, four flip fixtures, the ergodic-move fixture, and two queued
move scenarios. These tests remain present and can be run explicitly. Issue
[#91](https://github.com/acgetchell/CDT-plusplus/issues/91) owns the 4,4 mutation repair, while
[#92](https://github.com/acgetchell/CDT-plusplus/issues/92) owns the Metropolis transition work.

## Setup

This project uses [CMake]+[Ninja] to build C++23 sources and [vcpkg] manifest mode to manage C++ libraries. macOS with
AppleClang is the primary v1.0.0 restoration target; the remaining compiler and platform matrix will be recorded as
it is verified.

### Prerequisites

The smallest pkgx-assisted host setup is:

- Xcode Command Line Tools on macOS, or a C++23 compiler and base build environment on Linux
- pkgx
- Just when invoking the recipes directly; `scripts/pkgx-build.sh` supplies Just when it is not installed globally

The pkgx launcher supplies Git, Bash, CMake, Ninja, Python, M4, Autoconf, Autoconf Archive, Automake, GNU
Libtool, Texinfo, and pkg-config. If pkgx is not installed, provide these tools conventionally through a package
manager such as [Homebrew] or apt:

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
just fix                   # Format changed C++ lines and the Justfile
just build                 # Bootstrap, configure, build, and smoke-test
just run --help            # Build as needed and run cdt with forwarded arguments
just ci                    # Comprehensive pre-commit/pre-push validation
just update-actions        # Update and repin Actions with pinact, then validate
```

`check` covers changed-line C++ formatting, YAML, GitHub Actions syntax and security, whitespace, and CMake preset
parsing. `ci` adds the pinact policy check and the supported build/test contract. Install the developer tools with
Homebrew, use equivalent system packages, or let pkgx supply them ephemerally; pkgx remains optional. For example:

```bash
pkgx +just.systems +git-scm.org +cmake.org +ninja-build.org +python.org +llvm.org \
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
are isolated under `out/build/reference`. On Windows, `scripts\build.bat` provides the corresponding developer-mode
entry point and `scripts\fast-build.bat` retains the user-mode build path.

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

The supported build produces `cdt`, `cdt-opt`, and `initialize` in `out/build/reference/src`. Run the primary `cdt`
executable through Just and pass its arguments after the recipe name:

```bash
just run --help
```

For troubleshooting, the equivalent direct command is `./out/build/reference/src/cdt --help`.

- `cdt-opt` is a simplified version with hard-coded inputs, mainly useful for debugging and scripting
- `cdt-viewer` is currently disabled and will be restored as an opt-in v1.0.0 target by
  [#98](https://github.com/acgetchell/CDT-plusplus/issues/98)
- `initialize` is used by [CometML] to run [parameter optimization](#optimizing-parameters)

## Usage

CDT-plusplus uses [program_options] to parse options from the help message, and so
understands long or short argument formats, provided the short argument given
is an unambiguous match to a longer one. The help message should be instructive:

~~~text
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
  -a [ --alpha ] arg            Negative squared geodesic length of 1-d
                                timelike edges
  -k [ --k ] arg                K = 1/(8*pi*G_newton)
  -l [ --lambda ] arg           K * Cosmological constant
  -p [ --passes ] arg (=100)    Number of passes
  -c [ --checkpoint ] arg (=10) Checkpoint every n passes
~~~

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
locally using the configuration file in `docs\Doxyfile` by simply typing at the top
level directory ([Doxygen] will recursively search):

~~~bash
doxygen ./docs/Doxyfile
~~~

This will generate a `docs/html/` directory containing
documentation generated from CDT++ source files. `USE_MATHJAX` has been enabled
in [Doxyfile] so that the LaTeX formulae can be rendered in the HTML
documentation using [MathJax]. `HAVE_DOT` is set to **YES** which allows
various graphs to be autogenerated by [Doxygen] using [GraphViz].
If you do not have GraphViz installed, set this option to **NO**
(along with `UML_LOOK`).

## Testing

Run `just build`; it delegates to `./scripts/build.sh`, builds the test target, and executes the ten supported smoke
tests. These include focused doctest runs for the Boost.Compat `function_ref` migration and the causal-foliation
construction code used as a regression oracle. Run `just ci` for the complete local validation gate.

The doctest executable can also be run directly:

````bash
./out/build/reference/tests/CDT_unit_tests
````

To rerun the supported smoke suite without rebuilding:

````bash
ctest --preset reference-smoke
````

To run every currently registered test, including the known 4,4 failures and the nondeterministic `cdt-opt`
simulation:

````bash
ctest --preset reference
````

In addition to the command line output, you can see detailed results in the
`out/build/reference/Testing` directory generated by CTest.

### Static Analysis

This project follows the [CppCore Guidelines][guidelines] as enforced by [ClangTidy], which you can install
and then run using the [clang-tidy.sh] script:

~~~bash
sudo apt-get install clang-tidy
cd scripts
./clang-tidy.sh
~~~

(Or use your favorite linter plugin for your editor/IDE.)

The [cppcheck.sh] script runs a quick static analysis using [cppcheck].

~~~bash
brew install cppcheck
cd scripts
./cppcheck-build.sh
~~~

[PVS-Studio] - static analyzer for C, C++, C#, and Java code.

~~~bash
cd scripts
./pvs-studio.sh
~~~

### Sanitizers

[AddressSanitizer] + [UndefinedBehaviorSanitizer], [LeakSanitizer], [MemorySanitizer],
and [ThreadSanitizer] are run with `scripts/asan.sh`, `scripts/lsan.sh`, `scripts/msan.sh`,
and `scripts/tsan.sh`. Their release-gate consolidation is tracked by
[#95](https://github.com/acgetchell/CDT-plusplus/issues/95).

## Optimizing Parameters

[CometML] is used to record [Experiments] which conduct [Model Optimization]. The script to do
this is `optimize-initialize.py`. In order for this to work, you must install the following
into your Python [virtual environment].

~~~bash
pip install tensorflow
pip install comet-ml
~~~

You can then run experiments and look at results on https://www.comet.ml!

## Visualization

The Qt-based `cdt-viewer` target is currently disabled. Its opt-in dependency feature, deterministic smoke fixture,
supported desktop matrix, and v1.0.0 restoration are tracked by
[#98](https://github.com/acgetchell/CDT-plusplus/issues/98); Qt and Eigen are intentionally absent from the default
headless build until that work is complete.

## Contributing

Please see [CONTRIBUTING.md] and our [CODE_OF_CONDUCT.md].

Your code should pass Continuous Integration:

- `just fix` for safe automatic formatting with the repository's [.clang-format]

- `just check` for fast, non-mutating source, YAML, workflow, and CMake validation

- `just ci` for the supported build and smoke-test contract before pushing

The slower [cppcheck], [Valgrind], and sanitizer workflows remain available through [GitHub Actions] and their
repository scripts when relevant to a change:

- [cppcheck] with [cppcheck.sh]

- [Valgrind] for leak diagnostics

- [AddressSanitizer], [UndefinedBehaviorSanitizer], [LeakSanitizer], [MemorySanitizer], and [ThreadSanitizer]

Optional:

- [ClangTidy] on all changed files

- [PVS-Studio] using [pvs-studio.sh] if you have it installed

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
[gcc]: https://gcc.gnu.org/
[doctest]: https://github.com/doctest/doctest
[guidelines]: https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines
[clang-tidy.sh]: https://github.com/acgetchell/CDT-plusplus/blob/main/scripts/clang-tidy.sh
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
[HDF5]: https://www.hdfgroup.org
[cppcheck]: http://cppcheck.sourceforge.net
[cppcheck.sh]: https://github.com/acgetchell/CDT-plusplus/blob/main/scripts/cppcheck.sh
[functional]: https://blog.knatten.org/2012/11/02/efficient-pure-functional-programming-in-c-using-move-semantics/
[TBB]: https://www.threadingbuildingblocks.org
[Doxyfile]: https://github.com/acgetchell/CDT-plusplus/blob/main/docs/Doxyfile
[Boost]: https://www.boost.org
[ClangTidy]: https://releases.llvm.org/6.0.1/tools/clang/tools/extra/docs/clang-tidy/index.html
[Valgrind]: http://valgrind.org/docs/manual/quick-start.html#quick-start.mcrun
[date]: https://howardhinnant.github.io/date/date.html
[BDD]: https://en.wikipedia.org/wiki/Behavior-driven_development
[TDD]: https://en.wikipedia.org/wiki/Test-driven_development
[CometML]: https://www.comet.ml/
[Experiments]: https://www.comet.ml/acgetchell/cdt-plusplus
[Model Optimization]: https://www.comet.ml/parameter-optimization
[virtual environment]: https://docs.python.org/3/tutorial/venv.html
[vcpkg]: https://github.com/Microsoft/vcpkg
[clang-15]: https://releases.llvm.org/15.0.0/tools/clang/docs/ReleaseNotes.html
[gcc-12]: https://gcc.gnu.org/gcc-12/
[C++]: https://isocpp.org/
[C++20]: https://en.cppreference.com/w/cpp/20
[development]: https://github.com/acgetchell/CDT-plusplus
[Pitchfork Layout]: https://api.csswg.org/bikeshed/?force=1&url=https://raw.githubusercontent.com/vector-of-bool/pitchfork/develop/data/spec.bs#tld.docs
[PCG]: http://www.pcg-random.org/paper.html
[TestU01]: http://simul.iro.umontreal.ca/testu01/tu01.html
[apt]: https://wiki.debian.org/Apt
[ms-gsl]: https://github.com/microsoft/GSL
[yasm-tool:x86-windows]: https://github.com/microsoft/vcpkg/issues/15956#issuecomment-782370823
[MSVC]: https://docs.microsoft.com/en-us/cpp/build/reference/compiling-a-c-cpp-program?view=vs-2019
[m4]: https://www.gnu.org/software/m4/
[1]: https://github.com/microsoft/vcpkg/issues/9082
[2]: https://github.com/microsoft/vcpkg/issues/9087
[3]: https://github.com/microsoft/vcpkg/issues/8627
[CONTRIBUTING.md]: https://github.com/acgetchell/CDT-plusplus/blob/main/.github/CONTRIBUTING.md
[CODE_OF_CONDUCT.md]: https://github.com/acgetchell/CDT-plusplus/blob/main/.github/CODE_OF_CONDUCT.md
[GitHub Actions]: https://github.com/features/actions
[Visual Studio 2019]: https://visualstudio.microsoft.com/vs/
[{fmt}]: https://github.com/fmtlib/fmt
[AddressSanitizer]: https://github.com/google/sanitizers/wiki/AddressSanitizer
[LeakSanitizer]: https://github.com/google/sanitizers/wiki/AddressSanitizerLeakSanitizer
[ThreadSanitizer]: https://github.com/google/sanitizers/wiki/ThreadSanitizerCppManual
[MemorySanitizer]: https://github.com/google/sanitizers/wiki/MemorySanitizer
[UndefinedBehaviorSanitizer]: https://clang.llvm.org/docs/UndefinedBehaviorSanitizer.html
[clang-format]: https://releases.llvm.org/10.0.0/tools/clang/docs/ReleaseNotes.html#clang-format
[.clang-format]: https://github.com/acgetchell/CDT-plusplus/blob/main/.clang-format
[asan.sh]: https://github.com/acgetchell/CDT-plusplus/blob/main/scripts/asan.sh
[lsan.sh]: https://github.com/acgetchell/CDT-plusplus/blob/main/scripts/lsan.sh
[msan.sh]: https://github.com/acgetchell/CDT-plusplus/blob/main/scripts/msan.sh
[tsan.sh]: https://github.com/acgetchell/CDT-plusplus/blob/main/scripts/tsan.sh
[PVS-Studio]: https://pvs-studio.com/en/pvs-studio/?utm_source=github&utm_medium=organic&utm_campaign=open_source
[pvs-studio.sh]: https://github.com/acgetchell/CDT-plusplus/blob/main/scripts/pvs-studio.sh
[spdlog]: https://github.com/gabime/spdlog
[Qt]: https://www.qt.io
