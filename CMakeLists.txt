cmake_minimum_required(VERSION 3.12...3.17)

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
# OPTIONS  ##
#

# Compiler options and static analyzers
include(cmake/StandardProjectSettings.cmake)
include(cmake/StaticAnalyzers.cmake)

# Link to project_options library to set sanitizer options
add_library(project_options INTERFACE)
target_compile_features(project_options INTERFACE cxx_std_20)
include(cmake/Sanitizers.cmake)
enable_sanitizers(project_options)

# Link to project_warnings library to set compiler warnings
add_library(project_warnings INTERFACE)
include(cmake/CompilerWarnings.cmake)
set_project_warnings(project_warnings)

option(ENABLE_TESTING "Enable building of tests" TRUE)

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
  target_link_libraries(
    initialize
    PRIVATE project_options
            project_warnings
            docopt
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
            fmt::fmt
            fmt::fmt-header-only
            Eigen3::Eigen
            TBB::tbb
            CGAL::CGAL)
endif()
target_compile_features(initialize PRIVATE cxx_std_20)

# add_executable(cdt-opt ${PROJECT_SOURCE_DIR}/src/cdt-opt.cpp)
# target_link_libraries( cdt-opt PRIVATE date::tz date::date Catch2::Catch2
# Eigen3::Eigen # TBB::tbb CGAL::CGAL) target_compile_features(cdt-opt PRIVATE
# cxx_std_17)

if(UNIX)
  add_executable(cdt-gv ${PROJECT_SOURCE_DIR}/src/cdt-gv.cpp)
  target_link_libraries(cdt-gv PRIVATE project_options project_warnings
                                       docopt_s CGAL::CGAL)
  target_compile_features(cdt-gv PRIVATE cxx_std_20)
endif()

#
# Tests    ##
#

# Catch
if(ENABLE_TESTING)
  enable_testing()
  add_subdirectory(tests)
  message(STATUS "Building tests. Check out /tests for unit tests.")
endif()

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
