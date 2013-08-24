cmake_minimum_required(VERSION 2.8)
project(CDT-plusplus C)

set(CDT-plusplus_MAJOR_VERSION 0)
set(CDT-plusplus_MINOR_VERSION 1)
set(CDT-plusplus_PATCH_VERSION 0)
set(CDT-plusplus_VERSION
  ${CDT-plusplus_MAJOR_VERSION}.${CDT-plusplus_MINOR_VERSION}.${CDT-plusplus_PATCH_VERSION})

# Offer the user the choice of overriding the installation directories
set(INSTALL_LIB_DIR lib CACHE PATH "Installation directory for libraries")
set(INSTALL_BIN_DIR bin CACHE PATH "Installation directory for executables")
set(INSTALL_INCLUDE_DIR include CACHE PATH
  "Installation directory for header files")
if(WIN32 AND NOT CYGWIN)
  set(DEF_INSTALL_CMAKE_DIR CMake)
else()
  set(DEF_INSTALL_CMAKE_DIR lib/CMake/CDT-plusplus)
endif()
set(INSTALL_CMAKE_DIR ${DEF_INSTALL_CMAKE_DIR} CACHE PATH
  "Installation directory for CMake files")

# Make relative paths absolute (needed later on)
foreach(p LIB BIN INCLUDE CMAKE)
  set(var INSTALL_${p}_DIR)
  if(NOT IS_ABSOLUTE "${${var}}")
    set(${var} "${CMAKE_INSTALL_PREFIX}/${${var}}")
  endif()
endforeach()

# set up include-directories
include_directories(
  "${PROJECT_SOURCE_DIR}"   # to find foo/foo.h
  "${PROJECT_BINARY_DIR}")  # to find foo/config.h

# Add sub-directories
add_subdirectory(foo)
add_subdirectory(bar)

# The interesting stuff goes here
# ===============================

# Add all targets to the build-tree export set
export(TARGETS foo bar
  FILE "${PROJECT_BINARY_DIR}/CDT-plusplusTargets.cmake")

# Export the package for use from the build-tree
# (this registers the build-tree with a global CMake-registry)
export(PACKAGE CDT-plusplus)

# Create the CDT-plusplusConfig.cmake and CDT-plusplusConfigVersion files
file(RELATIVE_PATH REL_INCLUDE_DIR "${INSTALL_CMAKE_DIR}"
   "${INSTALL_INCLUDE_DIR}")
# ... for the build tree
set(CONF_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}" "${PROJECT_BINARY_DIR}")
configure_file(CDT-plusplusConfig.cmake.in
  "${PROJECT_BINARY_DIR}/CDT-plusplusConfig.cmake" @ONLY)
# ... for the install tree
set(CONF_INCLUDE_DIRS "\${CDT-plusplus_CMAKE_DIR}/${REL_INCLUDE_DIR}")
configure_file(CDT-plusplusConfig.cmake.in
  "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CDT-plusplusConfig.cmake" @ONLY)
# ... for both
configure_file(CDT-plusplusConfigVersion.cmake.in
  "${PROJECT_BINARY_DIR}/CDT-plusplusConfigVersion.cmake" @ONLY)

# Install the CDT-plusplusConfig.cmake and CDT-plusplusConfigVersion.cmake
install(FILES
  "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CDT-plusplusConfig.cmake"
  "${PROJECT_BINARY_DIR}/CDT-plusplusConfigVersion.cmake"
  DESTINATION "${INSTALL_CMAKE_DIR}" COMPONENT dev)

# Install the export set for use with the install-tree
install(EXPORT CDT-plusplusTargets DESTINATION
  "${INSTALL_CMAKE_DIR}" COMPONENT dev)
