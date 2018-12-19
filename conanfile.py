from conans import ConanFile, CMake

class CausalDynamicalTriangulations(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    requires = "boost/1.69.0@conan/stable", "catch2/2.4.1@bincrafters/stable", \
               "docopt/0.6.2@conan/stable",\
               "date/2.4.1@bincrafters/stable", "gsl_microsoft/2.0.0@bincrafters/stable"
    generators = "cmake"
    default_options = "boost:shared=False"
    default_options = "boost:header_only=True"
    # default_options = "boost:magic_autolink=True"

    def build(self):
        cmake = CMake(self)
        cmake.verbose = True
        cmake.definitions["CMAKE_BUILD_TYPE"] = "RelWithDebInfo"
        cmake.configure(args=["CMAKE_EXPORT_COMPILE_COMMANDS=ON"])
        cmake.build()

    # def build_requirements(self):
    #     self.build_requires("cmake_installer/3.12.0@conan/stable")