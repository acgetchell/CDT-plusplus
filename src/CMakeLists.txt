add_executable(initialize ${PROJECT_SOURCE_DIR}/src/initialize.cpp)

# On macOS and Linux docopt builds an intermediate object, thus different targets than Windows See
# https://github.com/microsoft/vcpkg/issues/8666
if(WIN32)
  target_link_libraries(
    initialize
    PRIVATE project_options
            project_warnings
            docopt
            date::date
            date::date-tz
            fmt::fmt-header-only
            Eigen3::Eigen
            TBB::tbb
            CGAL::CGAL)
else()
  target_link_libraries(
    initialize
    PRIVATE project_options
            project_warnings
            docopt_s
            date::date
            date::date-tz
            fmt::fmt-header-only
            Eigen3::Eigen
            TBB::tbb
            CGAL::CGAL)
endif()
target_compile_features(initialize PRIVATE cxx_std_17)

# add_executable(cdt-opt ${PROJECT_SOURCE_DIR}/src/cdt-opt.cpp) target_link_libraries( cdt-opt PRIVATE date::tz
# date::date Catch2::Catch2 Eigen3::Eigen # TBB::tbb CGAL::CGAL) target_compile_features(cdt-opt PRIVATE cxx_std_17)

# if(UNIX) add_executable(cdt-gv ${PROJECT_SOURCE_DIR}/src/cdt-gv.cpp) target_link_libraries( cdt-gv PRIVATE
# project_options project_warnings docopt_s fmt::fmt-header-only CGAL::CGAL) target_compile_features(cdt-gv PRIVATE
# cxx_std_17) endif()

add_executable(cdt-opt ${PROJECT_SOURCE_DIR}/src/cdt-opt.cpp)
if(WIN32)
  target_link_libraries(
    cdt-opt
    PRIVATE project_options
            project_warnings
            docopt
            date::date
            date::date-tz
            fmt::fmt-header-only
            Eigen3::Eigen
            TBB::tbb
            CGAL::CGAL)
else()
  target_link_libraries(
    cdt-opt
    PRIVATE project_options
            project_warnings
            docopt_s
            date::date
            date::date-tz
            fmt::fmt-header-only
            Eigen3::Eigen
            TBB::tbb
            CGAL::CGAL)
endif()
target_compile_features(cdt-opt PRIVATE cxx_std_17)

add_executable(cdt ${PROJECT_SOURCE_DIR}/src/cdt.cpp)
if(WIN32)
  target_link_libraries(
    cdt
    PRIVATE project_options
            project_warnings
            docopt
            date::date
            date::date-tz
            fmt::fmt-header-only
            Eigen3::Eigen
            TBB::tbb
            CGAL::CGAL)
else()
  target_link_libraries(
    cdt
    PRIVATE project_options
            project_warnings
            docopt_s
            date::date
            date::date-tz
            fmt::fmt-header-only
            Eigen3::Eigen
            TBB::tbb
            CGAL::CGAL)
endif()
target_compile_features(cdt PRIVATE cxx_std_17)

#
# Tests    ##
#

add_test(NAME cdt COMMAND $<TARGET_FILE:cdt> --s -n64 -t3 -a0.6 -k1.1 -l0.1 -p10)
set_tests_properties(cdt PROPERTIES PASS_REGULAR_EXPRESSION "Writing to file S3-3-")

add_test(NAME cdt-triangle-inequalities COMMAND $<TARGET_FILE:cdt> --s -n64 -t3 -a0.4 -k1.1 -l0.1)
set_tests_properties(cdt-triangle-inequalities PROPERTIES PASS_REGULAR_EXPRESSION "Triangle inequalities violated")

add_test(NAME cdt-dimensionality COMMAND $<TARGET_FILE:cdt> --s -n64 -t3 -d4 -a0.4 -k1.1 -l0.1)
set_tests_properties(cdt-dimensionality PROPERTIES PASS_REGULAR_EXPRESSION "Currently, dimensions cannot be >3.")

add_test(NAME cdt-toroidal COMMAND $<TARGET_FILE:cdt> --t -n64 -t3 -a0.6 -k1.1 -l0.1)
set_tests_properties(cdt-toroidal PROPERTIES PASS_REGULAR_EXPRESSION "Toroidal triangulations not yet supported.")

add_test(NAME initialize COMMAND $<TARGET_FILE:initialize> --s -n640 -t4 -o)
set_tests_properties(initialize PROPERTIES PASS_REGULAR_EXPRESSION "Writing to file S3-4")

add_test(NAME initialize-minimum-simplices COMMAND $<TARGET_FILE:initialize> --s -n1 -t1 -o)
set_tests_properties(initialize-minimum-simplices
                     PROPERTIES PASS_REGULAR_EXPRESSION "Simplices and timeslices should be greater or equal to 2.")

add_test(NAME initialize-dimensionality COMMAND $<TARGET_FILE:initialize> --s -n64 -t3 -d4 -o)
set_tests_properties(initialize-dimensionality PROPERTIES PASS_REGULAR_EXPRESSION "Currently, dimensions cannot be >3.")

add_test(NAME initialize-toroidal COMMAND $<TARGET_FILE:initialize> --t -n64 -t3 -o)
set_tests_properties(initialize-toroidal PROPERTIES PASS_REGULAR_EXPRESSION
                                                    "Toroidal triangulations not yet supported.")

# if(UNIX) add_test(NAME cdt-gv COMMAND $<TARGET_FILE:cdt-gv>) set_tests_properties(cdt-gv PROPERTIES
# PASS_REGULAR_EXPRESSION "Usage:./cdt-gv") endif()

add_test(NAME cdt-opt COMMAND $<TARGET_FILE:cdt-opt>)
set_tests_properties(cdt-opt PROPERTIES PASS_REGULAR_EXPRESSION "64 simplices on 3 timeslices desired.")
