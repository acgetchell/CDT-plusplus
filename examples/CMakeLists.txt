add_executable(CDT_cpp_api_quickstart cpp_api_quickstart.cpp)
target_compile_features(CDT_cpp_api_quickstart PRIVATE cxx_std_23)
target_link_libraries(
  CDT_cpp_api_quickstart
  PRIVATE project_options
          project_warnings
          date::date-tz
          fmt::fmt-header-only
          spdlog::spdlog_header_only
          CGAL::CGAL)

if(ENABLE_TESTING)
  add_test(
    NAME cpp-api-quickstart
    COMMAND
      ${CMAKE_COMMAND}
      "-DTEST_EXECUTABLE=$<TARGET_FILE:CDT_cpp_api_quickstart>"
      "-DTEST_OUTPUT=${CMAKE_CURRENT_BINARY_DIR}/cpp-api-quickstart.off"
      -P ${PROJECT_SOURCE_DIR}/cmake/RunCppApiQuickstartTest.cmake)
  set_tests_properties(cpp-api-quickstart PROPERTIES LABELS "example;integration"
                                                     TIMEOUT 30)
endif()
