cmake_minimum_required(VERSION 3.11)

add_executable(
  CDT_test
  ${PROJECT_SOURCE_DIR}/tests/main.cpp
  Apply_move_test.cpp
  Ergodic_moves_3_test.cpp
  Foliated_triangulation_test.cpp
  Geometry_test.cpp
  Manifold_test.cpp
  Move_command_test.cpp
  # MoveManagerTest.cpp
  S3ActionTest.cpp
  SphereTest.cpp
  Tetrahedron_test.cpp
  Torus_test.cpp
  Utilities_test.cpp
  VertexTest.cpp)

target_link_libraries(CDT_test Catch2::Catch2 CGAL::CGAL)

# Activate C++17
target_compile_features(CDT_test PRIVATE cxx_std_17)

# Run unit tests
add_test(NAME CDT-unit-tests COMMAND $<TARGET_FILE:CDT_test>)
