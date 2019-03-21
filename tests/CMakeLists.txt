cmake_minimum_required(VERSION 3.11)

add_executable(CDT_test
               ${PROJECT_SOURCE_DIR}/tests/main.cpp
               Function_ref_test.cpp
               Utilities_test.cpp
               VertexTest.cpp
               Torus_test.cpp
               Tetrahedron_test.cpp
               SphereTest.cpp
               SimulationTest.cpp
               # SimplicialManifoldTest.cpp
               S3ErgodicMovesTest.cpp S3ActionTest.cpp
               # MoveManagerTest.cpp
               MoveAlgorithmsTest.cpp
               # MeasurementsTest.cpp
               Manifold_test.cpp
               Geometry_test.cpp
               Foliated_triangulation_test.cpp
               Move_command_test.cpp
               Move_guard_test.cpp
               Ergodic_moves_3_test.cpp)
target_link_libraries(CDT_test CONAN_PKG::Catch2
                      # CONAN_PKG::TBB
                      CONAN_PKG::date CONAN_PKG::gsl_microsoft)
# Activate C++17
target_compile_features(CDT_test PRIVATE cxx_std_17)

# Run unit tests
add_test(NAME CDT-unit-tests COMMAND $<TARGET_FILE:CDT_test>)
