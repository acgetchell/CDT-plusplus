# Changelog

All notable changes to CDT++ are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.0.0-rc3] - 2026-07-27

### Added

- Restore optional parallel insertion and removal [`d4dabbe`](https://github.com/acgetchell/CDT-plusplus/commit/d4dabbe2fc24a2b98864ae1cd5ca720432dc9418)

- Add value-oriented move-run orchestration [`abfa888`](https://github.com/acgetchell/CDT-plusplus/commit/abfa888b33ace3c63efa85e475391be3691ba011)

- [**breaking**] Add invariant-bearing move preparation [`e2f7d70`](https://github.com/acgetchell/CDT-plusplus/commit/e2f7d70ae819d3f9e3ae8eeda83080d22fa4da5d)

- [**breaking**] Modernize the CGAL 6.2 integration boundary [`ae7a39b`](https://github.com/acgetchell/CDT-plusplus/commit/ae7a39b74dcf9ba9a45a40810e9de033204f3f3e)

- [**breaking**] Finalize parallel triangulation support [`862637a`](https://github.com/acgetchell/CDT-plusplus/commit/862637ae0be485f36816a346f3c7ede560371a82)

- [**breaking**] Publish deterministic C++ oracle fixtures [`ec53dfa`](https://github.com/acgetchell/CDT-plusplus/commit/ec53dfaf3307e7a862ebc0994827a2b4974f52a7)

### Changed

- [**breaking**] Normalize public declarations under cdt [`a02a732`](https://github.com/acgetchell/CDT-plusplus/commit/a02a732267a02860c705dc32f94284f544d02888)

- Make MoveAlways replay checks deterministic [`d7fa637`](https://github.com/acgetchell/CDT-plusplus/commit/d7fa637abf03d9afb892d48f7119feb8c3580fb5)

- Own move-run callbacks by value [`f59c06f`](https://github.com/acgetchell/CDT-plusplus/commit/f59c06f159cd1b688108e2c3f77afb0cd1d186db)

- Make seeded replay fixtures cross-platform [`3c322ee`](https://github.com/acgetchell/CDT-plusplus/commit/3c322ee242a1d3b2e4d475196950d2dd6ad35c4c)

- Make producer path assertion cross-platform [`390adf3`](https://github.com/acgetchell/CDT-plusplus/commit/390adf3bd7b34d310d9092e8cb4305ce397f3b90)

### Fixed

- Generate GCC 16 reports with LCOV 2.5 [`324a2aa`](https://github.com/acgetchell/CDT-plusplus/commit/324a2aa82a8cfa646827229b959981ec7258fa2d)

- Install LCOV documentation dependencies [`42fd262`](https://github.com/acgetchell/CDT-plusplus/commit/42fd2629aa71931b4184fd1fdc72ae5daabf7665)

- Support pkgx-backed IDE environments [`3ce8780`](https://github.com/acgetchell/CDT-plusplus/commit/3ce8780e082760572740b0695606188e9d40ef3e)

- Retain function data during LCOV capture [`898a453`](https://github.com/acgetchell/CDT-plusplus/commit/898a4538bf47e87d656ecf447be0c6ada07b5a8e)

- Bound GCC branch inconsistencies [`c7d6a35`](https://github.com/acgetchell/CDT-plusplus/commit/c7d6a353d4684a0ecf7024c946d06cedce3fd05a)

- Keep oneTBB out of ThreadSanitizer [`35b241e`](https://github.com/acgetchell/CDT-plusplus/commit/35b241e6a3358fd818febf1a748092a019301163)

- Harden parallel build and provenance [`6ef7dd6`](https://github.com/acgetchell/CDT-plusplus/commit/6ef7dd62c1d1a71bb834611a4a89a15a47f7de57)

- Resolve pkgx certificate bundles before vcpkg [`a123ab8`](https://github.com/acgetchell/CDT-plusplus/commit/a123ab88ceb0a07f3d7165a30f718285e33c3f17)

- Restore CodeQL extraction and bound parallel builds [`56f13cd`](https://github.com/acgetchell/CDT-plusplus/commit/56f13cdcf5bd602e59921a3c3af635f611899d4f)

- Require fresh fixture provenance [`6972970`](https://github.com/acgetchell/CDT-plusplus/commit/6972970c77248c3700661b8293f602f58a1deed1)

- Stabilize cross-platform validation [`b0c3169`](https://github.com/acgetchell/CDT-plusplus/commit/b0c3169b90111da99731459155490ffc5ed50544)

- Stabilize Windows and Semgrep validation [`b282c0b`](https://github.com/acgetchell/CDT-plusplus/commit/b282c0bc126f9d0fcc8bddc248ddd149d1144b4b)

- Stabilize cached builds and action policy validation [`930238a`](https://github.com/acgetchell/CDT-plusplus/commit/930238ab99aff4dfc90d3834a8eefe99f1c57966)

- Enforce action policy on nested paths [`745d402`](https://github.com/acgetchell/CDT-plusplus/commit/745d4029b5fb5258ae69617b2d393df7169bccd1)

- Enforce action policy on shorthand steps [`406a482`](https://github.com/acgetchell/CDT-plusplus/commit/406a4827230459efe2026395dde11dab8cf7889c)

### Maintenance

- Suppress ShellCheck for Zsh path expansion [`189cc0d`](https://github.com/acgetchell/CDT-plusplus/commit/189cc0d33529ef2a9fcde0e3e02479b134a7e881)

- Bump actions/checkout from 7.0.0 to 7.0.1 [`397e0b4`](https://github.com/acgetchell/CDT-plusplus/commit/397e0b40ccc514edb187420451206f4d0f27b21a)

- Bump taiki-e/install-action from 2.83.2 to 2.83.4 [`30c83fd`](https://github.com/acgetchell/CDT-plusplus/commit/30c83fd98861b777e7071522beccab1192911275)

- Bump github/codeql-action/init from 4.37.0 to 4.37.1 [`fd04186`](https://github.com/acgetchell/CDT-plusplus/commit/fd04186063ddd0504ba66e6cf5c10a3497895b4a)

- Bump github/codeql-action/analyze from 4.37.0 to 4.37.1 [`16d8fcd`](https://github.com/acgetchell/CDT-plusplus/commit/16d8fcd2f98b8f139a60d8de0072958b4eaccc10)

- Enforce pkgx-first pinned toolchains [`f01fca6`](https://github.com/acgetchell/CDT-plusplus/commit/f01fca697d36f7c9a49c5665fb4b7adee78e868d)

- Streamline and harden supported build workflows [`e29f69a`](https://github.com/acgetchell/CDT-plusplus/commit/e29f69a676bc7c10c12afbea4791fa70e9298de9)

### Performance

- [**breaking**] Avoid rebuilding caches during move validation [`f574f0f`](https://github.com/acgetchell/CDT-plusplus/commit/f574f0f381d202fcdc3cbaf78a815af983a1815d)

## [1.0.0-rc2] - 2026-07-22

### Fixed

- Verify Zenodo delivery for the current tag [`05a6e97`](https://github.com/acgetchell/CDT-plusplus/commit/05a6e97dff1243015a8e2237190defb1683b9ef3)

## [1.0.0-rc1] - 2026-07-22

### Added

- Persist verifiable stochastic run provenance [`ce90551`](https://github.com/acgetchell/CDT-plusplus/commit/ce90551ff28368235844b8fe0b068c050f529ab1)

### Fixed

- [**breaking**] Enforce atomic moves and validated runtime state [#111](https://github.com/acgetchell/CDT-plusplus/pull/111) [`4245ac0`](https://github.com/acgetchell/CDT-plusplus/commit/4245ac0ca401075a3ba88754745fc6ecb680fb6c)

- [**breaking**] Make Metropolis transitions reversible and reproducible [#112](https://github.com/acgetchell/CDT-plusplus/pull/112) [`1d7269c`](https://github.com/acgetchell/CDT-plusplus/commit/1d7269c64e81c0eb0ff08c7880b9c122ef096729)

- Harden optional Python experiment lifecycle [#113](https://github.com/acgetchell/CDT-plusplus/pull/113) [`31f95ab`](https://github.com/acgetchell/CDT-plusplus/commit/31f95ab37e4efad6f1da23ffe7c8c4601fc25e53)

- Enforce causal ergodic move contracts [`bb53988`](https://github.com/acgetchell/CDT-plusplus/commit/bb5398870c591e4c8f37ee7c1d918cde6dd05def)

- Restore sanitizer and coverage reliability [`f4c7df9`](https://github.com/acgetchell/CDT-plusplus/commit/f4c7df9637592cafaf41014d20fba89288000249)

- Harden ergodic move rejection [`cec15b5`](https://github.com/acgetchell/CDT-plusplus/commit/cec15b57fd6b49338fe1bdfc5ffdf1f227a1aa4e)

- Make runtime configuration header self-contained [`cef03b5`](https://github.com/acgetchell/CDT-plusplus/commit/cef03b5fb1aebcb72a85096267cdbcae7f43e9af)

### Maintenance

- [**breaking**] Consolidate develop into main [#100](https://github.com/acgetchell/CDT-plusplus/pull/100) [`2459016`](https://github.com/acgetchell/CDT-plusplus/commit/2459016f0e47decbf1c56059109bff09aeed498f)

- Align repository status reporting with main [#109](https://github.com/acgetchell/CDT-plusplus/pull/109) [`c37fa09`](https://github.com/acgetchell/CDT-plusplus/commit/c37fa0914f5cae3049c25b449ad1c9850121c397)

- Add release tooling and repair coverage [`1416708`](https://github.com/acgetchell/CDT-plusplus/commit/14167080aa06a0fbe6e8c81664ba516b8dcfe96e)

- Update docs/RELEASING.md [`26196da`](https://github.com/acgetchell/CDT-plusplus/commit/26196daa76b1094ea9364a20b3618ef61a5c4185)

[1.0.0-rc3]: https://github.com/acgetchell/CDT-plusplus/compare/v1.0.0-rc2...v1.0.0-rc3
[1.0.0-rc2]: https://github.com/acgetchell/CDT-plusplus/compare/v1.0.0-rc1...v1.0.0-rc2
[1.0.0-rc1]: https://github.com/acgetchell/CDT-plusplus/compare/0.1.8...v1.0.0-rc1
