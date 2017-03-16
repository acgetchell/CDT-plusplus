#Created by the script cgal_create_cmake_script
#This is the CMake script for compiling a CGAL application.

project( CDT-plusplus )

cmake_minimum_required(VERSION 3.7.2)

#General
###############################################################################
#Activate C++14 support
#add_definitions(-std =c++1y)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#Disable CLion generation of RelWithDebInfo and MinSizeRel to avoid conflicts
#with CGAL_SetupFlags.cmake
set(CMAKE_CONFIGURATION_TYPES "Release" "Debug" CACHE STRING "" FORCE)

#Extra warnings
add_definitions(-Wall)

###############################################################################
#Turn on / off Google Mock Tests
set(GMOCK_TESTS TRUE)
#Turn on / off TBB
set(TBB_ON TRUE)
#Set mimumum Boost
set(BOOST_MIN_VERSION "1.63.0")

#Main program executable
###############################################################################
set(EXECUTABLE_NAME "CDT")

find_package(CGAL QUIET COMPONENTS Core )

# Need working version of boost::optional
set(Boost_DEBUG TRUE)
find_package(Boost ${BOOST_MIN_VERSION} REQUIRED)
if (NOT Boost_FOUND)
    message(FATAL_ERROR "Boost >= 1.58.0 required.\n")
endif ()

if ( CGAL_FOUND )
  # Don't let CGAL override flags
  set( CGAL_DONT_OVERRIDE_CMAKE_FLAGS TRUE CACHE BOOL "Force CGAL to maintain
   CMAKE flags" )

  include( ${CGAL_USE_FILE} )

#Experimental TBB support
#TBB 4.3 compatibility fixed in CGAL 4.5.1
    find_package( TBB )
    if (TBB_FOUND AND TBB_ON)
      include( ${TBB_USE_FILE})

      list(APPEND CGAL_3RD_PARTY_LIBRARIES ${TBB_LIBRARIES})
      MESSAGE(${TBB_LIBRARIES})
    endif()

  include( CGAL_CreateSingleSourceCGALProgram )

  find_package(Eigen3)
  if (EIGEN3_FOUND)
      include( ${EIGEN3_USE_FILE})

      include_directories (BEFORE "../../include")

#create_single_source_cgal_program("src/container_test.cpp")
      create_single_source_cgal_program("src/cdt-gv.cpp"
                                        "src/docopt/docopt.cpp")
      create_single_source_cgal_program( "src/cdt.cpp" "src/docopt/docopt.cpp")
      create_single_source_cgal_program( "src/cdt-opt.cpp")

  else()

    message(STATUS "NOTICE: This program requires the Eigen3 library,
                    and will not be compiled.")

  endif()

else()

    message(STATUS "This program requires the CGAL library,
                    and will not be compiled.")

endif()

#GMock Tests executable
###############################################################################
if (GMOCK_TESTS)
  
  set(UT_EXECUTABLE_NAME "unittests")

#Test files
  file(GLOB TEST_FILES "unittests/*.cpp")

  add_executable("${UT_EXECUTABLE_NAME}" ${TEST_FILES} ${SRC_FILES})

#Set link libraries(order matters)
  target_link_libraries("${UT_EXECUTABLE_NAME}"
                         ${TBB_LIBRARIES} gmock gtest pthread)

#Include root directory
  include_directories(BEFORE ".")
  include_directories(BEFORE "src/")

endif()

#CTest basic testing
include( CTest )

#does the usage message work ?

add_test (CDT-Usage cdt)
set_tests_properties (CDT-Usage
  PROPERTIES
  PASS_REGULAR_EXPRESSION "Usage:./cdt")

#Run an         S3
#Disabled until Metropolis() fixed

#add_test(CDT - S3Runs cdt-- s - n6400 - t16 - a1 .1 - k2 .2 - l3 .3)
#set_tests_properties(CDT - S3Runs
#PROPERTIES
#PASS_REGULAR_EXPRESSION "Writing to file S")

#Make a T3

#add_test(CDT - T3Runs cdt-- toroidal-- time 30 - n 6000)
#set_tests_properties(CDT - T3Runs
#PROPERTIES
#PASS_REGULAR_EXPRESSION "Writing to file T")

#Dimensions = 3

add_test (CDT-3Donly cdt --s -n6400 -t16 -a1.1 -k2.2 -l3.3 -d4)
set_tests_properties (CDT-3Donly
  PROPERTIES
  PASS_REGULAR_EXPRESSION "Invalid parameter ... Exiting.")

# 3D Triangle inequality
add_test (CDT-3Dtriangle cdt --s -n6400 -t16 -a0.3 -k2.2 -l3.3 -d3)
set_tests_properties (CDT-3Dtriangle
  PROPERTIES
  PASS_REGULAR_EXPRESSION "Triangle inequalities violated ... Exiting.")

#cdt-gv
#This program takes a file to load into GemoView for visualization
add_test(CDT-GeomView cdt-gv)
set_tests_properties(CDT-GeomView
        PROPERTIES
        PASS_REGULAR_EXPRESSION "Usage:./cdt-gv --file FILE")

# This takes a long time now so cancel
#cdt-opt
#add_test(CDT-Optimizer cdt-opt)
