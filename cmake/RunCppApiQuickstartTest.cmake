if(NOT DEFINED TEST_EXECUTABLE OR NOT EXISTS "${TEST_EXECUTABLE}")
  message(FATAL_ERROR "TEST_EXECUTABLE must name a built executable")
endif()

if(NOT DEFINED TEST_OUTPUT OR TEST_OUTPUT STREQUAL "")
  message(FATAL_ERROR "TEST_OUTPUT must name the quickstart payload")
endif()

execute_process(
  COMMAND "${TEST_EXECUTABLE}" "${TEST_OUTPUT}"
  RESULT_VARIABLE run_result
  OUTPUT_VARIABLE run_output
  ERROR_VARIABLE run_error)
if(NOT run_result EQUAL 0)
  message(FATAL_ERROR "C++ API quickstart failed:\n${run_output}\n${run_error}")
endif()

set(move_pattern "[(](2,3|3,2|2,6|6,2|4,4)[)]")
set(result_pattern "accepted=(yes|no), successful=(yes|no)")
foreach(proposal RANGE 1 10)
  if(NOT run_output
     MATCHES "proposal ${proposal}: move ${move_pattern}, ${result_pattern}")
    message(
      FATAL_ERROR
        "Quickstart did not report proposal ${proposal} completely:\n${run_output}")
  endif()
endforeach()

string(
  REGEX MATCHALL
        "proposal [0-9]+: move ${move_pattern}, ${result_pattern}"
        proposal_lines "${run_output}")
list(LENGTH proposal_lines proposal_count)
if(NOT proposal_count EQUAL 10)
  message(FATAL_ERROR "Expected 10 proposal reports, found ${proposal_count}:\n${run_output}")
endif()

if(NOT run_output
   MATCHES
   "There were 10 proposed moves with [0-9]+ accepted moves and [0-9]+ rejected moves[.]")
  message(FATAL_ERROR "Quickstart aggregate proposal summary is missing:\n${run_output}")
endif()
if(NOT run_output
   MATCHES
   "There were 10 candidate construction attempts with [0-9]+ successful candidates and [0-9]+ failed candidates[.]")
  message(FATAL_ERROR "Quickstart candidate summary is missing:\n${run_output}")
endif()

if(NOT EXISTS "${TEST_OUTPUT}" OR NOT EXISTS "${TEST_OUTPUT}.meta")
  message(FATAL_ERROR "Quickstart did not publish its payload and metadata sidecar")
endif()
file(READ "${TEST_OUTPUT}.meta" metadata)
if(NOT metadata MATCHES "transition_trace[.]count=10")
  message(FATAL_ERROR "Quickstart metadata does not record all ten transitions:\n${metadata}")
endif()
if(NOT metadata MATCHES "configured_attempts=10")
  message(FATAL_ERROR "Quickstart metadata does not record its explicit attempt plan:\n${metadata}")
endif()
