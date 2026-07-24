# 2+1D CDT ergodic move audit

This note records the scientific and implementation contract for the five
supported 2+1-dimensional causal dynamical triangulation (CDT) moves. It is the
audit record for issue #106 and distinguishes three separate claims:

1. the local replacement is one of the Lorentzian Monte Carlo moves;
2. CGAL preserves the validity of the combinatorial triangulation data
   structure; and
3. CDT++ independently verifies the stronger causal, foliation, metadata, and
   failure-atomicity invariants.

The primary CDT source is Ambjørn, Jurkiewicz, and Loll, *Dynamically
triangulating Lorentzian quantum gravity*, section 7.1, equations (56)-(58),
with the corresponding f-vector changes in equations (71)-(73). See
@ref cdt-framework-2001 "CDT framework (2001)". The topological background is
@ref pachner-moves "Pachner's theorem", and the implementation primitive
contracts come from the @ref cgal-triangulations "CGAL 6.2 manual".

## Conventions and common contract

A `(p,q)` tetrahedron has `p` vertices on slice `t` and `q` vertices on slice
`t+1`. Every accepted finite tetrahedron must therefore have exactly two
adjacent time values and type `(3,1)`, `(2,2)`, or `(1,3)`. A spacelike
subsimplex has equal endpoint times; a timelike one spans adjacent slices.

Every high-level `do_*_move()` and `propose_*_move()` operation mutates an
owning snapshot. An inapplicable site, a checked CGAL rejection, or a failed
postcondition leaves the source manifold's canonical topology, vertex and cell
metadata, geometry counts, time bounds, foliation parameters, and handle caches
unchanged. A successful move constructs a new `FoliatedTriangulation`, which
rebuilds all handle-bearing caches and scalar geometry from the moved canonical
triangulation.

Raw proposal sites do not flow directly into mutation. Each move has a
move-specific `detail::Applicable*Move` value constructed by a
`detail::prepare_*()` boundary. Construction proves the local topology,
causality, simplex classification, adjacency, and metadata requirements for
that move. The value stores owned point-value locators rather than CGAL handles;
execution resolves handles only at the mutation boundary and reports a
`STALE_CANDIDATE` if the prepared site is no longer present. The applicable
value is then consumed by `detail::execute()`, which does not rediscover the CDT
preconditions and leaves CGAL's checked flip or retriangulation operation as the
remaining fallible effect.

The public high-level result is `MoveResult<Manifold>`, an allocation-free
`std::expected` whose error is `MoveError`. `MoveFailure` distinguishes no raw
candidate, invalid topology, causal invalidity, a stale prepared site, checked
execution failure, post-mutation invariant failure, and an unknown move.
`MoveOutcome` maps those typed failures and successful candidates into command
and Metropolis accounting, keeping an inapplicable proposal distinct from an
execution failure and from a valid Metropolis rejection.

CGAL 6.2 documents that checked three-dimensional flips preserve vertex handles
and invalidate only affected cell handles. CDT++ never carries affected cell
handles in prepared values or past a successful flip. A copied triangulation
has different handles; every applicable-move implementation therefore captures
stable point values and re-resolves vertices, cells, or edges in the owning
candidate immediately before mutation.

None of these moves is required to preserve the Euclidean empty-sphere
(Delaunay) property of the representative coordinates. The scientific state is
a valid causal combinatorial triangulation. `tds().is_valid()` establishes the
CGAL incidence and adjacency contract; `Manifold::is_correct()` adds foliation
and causal cell-metadata checks without rebuilding derived caches. Call
`Manifold::is_correct_with_diagnostics()` to opt into a full derived-cache
comparison outside hot move paths.

## Move-by-move record

