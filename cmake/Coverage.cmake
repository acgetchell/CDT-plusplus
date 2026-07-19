option(ENABLE_COVERAGE "Enable GCC/Clang coverage instrumentation" OFF)

function(enable_coverage target)
  if(NOT ENABLE_COVERAGE)
    return()
  endif()

  if(NOT (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES ".*Clang"))
    message(FATAL_ERROR "Coverage instrumentation requires GCC or Clang")
  endif()

  target_compile_options(${target} INTERFACE --coverage -O0 -g)
  target_link_options(${target} INTERFACE --coverage)
  message(STATUS "Coverage instrumentation enabled")
endfunction()
