from conans import ConanFile, CMake

class CausalDynamicalTriangulations(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    requires = "boost/1.68.0@conan/stable", "catch2/2.3.0@bincrafters/stable", "TBB/2018_U5@conan/stable",\
               "eigen/3.3.5@conan/stable", "libcurl/7.60.0@bincrafters/stable", "docopt/0.6.2@conan/stable"
    generators = "cmake"
    default_options = "boost:without_thread=False"
    # default_options = "Boost:header_only=True"

    def build(self):
        cmake = CMake(self)
        cmake.verbose = True
        cmake.configure(args=["CMAKE_BUILD_TYPE=Release"])
        cmake.build()

    def build_requirements(self):
        self.build_requires("cmake_installer/3.11.3@conan/stable")