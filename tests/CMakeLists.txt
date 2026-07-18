add_executable(
  CDT_unit_tests
  ${PROJECT_SOURCE_DIR}/tests/main.cpp
  Apply_move_test.cpp
  Bistellar_flip_test.cpp
  Ergodic_moves_3_test.cpp
  Foliated_triangulation_test.cpp
  Function_ref_test.cpp
  Geometry_test.cpp
  Manifold_test.cpp
  Metropolis_test.cpp
  Move_always_test.cpp
  Move_command_test.cpp
  Move_tracker_test.cpp
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
          fmt::fmt-header-only
          TBB::tbb
          CGAL::CGAL)

# Run unit tests
add_test(NAME cdt-unit-tests COMMAND $<TARGET_FILE:CDT_unit_tests>)

# Keep deterministic regression-oracle coverage in the supported smoke suite.
add_test(
  NAME cdt-function-ref-tests
  COMMAND $<TARGET_FILE:CDT_unit_tests> --test-suite=function_ref)
add_test(
  NAME cdt-foliated-triangulation-tests
  COMMAND $<TARGET_FILE:CDT_unit_tests> --test-suite=foliated_triangulation)
set_tests_properties(
  cdt-function-ref-tests
  cdt-foliated-triangulation-tests
  PROPERTIES TIMEOUT 60)
