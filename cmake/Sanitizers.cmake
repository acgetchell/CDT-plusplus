# Derived from:
#
# https://github.com/lefticus/cpp_starter_project/blob/master/cmake/Sanitizers.cmake
#

function(enable_sanitizers project_name)

  if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL
                                             "Clang")
    option(ENABLE_COVERAGE "Enable coverage reporting for gcc/clang" FALSE)

    if(ENABLE_COVERAGE)
      target_compile_options(project_options INTERFACE --coverage -O0 -g)
      target_link_libraries(project_options INTERFACE --coverage)
      message(STATUS "Coverage enabled.")
    endif()

    option(ENABLE_VALGRIND "Enable Valgrind" FALSE)
    if(ENABLE_VALGRIND)
      target_compile_options(project_options INTERFACE -g -O0
                                                       -fsanitize=address)
      target_link_libraries(project_options INTERFACE -fsanitize=address)
      message(STATUS "Valgrind enabled.")
      set(MEMORYCHECK_COMMAND_OPTIONS
          "${MEMORYCHECK_COMMAND_OPTIONS} --leak-check=full")
      set(MEMORYCHECK_COMMAND_OPTIONS
          "${MEMORYCHECK_COMMAND_OPTIONS} --track-fds=yes")
      set(MEMORYCHECK_COMMAND_OPTIONS
          "${MEMORYCHECK_COMMAND_OPTIONS} --trace-children=yes")
      set(MEMORYCHECK_COMMAND_OPTIONS
          "${MEMORYCHECK_COMMAND_OPTIONS} --error-exitcode=1")
    endif()

    set(SANITIZERS "")

    option(ENABLE_SANITIZER_ADDRESS "Enable address sanitizer" FALSE)
    if(ENABLE_SANITIZER_ADDRESS)
      list(APPEND SANITIZERS "address")
      message(STATUS "AddressSanitizer enabled.")
    endif()

    option(ENABLE_SANITIZER_MEMORY "Enable memory sanitizer" FALSE)
    if(ENABLE_SANITIZER_MEMORY)
      list(APPEND SANITIZERS "memory")
    endif()

    option(ENABLE_SANITIZER_UNDEFINED_BEHAVIOR
           "Enable undefined behavior sanitizer" FALSE)
    if(ENABLE_SANITIZER_UNDEFINED_BEHAVIOR)
      list(APPEND SANITIZERS "undefined")
      message(STATUS "UndefinedBehaviorSanitizer enabled.")
    endif()

    option(ENABLE_SANITIZER_THREAD "Enable thread sanitizer" FALSE)
    if(ENABLE_SANITIZER_THREAD)
      list(APPEND SANITIZERS "thread")
    endif()

    list(JOIN SANITIZERS "," LIST_OF_SANITIZERS)

  endif()

  if(LIST_OF_SANITIZERS)
    if(NOT "${LIST_OF_SANITIZERS}" STREQUAL "")
      target_compile_options(${project_name}
                             INTERFACE -g -O1 -fno_omit_frame_pointer -fsanitize=${LIST_OF_SANITIZERS})
      target_link_libraries(${project_name}
                            INTERFACE -fsanitize=${LIST_OF_SANITIZERS})
    endif()
  endif()

endfunction()
