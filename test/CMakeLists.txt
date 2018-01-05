cmake_minimum_required(VERSION 3.8.2)

if (TESTS)

include_directories(${PROJECT_SOURCE_DIR}/src)

#file(GLOB_RECURSE TEST_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
#add_executable(CDT_test ${TEST_SOURCE} ${SRC_FILES} ../src/date/src/tz.cpp)

add_executable(CDT_test main.cpp ${SRC_FILES} ../src/date/src/tz.cpp FunctionRefTest.cpp UtilitiesTest.cpp VertexTest.cpp
                        TorusTest.cpp TetrahedronTest.cpp SphereTest.cpp SimulationTest.cpp SimplicialManifoldTest.cpp
                        S3TriangulationTest.cpp S3ErgodicMovesTest.cpp S3ActionTest.cpp MoveManagerTest.cpp MoveAlgorithmsTest.cpp
                        MeasurementsTest.cpp)
target_link_libraries(CDT_test ${TBB_LIBRARIES} curl)

# Run unit tests
add_test(NAME CDT-unit-tests COMMAND $<TARGET_FILE:CDT_test>)

endif ()
