# Derived from:
#
# https://github.com/lefticus/cpp_starter_project/blob/master/cmake/Sanitizers.cmake
#

function(enable_sanitizers project_name)

  if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
    option(ENABLE_COVERAGE "Enable coverage reporting for gcc/clang" OFF)

    if(ENABLE_COVERAGE)
      target_compile_options(${project_name} INTERFACE --coverage -O0 -g)
      target_link_libraries(${project_name} INTERFACE --coverage)
      message(STATUS "Coverage enabled.")
    endif()

    option(ENABLE_VALGRIND "Enable Valgrind" OFF)
    if(ENABLE_VALGRIND)
      target_compile_options(project_options INTERFACE -g -O0 -fsanitize=address)
      target_link_libraries(project_options INTERFACE -fsanitize=address)
      message(STATUS "Valgrind enabled.")
      set(MEMORYCHECK_COMMAND_OPTIONS "${MEMORYCHECK_COMMAND_OPTIONS} --leak-check=full")
      set(MEMORYCHECK_COMMAND_OPTIONS "${MEMORYCHECK_COMMAND_OPTIONS} --track-fds=yes")
      set(MEMORYCHECK_COMMAND_OPTIONS "${MEMORYCHECK_COMMAND_OPTIONS} --trace-children=yes")
      set(MEMORYCHECK_COMMAND_OPTIONS "${MEMORYCHECK_COMMAND_OPTIONS} --error-exitcode=1")
    endif()

    set(SANITIZERS "")

    option(ENABLE_SANITIZER_ADDRESS "Enable address sanitizer" OFF)
    if(ENABLE_SANITIZER_ADDRESS)
      list(APPEND SANITIZERS "address")
      message(STATUS "AddressSanitizer enabled.")
      set(ENV{ASAN_OPTIONS} "fast_unwind_on_malloc=0")
      set(ENV{ASAN_OPTIONS} "$ENV{ASAN_OPTIONS}:help=1")
      set(ENV{ASAN_OPTIONS} "$ENV{ASAN_OPTIONS}:symbolize=1")
      set(ENV{ASAN_OPTIONS} "$ENV{ASAN_OPTIONS}:verbosity=2")
      message(STATUS "ASAN_OPTIONS=$ENV{ASAN_OPTIONS}")
    endif()

    option(ENABLE_SANITIZER_LEAK "Enable leak sanitizer" OFF)
    if(ENABLE_SANITIZER_LEAK)
      if(${APPLE})
        message(FATAL_ERROR "Leak sanitizer not supported on x86_64-apple-darwin")
      else()
        list(APPEND SANITIZERS "leak")
        message(STATUS "LeakSanitizer enabled.")
      endif()
    endif()

    option(ENABLE_SANITIZER_UNDEFINED_BEHAVIOR "Enable undefined behavior sanitizer" OFF)
    if(ENABLE_SANITIZER_UNDEFINED_BEHAVIOR)
      list(APPEND SANITIZERS "undefined")
      message(STATUS "UndefinedBehaviorSanitizer enabled.")
      set(ENV{UBSAN_OPTIONS} "print_stacktrace=1")
      message(STATUS "UBSAN_OPTIONS=$ENV{UBSAN_OPTIONS}")
    endif()

    option(ENABLE_SANITIZER_THREAD "Enable thread sanitizer" OFF)
    if(ENABLE_SANITIZER_THREAD)
      if("address" IN_LIST SANITIZERS OR "leak" IN_LIST SANITIZERS)
        message(WARNING "Thread sanitizer does not work with Address and Leak sanitizer enabled.")
      else()
        list(APPEND SANITIZERS "thread")
        message(STATUS "ThreadSanitizer enabled.")
      endif()
    endif()

    option(ENABLE_SANITIZER_MEMORY "Enable memory sanitizer" OFF)
    if(ENABLE_SANITIZER_MEMORY AND CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
      if(${APPLE})
        message(FATAL_ERROR "Memory sanitizer not supported on x86_64-apple-darwin")
      else()
        if("address" IN_LIST SANITIZERS
           OR "thread" IN_LIST SANITIZERS
           OR "leak" IN_LIST SANITIZERS)
          message(WARNING "Memory sanitizer does not work with Address, Thread, and Leak sanitizer enabled.")
        else()
          list(APPEND SANITIZERS "memory")
          message(STATUS "Memory sanitizer enabled.")
        endif()
      endif()
    endif()

    list(
      JOIN
      SANITIZERS
      ","
      LIST_OF_SANITIZERS)

  endif()

  if(LIST_OF_SANITIZERS)
    if(NOT
       "${LIST_OF_SANITIZERS}"
       STREQUAL
       "")
      target_compile_options(
        ${project_name}
        INTERFACE -g
                  -O1
                  -fsanitize=${LIST_OF_SANITIZERS}
                  -fno-omit-frame-pointer)
      target_link_libraries(${project_name} INTERFACE -fsanitize=${LIST_OF_SANITIZERS})
    endif()
  endif()

endfunction()
