if(NOT DEFINED INITIALIZE_EXECUTABLE OR NOT EXISTS "${INITIALIZE_EXECUTABLE}")
  message(FATAL_ERROR "INITIALIZE_EXECUTABLE must name a built initialize program")
endif()
if(NOT DEFINED CDT_EXECUTABLE OR NOT EXISTS "${CDT_EXECUTABLE}")
  message(FATAL_ERROR "CDT_EXECUTABLE must name a built cdt program")
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

set(initialize_directory "${normalized_test_directory}/initialize")
set(uninterrupted_directory "${normalized_test_directory}/uninterrupted")
set(resumed_directory "${normalized_test_directory}/resumed")
set(extended_directory "${normalized_test_directory}/extended")
set(rejected_directory "${normalized_test_directory}/rejected")
set(completed_directory "${normalized_test_directory}/completed")
set(corrupt_directory "${normalized_test_directory}/corrupt")
set(thread_limit_directory "${normalized_test_directory}/thread-limit")
file(REMOVE_RECURSE "${normalized_test_directory}")
file(MAKE_DIRECTORY
     "${initialize_directory}"
     "${uninterrupted_directory}"
     "${resumed_directory}"
     "${extended_directory}"
     "${rejected_directory}"
     "${completed_directory}"
     "${corrupt_directory}"
     "${thread_limit_directory}")

execute_process(
  COMMAND "${INITIALIZE_EXECUTABLE}" -s -n64 -t3 -o --seed 92
  WORKING_DIRECTORY "${initialize_directory}"
  RESULT_VARIABLE initialize_result
  OUTPUT_VARIABLE initialize_output
  ERROR_VARIABLE initialize_error)
if(NOT initialize_result EQUAL 0)
  message(FATAL_ERROR "initialize failed:\n${initialize_output}\n${initialize_error}")
endif()
file(GLOB initial_payloads LIST_DIRECTORIES false "${initialize_directory}/*.off")
list(LENGTH initial_payloads initial_payload_count)
if(NOT initial_payload_count EQUAL 1)
  message(FATAL_ERROR "Expected one initialized payload, found ${initial_payload_count}")
endif()
list(GET initial_payloads 0 initial_payload)

execute_process(
  COMMAND
    "${CDT_EXECUTABLE}" --input "${initial_payload}" -a0.6 -k1.1 -l0.1
    -p4 -c2 --seed 93
  WORKING_DIRECTORY "${uninterrupted_directory}"
  RESULT_VARIABLE uninterrupted_result
  OUTPUT_VARIABLE uninterrupted_output
  ERROR_VARIABLE uninterrupted_error)
if(NOT uninterrupted_result EQUAL 0)
  message(
    FATAL_ERROR
      "Uninterrupted run failed:\n${uninterrupted_output}\n${uninterrupted_error}")
endif()

file(GLOB pass_two_payloads LIST_DIRECTORIES false "${uninterrupted_directory}/*-pass-2.off")
file(GLOB pass_four_payloads LIST_DIRECTORIES false "${uninterrupted_directory}/*-pass-4.off")
list(LENGTH pass_two_payloads pass_two_count)
list(LENGTH pass_four_payloads pass_four_count)
if(NOT pass_two_count EQUAL 1 OR NOT pass_four_count EQUAL 1)
  message(
    FATAL_ERROR
      "Expected pass-2 and pass-4 checkpoints; found ${pass_two_count} and ${pass_four_count}")
endif()
list(GET pass_two_payloads 0 pass_two_checkpoint)
list(GET pass_four_payloads 0 pass_four_checkpoint)
file(READ "${pass_two_checkpoint}.meta" pass_two_metadata)
foreach(
    required
    "artifact=checkpoint"
    "resume_supported=true"
    "configured_passes=4"
    "completed_passes=2"
    "random.transition_state="
    "moves.proposed="
    "moves.accepted="
    "moves.rejected="
    "moves.attempted="
    "moves.succeeded="
    "moves.failed=")
  if(NOT pass_two_metadata MATCHES "${required}")
    message(FATAL_ERROR "Checkpoint metadata is missing '${required}':\n${pass_two_metadata}")
  endif()
