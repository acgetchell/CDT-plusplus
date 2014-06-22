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

# ################################################################################
# # GTest - http://code.google.com/p/googlemock/
# ################################################################################
# add_subdirectory(gmock-1.7.0)
# enable_testing(true)
# include_directories(${gmock_SOURCE_DIR}/include ${gmock_SOURCE_DIR})

find_package(CGAL QUIET COMPONENTS Core )

if ( CGAL_FOUND )

  include( ${CGAL_USE_FILE} )

  include( CGAL_CreateSingleSourceCGALProgram )

  create_single_source_cgal_program( "cdt.cpp" )
  #create_single_source_cgal_program( "sphere_d.cpp")

else()

    message(STATUS "This program requires the CGAL library, and will not be compiled.")

endif()

# CTest basic testing
include( CTest )

# does the usage message work?

add_test (CDT-Usage cdt)
set_tests_properties (CDT-Usage
  PROPERTIES
  PASS_REGULAR_EXPRESSION "Usage:")

# Make an S3

add_test (CDT-S3Runs cdt --s --time 25 -n 5000)
set_tests_properties (CDT-S3Runs
  PROPERTIES
  PASS_REGULAR_EXPRESSION "Writing to file S")

# Make a T3

add_test (CDT-T3Runs cdt --toroidal --time 30 -n 6000)
set_tests_properties (CDT-T3Runs
  PROPERTIES
  PASS_REGULAR_EXPRESSION "Writing to file T")

# Dimensions = 3

add_test (CDT-3Donly cdt --s --time 25 -n 5000 -d 4)
set_tests_properties (CDT-3Donly
  PROPERTIES
  PASS_REGULAR_EXPRESSION "Currently, number of dimensions cannot be higher than 3.")

# Simplices = 7000

add_test (CDT-Simplices cdt --s --time 35 -n 7000)
set_tests_properties (CDT-Simplices
  PROPERTIES
  PASS_REGULAR_EXPRESSION "Number of simplices = 7000")

# Timeslices = 50
add_test (CDT-Timeslices cdt --p --time 50 -n 8000)
set_tests_properties (CDT-Timeslices
  PROPERTIES
  PASS_REGULAR_EXPRESSION "Number of timeslices = 50")
