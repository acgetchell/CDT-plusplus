cmake_minimum_required(VERSION 3.11)

add_executable(CDT_test ${PROJECT_SOURCE_DIR}/tests/main.cpp FunctionRefTest.cpp UtilitiesTest.cpp VertexTest.cpp
                        TorusTest.cpp TetrahedronTest.cpp SphereTest.cpp SimulationTest.cpp SimplicialManifoldTest.cpp
                        S3TriangulationTest.cpp S3ErgodicMovesTest.cpp S3ActionTest.cpp MoveManagerTest.cpp MoveAlgorithmsTest.cpp
                        MeasurementsTest.cpp)
target_link_libraries(CDT_test CONAN_PKG::catch2 CONAN_PKG::TBB CONAN_PKG::libcurl CONAN_PKG::date CONAN_PKG::gsl_microsoft)
#Activate C++17
target_compile_features(CDT_test PRIVATE cxx_std_17)

# Run unit tests
add_test(NAME CDT-unit-tests COMMAND $<TARGET_FILE:CDT_test>)