endforeach()

execute_process(
  COMMAND "${CDT_EXECUTABLE}" --resume "${pass_two_checkpoint}"
  WORKING_DIRECTORY "${resumed_directory}"
  RESULT_VARIABLE resumed_result
  OUTPUT_VARIABLE resumed_output
  ERROR_VARIABLE resumed_error)
if(NOT resumed_result EQUAL 0)
  message(FATAL_ERROR "Resumed run failed:\n${resumed_output}\n${resumed_error}")
endif()
foreach(
    expected
    "Completed checkpoint passes: 2"
    "Target total passes: 4"
    "Number of passes to execute: 2"
    "=== Pass 3 ==="
    "=== Pass 4 ==="
    "Writing checkpoint for pass 4")
  if(NOT resumed_output MATCHES "${expected}")
    message(FATAL_ERROR "Resume output is missing '${expected}':\n${resumed_output}")
  endif()
endforeach()
file(GLOB resumed_pass_two_payloads LIST_DIRECTORIES false "${resumed_directory}/*-pass-2.off")
file(GLOB resumed_pass_four_payloads LIST_DIRECTORIES false "${resumed_directory}/*-pass-4.off")
list(LENGTH resumed_pass_two_payloads resumed_pass_two_count)
list(LENGTH resumed_pass_four_payloads resumed_pass_four_count)
if(NOT resumed_pass_two_count EQUAL 0 OR NOT resumed_pass_four_count EQUAL 1)
  message(
    FATAL_ERROR
      "Resume did not preserve global checkpoint numbering: pass-2=${resumed_pass_two_count}, pass-4=${resumed_pass_four_count}")
endif()

function(find_final_manifest directory output_variable)
  file(GLOB candidate_manifests LIST_DIRECTORIES false "${directory}/*.off.meta")
  set(final_manifest "")
  foreach(candidate IN LISTS candidate_manifests)
    file(READ "${candidate}" candidate_metadata)
    if(candidate_metadata MATCHES "(^|[\r\n])artifact=final-triangulation([\r\n]|$)")
      if(NOT final_manifest STREQUAL "")
        message(FATAL_ERROR "Found more than one final manifest in ${directory}")
      endif()
      set(final_manifest "${candidate}")
    endif()
  endforeach()
  if(final_manifest STREQUAL "")
    message(FATAL_ERROR "Did not find a final manifest in ${directory}")
  endif()
  set("${output_variable}" "${final_manifest}" PARENT_SCOPE)
endfunction()

find_final_manifest("${uninterrupted_directory}" uninterrupted_final_manifest)
find_final_manifest("${resumed_directory}" resumed_final_manifest)
file(READ "${uninterrupted_final_manifest}" uninterrupted_final_metadata)
file(READ "${resumed_final_manifest}" resumed_final_metadata)

function(compare_metadata_field field)
  string(REPLACE "." "[.]" field_pattern "${field}")
  if(NOT uninterrupted_final_metadata MATCHES "(^|[\r\n])${field_pattern}=([^\r\n]+)")
    message(FATAL_ERROR "Uninterrupted metadata is missing ${field}")
  endif()
  set(uninterrupted_value "${CMAKE_MATCH_2}")
  if(NOT resumed_final_metadata MATCHES "(^|[\r\n])${field_pattern}=([^\r\n]+)")
    message(FATAL_ERROR "Resumed metadata is missing ${field}")
  endif()
  set(resumed_value "${CMAKE_MATCH_2}")
  if(NOT uninterrupted_value STREQUAL resumed_value)
    message(
      FATAL_ERROR
        "Scientific resume mismatch for ${field}: uninterrupted='${uninterrupted_value}', resumed='${resumed_value}'")
  endif()
endfunction()

