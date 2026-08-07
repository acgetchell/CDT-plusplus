cmake_minimum_required(VERSION 4.4.0)

project(cgal-no-gmp-probe LANGUAGES CXX)

find_package(CGAL 6.2 EXACT CONFIG REQUIRED)

add_executable(cgal_no_gmp_probe main.cpp)
target_compile_features(cgal_no_gmp_probe PRIVATE cxx_std_23)
target_link_libraries(cgal_no_gmp_probe PRIVATE CGAL::CGAL)
