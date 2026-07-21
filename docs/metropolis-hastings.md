# Metropolis-Hastings transition contract

CDT++ samples a triangulation `T` with target weight proportional to `exp(-S(T))`.
For a proposed triangulation `T'`, the implemented acceptance probability is

```text
min(1, exp(S(T) - S(T')) * q(T | T') / q(T' | T)).
```

This is the Metropolis-Hastings rule of Hastings
[Hastings1970](../REFERENCES.md#metropolis-hastings-algorithm). The
three-dimensional causal triangulations, Regge action, and local move set
follow Ambjørn, Jurkiewicz, and Loll
[AmbjornJurkiewiczLoll2001-3D](../REFERENCES.md#three-dimensional-cdt-2001).

## Proposal kernel

Each transition first chooses one of the five 3D move types uniformly. It then
chooses one raw site uniformly from the move-specific domain below. The move is
attempted only at that site. An inapplicable site, failed construction, or
invalid geometry delta is an explicit rejected self-transition.

| Move | Raw proposal sites in `T` | Reverse move |
| --- | --- | --- |
| `(2,3)` | `N3_22(T)` two-two cells | `(3,2)` |
| `(3,2)` | `N1_TL(T)` timelike edges | `(2,3)` |
| `(2,6)` | `N3_13(T)` one-three cells | `(6,2)` |
| `(6,2)` | `N0(T)` vertices | `(2,6)` |
| `(4,4)` | `N1_SL(T)` spacelike edges | `(4,4)` |

For a raw-site count `C_m(T)`, a particular proposal has probability

```text
q(T' | T) = 1 / (5 * C_m(T)).
```

The factor `1/5` cancels between a move and its reverse, giving

```text
q(T | T') / q(T' | T) = C_m(T) / C_reverse(m)(T').
```

The site definitions give a unique inverse site for each successful local
retriangulation: a `(2,3)` face becomes the timelike edge used by `(3,2)`; a
successful `(2,6)` move creates the vertex used by `(6,2)`; and a `(4,4)` pivot
edge becomes the opposite pivot edge. Each `(2,6)` proposal examines exactly
one uniformly selected `(1,3)` cell. Failed raw sites are not silently skipped,
because conditioning on only the movable subset would change `q` and invalidate
the count ratio above.

Move-type selection, raw-site selection, internal candidate-construction
ordering, and the acceptance draw all consume the same run-owned random engine.
Given the same starting manifold, an explicit seed therefore replays the
complete transition sequence, including the technical edge ordering used to
construct a `(6,2)` candidate.
Initialization uses a separate named stream derived from the same root seed, so
changes in point-generation draw counts do not shift the transition sequence.
The CLI, checkpoint metadata, stream ownership, and future parallel policy are
documented in [Reproducible random runs](reproducibility.md).

## State and geometry deltas

Candidates are constructed off to the side and checked against the exact
topology delta before the acceptance calculation. Only an accepted candidate
replaces the canonical manifold. Thus every decision uses the state committed
by the immediately preceding transition.

| Move | `ΔN3` | `ΔN3_31_13` | `ΔN3_22` | `ΔN1_TL` |
| --- | ---: | ---: | ---: | ---: |
| `(2,3)` | `+1` | `0` | `+1` | `+1` |
| `(3,2)` | `-1` | `0` | `-1` | `-1` |
| `(2,6)` | `+4` | `+4` | `0` | `+2` |
| `(6,2)` | `-4` | `-4` | `0` | `-2` |
| `(4,4)` | `0` | `0` | `0` | `0` |

In particular, `(6,2)` removes two `(3,1)` and two `(1,3)` simplices while
leaving `N3_22` unchanged. The runtime invariant check also covers `N2`, total
and classified edges, spacelike edges, vertices, validity, and foliation bounds.

## Counters

The counters obey two identities:

```text
proposed = accepted + rejected
attempted = succeeded + failed = proposed
```

`succeeded` means that candidate construction produced a valid manifold. Such a
candidate can still be rejected by Metropolis-Hastings. `failed` means the raw
site was inapplicable or candidate construction violated an invariant; every
failure is therefore also a rejection.

## Numerical policy

The action, action difference, exponential, proposal ratio, and acceptance
probability remain MPFR values at 256-bit precision. Every MPFR operation uses
round-to-nearest with ties to an even significand (`MPFR_RNDN`). Conversion to
`long double` occurs only for diagnostic output; an acceptance draw is compared
directly with the MPFR probability.

## References

Bibliographic metadata for
[Hastings1970](../REFERENCES.md#metropolis-hastings-algorithm) and
[AmbjornJurkiewiczLoll2001-3D](../REFERENCES.md#three-dimensional-cdt-2001) is
maintained in the repository-wide [`REFERENCES.md`](../REFERENCES.md).
