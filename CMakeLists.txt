cmake_minimum_required(VERSION 4.4.0)

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
set(CDT_SOURCE_REVISION "unknown")
find_package(Git QUIET)
if(GIT_FOUND)
  execute_process(
    COMMAND "${GIT_EXECUTABLE}" rev-parse HEAD
    WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
    RESULT_VARIABLE _git_revision_result
    OUTPUT_VARIABLE _git_revision
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET)
  if(_git_revision_result EQUAL 0)
    set(CDT_SOURCE_REVISION "${_git_revision}")
    execute_process(
      COMMAND "${GIT_EXECUTABLE}" status --porcelain --untracked-files=no
      WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
      RESULT_VARIABLE _git_status_result
      OUTPUT_VARIABLE _git_status
      OUTPUT_STRIP_TRAILING_WHITESPACE
      ERROR_QUIET)
    if(_git_status_result EQUAL 0 AND NOT _git_status STREQUAL "")
      string(APPEND CDT_SOURCE_REVISION "-dirty")
    endif()
  endif()
  unset(_git_revision)
  unset(_git_revision_result)
  unset(_git_status)
  unset(_git_status_result)
endif()

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
option(ENABLE_DEPRECATION_ERRORS
       "Treat deprecated C++ and CGAL API use in project targets as an error" ON)

if(ENABLE_PARALLEL_TRIANGULATION AND ENABLE_SANITIZER_THREAD)
  message(
    FATAL_ERROR
      "The pinned CGAL/oneTBB parallel path is not a supported ThreadSanitizer combination. Use the sequential tsan preset and the parallel asan preset."
  )
endif()

target_compile_definitions(
  project_options
  INTERFACE CDT_ENABLE_PARALLEL_TRIANGULATION=$<BOOL:${ENABLE_PARALLEL_TRIANGULATION}>)

if(ENABLE_DEPRECATION_ERRORS)
  if(MSVC)
    target_compile_options(project_options INTERFACE /we4996)
  else()
    target_compile_options(project_options INTERFACE -Werror=deprecated-declarations)
  endif()
endif()

# Modules and scripts ##
include(CTest)
include(CMakeDependentOption)

# Minimum compiler versions required by the intersection of C++23 library
# support and CGAL 6.2's tested platforms:
# - MSVC 19.34 (Visual Studio 2022 version 17.4)
# - GCC 13.3
# - AppleClang 15.0 (Xcode 15)
# - Clang 22.0
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS "19.34")
  message(FATAL_ERROR "MSVC 19.34 or higher required for C++23 support")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS "13.3")
  message(FATAL_ERROR "GCC 13.3 or higher required for CGAL 6.2 support")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS "15.0")
  message(FATAL_ERROR "AppleClang 15.0 (Xcode 15) or higher required for C++23 std::expected support")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS "22.0")
  message(FATAL_ERROR "Clang 22.0 or higher required for CGAL 6.2 support")
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
find_package(CGAL 6.2 EXACT CONFIG REQUIRED)

# https://howardhinnant.github.io/date/date.html
find_package(date CONFIG REQUIRED)

# https://github.com/doctest/doctest
find_package(doctest CONFIG REQUIRED)

# https://github.com/fmtlib/fmt
find_package(fmt CONFIG REQUIRED)

# https://github.com/microsoft/GSL
find_package(Microsoft.GSL CONFIG REQUIRED)
target_link_libraries(project_options INTERFACE Microsoft.GSL::GSL)

# https://www.pcg-random.org
find_path(PCG_INCLUDE_DIRS "pcg_extras.hpp" REQUIRED)
target_include_directories(project_options SYSTEM INTERFACE "${PCG_INCLUDE_DIRS}")

find_package(Boost CONFIG REQUIRED COMPONENTS compat program_options)
target_link_libraries(project_options INTERFACE Boost::compat)

# https://github.com/gabime/spdlog
find_package(spdlog CONFIG REQUIRED)

if(ENABLE_PARALLEL_TRIANGULATION)
  # https://github.com/intel/tbb
  find_package(TBB CONFIG REQUIRED)
  include(CGAL_TBB_support)
  if(NOT TARGET CGAL::TBB_support)
    message(FATAL_ERROR "CGAL parallel triangulation requires CGAL::TBB_support")
  endif()
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
