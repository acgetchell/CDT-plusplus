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

find_package(CGAL QUIET COMPONENTS Core )

if ( CGAL_FOUND )

  include( ${CGAL_USE_FILE} )

  include( CGAL_CreateSingleSourceCGALProgram )

  create_single_source_cgal_program( "cdt.cpp" )

else()
  
    message(STATUS "This program requires the CGAL library, and will not be compiled.")
  
endif()

include( CTest )

# does the application run?

add_test (CDT-S3Runs cdt -s 5000)
set_tests_properties (CDT-S3Runs
  PROPERTIES
  PASS_REGULAR_EXPRESSION "Number of simplices = 5000")
add_test (CDT-T3Runs cdt -p 5000)
set_tests_properties (CDT-T3Runs
  PROPERTIES
  PASS_REGULAR_EXPRESSION "Final triangulation has")

# does the usage message work?

add_test (CDT-Usage cdt)
set_tests_properties (CDT-Usage
  PROPERTIES
  PASS_REGULAR_EXPRESSION "Usage: .*number")

add_test (CDT-3Donly cdt -d 4)
set_tests_properties (CDT-3Donly
  PROPERTIES
  PASS_REGULAR_EXPRESSION "Currently, number of dimensions cannot be higher than 3.")

# set number of time slices
add_test (CDT-Timeslices cdt -s 5000 -the 256)
set_tests_properties (CDT-Timeslices
  PROPERTIES
  PASS_REGULAR_EXPRESSION "Final triangulation has")
