cmake_minimum_required(VERSION 3.5)

include_directories(${PROJECT_SOURCE_DIR}/src)

#file(GLOB_RECURSE TEST_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
#add_executable(CDT_test ${TEST_SOURCE} ${SRC_FILES} ../src/date/src/tz.cpp)

add_executable(CDT_test main.cpp ${SRC_FILES} ../src/date/src/tz.cpp FunctionRef.cpp Utilities.cpp Vertex.cpp Torus.cpp Tetrahedron.cpp Sphere.cpp Simulation.cpp SimplicialManifold.cpp S3Triangulation.cpp S3ErgodicMoves.cpp S3Action.cpp MoveManager.cpp)
target_link_libraries(CDT_test ${TBB_LIBRARIES} curl)

# Run unit tests
add_test(NAME CDT-unit-tests COMMAND $<TARGET_FILE:CDT_test>)
