# Contributing to CDT++

Thank you for helping improve CDT++.

CDT++ v1.0.0-rc3 is the current C++23 release candidate. The project remains maintained as a scientific reference
implementation and regression oracle for
[causal-triangulations](https://github.com/acgetchell/causal-triangulations), its supported Rust successor. The
v1.0.0 release scope is tracked by [issue #90](https://github.com/acgetchell/CDT-plusplus/issues/90). Archiving the
GitHub repository or making it read-only is a separate future maintainer decision, not settled policy for immediately
after v1.0.0.

## Accepted work

Contributions should be limited to:

- correctness fixes, especially for causal invariants, bistellar moves, and scientific results;
- deterministic tests, reproducibility improvements, and cross-implementation validation;
- documentation, build, CI, portability, dependency, and release-readiness fixes;
- work already approved in a project issue or release milestone, including
  [Qt restoration #98](https://github.com/acgetchell/CDT-plusplus/issues/98) and
  [domain invariant work #101](https://github.com/acgetchell/CDT-plusplus/issues/101) and
  [offline C++/Rust comparison #104](https://github.com/acgetchell/CDT-plusplus/issues/104); and
- changes required to complete the v1.0.0 release and Zenodo deposit.

New simulation features, 3+1D/4D development, and unrelated C++ expansion remain out of scope unless a project issue
explicitly approves them. Potential future 4D research does not authorize implementation by itself. If a change is
not already tracked, open an issue before investing in a substantial implementation so its maintenance value and
scope can be agreed upon.

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
   just build-parallel
   ```

   `just check` is the fast, non-mutating source and tooling gate, including the
   repository-owned Semgrep policy and its fixtures. `just ci` adds the
   supported build and complete 128-entry CTest suite: 104 doctest unit
   scenarios, 23 CLI integration tests, and one arithmetic-backend correctness
   test. `just build-parallel` builds the distinct CGAL/oneTBB configuration and
   runs its 128-entry suite: 103 ordinary doctest scenarios, one replayable
   parallel stress launcher containing five scenarios, the same 23 CLI
   integration tests, and the arithmetic correctness test. When changing C++
   behavior, also run `just clang-tidy` with the pinned LLVM 22 toolchain and
   review its advisory diagnostics.
   GitHub Actions runs `just ci` in its Ubuntu GCC, Ubuntu Clang, macOS AppleClang, and Windows MSVC jobs. The two
   Ubuntu jobs also run `just build-parallel` to exercise the opt-in CGAL/oneTBB contract. Sanitizer and coverage
   builds keep Release assertion semantics while adding their own debug information and optimization settings. A
   separate full-suite Debug job is intentionally omitted because several fixtures traverse invalid intermediate
   triangulations and abort on CDT++ invariant assertions. Use `just build-debug` to compile production targets with
   CDT++ assertions enabled and run the 21 compatible CLI integration CTests. That preset defines `CGAL_NDEBUG` for
   supported move paths while excluding `cdt`, `cdt-no-output`, and the doctest unit fixtures because they trip
   project assertions on deliberately invalid intermediate states. The Windows job continues to compile with native
   MSVC; LLVM tooling is used only for source formatting. Toolchain setup is pkgx-first;
   because pkgx does not currently publish its CMake and Ninja packages for Windows, that job uses the exact Justfile
   pins available as PyPI wheels through `uv tool install --no-build`.

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

## Release and repository lifecycle

Release candidates, the final tag, and the Zenodo deposit follow the
[`docs/RELEASING.md`](../docs/RELEASING.md) runbook. The final v1.0.0 release will be tagged from `main`; release
metadata and the Zenodo handoff are tracked by [issue #96](https://github.com/acgetchell/CDT-plusplus/issues/96).
Making the GitHub repository read-only would require a separate explicit maintainer decision. Until then, issues and
pull requests remain governed by the scoped contribution policy above.

Contributors retain credit through the repository history and resulting project citation metadata.

[doctest]: https://github.com/doctest/doctest
[Zenodo]: https://zenodo.org/
