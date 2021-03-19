cmake_minimum_required(VERSION 3.12...3.19)

# vcpkg settings must be set before project()
if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
      CACHE STRING "")
endif()

# Stops CMake from invoking vcpkg automatically
option(VCPKG_MANIFEST_MODE "Build vcpkg ports from manifest" OFF)

project(
  CDT-plusplus
  VERSION 0.1.8
  LANGUAGES CXX)

# Project settings
include(cmake/StandardProjectSettings.cmake)

# Prevent in source builds
include(cmake/PreventInSourceBuilds.cmake)

# Link this 'library' to set the c++ standard / compile-time options requested
add_library(project_options INTERFACE)
target_compile_features(project_options INTERFACE cxx_std_17)

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

option(BUILD_SHARED_LIBS "Enable compilation of shared libraries" OFF)
option(ENABLE_TESTING "Enable building of tests" ON)

# Modules and scripts ##
include(CTest)
include(CMakeDependentOption)

# Project vcpkg dependencies

# https://github.com/catchorg/Catch2
find_package(Catch2 CONFIG REQUIRED)

# https://github.com/docopt/docopt.cpp
find_package(docopt CONFIG REQUIRED)

# https://howardhinnant.github.io/date/date.html
find_package(date CONFIG REQUIRED)

# https://github.com/fmtlib/fmt
find_package(fmt CONFIG REQUIRED)

# https://www.boost.org
find_package(Boost REQUIRED)

# https://eigen.tuxfamily.org/index.php?title=Main_Page
find_package(Eigen3 CONFIG REQUIRED)

# https://www.pcg-random.org
find_path(PCG_INCLUDE_DIRS "pcg_extras.hpp")

# https://github.com/intel/tbb
find_package(TBB CONFIG REQUIRED)

# https://github.com/CGAL/cgal
find_package(CGAL CONFIG REQUIRED)
# Don't let CGAL override flags
set(CGAL_DONT_OVERRIDE_CMAKE_FLAGS
    TRUE
    CACHE BOOL "Force CGAL to maintain CMAKE flags")
set(CGAL_DO_NOT_WARN_ABOUT_CMAKE_BUILD_TYPE TRUE)

# Header files
include_directories(BEFORE ${PROJECT_SOURCE_DIR}/include ${PCG_INCLUDE_DIRS})

# Catch
if(ENABLE_TESTING)
  enable_testing()
  message(STATUS "Building tests. Check out /tests for unit tests.")
  add_subdirectory(tests)
endif()

add_subdirectory(src)
