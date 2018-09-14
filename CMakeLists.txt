cmake_minimum_required(VERSION 3.11)

project(CDT-plusplus VERSION 0.1.8 LANGUAGES CXX)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
  message(FATAL_ERROR "Do not build in-source.
                       Please remove CMakeCache.txt and the CMakeFiles/ directory. Then build out-of-source.")
endif()

# Run Conan from CMake
# Download automatically, you can also just copy the conan.cmake file
if(NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
  message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
  file(DOWNLOAD "https://raw.githubusercontent.com/conan-io/cmake-conan/v0.12/conan.cmake"
          "${CMAKE_BINARY_DIR}/conan.cmake")
endif()

include(${CMAKE_BINARY_DIR}/conan.cmake)
conan_cmake_run(CONANFILE conanfile.py  # or relative build/conanfile.txt
        BASIC_SETUP CMAKE_TARGETS
        BUILD missing)



############################
##  Modules and scripts   ##
############################

#Standard CMake modules
include(CTest)
include(CMakeDependentOption)

#Coverage
if (CODECOV)
  set(CMAKE_BUILD_TYPE Debug)
  set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -g")
  set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -O0")
  set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -fprofile-arcs")
  set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -ftest-coverage")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
endif ()

#Valgrind
if (VALGRIND)
  set(CMAKE_BUILD_TYPE Debug)
  set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -g")
  set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -O0")
  set(MEMORYCHECK_COMMAND_OPTIONS "${MEMORYCHECK_COMMAND_OPTIONS} --leak-check=full")
  set(MEMORYCHECK_COMMAND_OPTIONS "${MEMORYCHECK_COMMAND_OPTIONS} --track-fds=yes")
  set(MEMORYCHECK_COMMAND_OPTIONS "${MEMORYCHECK_COMMAND_OPTIONS} --trace-children=yes")
  set(MEMORYCHECK_COMMAND_OPTIONS "${MEMORYCHECK_COMMAND_OPTIONS} --error-exitcode=1")
  # Known Valgrind issue
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
endif()

###############
##  OPTIONS  ##
###############

#Turn on / off Catch Tests
option(TESTS ON)
#Turn on / off TBB
set(TBB_ON TRUE)
#Set mimumum Boost
#set(BOOST_MIN_VERSION "1.64.0")
#Compile commands for ClangTidy et. al
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


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

#Easier navigation in an IDE when projects are organized in folders.
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

###############
##  Project  ##
###############

set(EXECUTABLE_NAME "CDT")

#CGAL
find_package(CGAL QUIET COMPONENTS Core )
if(CGAL_FOUND)
#Don't let CGAL override flags
  set(CGAL_DO_NOT_WARN_ABOUT_CMAKE_BUILD_TYPE true)
  set(CGAL_DONT_OVERRIDE_CMAKE_FLAGS TRUE CACHE BOOL "Force CGAL to maintain CMAKE flags")
  include(${CGAL_USE_FILE})
  include(CGAL_CreateSingleSourceCGALProgram)
else()
  message(FATAL_ERROR "Notice: This program requires the CGAL library, and will not be compiled.")
endif()

include_directories (BEFORE ${PROJECT_SOURCE_DIR}/include)
add_executable(cdt ${PROJECT_SOURCE_DIR}/src/cdt.cpp)
target_link_libraries(cdt PRIVATE ${CGAL_LIBRARIES} CONAN_PKG::boost CONAN_PKG::eigen CONAN_PKG::TBB CONAN_PKG::libcurl CONAN_PKG::docopt CONAN_PKG::date CONAN_PKG::gsl_microsoft)
#Activate C++17
target_compile_features(cdt PRIVATE cxx_std_17)

add_executable(initialize ${PROJECT_SOURCE_DIR}/src/initialize.cpp)
target_link_libraries(initialize PRIVATE ${CGAL_LIBRARIES} CONAN_PKG::boost CONAN_PKG::docopt CONAN_PKG::date CONAN_PKG::gsl_microsoft)
target_compile_features(initialize PRIVATE cxx_std_17)

if (UNIX)
  add_executable(cdt-gv ${PROJECT_SOURCE_DIR}/src/cdt-gv.cpp)
  target_link_libraries(cdt-gv PRIVATE ${CGAL_LIBRARIES} CONAN_PKG::boost CONAN_PKG::docopt CONAN_PKG::gsl_microsoft)
  #Activate C++17
  target_compile_features(cdt-gv PRIVATE cxx_std_17)


  add_executable(cdt-opt ${PROJECT_SOURCE_DIR}/src/cdt-opt.cpp)
  target_link_libraries(cdt-opt PRIVATE ${CGAL_LIBRARIES} CONAN_PKG::boost CONAN_PKG::libcurl CONAN_PKG::date CONAN_PKG::gsl_microsoft)
  #Activate C++17
  target_compile_features(cdt-opt PRIVATE cxx_std_17)

endif(UNIX)

###############
##  Tests    ##
###############

#Catch
enable_testing()
add_subdirectory(tests)

#CTest basic testing

##Parameter initializer program
add_test(NAME CDT-Initialize COMMAND $<TARGET_FILE:initialize>)
set_tests_properties(CDT-Initialize
        PROPERTIES
        PASS_REGULAR_EXPRESSION "Usage:./initialize")

#Does the usage message work ?
add_test (NAME CDT-Usage COMMAND $<TARGET_FILE:cdt>)
set_tests_properties (CDT-Usage
        PROPERTIES
        PASS_REGULAR_EXPRESSION "Usage:./cdt")

#Dimensions != 4
add_test (NAME CDT-3Donly COMMAND $<TARGET_FILE:cdt> --s -n640 -t4 -a0.6 -k1.1 -l0.1 -d4 -p1 -c1)
set_tests_properties (CDT-3Donly
        PROPERTIES
        PASS_REGULAR_EXPRESSION "Invalid parameter ... Exiting.")

#Simplices and timeslices >= 2
add_test(NAME CDT-Simplices COMMAND $<TARGET_FILE:cdt> --s -n1 -t2 -a0.6 -k1.1 -l0.1 -d3 -p1 -c1)
set_tests_properties(CDT-Simplices
        PROPERTIES
        PASS_REGULAR_EXPRESSION "Simplices and timeslices should be greater or equal to 2.")
add_test(NAME CDT-Timeslices COMMAND $<TARGET_FILE:cdt> --s -n2 -t1 -a0.6 -k1.1 -l0.1 -d3 -p1 -c1)
set_tests_properties(CDT-Timeslices
        PROPERTIES
        PASS_REGULAR_EXPRESSION "Simplices and timeslices should be greater or equal to 2.")

# 3D Triangle inequality
add_test (NAME CDT-3Dtriangle COMMAND $<TARGET_FILE:cdt> --s -n640 -t4 -a0.3 -k1.1 -l0.1 -d3 -p1 -c1)
set_tests_properties (CDT-3Dtriangle
        PROPERTIES
        PASS_REGULAR_EXPRESSION "Triangle inequalities violated ... Exiting.")

#Minimum run
add_test(NAME CDT-MinimalS3 COMMAND $<TARGET_FILE:cdt> --s -n2 -t2 -a0.6 -k1.1 -l0.1 -p1 -c1)
set_tests_properties(CDT-MinimalS3
        PROPERTIES
        PASS_REGULAR_EXPRESSION "Writing to file S3")
#Run an S3
add_test(NAME CDT-S3Runs COMMAND $<TARGET_FILE:cdt> --s -n640 -t4 -a0.6 -k1.1 -l0.1 -p1 -c1)
set_tests_properties(CDT-S3Runs
        PROPERTIES
        PASS_REGULAR_EXPRESSION "Writing to file S3")
