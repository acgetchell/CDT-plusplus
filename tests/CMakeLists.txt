add_executable(
  CDT_unit_tests
  ${PROJECT_SOURCE_DIR}/tests/main.cpp
  Apply_move_test.cpp
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
# Activate C++20 features
target_compile_features(CDT_unit_tests PRIVATE cxx_std_20)
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
