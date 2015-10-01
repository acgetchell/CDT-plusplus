# Created by the script cgal_create_cmake_script
# This is the CMake script for compiling a CGAL application.


project( CDT-plusplus_ )

cmake_minimum_required(VERSION 2.6.2)
if("${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}" VERSION_GREATER 2.6)
  if("${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}.${CMAKE_PATCH_VERSION}" VERSION_GREATER 2.8.3)
    cmake_policy(VERSION 2.8.4)
  else()
    cmake_policy(VERSION 2.6)
  endif()
endif()

# General
###############################################################################
# Activate C++14 support
add_definitions(-std=c++1y)

# Extra warnings
#add_definitions(-Wextra)

# Turn on/off Google Mock Tests
###############################################################################
set(GMOCK_TESTS FALSE)

# Main program executable
###############################################################################
set(EXECUTABLE_NAME "CDT")

find_package(CGAL QUIET COMPONENTS Core )

if ( CGAL_FOUND )

  include( ${CGAL_USE_FILE} )

    # Experimental TBB support
    # TBB 4.3 compatibility fixed in CGAL 4.5.1
    find_package( TBB )
    if (TBB_FOUND)
      include( ${TBB_USE_FILE})

      list(APPEND CGAL_3RD_PARTY_LIBRARIES ${TBB_LIBRARIES})
      MESSAGE(${TBB_LIBRARIES})
    endif()

  include( CGAL_CreateSingleSourceCGALProgram )

  find_package(Eigen3)
  if (EIGEN3_FOUND)
      include( ${EIGEN3_USE_FILE})

      include_directories (BEFORE "../../include")

      create_single_source_cgal_program( "src/parallel_insertion_in_delaunay_3.cpp" )
      # create_single_source_cgal_program( "src/container_test.cpp" )
      create_single_source_cgal_program("src/cdt-gv.cpp" "src/docopt/docopt.cpp")
      create_single_source_cgal_program( "src/cdt.cpp" "src/docopt/docopt.cpp")

  else()

    message(STATUS "NOTICE: This program requires the Eigen3 library, and will not be compiled.")

  endif()

else()

    message(STATUS "This program requires the CGAL library, and will not be compiled.")

endif()

# GMock Tests executable
###############################################################################
if (GMOCK_TESTS)
  set(GMOCK_HOME "$ENV{GMOCK_HOME}")

  include_directories("${GMOCK_HOME}/include")
  include_directories("${GMOCK_HOME}/gtest/include")
  link_directories("${GMOCK_HOME}/lib/.libs")
  link_directories("${GMOCK_HOME}/gtest/lib/.libs")

  set(UT_EXECUTABLE_NAME "unittests")

  # Test files
  file(GLOB TEST_FILES "unittests/*.cpp")

  add_executable("${UT_EXECUTABLE_NAME}" ${TEST_FILES} ${SRC_FILES})

  # Set link libraries (order matters)
  target_link_libraries("${UT_EXECUTABLE_NAME}" gmock gtest pthread)

  # Include root directory
  include_directories(BEFORE ".")
  include_directories(BEFORE "src/")

endif()

# CTest basic testing
include( CTest )

# does the usage message work?

add_test (CDT-Usage cdt)
set_tests_properties (CDT-Usage
  PROPERTIES
  PASS_REGULAR_EXPRESSION "Usage:")

# Make an S3

add_test (CDT-S3Runs cdt --s -n6400 -t16 -a1.1 -k2.2 -l3.3)
set_tests_properties (CDT-S3Runs
  PROPERTIES
  PASS_REGULAR_EXPRESSION "Writing to file S")

# Make a T3

# add_test (CDT-T3Runs cdt --toroidal --time 30 -n 6000)
# set_tests_properties (CDT-T3Runs
#   PROPERTIES
#   PASS_REGULAR_EXPRESSION "Writing to file T")

# Dimensions = 3

add_test (CDT-3Donly cdt --s -n6400 -t16 -a1.1 -k2.2 -l3.3 -d4)
set_tests_properties (CDT-3Donly
  PROPERTIES
  PASS_REGULAR_EXPRESSION "Currently, dimensions cannot be higher than 3.")

# 3D Triangle inequality
add_test (CDT-3Dtriangle cdt --s -n6400 -t16 -a0.3 -k2.2 -l3.3 -d3)
set_tests_properties (CDT-3Dtriangle
  PROPERTIES
  PASS_REGULAR_EXPRESSION "Triangle inequalities violated ... Exiting.")
