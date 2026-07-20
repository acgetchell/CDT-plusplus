if(NOT DEFINED TEST_EXECUTABLE OR NOT EXISTS "${TEST_EXECUTABLE}")
  message(FATAL_ERROR "TEST_EXECUTABLE must name a built executable")
endif()

if(NOT DEFINED EXPECTED_REGEX OR EXPECTED_REGEX STREQUAL "")
  message(FATAL_ERROR "EXPECTED_REGEX must describe the expected diagnostic")
endif()

execute_process(
  COMMAND "${TEST_EXECUTABLE}" ${TEST_ARGUMENTS}
  RESULT_VARIABLE test_result
  OUTPUT_VARIABLE test_output
  ERROR_VARIABLE test_error)

if(NOT test_result MATCHES "^[0-9]+$")
  message(
    FATAL_ERROR
      "Command did not exit normally (${test_result}):\n${test_output}\n${test_error}")
endif()

if(test_result EQUAL 0)
  message(
    FATAL_ERROR
      "Command unexpectedly succeeded:\n${test_output}\n${test_error}")
endif()

set(combined_output "${test_output}\n${test_error}")
if(NOT combined_output MATCHES "${EXPECTED_REGEX}")
  message(
    FATAL_ERROR
      "Command failed without the expected diagnostic '${EXPECTED_REGEX}':\n${combined_output}")
endif()
