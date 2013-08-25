cmake_minimum_required (VERSION 2.8)
project (CDT)

# The version number.
set (CDT_VERSION_MAJOR 1)
set (CDT_VERSION_MINOR 0)

# configure a header file to pass some of the CMake settings
# to the source code
configure_file (
  "${PROJECT_SOURCE_DIR}/CDTConfig.h.in"
  "${PROJECT_BINARY_DIR}/CDTConfig.h"
  )

# add the binary tree to the search path for include files
# so that we will find CDTConfig.h
include_directories ("${PROJECT_BINARY_DIR}")

# add the executable
add_executable (CDT CDT.cpp)
target_link_libraries (CDT  ${EXTRA_LIBS})

# add the install targets
install (TARGETS CDT DESTINATION bin)
install (FILES "${PROJECT_BINARY_DIR}/CDTConfig.h" 
  DESTINATION include)


# enable testing
enable_testing ()

# does the usage message work?
add_test (CDTUsage CDT)
set_tests_properties (CDTUsage
  PROPERTIES 
  PASS_REGULAR_EXPRESSION "Usage:.*number"
  )
