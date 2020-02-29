cmake_minimum_required(VERSION 3.12...3.16)

project(
  CDT-plusplus
  VERSION 0.1.8
  LANGUAGES CXX)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
  message(
    FATAL_ERROR
      "Do not build in-source.
                       Please remove CMakeCache.txt and the CMakeFiles/ directory. Then build out-of-source."
  )
endif()

#
# Modules and scripts ##
#
include(CTest)
include(CMakeDependentOption)

# Valgrind
if(VALGRIND)
  message(STATUS "Valgrind enabled.")
  set(CMAKE_VERBOSE_MAKEFILE ON)
  set(CMAKE_BUILD_TYPE Debug)
  add_compile_options(-g -O0 -fsanitize=address)
  set(MEMORYCHECK_COMMAND_OPTIONS
      "${MEMORYCHECK_COMMAND_OPTIONS} --leak-check=full")
  set(MEMORYCHECK_COMMAND_OPTIONS
      "${MEMORYCHECK_COMMAND_OPTIONS} --track-fds=yes")
  set(MEMORYCHECK_COMMAND_OPTIONS
      "${MEMORYCHECK_COMMAND_OPTIONS} --trace-children=yes")
  set(MEMORYCHECK_COMMAND_OPTIONS
      "${MEMORYCHECK_COMMAND_OPTIONS} --error-exitcode=1")
endif()

if(ASAN)
  message(STATUS "ASAN enabled.")
  set(CMAKE_VERBOSE_MAKEFILE ON)
  set(CMAKE_BUILD_TYPE Debug)
  add_compile_options(-g -O1 -fsanitize=address -fno-omit-frame-pointer
                      -fno-optimize-sibling-calls)
endif()

#
# OPTIONS  ##
#

# Set minimum Boost
set(BOOST_MIN_VERSION "1.71.0")

# Compile commands for ClangTidy et. al
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Use C++17 for std::optional
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Turn on / off TBB
set(TBB_ON TRUE)

# Threads
set(CMAKE_THREAD_PREFER_PTHREAD TRUE)

# gcc flags
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  add_compile_options(
    -frounding-math -Wmisleading-indentation -Wduplicated-cond
    -Wduplicated-branches -Wlogical-op -Wuseless-cast)
endif()

# Turn on Visual Studio warnings
if(WIN32)
  if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
    string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
  endif()
else()
  # Extra warnings for non-Visual Studio
  add_compile_options(
    -Wall
    -Wno-comment
    -Wextra
    -Wshadow
    -Wnon-virtual-dtor
    -Wold-style-cast
    -Wcast-align
    -Wunused
    -Woverloaded-virtual
    -Wpedantic
    -Wconversion
    -Wsign-conversion
    -Wnull-dereference
    -Wdouble-promotion
    -Wformat=2
    -Werror=return-type)
endif()
# Turn off CGAL Triangulation Assertions and Postconditions
add_definitions(-DCGAL_TRIANGULATION_NO_ASSERTIONS
                -DCGAL_TRIANGULATION_NO_POSTCONDITIONS)

# Disable CLion generation of MinSizeRel to avoid conflicts with
# CGAL_SetupFlags.cmake
set(CMAKE_CONFIGURATION_TYPES
    "Release" "Debug" "RelWithDebInfo"
    CACHE STRING "" FORCE)

# Easier navigation in an IDE when projects are organized in folders.
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Link time optimization
include(CheckIPOSupported)
check_ipo_supported(RESULT ipo_support)
if(ipo_support)
  set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
else()
  message(WARNING "IPO is not supported: ${output}")
endif()

#
# Project  ##
#

# https://github.com/catchorg/Catch2
find_package(Catch2 CONFIG REQUIRED)

# https://github.com/docopt/docopt.cpp
find_package(docopt CONFIG REQUIRED)

# https://github.com/fmtlib/fmt
find_package(fmt CONFIG REQUIRED)

# https://www.boost.org
find_package(Boost REQUIRED)

# https://eigen.tuxfamily.org/index.php?title=Main_Page
find_package(Eigen3 CONFIG REQUIRED)

# https://github.com/intel/tbb
find_package(TBB CONFIG REQUIRED)

# https://github.com/CGAL/cgal
find_package(CGAL CONFIG REQUIRED)
# Don't let CGAL override flags
set(CGAL_DONT_OVERRIDE_CMAKE_FLAGS
    TRUE
    CACHE BOOL "Force CGAL to maintain CMAKE flags")
set(CGAL_DO_NOT_WARN_ABOUT_CMAKE_BUILD_TYPE TRUE)

include_directories(BEFORE ${PROJECT_SOURCE_DIR}/include
                    ${PROJECT_SOURCE_DIR}/external/pcg-cpp/include)
add_executable(initialize ${PROJECT_SOURCE_DIR}/src/initialize.cpp)

# On macOS and Linux docopt builds an intermediate object, thus different
# targets than Windows See https://github.com/microsoft/vcpkg/issues/8666
if(WIN32)
  target_link_libraries(initialize PRIVATE docopt fmt::fmt-header-only
                                           Eigen3::Eigen TBB::tbb CGAL::CGAL)
else()
  target_link_libraries(
    initialize PRIVATE docopt_s fmt::fmt fmt::fmt-header-only Eigen3::Eigen
                       TBB::tbb CGAL::CGAL)
endif()
target_compile_features(initialize PRIVATE cxx_std_17)

# add_executable(cdt-opt ${PROJECT_SOURCE_DIR}/src/cdt-opt.cpp)
# target_link_libraries( cdt-opt PRIVATE date::tz date::date Catch2::Catch2
# Eigen3::Eigen # TBB::tbb CGAL::CGAL) target_compile_features(cdt-opt PRIVATE
# cxx_std_17)

if(UNIX)
  add_executable(cdt-gv ${PROJECT_SOURCE_DIR}/src/cdt-gv.cpp)
  target_link_libraries(cdt-gv PRIVATE docopt_s CGAL::CGAL)
  target_compile_features(cdt-gv PRIVATE cxx_std_17)
endif()

#
# Tests    ##
#

# Catch
enable_testing()
add_subdirectory(tests)

# CTest basic testing Parameter initializer program
add_test(NAME CDT-Initialize COMMAND $<TARGET_FILE:initialize>)
set_tests_properties(CDT-Initialize PROPERTIES PASS_REGULAR_EXPRESSION
                                               "Usage:./initialize")
if(UNIX)
  add_test(NAME CDT-gv COMMAND $<TARGET_FILE:cdt-gv>)
  set_tests_properties(CDT-gv PROPERTIES PASS_REGULAR_EXPRESSION
                                         "Usage:./cdt-gv")
endif()

# add_test(NAME CDT-Opt COMMAND $<TARGET_FILE:cdt-opt>) set_tests_properties(
# CDT-Opt PROPERTIES PASS_REGULAR_EXPRESSION "64000 simplices on 16 timeslices
# desired.")
