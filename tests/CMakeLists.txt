add_executable(
  CDT_unit_tests
  ${PROJECT_SOURCE_DIR}/tests/main.cpp
  Apply_move_test.cpp
  Bistellar_flip_test.cpp
  CGAL_integration_test.cpp
  Ergodic_moves_3_audit_test.cpp
  Ergodic_moves_3_test.cpp
  Foliated_triangulation_test.cpp
  Function_ref_test.cpp
  Geometry_test.cpp
  Manifold_test.cpp
  Metropolis_test.cpp
  Move_always_test.cpp
  Move_command_test.cpp
  Move_outcome_test.cpp
  Move_run_test.cpp
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
          CGAL::CGAL)

if(ENABLE_PARALLEL_TRIANGULATION)
  add_executable(CDT_parallel_triangulation_test
                 ${PROJECT_SOURCE_DIR}/tests/main.cpp
                 Parallel_triangulation_test.cpp)
  target_compile_features(CDT_parallel_triangulation_test PRIVATE cxx_std_23)
  target_link_libraries(
    CDT_parallel_triangulation_test
    PRIVATE project_options
            project_warnings
            doctest::doctest
            fmt::fmt-header-only
            CGAL::CGAL
            CGAL::TBB_support)
endif()

# Compile Random.hpp through only the repository-owned project boundary so PCG
# cannot remain available solely through another vcpkg target's include path.
add_library(CDT_random_header_compile_contract OBJECT Random_header_consumer.cpp)
target_link_libraries(
  CDT_random_header_compile_contract
  PRIVATE project_options project_warnings)

# Compile a minimal downstream translation unit without namespace imports or
# test-only access to implementation helpers.
add_library(CDT_public_api_compile_contract OBJECT Public_api_consumer.cpp)
target_link_libraries(
  CDT_public_api_compile_contract
  PRIVATE project_options
          project_warnings
          date::date-tz
          fmt::fmt-header-only
          spdlog::spdlog_header_only
          CGAL::CGAL)

# Compile every supported header as the first include in an independent
# translation unit. Experimental archival headers are not part of this target.
# This catches include-order dependencies and header-local ODR/linkage mistakes
# at the downstream boundary.
set(
  cdt_supported_headers
  Apply_move.hpp
  Ergodic_moves_3.hpp
  Foliated_triangulation.hpp
  Formatters.hpp
  Geometry.hpp
  Manifold.hpp
  Metropolis.hpp
  Move_always.hpp
  Move_command.hpp
  Move_outcome.hpp
  Move_run.hpp
  Move_strategy.hpp
  Move_tracker.hpp
  Mpfr_value.hpp
  Random.hpp
  Runtime_config.hpp
  S3Action.hpp
  Settings.hpp
  Triangulation_traits.hpp
  Utilities.hpp)
set(cdt_header_contract_sources)
foreach(header IN LISTS cdt_supported_headers)
  string(MAKE_C_IDENTIFIER "${header}" header_id)
  set(source "${CMAKE_CURRENT_BINARY_DIR}/header_${header_id}.cpp")
  file(GENERATE OUTPUT "${source}" CONTENT "#include \"${header}\"\n")
  list(APPEND cdt_header_contract_sources "${source}")
endforeach()
set(cdt_header_contract_main
    "${CMAKE_CURRENT_BINARY_DIR}/header_contract_main.cpp")
file(GENERATE OUTPUT "${cdt_header_contract_main}"
     CONTENT "int main() { return 0; }\n")
add_executable(CDT_header_compile_contracts ${cdt_header_contract_sources}
                                            "${cdt_header_contract_main}")
target_link_libraries(
  CDT_header_compile_contracts
  PRIVATE project_options
          project_warnings
          date::date-tz
          fmt::fmt-header-only
          spdlog::spdlog_header_only
          CGAL::CGAL)

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

# Keep the issue #102 integration benchmark buildable without registering it
# as a correctness or performance test. Run it through `just benchmark-cgal`.
add_executable(CDT_cgal_benchmark CGAL_benchmark.cpp)
target_compile_features(CDT_cgal_benchmark PRIVATE cxx_std_23)
target_link_libraries(
  CDT_cgal_benchmark
  PRIVATE project_options
          project_warnings
          date::date-tz
          fmt::fmt-header-only
          spdlog::spdlog_header_only
          CGAL::CGAL)

# Run unit tests
add_test(NAME cdt-unit-tests COMMAND $<TARGET_FILE:CDT_unit_tests>)
if(ENABLE_PARALLEL_TRIANGULATION)
  add_test(NAME cdt-parallel-triangulation
           COMMAND $<TARGET_FILE:CDT_parallel_triangulation_test>)
  set_tests_properties(
    cdt-parallel-triangulation
    PROPERTIES LABELS "unit;parallel;configuration" TIMEOUT 60)
endif()
set(cdt_unit_test_timeout 180)
if(ENABLE_SANITIZER_ADDRESS)
  # CGAL-heavy unit tests routinely need 12-19 minutes under ASan on hosted runners.
  set(cdt_unit_test_timeout 1800)
elseif(ENABLE_COVERAGE)
  # A per-test timeout overrides `ctest --timeout`; keep coverage aligned with
  # the Codecov workflow's instrumentation-aware allowance.
  set(cdt_unit_test_timeout 600)
endif()
set_tests_properties(
  cdt-unit-tests PROPERTIES LABELS "unit" TIMEOUT "${cdt_unit_test_timeout}")
