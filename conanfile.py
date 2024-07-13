from conan import ConanFile
from conan.tools.cmake import CMake, cmake_layout
from conan.tools.files import copy
from conan.tools.build import check_min_cppstd


class SloRecipe(ConanFile):
    name = "slo"
    version = "0.1"
    package_type = "header-library"

    # Optional metadata
    license = "MIT"
    author = "Tsche che@palliate.io"
    url = "<Package recipe repository url here, for issues about the package>"
    description = "<Description of slo package here>"
    topics = ("<Put some tag here>", "<here>", "<and here>")

    # Binary configuration
    settings = "os", "compiler", "build_type", "arch"
    options = {
        "benchmark": [True, False],
        "sanitizers": [True, False],
        "coverage": [True, False],
    }
    default_options = {"benchmark": False, "sanitizers": False,
                       "coverage": False}
    generators = "CMakeToolchain", "CMakeDeps"

    exports_sources = "CMakeLists.txt", "include/*"

    def validate(self):
        if self.settings.compiler.get_safe("cppstd"):
            check_min_cppstd(self, "23")

    def requirements(self):
        if self.options.benchmark:
            self.requires("benchmark/1.8.3")

        self.test_requires("gtest/1.14.0")

    def layout(self):
        cmake_layout(self)

    def build(self):
        if not self.conf.get("tools.build:skip_test", default=False):
            cmake = CMake(self)
            cmake.configure(
                variables={
                    "ENABLE_BENCHMARK": self.options.benchmark,
                    "ENABLE_SANITIZERS": self.options.sanitizers,
                    "ENABLE_COVERAGE": self.options.coverage,
                }
            )
            cmake.build()

    def package(self):
        copy(self, "*.h", self.source_folder, self.package_folder)

    def package_info(self):
        self.cpp_info.bindirs = []
        self.cpp_info.libdirs = []

    def package_id(self):
        self.info.clear()
