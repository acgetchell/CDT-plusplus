add_executable(initialize ${PROJECT_SOURCE_DIR}/src/initialize.cpp)
target_link_libraries(
  initialize
  PRIVATE project_options
          project_warnings
          date::date-tz
          Boost::program_options
          fmt::fmt-header-only
          spdlog::spdlog_header_only
          TBB::tbb
          CGAL::CGAL)
target_compile_features(initialize PRIVATE cxx_std_23)

add_executable(cdt ${PROJECT_SOURCE_DIR}/src/cdt.cpp)
target_link_libraries(
  cdt
  PRIVATE project_options
          project_warnings
          date::date-tz
          Boost::program_options
          fmt::fmt-header-only
          spdlog::spdlog_header_only
          TBB::tbb
          CGAL::CGAL)
target_compile_features(cdt PRIVATE cxx_std_23)

#
# Tests    ##
#

function(add_cli_failure_test test_name target expected_regex)
  add_test(
    NAME ${test_name}
    COMMAND
      ${CMAKE_COMMAND} "-DTEST_EXECUTABLE=$<TARGET_FILE:${target}>"
      "-DTEST_ARGUMENTS=${ARGN}" "-DEXPECTED_REGEX=${expected_regex}"
      -P ${PROJECT_SOURCE_DIR}/cmake/ExpectCommandFailure.cmake)
  set_tests_properties(${test_name} PROPERTIES LABELS "integration;cli-boundary")
endfunction()

add_test(NAME cdt COMMAND $<TARGET_FILE:cdt> -s -n127 -t4 -a0.6 -k1.1 -l0.1 -p1 --seed 92)
set_tests_properties(
  cdt
  PROPERTIES LABELS "integration"
             PASS_REGULAR_EXPRESSION "Writing to file S3-[0-9]+-")

add_test(
  NAME cdt-no-output
  COMMAND
    ${CMAKE_COMMAND} -DCDT_EXECUTABLE=$<TARGET_FILE:cdt>
    -DTEST_DIRECTORY=${CMAKE_CURRENT_BINARY_DIR}/cdt-no-output
    -P ${PROJECT_SOURCE_DIR}/cmake/RunCdtNoOutputTest.cmake)
set_tests_properties(cdt-no-output PROPERTIES LABELS "integration")

add_cli_failure_test(cdt-triangle-inequalities cdt "Triangle inequalities violated" -s -n64 -t3 -a0.4
                     -k1.1 -l0.1 --seed 92)
add_cli_failure_test(cdt-dimensionality cdt "Only three-dimensional triangulations are supported." -s -n64 -t3
                     -d4 -a0.4 -k1.1 -l0.1 --seed 92)
add_cli_failure_test(cdt-toroidal cdt "Toroidal triangulations are not yet supported." -e -n64 -t3 -a0.6 -k1.1
                     -l0.1 --seed 92)
add_cli_failure_test(cdt-alpha-negative cdt "Alpha in 3D must be greater than 1/2." -s -n64 -t3 -a-0.6 -k1.1
                     -l0.1 --seed 92)
add_cli_failure_test(cdt-alpha-boundary cdt "Alpha in 3D must be greater than 1/2." -s -n64 -t3 -a0.5 -k1.1
                     -l0.1 --seed 92)
add_cli_failure_test(cdt-alpha-nonfinite cdt "Alpha must be finite." -s -n64 -t3 -anan -k1.1 -l0.1 --seed 92)
add_cli_failure_test(cdt-passes-zero cdt "Passes must be positive." -s -n64 -t3 -a0.6 -k1.1 -l0.1 -p0 --seed 92)
add_cli_failure_test(cdt-checkpoint-zero cdt "Checkpoint interval must be positive." -s -n64 -t3 -a0.6 -k1.1
                     -l0.1 -c0 --seed 92)
add_cli_failure_test(cdt-radius-zero cdt "Initial radius must be positive." -s -n64 -t3 -a0.6 -k1.1 -l0.1 -i0 --seed 92)
add_cli_failure_test(cdt-spacing-zero cdt "Foliation spacing must be positive." -s -n64 -t3 -a0.6 -k1.1 -l0.1 -f0 --seed 92)
add_cli_failure_test(cdt-empty-population cdt "would create an empty triangulation" -s -n3 -t2 -a0.6 -k1.1 -l0.1 --seed 92)
add_cli_failure_test(cdt-integer-narrowing cdt "exceeds the supported integer range" -s -n2147483648 -t3 -a0.6
                     -k1.1 -l0.1 --seed 92)

add_test(NAME initialize COMMAND $<TARGET_FILE:initialize> -s -n640 -t4 -o --seed 92)
set_tests_properties(
  initialize
  PROPERTIES LABELS "integration"
             PASS_REGULAR_EXPRESSION "Writing to file S3-4")

add_cli_failure_test(initialize-minimum-simplices initialize "Simplices and timeslices must each be at least 2." -s -n1
                     -t1 -o --seed 92)
add_cli_failure_test(initialize-dimensionality initialize "Only three-dimensional triangulations are supported." -s
                     -n64 -t3 -d2 -o --seed 92)
add_cli_failure_test(initialize-toroidal initialize "Toroidal triangulations are not yet supported." -e -n64 -t3 -o --seed 92)
add_cli_failure_test(initialize-integer-narrowing initialize "exceeds the supported integer range" -s -n2147483648 -t3 --seed 92)
add_cli_failure_test(initialize-spacing-zero initialize "Foliation spacing must be positive." -s -n64 -t3 -f0 --seed 92)
