# References and Citations

## How to Cite This Software

If CDT++ contributes to research or a project, cite it using the structured
metadata in
[`CITATION.cff`](https://github.com/acgetchell/CDT-plusplus/blob/main/CITATION.cff).
GitHub and other citation tools can generate BibTeX, APA, and additional
formats from that file.

Quick citation for the current declared release:

```text
Adam Getchell. 2026. CDT-plusplus: Causal Dynamical Triangulations in C++.
Version 1.0.0. GitHub. https://github.com/acgetchell/CDT-plusplus
Zenodo concept DOI: https://doi.org/10.5281/zenodo.21487043
```

The sections below are grouped by scientific domain. Cite the entries relevant
to the algorithms or results used in a given work. Source documentation links
directly to the applicable entry so that provenance stays close to the
implementation while complete metadata remains centralized here.

## Foundational Causal Dynamical Triangulations Theory

### Original CDT framework

#### CDT framework (2001)

J. Ambjørn, J. Jurkiewicz, and R. Loll, “Dynamically triangulating Lorentzian
quantum gravity,” *Nuclear Physics B* 610, no. 1–2 (2001), 347–382.
DOI: [10.1016/S0550-3213(01)00297-8](<https://doi.org/10.1016/S0550-3213(01)00297-8>)

#### Three-dimensional CDT (2001)

J. Ambjørn, J. Jurkiewicz, and R. Loll, “Nonperturbative 3D Lorentzian quantum
gravity,” *Physical Review D* 64, no. 4 (2001), 044011.
DOI: [10.1103/PhysRevD.64.044011](https://doi.org/10.1103/PhysRevD.64.044011)

## Monte Carlo Methods

### Metropolis algorithm

N. Metropolis, A. W. Rosenbluth, M. N. Rosenbluth, A. H. Teller, and E. Teller,
“Equation of state calculations by fast computing machines,” *The Journal of
Chemical Physics* 21, no. 6 (1953), 1087–1092.
DOI: [10.1063/1.1699114](https://doi.org/10.1063/1.1699114)

### Metropolis-Hastings algorithm

W. K. Hastings, “Monte Carlo sampling methods using Markov chains and their
applications,” *Biometrika* 57, no. 1 (1970), 97–109.
DOI: [10.1093/biomet/57.1.97](https://doi.org/10.1093/biomet/57.1.97)

## Regge Calculus and Discrete Action

### Regge calculus

T. Regge, “General relativity without coordinates,” *Il Nuovo Cimento* 19,
no. 3 (1961), 558–571.
DOI: [10.1007/BF02733251](https://doi.org/10.1007/BF02733251)

## Simplicial Topology and Local Moves

### Pachner moves

U. Pachner, “P.L. homeomorphic manifolds are equivalent by elementary
shellings,” *European Journal of Combinatorics* 12, no. 2 (1991), 129–145.
DOI: [10.1016/S0195-6698(13)80080-7](<https://doi.org/10.1016/S0195-6698(13)80080-7>)

## Computational Geometry and Random-Number Generation

### Delaunay empty-sphere construction

B. Delaunay, “Sur la sphère vide. À la mémoire de Georges Voronoï,” *Bulletin
de l’Académie des Sciences de l’URSS. Classe des sciences mathématiques et
naturelles*, no. 6 (1934), 793–800.
[Primary-source scan and bibliographic record](https://www.mathnet.ru/eng/im4937).

### CGAL design

E. Fogel and M. Teillaud, “The computational geometry algorithms library
CGAL,” *ACM Communications in Computer Algebra* 47, no. 3/4 (2014), 85–87.
DOI: [10.1145/2576802.2576806](https://doi.org/10.1145/2576802.2576806)

### CGAL triangulations

The CGAL Project, *CGAL User and Reference Manual*. CGAL Editorial Board, 6.2
edition (2026). <https://doc.cgal.org/6.2/Manual/packages.html>

### Robust geometric predicates

J. R. Shewchuk, “Adaptive precision floating-point arithmetic and fast robust
geometric predicates,” *Discrete & Computational Geometry* 18, no. 3 (1997),
305–363.
DOI: [10.1007/PL00009321](https://doi.org/10.1007/PL00009321)

This paper provides primary background for adaptive robust-predicate
methodology. CDT++ delegates its production predicates to CGAL's EPICK kernel;
it does not claim to reimplement Shewchuk's predicate code.

### Random Voronoi and Delaunay expected complexity

R. A. Dwyer, “Higher-dimensional Voronoi diagrams in linear expected time,”
*Discrete & Computational Geometry* 6 (1991), 343–367.
DOI: [10.1007/BF02574694](https://doi.org/10.1007/BF02574694)

### Three-dimensional Delaunay complexity

S.-W. Cheng, T. K. Dey, and J. R. Shewchuk, *Delaunay Mesh Generation*,
Chapter 4, “Three-dimensional Delaunay triangulation.” CRC Press (2013),
ISBN 978-1-58488-730-0. DOI metadata issued 2016: `10.1201/b12987`;
see the [publisher record](https://www.taylorfrancis.com/books/9781584887317).
The authors’ [Chapter 4
preprint](<https://www.cs.purdue.edu/homes/tamaldey/course/531/Delaunay%283D%29.pdf>)
states the finite three-dimensional tetrahedron bound used by CDT++.

### PCG random-number generators

M. E. O’Neill, “PCG: A family of simple fast space-efficient statistically good
algorithms for random number generation,” Harvey Mudd College Computer Science
Department technical report HMC-CS-2014-0905 (2014).
<https://www.pcg-random.org/paper.html>
