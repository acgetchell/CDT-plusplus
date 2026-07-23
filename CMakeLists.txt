cmake_minimum_required(VERSION 3.21)

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
set(_vcpkg_install_options ${VCPKG_INSTALL_OPTIONS})
list(APPEND _vcpkg_install_options "--no-print-usage")
list(REMOVE_DUPLICATES _vcpkg_install_options)
set(VCPKG_INSTALL_OPTIONS "${_vcpkg_install_options}" CACHE STRING "Additional options passed to vcpkg install" FORCE)
unset(_vcpkg_install_options)

project(
  CDT-plusplus
  VERSION 1.0.0
  DESCRIPTION "Fast Causal Dynamical Triangulations in C++"
  LANGUAGES CXX)

# CMake project versions are numeric, so keep the release-candidate suffix in
# the product version used by executables and release metadata.
set(CDT_VERSION_SUFFIX "-rc2")
set(CDT_VERSION "${PROJECT_VERSION}${CDT_VERSION_SUFFIX}")

# Project settings
include(cmake/StandardProjectSettings.cmake)
configure_file(cmake/Version.hpp.in "${PROJECT_BINARY_DIR}/Version.hpp" @ONLY)

# Prevent in source builds
include(cmake/PreventInSourceBuilds.cmake)

# Link this 'library' to set the c++ standard / compile-time options requested
add_library(project_options INTERFACE)
target_compile_features(project_options INTERFACE cxx_std_23)
target_compile_definitions(
  project_options INTERFACE CDT_BUILD_CONFIGURATION="$<CONFIG>")

if(CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
  option(ENABLE_BUILD_WITH_TIME_TRACE "Enable -ftime-trace to generate time tracing .json files on clang" OFF)
  if(ENABLE_BUILD_WITH_TIME_TRACE)
    target_compile_options(project_options INTERFACE -ftime-trace)
  endif()
endif()

# Link this 'library' to use the warnings specified in CompilerWarnings.cmake
add_library(project_warnings INTERFACE)

# enable cache system
include(cmake/Cache.cmake)

# standard compiler warnings
include(cmake/CompilerWarnings.cmake)
set_project_warnings(project_warnings)

# diagnostic compile and link options
include(cmake/Coverage.cmake)
enable_coverage(project_options)

include(cmake/Sanitizers.cmake)
enable_sanitizers(project_options)

# allow for static analysis options
include(cmake/StaticAnalyzers.cmake)

# Options ##
option(BUILD_SHARED_LIBS "Enable compilation of shared libraries" OFF)
option(ENABLE_TESTING "Enable building of tests" ON)
option(ENABLE_PARALLEL_TRIANGULATION
       "Enable CGAL parallel Delaunay insertion and removal in production targets" OFF)

# Modules and scripts ##
include(CTest)
include(CMakeDependentOption)

# Minimum compiler versions required for C++23 support:
# - MSVC 19.34 (Visual Studio 2022 version 17.4)
# - GCC 12.2
# - AppleClang 15.0 (Xcode 15)
# - Clang 16.0
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS "19.34")
  message(FATAL_ERROR "MSVC 19.34 or higher required for C++23 support")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS "12.2")
  message(FATAL_ERROR "GCC 12.2 or higher required for C++23 support")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS "15.0")
  message(FATAL_ERROR "AppleClang 15.0 (Xcode 15) or higher required for C++23 std::expected support")
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
# Do not let CGAL override project compiler/linker flags during package discovery.
set(CGAL_DONT_OVERRIDE_CMAKE_FLAGS
    TRUE
    CACHE BOOL "Force CGAL to maintain CMAKE flags")
find_package(CGAL CONFIG REQUIRED)

# https://howardhinnant.github.io/date/date.html
find_package(date CONFIG REQUIRED)

# https://github.com/doctest/doctest
find_package(doctest CONFIG REQUIRED)

# https://github.com/fmtlib/fmt
find_package(fmt CONFIG REQUIRED)

# https://github.com/microsoft/GSL
find_package(Microsoft.GSL CONFIG REQUIRED)

# https://www.pcg-random.org
find_path(PCG_INCLUDE_DIRS "pcg_extras.hpp" REQUIRED)
target_include_directories(project_options SYSTEM INTERFACE "${PCG_INCLUDE_DIRS}")

find_package(Boost CONFIG REQUIRED COMPONENTS compat program_options)
target_link_libraries(project_options INTERFACE Boost::compat)

# https://github.com/gabime/spdlog
find_package(spdlog CONFIG REQUIRED)

# https://github.com/intel/tbb
find_package(TBB CONFIG REQUIRED)
include(CGAL_TBB_support)
if(NOT TARGET CGAL::TBB_support)
  message(FATAL_ERROR "CGAL parallel triangulation requires CGAL::TBB_support")
endif()
if(ENABLE_PARALLEL_TRIANGULATION)
  target_link_libraries(project_options INTERFACE CGAL::TBB_support)
endif()

# Header files
include_directories(BEFORE ${PROJECT_BINARY_DIR} ${PROJECT_SOURCE_DIR}/include)

# doctest
if(ENABLE_TESTING)
  enable_testing()
  message(STATUS "Building tests.")
  add_subdirectory(tests)
endif()

add_subdirectory(src)
