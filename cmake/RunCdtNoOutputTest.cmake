if(NOT DEFINED CDT_EXECUTABLE OR NOT EXISTS "${CDT_EXECUTABLE}")
  message(FATAL_ERROR "CDT_EXECUTABLE must name the built cdt executable")
endif()

if(NOT DEFINED TEST_ROOT OR TEST_ROOT STREQUAL "")
  message(FATAL_ERROR "TEST_ROOT must name the owning test root")
endif()

if(NOT DEFINED TEST_DIRECTORY OR TEST_DIRECTORY STREQUAL "")
  message(FATAL_ERROR "TEST_DIRECTORY must name a dedicated test directory")
endif()

set(normalized_test_root "${TEST_ROOT}")
set(normalized_test_directory "${TEST_DIRECTORY}")
cmake_path(ABSOLUTE_PATH normalized_test_root NORMALIZE)
cmake_path(ABSOLUTE_PATH normalized_test_directory NORMALIZE)
cmake_path(
  IS_PREFIX normalized_test_root "${normalized_test_directory}" NORMALIZE
  test_directory_is_owned)
if(NOT test_directory_is_owned OR normalized_test_directory STREQUAL normalized_test_root)
  message(FATAL_ERROR "TEST_DIRECTORY must be a strict descendant of TEST_ROOT")
endif()

execute_process(
  COMMAND "${CDT_EXECUTABLE}" --help
  RESULT_VARIABLE help_result
  OUTPUT_VARIABLE help_output
  ERROR_VARIABLE help_error)
if(NOT help_result EQUAL 0)
  message(FATAL_ERROR "cdt --help failed:\n${help_output}\n${help_error}")
endif()
if(NOT help_output MATCHES "--no-output" OR NOT help_output MATCHES "--seed")
  message(FATAL_ERROR "cdt --help does not document --no-output and --seed")
endif()

file(REMOVE_RECURSE "${normalized_test_directory}")
file(MAKE_DIRECTORY "${normalized_test_directory}")

execute_process(
  COMMAND "${CDT_EXECUTABLE}" -s -n64 -t3 -a0.6 -k1.1 -l0.1 -p1 -c1 --no-output --seed 92
  WORKING_DIRECTORY "${normalized_test_directory}"
  RESULT_VARIABLE run_result
  OUTPUT_VARIABLE run_output
  ERROR_VARIABLE run_error)
if(NOT run_result EQUAL 0)
  message(FATAL_ERROR "cdt --no-output failed:\n${run_output}\n${run_error}")
endif()
if(run_output MATCHES "Writing to file" OR run_error MATCHES "Writing to file")
  message(FATAL_ERROR "cdt --no-output attempted to write a triangulation file")
endif()
if(NOT run_output MATCHES "Effective random seed: 92")
  message(FATAL_ERROR "cdt did not report the requested effective seed:\n${run_output}")
endif()

file(GLOB_RECURSE generated_files LIST_DIRECTORIES false "${normalized_test_directory}/*")
if(generated_files)
  list(JOIN generated_files "\n  " generated_file_list)
  message(FATAL_ERROR "cdt --no-output created files:\n  ${generated_file_list}")
endif()
