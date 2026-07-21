add_executable(
  CDT_unit_tests
  ${PROJECT_SOURCE_DIR}/tests/main.cpp
  Apply_move_test.cpp
  Bistellar_flip_test.cpp
  Ergodic_moves_3_audit_test.cpp
  Ergodic_moves_3_test.cpp
  Foliated_triangulation_test.cpp
  Function_ref_test.cpp
  Geometry_test.cpp
  Manifold_test.cpp
  Metropolis_test.cpp
  Move_always_test.cpp
  Move_command_test.cpp
  Move_tracker_test.cpp
  Random_test.cpp
  Runtime_config_test.cpp
  S3Action_test.cpp
  Settings_test.cpp
  Tetrahedron_test.cpp
  Torus_test.cpp
  Utilities_test.cpp
  Vertex_test.cpp)
# Activate C++23 features
target_compile_features(CDT_unit_tests PRIVATE cxx_std_23)
target_link_libraries(
  CDT_unit_tests
  PRIVATE project_options
          project_warnings
          date::date-tz
          doctest::doctest
          fmt::fmt-header-only
          TBB::tbb
          CGAL::CGAL)

# Compile Random.hpp through only the repository-owned project boundary so PCG
# cannot remain available solely through another vcpkg target's include path.
add_library(CDT_random_header_compile_contract OBJECT Random_header_consumer.cpp)
target_link_libraries(
  CDT_random_header_compile_contract
  PRIVATE project_options project_warnings)

# Keep the issue #105 before/after diagnostic buildable without registering it
# as a correctness test. Run it through `just benchmark-rng`.
add_executable(CDT_rng_benchmark Random_benchmark.cpp)
target_compile_features(CDT_rng_benchmark PRIVATE cxx_std_23)
target_link_libraries(
  CDT_rng_benchmark
  PRIVATE project_options
          project_warnings
          date::date-tz
          fmt::fmt-header-only
          spdlog::spdlog_header_only)

# Run unit tests
add_test(NAME cdt-unit-tests COMMAND $<TARGET_FILE:CDT_unit_tests>)
set(cdt_unit_test_timeout 180)
if(ENABLE_SANITIZER_ADDRESS)
  # CGAL-heavy unit tests routinely need 12-19 minutes under ASan on hosted runners.
  set(cdt_unit_test_timeout 1800)
endif()
set_tests_properties(
  cdt-unit-tests PROPERTIES LABELS "unit" TIMEOUT "${cdt_unit_test_timeout}")
