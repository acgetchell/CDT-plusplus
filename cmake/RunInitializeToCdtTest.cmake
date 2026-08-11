if(NOT DEFINED INITIALIZE_EXECUTABLE OR NOT EXISTS "${INITIALIZE_EXECUTABLE}")
  message(FATAL_ERROR "INITIALIZE_EXECUTABLE must name the built initialize program")
endif()

if(NOT DEFINED CDT_EXECUTABLE OR NOT EXISTS "${CDT_EXECUTABLE}")
  message(FATAL_ERROR "CDT_EXECUTABLE must name the built cdt program")
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
set(cdt_directory "${normalized_test_directory}/cdt")
set(checkpoint_directory "${normalized_test_directory}/checkpoint")
set(unmanifested_directory "${normalized_test_directory}/unmanifested")
file(REMOVE_RECURSE "${normalized_test_directory}")
file(MAKE_DIRECTORY
     "${initialize_directory}"
     "${cdt_directory}"
     "${checkpoint_directory}"
     "${unmanifested_directory}")

execute_process(
  COMMAND "${INITIALIZE_EXECUTABLE}" -s -n64 -t3 -o --seed 92
  WORKING_DIRECTORY "${initialize_directory}"
  RESULT_VARIABLE initialize_result
  OUTPUT_VARIABLE initialize_output
  ERROR_VARIABLE initialize_error)
if(NOT initialize_result EQUAL 0)
  message(
    FATAL_ERROR
      "initialize failed:\n${initialize_output}\n${initialize_error}")
endif()

file(GLOB initial_payloads LIST_DIRECTORIES false "${initialize_directory}/*.off")
file(GLOB initial_manifests LIST_DIRECTORIES false "${initialize_directory}/*.off.meta")
list(LENGTH initial_payloads initial_payload_count)
list(LENGTH initial_manifests initial_manifest_count)
if(NOT initial_payload_count EQUAL 1 OR NOT initial_manifest_count EQUAL 1)
  message(
    FATAL_ERROR
      "initialize must publish exactly one OFF/metadata pair; found ${initial_payload_count} payloads and ${initial_manifest_count} manifests")
endif()
list(GET initial_payloads 0 initial_payload)
list(GET initial_manifests 0 initial_manifest)
if(NOT initial_manifest STREQUAL "${initial_payload}.meta")
  message(FATAL_ERROR "initialize metadata is not paired with its payload")
endif()

file(READ "${initial_manifest}" initial_metadata)
if(NOT initial_metadata MATCHES "(^|[\r\n])artifact=initial-triangulation([\r\n]|$)")
  message(FATAL_ERROR "initialize did not publish an initial-triangulation artifact")
endif()
if(NOT initial_metadata MATCHES "(^|[\r\n])topology[.]fnv1a64=([0-9a-f]+)([\r\n]|$)")
  message(FATAL_ERROR "initialize metadata is missing its topology fingerprint")
endif()
set(initial_topology_fingerprint "${CMAKE_MATCH_2}")
if(NOT initial_metadata MATCHES "(^|[\r\n])placement[.]fnv1a64=([0-9a-f]+)([\r\n]|$)")
  message(FATAL_ERROR "initialize metadata is missing its placement fingerprint")
endif()
set(initial_placement_fingerprint "${CMAKE_MATCH_2}")

set(unmanifested_payload "${unmanifested_directory}/initial.off")
file(COPY_FILE "${initial_payload}" "${unmanifested_payload}" ONLY_IF_DIFFERENT)
execute_process(
  COMMAND
    "${CDT_EXECUTABLE}" --input "${unmanifested_payload}" -a0.6 -k1.1 -l0.1
    -p1 --seed 93 --no-output
  WORKING_DIRECTORY "${unmanifested_directory}"
  RESULT_VARIABLE unmanifested_result
  OUTPUT_VARIABLE unmanifested_output
  ERROR_VARIABLE unmanifested_error)
set(unmanifested_log "${unmanifested_output}\n${unmanifested_error}")
if(unmanifested_result EQUAL 0
   OR NOT unmanifested_log MATCHES "requires a persistence metadata sidecar")
  message(
    FATAL_ERROR
      "cdt accepted an unmanifested initial payload:\n${unmanifested_output}\n${unmanifested_error}")
endif()

execute_process(
  COMMAND
    "${CDT_EXECUTABLE}" --input "${initial_payload}" -s -a0.6 -k1.1 -l0.1
    -p1 --seed 93 --no-output
  WORKING_DIRECTORY "${cdt_directory}"
  RESULT_VARIABLE conflict_result
  OUTPUT_VARIABLE conflict_output
  ERROR_VARIABLE conflict_error)
set(conflict_log "${conflict_output}\n${conflict_error}")
if(conflict_result EQUAL 0
   OR NOT conflict_log MATCHES "--input cannot be combined with")
  message(
    FATAL_ERROR
      "cdt accepted conflicting input/construction options:\n${conflict_output}\n${conflict_error}")
endif()

execute_process(
  COMMAND
    "${CDT_EXECUTABLE}" --input "${initial_payload}" -a0.6 -k1.1 -l0.1
    -p1 -c10 --seed 93
  WORKING_DIRECTORY "${cdt_directory}"
  RESULT_VARIABLE cdt_result
  OUTPUT_VARIABLE cdt_output
  ERROR_VARIABLE cdt_error)
