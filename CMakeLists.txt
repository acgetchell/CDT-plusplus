cmake_minimum_required(VERSION 3.8.2)

project("CDT-plusplus" C CXX)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
  message(FATAL_ERROR "Do not build in-source. Please remove CMakeCache.txt and the CMakeFiles/ directory. Then build out-of-source.")
endif()

############################
##  Modules and scripts   ##
############################

#Standard CMake modules
include(CTest)
include(CMakeDependentOption)

#Coverage
if (ENABLE_COVERAGE)
  set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -g")
  set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -O0")
  set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -fprofile-arcs")
  set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -ftest-coverage")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
endif ()

#Valgrind
set(MEMORYCHECK_COMMAND_OPTIONS "${MEMORYCHECK_COMMAND_OPTIONS} --leak-check=full")
set(MEMORYCHECK_COMMAND_OPTIONS "${MEMORYCHECK_COMMAND_OPTIONS} --track-fds=yes")
set(MEMORYCHECK_COMMAND_OPTIONS "${MEMORYCHECK_COMMAND_OPTIONS} --trace-children=yes")
set(MEMORYCHECK_COMMAND_OPTIONS "${MEMORYCHECK_COMMAND_OPTIONS} --error-exitcode=1")

###############
##  OPTIONS  ##
###############

#Turn on / off Catch Tests
option(TESTS ON)
#Turn on / off TBB
set(TBB_ON TRUE)
#Set mimumum Boost
set(BOOST_MIN_VERSION "1.64.0")

#Activate C++ 17 support
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

#Threads
set(CMAKE_THREAD_PREFER_PTHREAD TRUE)

#Compiler flags
if (CMAKE_COMPILER_IS_GNUCXX)
  set(CMAKE_CXX_FLAGS  "-frounding-math")
endif()

#Turn off CGAL Triangulation Assertions and Postconditions
add_definitions(-DCGAL_TRIANGULATION_NO_ASSERTIONS -DCGAL_TRIANGULATION_NO_POSTCONDITIONS)

#Disable CLion generation of RelWithDebInfo and MinSizeRel to avoid conflicts
#with CGAL_SetupFlags.cmake
set(CMAKE_CONFIGURATION_TYPES "Release" "Debug" CACHE STRING "" FORCE)

#Extra warnings
add_compile_options(-Wall -Wno-comment)

###############
##  Project  ##
###############

set(EXECUTABLE_NAME "CDT")

#Boost
set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREAD ON)
find_package(Boost ${BOOST_MIN_VERSION} REQUIRED)
if (Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIR})
  else()
message(FATAL_ERROR "Boost >= ${BOOST_MIN_VERSION} required.\n")
endif ()

#CGAL
find_package(CGAL QUIET COMPONENTS Core )
if(CGAL_FOUND)
#Don't let CGAL override flags
  set(CGAL_DONT_OVERRIDE_CMAKE_FLAGS TRUE CACHE BOOL "Force CGAL to maintain CMAKE flags")
  include(${CGAL_USE_FILE})
  include(CGAL_CreateSingleSourceCGALProgram)
#TBB
  find_package(TBB)
  if(TBB_FOUND AND TBB_ON)
    include(${TBB_USE_FILE})
    list(APPEND CGAL_3RD_PARTY_LIBRARIES ${TBB_LIBRARIES})
    MESSAGE(${TBB_LIBRARIES})
  endif()
#Eigen
  find_package(Eigen3)
  if(EIGEN3_FOUND)
    include(${EIGEN3_USE_FILE})
  else()
    message(FATAL_ERROR "NOTICE: This program requires the Eigen3 library, and will not be compiled.")
  endif()
else()
  message(FATAL_ERROR "This program requires the CGAL library, and will not be compiled.")
endif()

#Curl
find_package(CURL)
if(!CURL_FOUND)
  message(FATAL_ERROR "Notice: This program requires the Curl library, and will not be compiled.")
endif()

include_directories (BEFORE ${PROJECT_SOURCE_DIR}/include ${PROJECT_SOURCE_DIR}/src/date/include ${CURL_INCLUDE_DIR})
add_executable(cdt src/cdt.cpp src/docopt/docopt.cpp src/date/src/tz.cpp)
target_link_libraries(cdt ${CGAL_LIBRARIES} ${CGAL_3RD_PARTY_LIBRARIES} curl)

if (UNIX)
  create_single_source_cgal_program("src/cdt-gv.cpp" "src/docopt/docopt.cpp")
endif(UNIX)

add_executable(cdt-opt src/cdt-opt.cpp ${PROJECT_SOURCE_DIR}/src/date/src/tz.cpp)
target_link_libraries(cdt-opt ${CGAL_LIBRARIES} ${CGAL_3RD_PARTY_LIBRARIES} ${CURL_LIBRARIES})

###############
##  Tests    ##
###############

#Catch
add_subdirectory(test)

#CTest basic testing

#Does the usage message work ?
add_test (CDT-Usage cdt)
set_tests_properties (CDT-Usage
  PROPERTIES
  PASS_REGULAR_EXPRESSION "Usage:./cdt")

#Dimensions != 4
add_test (CDT-3Donly cdt --s -n640 -t4 -a0.6 -k1.1 -l0.1 -d4 -p10 -c1)
set_tests_properties (CDT-3Donly
  PROPERTIES
  PASS_REGULAR_EXPRESSION "Invalid parameter ... Exiting.")

#Simplices and timeslices >= 2
add_test(CDT-Simplices cdt --s -n1 -t2 -a0.6 -k1.1 -l0.1 -d3 -p10 -c1)
set_tests_properties(CDT-Simplices
        PROPERTIES
        PASS_REGULAR_EXPRESSION "Simplices and timeslices should be greater or equal to 2.")
add_test(CDT-Timeslices cdt --s -n2 -t1 -a0.6 -k1.1 -l0.1 -d3 -p10 -c1)
set_tests_properties(CDT-Timeslices
        PROPERTIES
        PASS_REGULAR_EXPRESSION "Simplices and timeslices should be greater or equal to 2.")

# 3D Triangle inequality
add_test (CDT-3Dtriangle cdt --s -n640 -t4 -a0.3 -k1.1 -l0.1 -d3 -p10 -c1)
set_tests_properties (CDT-3Dtriangle
  PROPERTIES
  PASS_REGULAR_EXPRESSION "Triangle inequalities violated ... Exiting.")

#Minimum run
add_test(CDT-MinimalS3 cdt --s -n2 -t2 -a0.6 -k1.1 -l0.1 -p10 -c1)
set_tests_properties(CDT-MinimalS3
        PROPERTIES
        PASS_REGULAR_EXPRESSION "Writing to file S3")
#Run an S3
add_test(CDT-S3Runs cdt --s -n640 -t4 -a0.6 -k1.1 -l0.1 -p10 -c1)
set_tests_properties(CDT-S3Runs
        PROPERTIES
        PASS_REGULAR_EXPRESSION "Writing to file S3")
