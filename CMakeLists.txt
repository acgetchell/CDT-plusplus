cmake_minimum_required(VERSION 3.8.0)

project(CDT-plusplus)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}")

#General
###############################################################################
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

#Configuration
###############################################################################
#Turn on / off Catch Tests
option(TESTS ON)
#Turn on / off TBB
set(TBB_ON TRUE)
#Set mimumum Boost
set(BOOST_MIN_VERSION "1.64.0")

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

#Clang - Tidy
if (ENABLE_CLANG_TIDY)
  find_program(CLANG_TIDY_BIN clang-tidy-5.0)
  find_program(RUN_CLANG_TIDY_BIN run-clang-tidy-5.0.py)

  if(CLANG_TIDY_BIN STREQUAL "CLANG_TIDY_BIN-NOTFOUND")
    message(FATAL_ERROR "Unable to locate clang-tidy-5.0")
  endif()

  if(RUN_CLANG_TIDY_BIN STREQUAL "RUN_CLANG_TIDY_BIN-NOTFOUND")
    message(FATAL_ERROR "unable to locate run-clang-tidy-5.0.py")
  endif()

  list(APPEND RUN_CLANG_TIDY_BIN_ARGS
          -clang-tidy-binary ${CLANG_TIDY_BIN}
          -header-filter=.*
          -checks=-*,cert*,clang*,cppcoreguidelines*,misc-assert*,misc-b*,misc-dangling*,misc-f*,misc-i*,misc-move-const*,misc-n*,misc-r*,misc-s*,misc-t*,misc-u*,misc-v*,modernize*,performance*)

  add_custom_target(
          tidy
          COMMAND ${RUN_CLANG_TIDY_BIN} ${RUN_CLANG_TIDY_BIN_ARGS}
          COMMENT "Running Clang-Tidy"
  )
endif ()

#Main program executable
###############################################################################
set(EXECUTABLE_NAME "CDT")

find_package(CGAL QUIET COMPONENTS Core )

#Need working version of boost::optional
#set(Boost_DEBUG FALSE)
#find_package(Boost ${BOOST_MIN_VERSION} REQUIRED)
##Boost debugging
#message(STATUS "Found Boost include: ${Boost_INCLUDE_DIR}")
#message(STATUS "Boost libraries to link: ${Boost_LIBRARIES}")
#message(STATUS "Found Boost libraries: ${Boost_LIBRARY_DIRS}")
#if (NOT Boost_FOUND)
#message(FATAL_ERROR "Boost >= 1.63.0 required.\n")
#endif()

#Boost
set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREAD ON)
find_package(Boost ${BOOST_MIN_VERSION} REQUIRED)
if (Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIR})
  else()
message(FATAL_ERROR "Boost >= ${BOOST_MIN_VERSION} required.\n")
endif ()

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
    message(FATAL_ERROR "NOTICE: This program requires the Eigen3 library,
                    and will not be compiled.")
  endif()
else()
  message(FATAL_ERROR "This program requires the CGAL library,
                  and will not be compiled.")
endif()

include_directories (BEFORE ${PROJECT_SOURCE_DIR}/include ${PROJECT_SOURCE_DIR}/src/date/include)
add_executable(cdt src/cdt.cpp src/docopt/docopt.cpp src/date/src/tz.cpp)
target_link_libraries(cdt ${CGAL_LIBRARIES} ${CGAL_3RD_PARTY_LIBRARIES} curl)

if (UNIX)
  create_single_source_cgal_program("src/cdt-gv.cpp" "src/docopt/docopt.cpp")
endif(UNIX)

add_executable(cdt-opt src/cdt-opt.cpp ${PROJECT_SOURCE_DIR}/src/date/src/tz.cpp)
target_link_libraries(cdt-opt ${CGAL_LIBRARIES} ${CGAL_3RD_PARTY_LIBRARIES} curl)
#Catch
enable_testing()
add_subdirectory(test)
#CTest basic testing
include(CTest)

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
