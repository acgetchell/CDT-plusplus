# Contributing to CDT++

CDT++ v1.0.0 is the final planned C++23 feature release. The repository is preserved as a
historical scientific reference. Active development, new scientific features, general support, and
new reports belong in
[causal-triangulations](https://github.com/acgetchell/causal-triangulations), the supported Rust
successor.

Issue [#97](https://github.com/acgetchell/CDT-plusplus/issues/97) owns the final tag, GitHub release,
and Zenodo record verification. Issue
[#155](https://github.com/acgetchell/CDT-plusplus/issues/155) owns the separate stabilization gate,
tracker closure, and eventual repository archival. Archival is not immediate: the repository remains
maintenance-only until the owner determines that no release blockers remain.

## Maintenance scope

Before archival, pull requests are limited to release-blocking correctness, reproducibility,
security, documentation, and metadata corrections. A correction must:

- be tied to an existing release issue;
- preserve the bounded 2+1-dimensional spherical scientific contract unless it fixes a documented
  defect;
- include deterministic regression evidence when behavior changes;
- update the user, scientific, API, or release documentation owned by the affected behavior; and
- preserve the release, citation, reference-fixture, and archival metadata contracts.

A critical defect found after v1.0.0 is published requires a new patch release; the v1.0.0 tag is
immutable. Do not use this repository for speculative refactors, dependency churn, new platforms,
new topologies or dimensions, broad performance programs, or new scientific capabilities.

## Environment setup

The repository uses [CMake] and [Ninja] for [C++23] sources, [vcpkg] manifest mode for C++ dependencies,
[Just] for its command interface, and [uv] for [Python] support scripts. The smallest [pkgx]-assisted Unix
setup is:

- [Xcode Command Line Tools] on macOS, or a [C++23] compiler and base build environment on Linux;
- [pkgx], which supplies the pinned Unix tool environment ephemerally;
- [Just] 1.58.0 or newer; and
- [Python] 3.14 and [uv] when running Python-backed checks directly.

Without pkgx, provide [Git], [Bash], [CMake] 4.4.0 or newer, [Ninja], [Python], [GNU M4], [Autoconf],
[Autoconf Archive], [Automake], [GNU Libtool], [Texinfo], and [pkg-config] through the host package manager.
Documentation work also requires [Doxygen] 1.16.1 and [Graphviz] 15.1.0. The build does not require a personal vcpkg
checkout, a fork, a submodule, Docker, or a hosted development environment.

For native Windows work, use an x64 [Developer Command Prompt or Developer PowerShell] for
[Visual Studio] 2022 17.4 or newer, with [MSVC] 19.34 or newer available. Install [Git for Windows]
and expose [Git Bash] on `PATH`, because the Justfile uses Bash as its recipe shell. The supported build path also
requires [Just] 1.58.0 or newer, [Python] 3.14 with `python.exe` on `PATH`, [CMake] 4.4.0 or newer, and
[Ninja]. The tested Windows cell uses Python 3.14.6, CMake 4.4.1, and Ninja 1.13.0. Complete local
validation additionally requires [uv] 0.12.3, [typos] 1.49.0, and [Go] or [pinact] 4.1.1 for the workflow
policy checks. The CI cell sets `VCPKG_DEFAULT_TRIPLET=x64-windows`; set the same value when a local
vcpkg environment would otherwise select a different triplet.

Start with the canonical build and locked Python environment:

```bash
just python-sync
just build
```

`just build` creates an ignored `.cache/vcpkg` checkout at the `builtin-baseline` recorded in
`vcpkg.json`, installs the manifest dependencies, builds under `out/build/reference`, and runs the
supported CTest suite. On Unix, `scripts/pkgx-env.sh` can be sourced to expose the same pinned tool
environment and repository-owned `VCPKG_ROOT` in an interactive shell or IDE. The underlying
`./scripts/build.sh` and `scripts\build.bat` entry points are available for native troubleshooting.

### Tested release matrix

| CI cell | Host | Compiler | Standard library | Required contract |
| --- | --- | --- | --- | --- |
| Ubuntu GCC | `ubuntu-latest` | [GCC] 16 | libstdc++ | `just ci` and `just build-parallel` |
| Ubuntu Clang | `ubuntu-latest` | [Clang] 22 | libstdc++ | `just ci` and `just build-parallel` |
| macOS AppleClang | `macos-latest` | [Runner AppleClang][Xcode] | libc++ | `just ci` and `just viewer-build` |
| Windows MSVC | `windows-latest`, x64 | [Runner MSVC][MSVC] | MSVC STL | `just ci` |

Linux compiler packages are pinned by the [`Justfile`](../Justfile). Native macOS and Windows
compilers follow the GitHub-hosted runner images. CMake enforces the minimum compiler floor: GCC
13.3, Clang 22, AppleClang 15, and MSVC 19.34. These are tested release cells, not a support promise
for every host and compiler pairing.

## Maintainer workflow

Run the narrowest relevant validator while editing, then the complete contract before opening or
updating a pull request. The primary recipes are:

| Command | Purpose |
| --- | --- |
| `just build` | Bootstrap, configure, build, and run the headless smoke suite. |
| `just build-debug` | Build production Debug targets and run compatible CLI integration tests. |
| `just build-parallel` | Build and test the opt-in CGAL/oneTBB configuration. |
| `just check` | Fast, non-mutating repository checks. |
| `just ci` | Run the complete local pre-commit and pre-push validation contract. |
| `just clang-tidy` | Analyze project C++ with LLVM 22. |
| `just coverage` | Generate the Linux GCC LCOV and HTML coverage reports. |
| `just docs-check` | Generate and validate documentation without changing the worktree. |
| `just fix` | Format C++, Python, and the Justfile. |
| `just initialize [ARGS]` | Build as needed and generate an initial triangulation. |
| `just load INPUT [ARGS]` | Load an initialized triangulation and start a new CDT move series. |
| `just reference-check` | Validate the committed reference package offline. |
| `just release-check` | Validate synchronized release metadata and citation fields. |
| `just resume CHECKPOINT [ARGS]` | Resume the identical CDT move series from a checkpoint. |
| `just run [ARGS]` | Build as needed and run the primary CDT++ simulation. |
| `just sanitize KIND` | Run the selected Linux sanitizer preset. |
| `just viewer-check` | Validate viewer fixtures, manifests, and the tracked image. |

`just check` covers C++ and Python formatting, Python lint and types, spelling, release and citation
metadata, YAML, GitHub Actions syntax and security, whitespace, CMake preset parsing, Semgrep policy,
reference-package consistency, and viewer artifacts. `just ci` adds action-pin policy, the supported
build and test contract, regenerated-reference drift checks, and Python package validation.

The GitHub Actions Ubuntu GCC, Ubuntu Clang, macOS AppleClang, and Windows MSVC cells run `just ci`.
Both Ubuntu cells also run `just build-parallel`; macOS additionally builds and smoke-tests the
viewer. Pull requests have separate coverage, generated-documentation, CodeQL, and sanitizer gates.

### Project layout

The primary source and generated-output boundaries are:

- `.github/` — repository policy, issue templates, and CI workflows;
- `cmake/` — CMake modules and validation helpers;
- `docs/` — source documentation and generated `docs/html/` output;
- `examples/` — compiled public API example;
- `include/` — C++ public and internal headers;
- `out/build/reference/` — ignored canonical headless build directory;
- `reference/` — versioned language-neutral fixtures and canonical records;
- `scripts/` — build, validation, comparison, and release support scripts;
- `src/` — command-line program sources;
- `tests/` — doctest, CTest, fixtures, and policy tests; and
- `viewer/` — versioned render fixtures, manifests, and schemas.

Do not hand-edit generated files or build output. Update their declared source and regenerate through
the owning Just recipe.

## Build and test validation

The canonical Release build runs 135 CTest registrations: 108 doctest scenarios, 25 CLI integration
tests, one compiled C++ API example, and one arithmetic-backend correctness test. The parallel
configuration registers 136 tests: the 108 ordinary doctest scenarios, one parallel launcher with
five scenarios, the same 25 integration tests, the C++ API example, and the arithmetic test.

To rerun the complete supported suite without rebuilding:

```bash
ctest --preset reference-smoke
```

To run a focused category:

```bash
ctest --preset reference-smoke -L unit
ctest --preset reference-smoke -L integration
```

The Debug build compiles the `cdt` and `initialize` production targets, then runs the 21
Debug-compatible CTest entries labeled `integration`. It defines `CGAL_NDEBUG` because supported move
paths deliberately traverse invalid intermediate triangulations while keeping CDT++ assertions
enabled. Release remains the canonical complete test configuration.

For behavior changes, add or update the smallest deterministic unit, integration, reference, or
compiled-example evidence that would have caught the defect. Randomized CGAL topology counts and
benchmark timings are diagnostic evidence, not exact correctness oracles.

Checkpoint continuation is a scientific correctness boundary. Its tests must demonstrate that an uninterrupted run
and the same run split at a persisted checkpoint retain the identical ordered transition trace, cumulative counters,
and final canonical topology—not merely that a checkpoint can be parsed.

## Documentation

The top-level [`README.md`](../README.md) is for people building, running, and consuming CDT++. Keep
maintenance mechanics in this guide and put detailed scientific or API contracts in their owning
pages under `docs/` or `reference/`.

Validate generated API documentation without changing the worktree:

```bash
just docs-check
```

Generate publishable output under `docs/html/` with:

```bash
just docs
```

Both recipes require Doxygen 1.16.1 and Graphviz 15.1.0 and use pkgx when matching local tools are
unavailable. Doxygen 1.16.1 is the archival pin because 1.17.0 duplicates linked labels, emits broken
alphabetical-index fragments for this repository, and injects an unused Mermaid CDN dependency.
`scripts/validate_generated_site.py` checks the actual HTML, local links and fragments, duplicate IDs
and link labels, and required assets.

The documentation workflow runs `just docs-check` as the stable pull-request gate. After a successful
`main` validation, a separate least-privilege job runs `just docs` and publishes its output to the
`gh-pages` branch.

## Static analysis and sanitizers

Python 3.14 is selected by `.python-version`; uv locks the environment in `uv.lock`. Ruff owns Python
formatting and linting, and ty owns static type checking. Use `just python-check` or
`just python-fix`; both are incorporated into the repository-wide validation recipes.

The C++ project follows the CppCore Guidelines as enforced by Clang-Tidy. Run the repository-pinned
LLVM 22 configuration with:

```bash
just clang-tidy
```

AddressSanitizer plus UndefinedBehaviorSanitizer, LeakSanitizer, MemorySanitizer, and ThreadSanitizer
share the repository-owned Linux driver and CMake presets:

```bash
just sanitize asan
just sanitize lsan
just sanitize msan
just sanitize tsan
```

MemorySanitizer remains experimental because third-party dependencies are not instrumented.
AddressSanitizer exercises the optional parallel CGAL/oneTBB path; ThreadSanitizer exercises the
default sequential configuration.

## Coverage

Coverage reporting requires Linux, GNU GCC and its matching gcov, CMake, Ninja, LCOV 2.5 or newer,
and `genhtml`:

```bash
CXX=g++ GCOV=gcov just coverage
```

The recipe writes `build/coverage.info` and `build/coverage-html/index.html`, retaining only
project-owned `include/` and `src/` paths. It reports line and branch coverage. Function coverage is
disabled because GCC can emit inconsistent function and line records for generated lambda bodies.

GCC 16 emits three known line-hit and branch-unhit records for templated assignments in
`Utilities.hpp`. The recipe requires that exact warning count so new or removed inconsistencies fail
for review. The Codecov workflow uploads only the filtered tracefile and preserves both reports as a
GitHub Actions artifact; use the LCOV artifact for the independent branch-coverage rate.

## vcpkg maintenance

`vcpkg.json` is the C++ dependency source of truth. Its `builtin-baseline` pins the official
`microsoft/vcpkg` registry commit. The repository-local `.cache/vcpkg` checkout is disposable tool
and cache infrastructure and must not be edited or committed.

To update the baseline intentionally, bootstrap the current checkout, run the upstream baseline
updater, synchronize the independently reviewed bootstrap-tool pins, inspect both diffs, and rebuild:

```bash
python3 scripts/bootstrap_vcpkg.py
export VCPKG_ROOT="$PWD/.cache/vcpkg"
"$VCPKG_ROOT/vcpkg" x-update-baseline
just sync-vcpkg-tool-pins
./scripts/build.sh
```

`just sync-vcpkg-tool-pins` reads the new baseline, fetches that exact upstream commit's tool
metadata, validates the official Windows amd64 and arm64 assets, and atomically updates the release
and SHA-256 pins in `scripts/bootstrap_vcpkg.py`. It leaves existing pins unchanged when an input
cannot be fetched or validated. On Windows, invoke the synchronizer with
`python.exe scripts\sync_vcpkg_tool_pins.py`.

CodeQL uses a two-phase manual build so third-party implementation findings stay outside CDT++
results. `just codeql-prepare` installs and configures dependencies before tracing;
`just codeql-build` then compiles only the `cdt` and `initialize` production targets.

## Submitting a correction

Open a focused pull request against `main` and identify the blocking release issue. The description
must summarize:

- the release-blocking defect or inconsistency;
- the correction and why it stays within the maintenance boundary;
- the validators and tests run;
- any scientific, compatibility, reproducibility, security, citation, or archival-metadata impact;
  and
- any platform-specific validation that could not be run locally.

Use a concise conventional title such as `fix:`, `docs:`, `test:`, `build:`, or `ci:`. All required
GitHub Actions checks must pass. Release tags, GitHub releases, Zenodo deposits, and repository
archival remain owned by their dedicated release issues and must not be performed from an ordinary
correction pull request.

## After archival

Do not fork CDT++ merely to continue its retired development line. Use the active successor for new
work. Historical forks remain subject to the BSD 3-Clause license but are not supported by this
repository and must not imply upstream maintenance or compatibility.

Security and support reporting is defined in [`SECURITY.md`](../SECURITY.md). The project
[Code of Conduct](CODE_OF_CONDUCT.md) remains part of the preserved project record. Contributors
retain credit through the repository history, `CITATION.cff`, and the Zenodo archive.

[Autoconf]: https://www.gnu.org/software/autoconf/
[Autoconf Archive]: https://www.gnu.org/software/autoconf-archive/
[Automake]: https://www.gnu.org/software/automake/
[Bash]: https://www.gnu.org/software/bash/
[C++23]: https://en.cppreference.com/w/cpp/23
[Clang]: https://clang.llvm.org
[CMake]: https://cmake.org
[Developer Command Prompt or Developer PowerShell]: https://learn.microsoft.com/cpp/build/building-on-the-command-line
[Doxygen]: https://www.doxygen.nl
[GCC]: https://gcc.gnu.org
[Git]: https://git-scm.com
[Git Bash]: https://gitforwindows.org
[Git for Windows]: https://gitforwindows.org
[GNU Libtool]: https://www.gnu.org/software/libtool/
[GNU M4]: https://www.gnu.org/software/m4/
[Go]: https://go.dev/doc/install
[Graphviz]: https://graphviz.org
[Just]: https://just.systems
[MSVC]: https://learn.microsoft.com/cpp/overview/visual-cpp-in-visual-studio
[Ninja]: https://ninja-build.org
[pkg-config]: https://www.freedesktop.org/wiki/Software/pkg-config/
[pkgx]: https://pkgx.sh
[pinact]: https://github.com/suzuki-shunsuke/pinact
[Python]: https://www.python.org/downloads/
[Texinfo]: https://www.gnu.org/software/texinfo/
[typos]: https://github.com/crate-ci/typos
[uv]: https://docs.astral.sh/uv/
[Visual Studio]: https://visualstudio.microsoft.com/vs/
[vcpkg]: https://learn.microsoft.com/vcpkg/
[Xcode]: https://developer.apple.com/xcode/
[Xcode Command Line Tools]: https://developer.apple.com/xcode/resources/
