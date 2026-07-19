option(ENABLE_VALGRIND "Enable Valgrind diagnostics" OFF)

if(ENABLE_VALGRIND)
  set(MEMORYCHECK_COMMAND_OPTIONS
      "--leak-check=full --track-fds=yes --trace-children=yes --error-exitcode=1")
endif()

function(enable_valgrind target)
  if(NOT ENABLE_VALGRIND)
    return()
  endif()

  if(NOT CMAKE_SYSTEM_NAME STREQUAL "Linux")
    message(FATAL_ERROR "This project's Valgrind configuration is supported only on Linux")
  endif()

  if(NOT (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES ".*Clang"))
    message(FATAL_ERROR "Valgrind diagnostics require GCC or Clang")
  endif()

  target_compile_options(${target} INTERFACE -g -O0)
  message(STATUS "Valgrind diagnostics enabled")
endfunction()
