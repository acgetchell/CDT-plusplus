add_executable(
  CDT_unit_tests
  ${PROJECT_SOURCE_DIR}/tests/main.cpp
  Apply_move_test.cpp
  Bistellar_flip_test.cpp
  Checkpoint_4_test.cpp
  Detailed_balance_test.cpp
  Ergodic_moves_3_test.cpp
  Ergodic_moves_4_test.cpp
  Foliated_triangulation_test.cpp
  Foliated_triangulation_4_test.cpp
  Function_ref_test.cpp
  Geometry_test.cpp
  Geometry_4_test.cpp
  Manifold_test.cpp
  Manifold_4_test.cpp
  Metropolis_test.cpp
  Metropolis_4_test.cpp
  Move_always_test.cpp
  Move_command_test.cpp
  Move_tracker_test.cpp
  Observables_4_test.cpp
  Phase_analysis_test.cpp
  S3Action_test.cpp
  S4Action_test.cpp
  Settings_test.cpp
  Tetrahedron_test.cpp
  Torus_test.cpp
  Time_reversal_4_test.cpp
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
          CGAL::Eigen3_support
          TBB::tbb
          CGAL::CGAL)

# Run unit tests
add_test(NAME cdt-unit-tests COMMAND $<TARGET_FILE:CDT_unit_tests>)
