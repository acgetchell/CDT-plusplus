# Contributing to CDT++

CDT++ v1.0.0 is the final planned C++23 feature release. The repository is
preserved as a historical scientific reference and regression oracle for
[causal-triangulations](https://github.com/acgetchell/causal-triangulations),
the supported Rust successor. Active development, new scientific features,
and general support belong in that project.

Issue [#97](https://github.com/acgetchell/CDT-plusplus/issues/97) owns the
final tag, GitHub release, and Zenodo record verification. Issue
[#155](https://github.com/acgetchell/CDT-plusplus/issues/155) owns the separate
stabilization gate, tracker closure, and eventual repository archival.
Archival is not immediate: the repository remains maintenance-only until the
owner determines that no release blockers remain, after which GitHub will make
it read-only.

## Final release corrections

Before archival, pull requests are limited to release-blocking correctness,
reproducibility, security, documentation, and metadata corrections. A
correction must be tied to an existing release issue, preserve the bounded
2+1-dimensional spherical scientific contract unless it fixes a documented
defect, and include deterministic regression evidence when behavior changes.
A critical defect found after v1.0.0 is published must use a new patch release;
the v1.0.0 tag is immutable.

Run the repository-owned validation appropriate to the correction:

```bash
just fix
just check
just release-check
just docs-check
just ci
just build-parallel
```

`just ci` is the supported headless build and test contract. The two Ubuntu
compiler cells also run `just build-parallel`; the macOS cell additionally
runs the opt-in archival viewer. Use `just clang-tidy` for C++ changes and the
relevant `just sanitize asan`, `just sanitize lsan`, or `just sanitize tsan`
workflow for memory, lifetime, undefined-behavior, or concurrency changes.
MemorySanitizer remains experimental because third-party dependencies are not
instrumented.

Open a focused pull request against `main`, identify the blocking release
issue, and explain any scientific, compatibility, reproducibility, citation,
or archival-metadata impact. All required GitHub Actions checks must pass.

## After archival

Do not fork CDT++ merely to continue its retired development line. Use the
active successor for new work. Historical forks remain subject to the BSD
3-Clause license but are not supported by this repository and must not imply
upstream maintenance or compatibility.

Security and support reporting is defined in
[`SECURITY.md`](../SECURITY.md). The project
[Code of Conduct](CODE_OF_CONDUCT.md) remains part of the preserved project
record.

Contributors retain credit through the repository history, `CITATION.cff`,
and the Zenodo archive.