| Move | Local cavity and time assignment | Independent delta `(N0, N1_SL, N1_TL, N2, N3_31, N3_22, N3_13, N3)` | Implementation and admissibility |
| --- | --- | --- | --- |
| `(2,3)` | Equation (58): `(3,1) 1345 + (2,2) 2345 -> (3,1) 1234 + (2,2) 1235 + (2,2) 1245`, or its time reflection. The shared triangle `345` is timelike and the new edge `12` is timelike. | `(0, 0, +1, +2, 0, +1, 0, +1)` | A checked `Triangulation_3::flip(facet)` is attempted only from correctly labelled `(2,2)` cells whose neighbor is a correctly labelled `(3,1)` or `(1,3)` cell and whose opposite vertices lie on adjacent slices. CGAL additionally rejects infinite, nonflippable, or geometrically inverted cavities. |
| `(3,2)` | The inverse of equation (58): two `(2,2)` cells and exactly one `(3,1)` or `(1,3)` cell meet at the timelike edge removed by the move. | `(0, 0, -1, -2, 0, -1, 0, -1)` | Before `Triangulation_3::flip(edge)`, CDT++ independently requires three finite incident cells spanning adjacent slices with the exact `2 x (2,2) + 1 x ((3,1) or (1,3))` causal composition. CGAL then rejects hull edges and enforces the total degree-three and geometric flippability contracts. Other degree-three timelike cavities can be combinatorially flippable but causally invalid. |
| `(2,6)` | Equation (56): `(1,3) 1345 + (3,1) 2345` share the spacelike triangle `345`. A new vertex `6` is inserted on the same slice and joined to all five old vertices, producing three tetrahedra above and three below. | `(+1, +3, +2, +8, +2, 0, +2, +4)` | `tds().insert_in_facet()` subdivides the common spacelike facet. The input cells and their metadata must be `(1,3)` and `(3,1)`, the shared vertices must have one time value, and the new star must contain six valid cells. The new point is the facet centroid and receives the facet time. |
| `(6,2)` | The inverse of equation (56): a degree-five vertex has six incident cells, exactly three `(3,1)` and three `(1,3)`, with no `(2,2)` cell. | `(-1, -3, -2, -8, -2, 0, -2, -4)` | On a private copy, a checked timelike edge flip reduces the candidate to degree four; `tds().remove_from_maximal_dimension_simplex()` then applies its documented degree-`dimension+1` removal. Exact finite incidence, causal types, metadata, output counts, and output cell types are checked before publication. |
| `(4,4)` | Equation (57): two `(1,3)` and two `(3,1)` tetrahedra form a diamond. The diagonal of the spatial quadrilateral is exchanged; the move is its own inverse. | `(0, 0, 0, 0, 0, 0, 0, 0)` | The pivot must be a spacelike edge with exactly four finite incident cells, two `(3,1)` and two `(1,3)`, all correctly labelled. A checked TDS facet flip creates the new diagonal, then a checked TDS edge flip removes the old one. The composition is performed on a private copy because the transient geometry can fail a `Triangulation_3` geometric flip even when the final abstract diamond is valid. |

## Independent delta derivations

For `(2,3)`, two tetrahedra sharing one face initially have five vertices, nine
edges, seven faces, and two cells. Replacing the shared face by the dual edge
adds that one timelike edge. The three resulting tetrahedra have nine distinct
faces, so `(N1, N2, N3)` changes by `(+1, +2, +1)`. Equation (58) changes one
`(3,1)` plus one `(2,2)` into one `(3,1)` plus two `(2,2)`.

For `(2,6)`, the inserted vertex is joined to all five old vertices. Its three
edges to the shared spatial triangle are spacelike and its two edges to the
opposite vertices are timelike. Thus `N0` changes by `+1` and `N1` by `+5`.
The local complex changes from two to six tetrahedra. With the boundary fixed,
the Euler relation for the three-ball gives
`delta N2 = delta N1 + delta N3 - delta N0 = 5 + 4 - 1 = 8`. Equation (56)
changes one tetrahedron of each time orientation into three of each, giving
`delta N3_31 = delta N3_13 = +2`. The `(6,2)` changes are the negatives.

For `(4,4)`, the six vertices and the boundary of the four-cell diamond are
unchanged. One spacelike diagonal replaces another, four tetrahedra replace
four tetrahedra, and the two tetrahedra of each time orientation remain two.
The fixed boundary and Euler relation then force every tracked count delta to
zero.

These derivations agree with the source's f-vectors (71)-(73) but do not call
`cdt::ergodic_moves::detail::check_move()` or any production count-delta
helper.

## Verification record

`tests/Ergodic_moves_3_audit_test.cpp` supplies:

- deterministic minimal successful fixtures for all five moves;
- direct enumeration of finite vertices, edges, facets, cells, edge types, and
  tetrahedron types from vertex time labels;
- reciprocal adjacency, Euler, TDS, foliation, time-bound, metadata, and cache
  checks after every successful move;
- canonical point/time/simplex representations independent of CGAL handle
  identity;
- exact inverse round trips for `(2,3)/(3,2)`, `(2,6)/(6,2)`, and `(4,4)`;
- malformed-handle, stale-metadata, wrong-cavity, non-applicable, and empty-state
  rejection checks with canonical failure-atomicity comparisons; and
- valid construction, forbidden default construction, and stale-locator
  behavior for the applicable-move boundary, including structured failure
  classification; and
- replay of every inverse pair with seeds `0`, `1`, `2`, `92`, `106`, and
  `20260721`.

The wrong-cavity `(3,2)` regression deliberately constructs a geometrically
flippable CGAL edge whose incident CDT types are invalid. It verifies that raw
CGAL accepts the flip while CDT++ rejects before mutation, directly testing the
boundary between combinatorial and scientific validity.
