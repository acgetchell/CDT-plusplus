# Set minimum Boost
set(BOOST_MIN_VERSION "1.71.0")

# Compile commands for ClangTidy et. al
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Use C++17 for std::optional
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Turn on / off TBB
set(TBB_ON ON)

# Threads
set(CMAKE_THREAD_PREFER_PTHREAD TRUE)

# Disable CLion generation of MinSizeRel to avoid conflicts with
# CGAL_SetupFlags.cmake
set(CMAKE_CONFIGURATION_TYPES
    "Release" "Debug" "RelWithDebInfo"
    CACHE STRING "" FORCE)

# Default build type
set(default_build_type "Release")
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(
    STATUS
      "Setting build type to '${default_build_type}' as none was specified.")
  set(CMAKE_BUILD_TYPE
      "${default_build_type}"
      CACHE STRING "Choose the type of build." FORCE)
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release"
                                               "RelWithDebInfo")
endif()

# Use Ccache
set(ENABLE_CCACHE "Enable Ccache" ON)
if(ENABLE_CCACHE)
  find_program(CCACHE ccache)
  if(CCACHE)
    message("using ccache")
    set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE})
  else()
    message("ccache not found cannot use")
  endif()
endif()

# Link time optimization
set(ENABLE_IPO
    "Enable Interprocedural Optimization, aka Link Time Optimization (LTO)" ON)
if(ENABLE_IPO)
  include(CheckIPOSupported)
  check_ipo_supported(RESULT ipo_support)
  if(ipo_support)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
  else()
    message(WARNING "IPO is not supported: ${output}")
  endif()
endif()

# Turn off CGAL Triangulation Assertions and Postconditions
add_definitions(-DCGAL_TRIANGULATION_NO_ASSERTIONS
                -DCGAL_TRIANGULATION_NO_POSTCONDITIONS)

# Easier navigation in an IDE when projects are organized in folders.
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
