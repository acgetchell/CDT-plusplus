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
            fmt::fmt
            fmt::fmt-header-only
            Eigen3::Eigen
            TBB::tbb
            CGAL::CGAL)
endif()
target_compile_features(initialize PRIVATE cxx_std_17)

# add_executable(cdt-opt ${PROJECT_SOURCE_DIR}/src/cdt-opt.cpp) target_link_libraries( cdt-opt PRIVATE date::tz
# date::date Catch2::Catch2 Eigen3::Eigen # TBB::tbb CGAL::CGAL) target_compile_features(cdt-opt PRIVATE cxx_std_17)

if(UNIX)
  add_executable(cdt-gv ${PROJECT_SOURCE_DIR}/src/cdt-gv.cpp)
  target_link_libraries(
    cdt-gv
    PRIVATE project_options
            project_warnings
            docopt_s
            fmt::fmt-header-only
            CGAL::CGAL)
  target_compile_features(cdt-gv PRIVATE cxx_std_17)
endif()

add_executable(cdt-opt ${PROJECT_SOURCE_DIR}/src/cdt-opt.cpp)
if (WIN32)
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
              CGAL::CGAL
  )
endif ()
target_compile_features(cdt-opt PRIVATE cxx_std_17)

#
# Tests    ##
#

# CTest basic testing Parameter initializer program
add_test(NAME CDT-Initialize COMMAND $<TARGET_FILE:initialize>)
set_tests_properties(CDT-Initialize PROPERTIES PASS_REGULAR_EXPRESSION "Usage:./initialize")

if(UNIX)
  add_test(NAME CDT-gv COMMAND $<TARGET_FILE:cdt-gv>)
  set_tests_properties(CDT-gv PROPERTIES PASS_REGULAR_EXPRESSION "Usage:./cdt-gv")
endif()

add_test(NAME CDT-Opt COMMAND $<TARGET_FILE:cdt-opt>)
set_tests_properties(CDT-Opt PROPERTIES PASS_REGULAR_EXPRESSION "cdt-opt started")
# add_test(NAME CDT-Opt COMMAND $<TARGET_FILE:cdt-opt>) set_tests_properties( CDT-Opt PROPERTIES PASS_REGULAR_EXPRESSION
# "64000 simplices on 16 timeslices desired.")
