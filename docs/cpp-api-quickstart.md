# C++ API quickstart

This is the canonical end-to-end example for the supported C++23 API. Its
source is built as `CDT_cpp_api_quickstart` in every normal CMake build and run
by CTest when `ENABLE_TESTING` is enabled, so the documentation cannot drift
silently away from the public headers.

## Workflow

The example validates raw configuration values before constructing a manifold,
splits a fixed root seed into named initialization and transition streams,
asks the strategy to sample and resolve ten proposals through the public
one-transition Metropolis API. Every proposal prints its move kind, whether
candidate construction and validation succeeded, and whether the
Metropolis-Hastings criterion accepted and committed it. The existing strategy
summary then reports the aggregate proposal and candidate counters.

“Successful” and “accepted” describe different stages. A successful proposal
produced a valid candidate; it may still be rejected by Metropolis-Hastings. An
accepted proposal is necessarily successful and replaces the canonical state.
The one-argument `attempt_transition()` overload returns both facts in a typed
transition report, so callers do not need to infer an individual outcome from
aggregate counters.

Finally, the example writes the resulting triangulation with a provenance
sidecar. Reading the payload back verifies the sidecar checksum, recorded
geometry, and topology fingerprints before the example compares the finite
simplex counts. Because the one-transition API does not complete a configured
move pass, the artifact records zero completed passes and a transition-trace
count of ten. Its optional `configured_attempts=10` field identifies the
explicit one-step execution plan independently of the pass cadence retained by
the strategy.

After Pachner evolution, the canonical state is required to preserve the CGAL
triangulation data structure and CDT causal manifold invariants. A bistellar
flip is not required to preserve CGAL's geometric Delaunay predicate, so the
round-trip check deliberately tests `tds().is_valid()` rather than
`Delaunay_t<3>::is_valid()`.

The strategy owns the named transition stream and draws each move kind,
acceptance trial, and candidate site in sequence. This preserves single
ownership of the stream and makes the complete sequence replayable from the
recorded root seed.

\include cpp_api_quickstart.cpp

## Build and run

From the repository root:

```bash
cmake --preset reference
cmake --build --preset reference --target CDT_cpp_api_quickstart
./out/build/reference/examples/CDT_cpp_api_quickstart /tmp/cdt-quickstart.off
ctest --test-dir out/build/reference -R '^cpp-api-quickstart$' --output-on-failure
```

The program accepts zero or one argument. Without an argument it writes
`cdt-quickstart.off` and `cdt-quickstart.off.meta` in the current directory.
With an argument, the sidecar is written next to that payload by appending
`.meta` to its name.

## Failure and ownership behavior

Configuration, construction, move execution, and persistence failures are
reported on standard error and produce a nonzero exit status. A caller-owned
random stream may already have advanced when a later operation throws; replay
therefore starts from the recorded root seed and named stream, not from an
engine object retained after failure.

CGAL handles and facet or edge descriptors borrow from the exact triangulation
that produced them. Do not use them with a copied triangulation or after an
invalidating topology mutation. `delaunay_snapshot()` instead returns an owning,
detached triangulation suitable for persistence or transfer across an ownership
boundary. See the [multithreaded CGAL contract](multithreading.md) for the full
lifetime and synchronization policy.
