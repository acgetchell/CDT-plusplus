# Contributing during maintenance mode

Thank you for helping improve CDT++.

CDT++ v1.0.0-rc2 is the release candidate for the final C++23 release, v1.0.0, after which this repository will be
archived. The project
is maintained as a scientific reference implementation and regression oracle for
[causal-triangulations](https://github.com/acgetchell/causal-triangulations), its supported Rust successor. The
maintenance and archival scope is tracked by
[issue #90](https://github.com/acgetchell/CDT-plusplus/issues/90).

## Accepted work

Until archival, contributions should be limited to:

- correctness fixes, especially for causal invariants, bistellar moves, and scientific results;
- deterministic tests, reproducibility improvements, and cross-implementation validation;
- documentation, build, CI, portability, dependency, and release-readiness fixes;
- work already approved in a project issue or release milestone, including
  [Qt restoration #98](https://github.com/acgetchell/CDT-plusplus/issues/98) and
  [domain invariant work #101](https://github.com/acgetchell/CDT-plusplus/issues/101); and
- changes required to complete the v1.0.0 release and Zenodo archival handoff.

New simulation features, 3+1D/4D development, and unrelated C++ expansion are out of scope. Propose ongoing feature
development in the Rust successor instead. If a change is not already tracked, open an issue before investing in a
substantial implementation so its maintenance value and scope can be agreed upon.

## Contribution workflow

1. Fork the repository and create a short-lived branch from `main`.

2. Keep the change focused and preserve scientific behavior unless the purpose of the change is to correct a
   documented bug.

3. Add or update deterministic tests for behavior changes. Tests use [doctest] and live in `tests`; register new test
   files in `tests/CMakeLists.txt`. Existing tests use descriptive BDD-style `SCENARIO`, `GIVEN`, `WHEN`, and `THEN`
   sections where that structure clarifies behavior.

4. Update relevant documentation and Doxygen comments when interfaces, supported workflows, or scientific behavior
   change.

5. Run the repository-owned validation commands:

   ```bash
   just fix
   just check
   just ci
   ```

   `just check` is the fast, non-mutating source and tooling gate, including the repository-owned Semgrep policy and
   its fixtures. `just ci` adds the supported build and complete 22-entry CTest suite: all 103 doctest unit scenarios
   and 21 CLI integration tests. When changing C++ behavior, also run
   `just clang-tidy` with the pinned LLVM 22 toolchain and review its advisory diagnostics.
   GitHub Actions runs the same `just ci` contract in its Ubuntu GCC, Ubuntu Clang, macOS AppleClang, and Windows
   MSVC jobs. The Windows job continues to compile with native MSVC; LLVM tooling is used only for source formatting.

6. Run the relevant Linux sanitizer configuration for changes involving memory, lifetime, undefined behavior, or
   concurrency:

   ```bash
   just sanitize asan
   just sanitize lsan
   just sanitize tsan
   ```

   MemorySanitizer is experimental because third-party dependencies are not instrumented; run it manually with
   `just sanitize msan` when useful.

7. Open a pull request against `main`, explain the maintenance issue being addressed, and identify any scientific or
   reproducibility implications. All required GitHub Actions checks must pass.

The project [Code of Conduct](CODE_OF_CONDUCT.md) applies to all participation.

## Style

The project uses Stroustrup-style formatting with Allman braces, enforced by the repository's `.clang-format` and
LLVM 22. The C++ Core Guidelines inform the `.clang-tidy` policy. Prefer minimal, readable changes over cosmetic
modernization, and do not suppress diagnostics without documenting why the check is inappropriate for this codebase.

Project source files belong in `src`, public or shared headers in `include`, and tests in `tests`. Use the existing
CMake targets, presets, CTest registrations, Just recipes, and pinned vcpkg manifest rather than introducing parallel
build paths.

## Release and archival

Release candidates, the final tag, and the archival handoff follow the
[`docs/RELEASING.md`](../docs/RELEASING.md) runbook. The final v1.0.0 release will be tagged from `main` and archived
through [Zenodo]. Release metadata and the archival handoff are tracked by
[issue #96](https://github.com/acgetchell/CDT-plusplus/issues/96). After archival, the repository will be read-only and
will no longer accept issues or pull requests. Further development should occur in
[causal-triangulations](https://github.com/acgetchell/causal-triangulations) or in an independent fork.

Contributors retain credit through the repository history and resulting project citation metadata.

[doctest]: https://github.com/doctest/doctest
[Zenodo]: https://zenodo.org/