foreach(
    field
    "topology.fnv1a64"
    "placement.fnv1a64"
    "transition_trace.fnv1a64"
    "transition_trace.count"
    "moves.proposed"
    "moves.accepted"
    "moves.rejected"
    "moves.attempted"
    "moves.succeeded"
    "moves.failed"
    "configured_passes"
    "completed_passes"
    "random.seed"
    "random.transition_stream"
    "alpha"
    "k"
    "lambda")
  compare_metadata_field("${field}")
endforeach()

execute_process(
  COMMAND "${CDT_EXECUTABLE}" --resume "${pass_four_checkpoint}"
  WORKING_DIRECTORY "${completed_directory}"
  RESULT_VARIABLE completed_result
  OUTPUT_VARIABLE completed_output
  ERROR_VARIABLE completed_error)
if(NOT completed_result EQUAL 0
   OR NOT completed_output MATCHES "no transitions remain")
  message(
    FATAL_ERROR
      "A completed checkpoint did not finalize cleanly:\n${completed_output}\n${completed_error}")
endif()
find_final_manifest("${completed_directory}" completed_final_manifest)

execute_process(
  COMMAND "${CDT_EXECUTABLE}" --resume "${pass_two_checkpoint}" --passes 6
  WORKING_DIRECTORY "${extended_directory}"
  RESULT_VARIABLE extended_result
  OUTPUT_VARIABLE extended_output
  ERROR_VARIABLE extended_error)
if(NOT extended_result EQUAL 0)
  message(
    FATAL_ERROR
      "Extended resume failed with result '${extended_result}':\n${extended_output}\n${extended_error}")
endif()
foreach(
    expected
    "Completed checkpoint passes: 2"
    "Target total passes: 6"
    "Number of passes to execute: 4"
    "=== Pass 3 ==="
    "=== Pass 4 ==="
    "=== Pass 5 ==="
    "=== Pass 6 ==="
    "Writing checkpoint for pass 4"
    "Writing checkpoint for pass 6")
  if(NOT extended_output MATCHES "${expected}")
    message(
      FATAL_ERROR
        "Extended resume output is missing '${expected}':\n${extended_output}")
  endif()
endforeach()
file(GLOB extended_pass_two_payloads LIST_DIRECTORIES false "${extended_directory}/*-pass-2.off")
file(GLOB extended_pass_four_payloads LIST_DIRECTORIES false "${extended_directory}/*-pass-4.off")
file(GLOB extended_pass_six_payloads LIST_DIRECTORIES false "${extended_directory}/*-pass-6.off")
list(LENGTH extended_pass_two_payloads extended_pass_two_count)
list(LENGTH extended_pass_four_payloads extended_pass_four_count)
list(LENGTH extended_pass_six_payloads extended_pass_six_count)
if(NOT extended_pass_two_count EQUAL 0
   OR NOT extended_pass_four_count EQUAL 1
   OR NOT extended_pass_six_count EQUAL 1)
  message(
    FATAL_ERROR
      "Extended resume did not preserve global checkpoint numbering: pass-2=${extended_pass_two_count}, pass-4=${extended_pass_four_count}, pass-6=${extended_pass_six_count}")
endif()
find_final_manifest("${extended_directory}" extended_final_manifest)
file(READ "${extended_final_manifest}" extended_final_metadata)
foreach(required "configured_passes=6" "completed_passes=6")
  if(NOT extended_final_metadata MATCHES "(^|[\r\n])${required}([\r\n]|$)")
    message(
      FATAL_ERROR
        "Extended final metadata is missing '${required}':\n${extended_final_metadata}")
  endif()
endforeach()

execute_process(
  COMMAND "${CDT_EXECUTABLE}" --resume "${pass_two_checkpoint}" --passes 1
  WORKING_DIRECTORY "${rejected_directory}"
  RESULT_VARIABLE rejected_result
  OUTPUT_VARIABLE rejected_output
  ERROR_VARIABLE rejected_error)
set(rejected_log "${rejected_output}\n${rejected_error}")
if(rejected_result EQUAL 0
   OR NOT rejected_log MATCHES
          "Resume target passes must be at least the completed checkpoint pass")
  message(
    FATAL_ERROR
      "cdt accepted a resume target below the completed pass:\n${rejected_log}")
