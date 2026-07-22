if(NOT DEFINED TEST_EXECUTABLE OR NOT EXISTS "${TEST_EXECUTABLE}")
  message(FATAL_ERROR "TEST_EXECUTABLE must name a built executable")
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

if(NOT DEFINED EXPECTED_ARTIFACT OR EXPECTED_ARTIFACT STREQUAL "")
  message(FATAL_ERROR "EXPECTED_ARTIFACT must name the persisted artifact kind")
endif()

file(REMOVE_RECURSE "${normalized_test_directory}")
file(MAKE_DIRECTORY "${normalized_test_directory}")

execute_process(
  COMMAND "${TEST_EXECUTABLE}" ${TEST_ARGUMENTS}
  WORKING_DIRECTORY "${normalized_test_directory}"
  RESULT_VARIABLE run_result
  OUTPUT_VARIABLE run_output
  ERROR_VARIABLE run_error)
if(NOT run_result EQUAL 0)
  message(FATAL_ERROR "Persistence command failed:\n${run_output}\n${run_error}")
endif()
if(NOT run_output MATCHES "Effective random seed: ([0-9]+)")
  message(FATAL_ERROR "Command did not report the requested seed:\n${run_output}")
endif()
set(reported_seed "${CMAKE_MATCH_1}")
if(DEFINED EXPECTED_SEED AND NOT reported_seed STREQUAL EXPECTED_SEED)
  message(FATAL_ERROR "Command reported seed ${reported_seed}, expected ${EXPECTED_SEED}")
endif()

file(GLOB payloads LIST_DIRECTORIES false "${normalized_test_directory}/*.off")
file(GLOB manifests LIST_DIRECTORIES false "${normalized_test_directory}/*.off.meta")
list(LENGTH payloads payload_count)
list(LENGTH manifests manifest_count)
if(NOT payload_count EQUAL 1 OR NOT manifest_count EQUAL 1)
  message(
    FATAL_ERROR
      "Expected one payload and one manifest, found ${payload_count} payloads and ${manifest_count} manifests")
endif()

list(GET payloads 0 payload)
list(GET manifests 0 manifest)
get_filename_component(payload_name "${payload}" NAME)
if(payload_name MATCHES ":")
  message(FATAL_ERROR "Generated filename is not Windows-portable: ${payload_name}")
endif()
if(NOT manifest STREQUAL "${payload}.meta")
  message(FATAL_ERROR "Manifest is not paired with its payload: ${manifest}")
endif()

file(READ "${manifest}" metadata)
foreach(
    required
    "cdt-plusplus-metadata-v1"
    "artifact=${EXPECTED_ARTIFACT}"
    "resume_supported=false"
    "fresh_topology_replay_supported=false"
    "transition_replay_requires_identical_start=true"
    "random.seed=${reported_seed}"
    "payload.size="
    "payload.fnv1a64="
    "placement.fnv1a64="
    "topology.fnv1a64="
    "cdt.version="
    "build.configuration="
    "dependency.cgal_version=")
if(NOT metadata MATCHES "${required}")
    message(FATAL_ERROR "Manifest is missing '${required}':\n${metadata}")
  endif()
endforeach()
if(EXPECTED_ARTIFACT STREQUAL "final-triangulation"
   AND (NOT metadata MATCHES "transition_trace.fnv1a64="
        OR NOT metadata MATCHES "transition_trace.count="))
  message(FATAL_ERROR "Simulation manifest does not record its transition trace:\n${metadata}")
endif()

file(GLOB temporary_files LIST_DIRECTORIES false "${normalized_test_directory}/*.tmp")
if(temporary_files)
  list(JOIN temporary_files "\n  " temporary_file_list)
  message(FATAL_ERROR "Persistence left temporary files behind:\n  ${temporary_file_list}")
endif()
