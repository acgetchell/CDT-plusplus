cmake_minimum_required(VERSION 3.16)

# vcpkg settings must be set before project()
if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  set(CMAKE_TOOLCHAIN_FILE
      "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
      CACHE STRING "")
endif()

if(POLICY CMP0167)
  cmake_policy(SET CMP0167 NEW)
endif ()

# In Manifest mode CMake invokes vcpkg automatically This makes setup easier, however, in CI or Docker we may want to
# turn this off
option(VCPKG_MANIFEST_MODE "Build vcpkg ports from manifest" ON)

# Ensure we install vcpkg ports in the same place so they can be reused between builds
set(_VCPKG_INSTALLED_DIR
    "${CMAKE_CURRENT_LIST_DIR}/vcpkg_installed"
    CACHE STRING "")

project(
  CDT-plusplus
  VERSION 0.1.8
  DESCRIPTION "Fast Causal Dynamical Triangulations in C++"
  LANGUAGES CXX)

execute_process(
  COMMAND git rev-parse --short HEAD
  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
  OUTPUT_VARIABLE CDT_GIT_COMMIT
  OUTPUT_STRIP_TRAILING_WHITESPACE
  ERROR_QUIET)
if(NOT CDT_GIT_COMMIT)
  set(CDT_GIT_COMMIT "unknown")
endif()
add_compile_definitions(CDT_GIT_COMMIT="${CDT_GIT_COMMIT}")

# Project settings
include(cmake/StandardProjectSettings.cmake)

# Prevent in source builds
include(cmake/PreventInSourceBuilds.cmake)

# Link this 'library' to set the c++ standard / compile-time options requested
add_library(project_options INTERFACE)
target_compile_features(project_options INTERFACE cxx_std_23)

if(CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
  option(ENABLE_BUILD_WITH_TIME_TRACE "Enable -ftime-trace to generate time tracing .json files on clang" OFF)
  if(ENABLE_BUILD_WITH_TIME_TRACE)
    add_compile_definitions(project_options INTERFACE -ftime-trace)
  endif()
endif()

# Link this 'library' to use the warnings specified in CompilerWarnings.cmake
add_library(project_warnings INTERFACE)

# enable cache system
include(cmake/Cache.cmake)

# standard compiler warnings
include(cmake/CompilerWarnings.cmake)
set_project_warnings(project_warnings)

# sanitizer options if supported by compiler
include(cmake/Sanitizers.cmake)
enable_sanitizers(project_options)

# allow for static analysis options
include(cmake/StaticAnalyzers.cmake)

# Options ##
option(BUILD_SHARED_LIBS "Enable compilation of shared libraries" OFF)
option(ENABLE_TESTING "Enable building of tests" ON)

# Modules and scripts ##
include(CTest)
include(CMakeDependentOption)

# Set CGAL_DATA_DIR to the location of the CGAL data files
set(ENV{CGAL_DATA_DIR} CMAKE_BINARY_DIR/Data)

# Minimum compiler versions required for C++23 support:
# - MSVC 19.34 (Visual Studio 2022 version 17.4)
# - GCC 12.2
# - AppleClang 14.0
# - Clang 16.0
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS "19.34")
  message(FATAL_ERROR "MSVC 19.34 or higher required for C++23 support")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS "12.2")
  message(FATAL_ERROR "GCC 12.2 or higher required for C++23 support")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS "14.0")
  message(FATAL_ERROR "AppleClang 14.0 or higher required for C++23 support")
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS "16.0")
  message(FATAL_ERROR "Clang 16.0 or higher required for C++23 support")
endif()

# Set NOMINMAX to avoid min/max macro errors on Windows in date.h
#if(WIN32)
#  # Workaround for https://github.com/CGAL/cgal/issues/4665 and https://github.com/microsoft/vcpkg/issues/23572
#  add_compile_options(/DNOMINMAX)
#endif()

# Project vcpkg dependencies

# https://github.com/CGAL/cgal
find_package(CGAL CONFIG REQUIRED)
# Don't let CGAL override flags
set(CGAL_DONT_OVERRIDE_CMAKE_FLAGS
    TRUE
    CACHE BOOL "Force CGAL to maintain CMAKE flags")
set(CGAL_DO_NOT_WARN_ABOUT_CMAKE_BUILD_TYPE TRUE)

# https://howardhinnant.github.io/date/date.html
find_package(date CONFIG REQUIRED)

# https://github.com/doctest/doctest
find_package(doctest CONFIG REQUIRED)

# https://eigen.tuxfamily.org/index.php?title=Main_Page
find_package(Eigen3 CONFIG REQUIRED)
include(CGAL_Eigen3_support)
if(NOT TARGET CGAL::Eigen3_support)
  message(FATAL_ERROR "CGAL Eigen3 support is required for Epick_d-based 4D triangulations.")
endif()
target_link_libraries(project_options INTERFACE CGAL::Eigen3_support)

# https://github.com/fmtlib/fmt
find_package(fmt CONFIG REQUIRED)

# https://github.com/microsoft/GSL
find_package(Microsoft.GSL CONFIG REQUIRED)

# https://www.pcg-random.org
find_path(PCG_INCLUDE_DIRS "pcg_extras.hpp")

find_package(Boost CONFIG REQUIRED COMPONENTS program_options)

# https://github.com/gabime/spdlog
find_package(spdlog CONFIG REQUIRED)

# https://github.com/intel/tbb
find_package(TBB CONFIG REQUIRED)

# https://github.com/TartanLlama/function_ref
find_package(tl-function-ref CONFIG REQUIRED)

# Header files
include_directories(BEFORE ${PROJECT_SOURCE_DIR}/include)

# doctest
if(ENABLE_TESTING)
  enable_testing()
  message(STATUS "Building tests. Look at /tests for unit tests.")
  message(STATUS "Look at /tests/logs for spdlog results from unit tests.")
  message(NOTICE "These logs can get quite big in DEBUG mode if you run all tests.")
  add_subdirectory(tests)
endif()

add_subdirectory(src)
