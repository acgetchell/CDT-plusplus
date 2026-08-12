# Changelog

All notable changes to CDT++ are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.0.0] - 2026-08-11

### Release summary

CDT++ 1.0.0 is the final planned feature release of the C++23 spherical 2+1-dimensional reference implementation.

- **Compatibility:** The supported source boundary is C++23 with the published GCC, Clang, AppleClang, and MSVC matrix. The project does not promise a stable binary ABI or package-registry distribution.
- **Scientific corrections:** The release completes and independently validates all five 2+1D moves, Metropolis-Hastings proposal ratios and atomic transitions, exact-predicate geometry boundaries, run-owned RNG streams, and causal persistence metadata.
- **Supported surface:** The default build is headless; CGAL/oneTBB bulk Delaunay operations and the macOS Qt viewer are explicit opt-ins. Historical toroidal and higher-dimensional prototypes are excluded from the supported API.
- **Limitations:** Seeds replay stochastic inputs, not necessarily fresh cospherical CGAL topology. Resumable checkpoints continue the identical Markov chain only on the recorded producer toolchain; they are restart artifacts rather than portable interchange files. Parallelism does not extend to Pachner moves or concurrent manifold access.
- **Migration:** Active development and new work move to [causal-triangulations](https://github.com/acgetchell/causal-triangulations). After the v1.0.0 GitHub and Zenodo handoff, CDT++ enters a maintenance-only stabilization window and becomes read-only only when the owner completes the archival gate.

### Added

- [**breaking**] Modernize Python experiment workflows [`def276a`](https://github.com/acgetchell/CDT-plusplus/commit/def276aa2fdb9eb473d7425d61e29e26511caa28)

- [**breaking**] Add offline C++/Rust comparison harness [`45b080e`](https://github.com/acgetchell/CDT-plusplus/commit/45b080e2388b2475bf26f4462fa4d1ebd5915c69)

- Restore reproducible archival rendering [`2662df9`](https://github.com/acgetchell/CDT-plusplus/commit/2662df9993469e21ec5a0c65c552202b9eec9ae6)

- [**breaking**] Expose reported Metropolis transitions [`16d1c76`](https://github.com/acgetchell/CDT-plusplus/commit/16d1c769afc67e06f07639f6fd9de8136974e282)

- Add initial-state loading and checkpoint resume [`e1305d6`](https://github.com/acgetchell/CDT-plusplus/commit/e1305d66663778e3c9bd7e04f8020f10919ecd1a)

### Changed

- Expose shared artifact lifecycle helpers [`2f3ba57`](https://github.com/acgetchell/CDT-plusplus/commit/2f3ba57d425f8ab4f1a0c5da0cedf7876b13eaa5)

- Evaluate Boost alternatives to GMP and MPFR [`9a02c39`](https://github.com/acgetchell/CDT-plusplus/commit/9a02c3960f7b7f45b4a29f844f2cddbd21ab0231)

- Harden backend evaluation probes [`d5c3b71`](https://github.com/acgetchell/CDT-plusplus/commit/d5c3b710b0af82651943f5a1e60c23b5ec818679)

- Format transition validation [`f183f07`](https://github.com/acgetchell/CDT-plusplus/commit/f183f072b44b3fe3b677090b8e5cd4983c193932)

- Verify unknown moves preserve transition traces [`70ab238`](https://github.com/acgetchell/CDT-plusplus/commit/70ab238eb07271a7754266407f9f61e28e199b49)

- Expect portable producer paths on Windows [`5a32b1d`](https://github.com/acgetchell/CDT-plusplus/commit/5a32b1d5ac1ca460ca78996e8e8b5ce42a5c54cb)

### Documentation

- Clean up archival documentation references [`703b474`](https://github.com/acgetchell/CDT-plusplus/commit/703b474ab8ba0f6ee86fef7842ad9c5b4b74a3e7)

- Exclude PCG shim from generated reference [`7e8734f`](https://github.com/acgetchell/CDT-plusplus/commit/7e8734fac7097406eb416df4dd241c6a6b5d9461)

### Fixed

- Serialize Dependabot automation per pull request [`7567372`](https://github.com/acgetchell/CDT-plusplus/commit/7567372f3f11136b4317b66e695e7f2aa7d2dd69)

- Deduplicate CodeRabbit review requests by revision [`7f23d53`](https://github.com/acgetchell/CDT-plusplus/commit/7f23d530e092114b69f7b8d42b5339e24495282d)

- Bind Dependabot auto-merge to reviewed revision [`72fc5a1`](https://github.com/acgetchell/CDT-plusplus/commit/72fc5a1181d8f1d4c4c0756c809b196c72929b43)

- Harden Dependabot review rearming on head updates [`5d571c8`](https://github.com/acgetchell/CDT-plusplus/commit/5d571c85150615f1a93f931f144e076eeb884d89)

- Authenticate CodeRabbit review requests as maintainer [`375c9ea`](https://github.com/acgetchell/CDT-plusplus/commit/375c9ea1fcaf775054ad0537d7bf5e0656148d21)

- Require reviewed Dependabot merges and refresh tooling [`7c3a84b`](https://github.com/acgetchell/CDT-plusplus/commit/7c3a84bdfb91ca33829ad4302f5debd275d824ee)

- Restore MPFR builds and fail closed on polling errors [`1d2e667`](https://github.com/acgetchell/CDT-plusplus/commit/1d2e667f7df090db7a5ef792fa1bbf9bec9c32b6)

- Prevent stale vcpkg [`9e8d033`](https://github.com/acgetchell/CDT-plusplus/commit/9e8d033e34c389808206e1d0644a7cd58b88336e)

- [**breaking**] Preserve coincident causal identities [`3f72a16`](https://github.com/acgetchell/CDT-plusplus/commit/3f72a16c636a21a05c7273f4667efc0b8c919288)

- Harden persistence and vcpkg pin synchronization [`3a66693`](https://github.com/acgetchell/CDT-plusplus/commit/3a66693a35b27d766b8ac022fd53eccc51f39f09)

- Canonicalize complete vertex-cell incidence [`52f9a58`](https://github.com/acgetchell/CDT-plusplus/commit/52f9a58e06898b563168dd75525ed53c02cd3ca4)

- Bound incidence fingerprint canonicalization [`307737c`](https://github.com/acgetchell/CDT-plusplus/commit/307737ce2c5c79af376d7f38b4183a70925992d2)

- Harden experiment and repository validation [`fb7e5b3`](https://github.com/acgetchell/CDT-plusplus/commit/fb7e5b36161ec11eb16ab9c7b88557dbe5a916ec)

- Harden cross-platform validation [`6b4d012`](https://github.com/acgetchell/CDT-plusplus/commit/6b4d012067002259a87c51915b6c7bbfaefd48d6)

- Pin Python for sanitizer bootstrap [`9181494`](https://github.com/acgetchell/CDT-plusplus/commit/9181494871400009192bfea67605543b7be78130)

- Refresh portable Python package checks [`96baa21`](https://github.com/acgetchell/CDT-plusplus/commit/96baa21d258ae50992f592f498cb87467211f2c6)

- Preserve local runs when Comet mirroring fails [`9cadfe7`](https://github.com/acgetchell/CDT-plusplus/commit/9cadfe772670abed767953079990a2080914e0a4)

- Harden portable harness validation [`da7ba14`](https://github.com/acgetchell/CDT-plusplus/commit/da7ba141eb362866942241135d325e1ae9df4fec)

- Select the root vcpkg manifest [`1c064fd`](https://github.com/acgetchell/CDT-plusplus/commit/1c064fd098c845e6efaea48290a43b023ebddbe3)

- Restore vcpkg binary caching [`2b27462`](https://github.com/acgetchell/CDT-plusplus/commit/2b2746202d8ebb09b6e2c801a5868794414398ca)

- Track canonical rendering fixture [`7078f69`](https://github.com/acgetchell/CDT-plusplus/commit/7078f69d7f19e54b0cd4c888666815f2578e5add)

- Harden archival rendering contract [`81d366e`](https://github.com/acgetchell/CDT-plusplus/commit/81d366ef9b58dfa750061f3fb9305cbbafb04a83)

- Harden transition and documentation validation [`e955ee6`](https://github.com/acgetchell/CDT-plusplus/commit/e955ee643caa5e081314dfc6c5a5168bc1f17319)

- Queue approved Dependabot updates for auto-merge [`e63e32f`](https://github.com/acgetchell/CDT-plusplus/commit/e63e32f627dfb1e83392714183c09032157fa74a)

- Enforce v1.0.0 release gates [`94ed107`](https://github.com/acgetchell/CDT-plusplus/commit/94ed107aa2390accd17f0191c4149df6201618ed)

- Fail fast and support Dependabot coverage [`2107f20`](https://github.com/acgetchell/CDT-plusplus/commit/2107f20d429b466eb7ec01693e769a0262dcd264)

- Stabilize macOS viewer and coverage gates [`deb24ab`](https://github.com/acgetchell/CDT-plusplus/commit/deb24ab0d7ce179469839a145f344ca1624c1275)

- Harden checkpoint resume contracts [`17bb7fb`](https://github.com/acgetchell/CDT-plusplus/commit/17bb7fb0a687837955b2a8932f028d76d029480b)

- Enforce portable checkpoint continuation [`95754c4`](https://github.com/acgetchell/CDT-plusplus/commit/95754c430c33d4913f8e7fc30c1775280cc554e6)

- Harden repository readiness contracts [`2840437`](https://github.com/acgetchell/CDT-plusplus/commit/2840437d393078aaeaaa7c654c150088ceedbf12)

- Preserve canonical producer paths on Windows [`026ba3f`](https://github.com/acgetchell/CDT-plusplus/commit/026ba3f20b496923197fb651fb77fa62414a2b3a)

- Harden boundary validation and diagnostics [`38be43d`](https://github.com/acgetchell/CDT-plusplus/commit/38be43de1ec0e9aa181e6e209ce80a3c6b6db7ef)

- Preserve complete subprocess diagnostics [`94a0c77`](https://github.com/acgetchell/CDT-plusplus/commit/94a0c7700b9b9aa3527863c16a9018ac48f71c13)

### Maintenance

- Gate automatic merges on CodeRabbit review [`8124227`](https://github.com/acgetchell/CDT-plusplus/commit/812422797620c2203c9214af880cb5957af4469b)

- Bump the github-actions group with 4 updates [#134](https://github.com/acgetchell/CDT-plusplus/pull/134) [`55d192d`](https://github.com/acgetchell/CDT-plusplus/commit/55d192df05ca9d34050fb8fa618984573be2eb97)

- Bump the github-actions group with 2 updates [#136](https://github.com/acgetchell/CDT-plusplus/pull/136) [`90a3ea6`](https://github.com/acgetchell/CDT-plusplus/commit/90a3ea67db84d553d5803bdbaa9bb60ed937aa33)

- Automate vcpkg tool pin sync [`9631335`](https://github.com/acgetchell/CDT-plusplus/commit/96313355bc851ae05f5787e95839d9e619a39d2c)

- Bump the dependencies group with 3 updates [#144](https://github.com/acgetchell/CDT-plusplus/pull/144) [`098822d`](https://github.com/acgetchell/CDT-plusplus/commit/098822d04b2a8b325f60529e8e98910845dd330c)

- Stagger Dependabot update schedules [`9cf5b9d`](https://github.com/acgetchell/CDT-plusplus/commit/9cf5b9d3a1f6fc1583a9b937446df56dc1626a06)

- Bump cryptography from 49.0.0 to 50.0.0 [#146](https://github.com/acgetchell/CDT-plusplus/pull/146) [`e23d3c4`](https://github.com/acgetchell/CDT-plusplus/commit/e23d3c4912b80754bd97c11f2ff8a50f2bc3d61f)

- Bump taiki-e/install-action [#152](https://github.com/acgetchell/CDT-plusplus/pull/152) [`426edb8`](https://github.com/acgetchell/CDT-plusplus/commit/426edb8085a67c3aae5a308fd1355aa05c800e9c)

## [1.0.0-rc3] - 2026-07-28

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

- Update v1.0.0-rc3 release date [`e8f45ea`](https://github.com/acgetchell/CDT-plusplus/commit/e8f45ea15fdb813c00f0d773f8ff30251c699207)

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

[1.0.0]: https://github.com/acgetchell/CDT-plusplus/compare/v1.0.0-rc3...v1.0.0
[1.0.0-rc3]: https://github.com/acgetchell/CDT-plusplus/compare/v1.0.0-rc2...v1.0.0-rc3
[1.0.0-rc2]: https://github.com/acgetchell/CDT-plusplus/compare/v1.0.0-rc1...v1.0.0-rc2
[1.0.0-rc1]: https://github.com/acgetchell/CDT-plusplus/compare/0.1.8...v1.0.0-rc1
