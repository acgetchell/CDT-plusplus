# 4D Standard-CDT Candidate Validation

This implementation treats `cdt -d4` as a standard-CDT candidate only when the
state passes the validator in `Foliated_triangulation_4.hpp`. The validator
requires periodic time, closed `S3` spatial-slice metadata, non-negative simplex
counts, non-negative proposal multiplicities, valid reciprocal gluing for
explicit combinatorial simplices, and exact agreement between `N4` and the sum
of `N41`, `N32`, `N23`, and `N14`.

The action convention follows the usual 4D CDT bare-coupling form

```text
S_E = -(kappa_0 + 6 Delta) N0
      + kappa_4 N4
      + Delta (2 N41_total + N32_total)
      + epsilon (N4 - target_N4)^2
```

where `N41_total = N41 + N14` and `N32_total = N32 + N23`.
The numerical setup follows the standard CDT requirements that Monte Carlo
moves preserve fixed topology, causality and detailed balance; see the CDT
transfer-matrix lecture notes for the seven-move 4D setup and for the
`cos^3` de Sitter spatial-volume profile and `N4^(1/4)`, `N4^(3/4)` scaling.

Sources:

- https://indico.tpi.uni-jena.de/event/76/attachments/38/132/Goerlich_-_Causal_Dynamical_Triangulations.pdf
- https://www.scholarpedia.org/article/Causal_Dynamical_Triangulation

## Move Catalogue

The source of truth is `Move_catalog_4.hpp`. Tests compare directly against this
table so documentation and runtime behavior cannot silently diverge.

| Move | Inverse | Proposal multiplicity | Delta `(N0,N1,N2,N3,N4,N41,N32,N23,N14)` |
| --- | --- | --- | --- |
| `TWO_FOUR` | `FOUR_TWO` | legal spacelike tetrahedra | `(0,+1,+4,+5,+2,+1,+1,0,0)` |
| `FOUR_TWO` | `TWO_FOUR` | removable timelike edges | `(0,-1,-4,-5,-2,-1,-1,0,0)` |
| `THREE_THREE` | `THREE_THREE` | legal mixed triangles | `(0,0,0,0,0,0,-1,+1,0)` or the time-reversed sign |
| `FOUR_SIX` | `SIX_FOUR` | legal mixed triangles | `(0,+1,+3,+4,+2,0,+1,+1,0)` |
| `SIX_FOUR` | `FOUR_SIX` | removable order-six local stars | `(0,-1,-3,-4,-2,0,-1,-1,0)` |
| `TWO_EIGHT` | `EIGHT_TWO` | legal spacelike tetrahedra | `(+1,+6,+10,+10,+6,+2,+1,+1,+2)` |
| `EIGHT_TWO` | `TWO_EIGHT` | removable spatial vertices | `(-1,-6,-10,-10,-6,-2,-1,-1,-2)` |

## Detailed Balance

For a proposed transition `A -> B`, the sampler uses

```text
acceptance = min(1, exp(-(S(B)-S(A))) * q(B -> A) / q(A -> B))
```

where `q` is computed from the current proposal inventory, not from historical
move frequencies. The enumerable detailed-balance tests build a small reachable
state graph, verify reverse transitions, and compare both sides of

```text
pi(A) q(A -> B) A(A -> B) = pi(B) q(B -> A) A(B -> A)
```

with `pi(T) = exp(-S(T))`.

## Phase Diagnostics

The single-run summary reports the conservative profile diagnostic for the
measurements available in that run. A full `c_ds_supported` finite-size claim
uses `diagnose_c_ds_finite_size()` and requires all of the following:

- centered ensemble profile is better fit by `cos^3` than collapsed or
  alternating-slice alternatives;
- finite-size width scales as `N4^(1/4)`;
- finite-size peak volume scales as `N4^(3/4)`;
- covariance/effective-action extraction has enough decorrelated samples;
- C_b diagnostics are not triggered.

Synthetic tests exercise the analysis layer. Production claims still require
real independent chains and decorrelated measurements written under
`results/<run-id>/`.