if(NOT cdt_result EQUAL 0)
  message(FATAL_ERROR "cdt input run failed:\n${cdt_output}\n${cdt_error}")
endif()
if(NOT cdt_output MATCHES "Input initial triangulation:")
  message(FATAL_ERROR "cdt did not report the loaded initial triangulation")
endif()
if(NOT cdt_output MATCHES "Input initialization seed: 92([\r\n]|$)")
  message(FATAL_ERROR "cdt did not report the input initialization seed")
endif()
if(NOT cdt_output MATCHES "Effective random seed: 93([\r\n]|$)")
  message(FATAL_ERROR "cdt did not report the new transition-run seed")
endif()

file(GLOB final_payloads LIST_DIRECTORIES false "${cdt_directory}/*.off")
file(GLOB final_manifests LIST_DIRECTORIES false "${cdt_directory}/*.off.meta")
list(LENGTH final_payloads final_payload_count)
list(LENGTH final_manifests final_manifest_count)
if(NOT final_payload_count EQUAL 1 OR NOT final_manifest_count EQUAL 1)
  message(
    FATAL_ERROR
      "cdt must publish exactly one evolved OFF/metadata pair; found ${final_payload_count} payloads and ${final_manifest_count} manifests")
endif()
list(GET final_payloads 0 final_payload)
list(GET final_manifests 0 final_manifest)
if(NOT final_manifest STREQUAL "${final_payload}.meta")
  message(FATAL_ERROR "cdt metadata is not paired with its payload")
endif()

file(READ "${final_manifest}" final_metadata)
foreach(
    required
    "artifact=final-triangulation"
    "random.seed=93"
    "input.artifact=initial-triangulation"
    "input.random.seed=92"
    "input.random.initialization_stream=0"
    "input.placement.fnv1a64=${initial_placement_fingerprint}"
    "input.topology.fnv1a64=${initial_topology_fingerprint}"
    "transition_trace.fnv1a64=")
  if(NOT final_metadata MATCHES "${required}")
    message(FATAL_ERROR "Evolved metadata is missing '${required}':\n${final_metadata}")
  endif()
endforeach()
if(NOT final_metadata MATCHES "transition_trace[.]count=([1-9][0-9]*)")
  message(FATAL_ERROR "The evolved artifact did not record any CDT transitions")
endif()

execute_process(
  COMMAND
    "${CDT_EXECUTABLE}" --input "${initial_payload}" -a0.6 -k1.1 -l0.1
    -p1 -c1 --seed 94
  WORKING_DIRECTORY "${checkpoint_directory}"
  RESULT_VARIABLE checkpoint_result
  OUTPUT_VARIABLE checkpoint_output
  ERROR_VARIABLE checkpoint_error)
if(NOT checkpoint_result EQUAL 0)
  message(
    FATAL_ERROR
      "cdt checkpoint run failed:\n${checkpoint_output}\n${checkpoint_error}")
endif()
file(GLOB checkpoint_payloads LIST_DIRECTORIES false
     "${checkpoint_directory}/*-pass-1.off")
list(LENGTH checkpoint_payloads checkpoint_payload_count)
if(NOT checkpoint_payload_count EQUAL 1)
  message(
    FATAL_ERROR
      "cdt must publish exactly one pass-1 checkpoint; found ${checkpoint_payload_count}")
endif()
list(GET checkpoint_payloads 0 checkpoint_payload)

execute_process(
  COMMAND
    "${CDT_EXECUTABLE}" --input "${checkpoint_payload}" -a0.6 -k1.1 -l0.1
    -p1 --seed 95 --no-output
  WORKING_DIRECTORY "${checkpoint_directory}"
  RESULT_VARIABLE checkpoint_input_result
  OUTPUT_VARIABLE checkpoint_input_output
  ERROR_VARIABLE checkpoint_input_error)
set(checkpoint_input_log
    "${checkpoint_input_output}\n${checkpoint_input_error}")
if(checkpoint_input_result EQUAL 0
   OR NOT checkpoint_input_log MATCHES "CDT input must be an initial-triangulation artifact")
  message(
    FATAL_ERROR
      "cdt accepted a checkpoint artifact as a new initial state:\n${checkpoint_input_output}\n${checkpoint_input_error}")
endif()

execute_process(
  COMMAND
    "${CDT_EXECUTABLE}" --input "${final_payload}" -a0.6 -k1.1 -l0.1 -p1
    --seed 94 --no-output
  WORKING_DIRECTORY "${cdt_directory}"
  RESULT_VARIABLE final_input_result
  OUTPUT_VARIABLE final_input_output
  ERROR_VARIABLE final_input_error)
set(final_input_log "${final_input_output}\n${final_input_error}")
if(final_input_result EQUAL 0
   OR NOT final_input_log MATCHES "CDT input must be an initial-triangulation artifact")
  message(
    FATAL_ERROR
      "cdt accepted a final artifact as a new initial state:\n${final_input_output}\n${final_input_error}")
endif()

file(GLOB temporary_files LIST_DIRECTORIES false "${normalized_test_directory}/*/*.tmp")
if(temporary_files)
  list(JOIN temporary_files "\n  " temporary_file_list)
  message(FATAL_ERROR "The handoff left temporary files behind:\n  ${temporary_file_list}")
endif()
