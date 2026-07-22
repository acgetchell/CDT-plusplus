# Changelog

All notable changes to CDT++ are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.0.0-rc1] - 2026-07-21

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

[1.0.0-rc1]: https://github.com/acgetchell/CDT-plusplus/compare/0.1.8...v1.0.0-rc1
