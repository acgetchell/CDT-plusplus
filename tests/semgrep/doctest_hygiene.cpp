// ruleid: cdt.cpp.doctest-framework-is-test-only
#include <doctest/doctest.h>

// ruleid: cdt.cpp.doctest-framework-is-test-only
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

// ruleid: cdt.cpp.doctest-framework-is-test-only
SCENARIO("Production code owns a doctest scenario") {}

void skipped_scenario()
{
  // ruleid: cdt.cpp.no-skipped-doctests
  auto const decorator = doctest::skip();
}

void diagnostic_only_output(Printable& printable)
{
  // ruleid: cdt.cpp.no-diagnostic-output-in-doctests
  fmt::print("intermediate value: {}\n", 42);
  // ruleid: cdt.cpp.no-diagnostic-output-in-doctests
  std::cout << "intermediate value: " << 42 << '\n';
  // ruleid: cdt.cpp.no-diagnostic-output-in-doctests
  printable.print_results();
}

void asserted_output(std::ostream& output)
{
  // ok: cdt.cpp.no-diagnostic-output-in-doctests
  output << "captured value: " << 42;
}

void temporary_files(std::string const& generated_name)
{
  // ruleid: cdt.cpp.test-temp-paths-are-unique
  auto const fixed  = std::filesystem::temp_directory_path() / "cdt-output.txt";
  // ok: cdt.cpp.test-temp-paths-are-unique
  auto const unique = std::filesystem::temp_directory_path() / generated_name;
}