endif()
file(GLOB rejected_artifacts LIST_DIRECTORIES false "${rejected_directory}/*")
if(rejected_artifacts)
  list(JOIN rejected_artifacts "\n  " rejected_artifact_list)
  message(
    FATAL_ERROR
      "Rejected resume published output artifacts:\n  ${rejected_artifact_list}")
endif()

set(corrupt_checkpoint "${corrupt_directory}/checkpoint.off")
file(COPY_FILE "${pass_two_checkpoint}" "${corrupt_checkpoint}" ONLY_IF_DIFFERENT)
file(COPY_FILE "${pass_two_checkpoint}.meta" "${corrupt_checkpoint}.meta" ONLY_IF_DIFFERENT)
file(READ "${corrupt_checkpoint}.meta" corrupt_metadata)
string(
  REGEX REPLACE
  "random[.]transition_state=[^\r\n]+"
  "random.transition_state=bad"
  corrupt_metadata
  "${corrupt_metadata}")
file(WRITE "${corrupt_checkpoint}.meta" "${corrupt_metadata}")
execute_process(
  COMMAND "${CDT_EXECUTABLE}" --resume "${corrupt_checkpoint}"
  WORKING_DIRECTORY "${corrupt_directory}"
  RESULT_VARIABLE corrupt_result
  OUTPUT_VARIABLE corrupt_output
  ERROR_VARIABLE corrupt_error)
set(corrupt_log "${corrupt_output}\n${corrupt_error}")
if(corrupt_result EQUAL 0 OR NOT corrupt_log MATCHES "invalid PCG state")
  message(FATAL_ERROR "cdt accepted corrupt resume state:\n${corrupt_log}")
endif()

execute_process(
  COMMAND "${CDT_EXECUTABLE}" --resume "${pass_two_checkpoint}" --seed 94
  WORKING_DIRECTORY "${resumed_directory}"
  RESULT_VARIABLE conflict_result
  OUTPUT_VARIABLE conflict_output
  ERROR_VARIABLE conflict_error)
set(conflict_log "${conflict_output}\n${conflict_error}")
if(conflict_result EQUAL 0 OR NOT conflict_log MATCHES "--resume restores seed")
  message(FATAL_ERROR "cdt accepted a conflicting resume seed:\n${conflict_log}")
endif()

set(thread_limit_checkpoint "${thread_limit_directory}/checkpoint.off")
file(COPY_FILE "${pass_two_checkpoint}" "${thread_limit_checkpoint}" ONLY_IF_DIFFERENT)
file(COPY_FILE "${pass_two_checkpoint}.meta" "${thread_limit_checkpoint}.meta"
     ONLY_IF_DIFFERENT)
file(READ "${thread_limit_checkpoint}.meta" thread_limit_metadata)
string(
  REGEX REPLACE
  "parallel[.]max_threads=[^\r\n]+"
  "parallel.max_threads=18446744073709551615"
  thread_limit_metadata
  "${thread_limit_metadata}")
file(WRITE "${thread_limit_checkpoint}.meta" "${thread_limit_metadata}")
execute_process(
  COMMAND "${CDT_EXECUTABLE}" --resume "${thread_limit_checkpoint}"
  WORKING_DIRECTORY "${thread_limit_directory}"
  RESULT_VARIABLE thread_limit_result
  OUTPUT_VARIABLE thread_limit_output
  ERROR_VARIABLE thread_limit_error)
set(thread_limit_log "${thread_limit_output}\n${thread_limit_error}")
if(thread_limit_result EQUAL 0
   OR NOT thread_limit_log MATCHES
          "Saved thread count exceeds the supported range")
  message(
    FATAL_ERROR
      "cdt accepted an unrepresentable saved thread count:\n${thread_limit_log}")
endif()

file(GLOB temporary_files LIST_DIRECTORIES false "${normalized_test_directory}/*/*.tmp")
if(temporary_files)
  list(JOIN temporary_files "\n  " temporary_file_list)
  message(FATAL_ERROR "Resume workflow left temporary files behind:\n  ${temporary_file_list}")
endif()
