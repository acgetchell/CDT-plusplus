# From Paul Fultz
#
# lcov.cmake
# `ctest --quiet -S lcov.cmake`

include(ProcessorCount)
ProcessorCount(N)

set(CTEST_PROJECT_NAME CoverageProject)
set(CTEST_SOURCE_DIRECTORY ${CMAKE_CURRENT_LIST_DIR})
set(CTEST_BINARY_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/build)
set(CTEST_CMAKE_GENERATOR "Ninja")
#set(CTEST_MEMORYCHECK_COMMAND /usr/bin/valgrind)

set(LCOV_OUT "${CTEST_BINARY_DIRECTORY}/lcov_html")

ctest_start(lcov)
message("configuring...")
ctest_configure(BUILD "${CTEST_BINARY_DIRECTORY}" OPTIONS "-DTESTS:BOOL=ON;-DCMAKE_CXX_FLAGS=-g -O0 -fprofile-arcs -ftest-coverage;-DCMAKE_BUILD_TYPE=Debug")
message("lcov: resetting counters...")
execute_process(COMMAND lcov -z -d ${CTEST_BINARY_DIRECTORY}
        WORKING_DIRECTORY ${CTEST_BINARY_DIRECTORY} OUTPUT_QUIET)

message("building...")
ctest_build(BUILD "${CTEST_BINARY_DIRECTORY}" FLAGS -j${N})


message("running tests...")
ctest_test(BUILD "${CTEST_BINARY_DIRECTORY}" PARALLEL_LEVEL ${N})

message("analyzing profiling data using lcov...")
execute_process(COMMAND lcov -c -d ${CTEST_BINARY_DIRECTORY} -o ${CTEST_BINARY_DIRECTORY}/stepcode.lcov
        WORKING_DIRECTORY ${CTEST_BINARY_DIRECTORY} OUTPUT_QUIET)

message("removing system headers, CGAL, Boost, TBB, docopt, date, test ...")
execute_process(COMMAND lcov -r ${CTEST_BINARY_DIRECTORY}/stepcode.lcov "*/usr/include" "*/usr/include/*" "*CGAL*" "*CGAL/*" "*boost*" "*boost/*" "*tbb*" "*tbb/*" "*/src/date/*" "*/docopt*" "*test/*"
        -o ${CTEST_BINARY_DIRECTORY}/stepcode_filtered.lcov
        WORKING_DIRECTORY ${CTEST_BINARY_DIRECTORY} OUTPUT_QUIET)
execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory ${LCOV_OUT})

message("creating html files...")
execute_process(COMMAND genhtml ${CTEST_BINARY_DIRECTORY}/stepcode_filtered.lcov
        WORKING_DIRECTORY ${LCOV_OUT} OUTPUT_QUIET)

message("html files are located in ${LCOV_OUT}/index.html")

message("================================================ Success! ================================================")
