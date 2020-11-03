cmake_minimum_required(VERSION 3.12...3.18)

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

#
# Modules and scripts ##
#
include(CTest)
include(CMakeDependentOption)

#
# Project  ##
#

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

include_directories(BEFORE ${PROJECT_SOURCE_DIR}/include ${PCG_INCLUDE_DIRS})
add_executable(initialize ${PROJECT_SOURCE_DIR}/src/initialize.cpp)

# On macOS and Linux docopt builds an intermediate object, thus different targets than Windows See
# https://github.com/microsoft/vcpkg/issues/8666
if(WIN32)
  target_link_libraries(
    initialize
    PRIVATE project_options
            project_warnings
            docopt
            date::date
            date::date-tz
            fmt::fmt-header-only
            Eigen3::Eigen
            TBB::tbb
            CGAL::CGAL)
else()
  target_link_libraries(
    initialize
    PRIVATE project_options
            project_warnings
            docopt_s
            date::date
            date::date-tz
            fmt::fmt
            fmt::fmt-header-only
            Eigen3::Eigen
            TBB::tbb
            CGAL::CGAL)
endif()
target_compile_features(initialize PRIVATE cxx_std_17)

# add_executable(cdt-opt ${PROJECT_SOURCE_DIR}/src/cdt-opt.cpp) target_link_libraries( cdt-opt PRIVATE date::tz
# date::date Catch2::Catch2 Eigen3::Eigen # TBB::tbb CGAL::CGAL) target_compile_features(cdt-opt PRIVATE cxx_std_17)

if(UNIX)
  add_executable(cdt-gv ${PROJECT_SOURCE_DIR}/src/cdt-gv.cpp)
  target_link_libraries(
    cdt-gv
    PRIVATE project_options
            project_warnings
            docopt_s
            fmt::fmt-header-only
            CGAL::CGAL)
  target_compile_features(cdt-gv PRIVATE cxx_std_17)
endif()

#
# Tests    ##
#

# Catch
if(ENABLE_TESTING)
  enable_testing()
  message(STATUS "Building tests. Check out /tests for unit tests.")
  add_subdirectory(tests)
endif()

# CTest basic testing Parameter initializer program
add_test(NAME CDT-Initialize COMMAND $<TARGET_FILE:initialize>)
set_tests_properties(CDT-Initialize PROPERTIES PASS_REGULAR_EXPRESSION "Usage:./initialize")
if(UNIX)
  add_test(NAME CDT-gv COMMAND $<TARGET_FILE:cdt-gv>)
  set_tests_properties(CDT-gv PROPERTIES PASS_REGULAR_EXPRESSION "Usage:./cdt-gv")
endif()

# add_test(NAME CDT-Opt COMMAND $<TARGET_FILE:cdt-opt>) set_tests_properties( CDT-Opt PROPERTIES PASS_REGULAR_EXPRESSION
# "64000 simplices on 16 timeslices desired.")
