# Derived from:
# https://github.com/cpp-best-practices/cmake_template/blob/main/cmake/Sanitizers.cmake

option(ENABLE_SANITIZER_ADDRESS "Enable AddressSanitizer" OFF)
option(ENABLE_SANITIZER_LEAK "Enable LeakSanitizer" OFF)
option(ENABLE_SANITIZER_MEMORY "Enable MemorySanitizer" OFF)
option(ENABLE_SANITIZER_THREAD "Enable ThreadSanitizer" OFF)
option(ENABLE_SANITIZER_UNDEFINED_BEHAVIOR "Enable UndefinedBehaviorSanitizer" OFF)

function(enable_sanitizers target)
  if(NOT
     ENABLE_SANITIZER_ADDRESS
     AND NOT ENABLE_SANITIZER_LEAK
     AND NOT ENABLE_SANITIZER_MEMORY
     AND NOT ENABLE_SANITIZER_THREAD
     AND NOT ENABLE_SANITIZER_UNDEFINED_BEHAVIOR)
    return()
  endif()

  if(NOT (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES ".*Clang"))
    message(FATAL_ERROR "The requested sanitizers require GCC or Clang")
  endif()

  set(_sanitizers "")

  if(ENABLE_SANITIZER_ADDRESS)
    list(APPEND _sanitizers "address")
    message(STATUS "AddressSanitizer enabled")
  endif()

  if(ENABLE_SANITIZER_LEAK)
    if(APPLE)
      message(FATAL_ERROR "Standalone LeakSanitizer is not supported on Apple platforms")
    endif()
    list(APPEND _sanitizers "leak")
    message(STATUS "LeakSanitizer enabled")
  endif()

  if(ENABLE_SANITIZER_UNDEFINED_BEHAVIOR)
    list(APPEND _sanitizers "undefined")
    message(STATUS "UndefinedBehaviorSanitizer enabled")
  endif()

  if(ENABLE_SANITIZER_THREAD)
    if(ENABLE_SANITIZER_ADDRESS OR ENABLE_SANITIZER_LEAK)
      message(FATAL_ERROR "ThreadSanitizer cannot be combined with AddressSanitizer or LeakSanitizer")
    endif()
    list(APPEND _sanitizers "thread")
    message(STATUS "ThreadSanitizer enabled")
  endif()

  if(ENABLE_SANITIZER_MEMORY)
    if(NOT CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
      message(FATAL_ERROR "MemorySanitizer requires Clang")
    endif()
    if(NOT CMAKE_SYSTEM_NAME MATCHES "Linux|FreeBSD|NetBSD")
      message(FATAL_ERROR "MemorySanitizer is not supported on ${CMAKE_SYSTEM_NAME}")
    endif()
    if(ENABLE_SANITIZER_ADDRESS OR ENABLE_SANITIZER_LEAK OR ENABLE_SANITIZER_THREAD)
      message(
        FATAL_ERROR
          "MemorySanitizer cannot be combined with AddressSanitizer, LeakSanitizer, or ThreadSanitizer")
    endif()
    message(
      WARNING
        "MemorySanitizer requires an instrumented standard library and instrumented dependencies; otherwise reports are not authoritative"
    )
    list(APPEND _sanitizers "memory")
    message(STATUS "MemorySanitizer enabled")
  endif()

  list(JOIN _sanitizers "," _sanitizer_flags)
  target_compile_options(
    ${target}
    INTERFACE -g
              -O1
              -fsanitize=${_sanitizer_flags}
              -fno-omit-frame-pointer)
  target_link_options(${target} INTERFACE -fsanitize=${_sanitizer_flags})
endfunction()
