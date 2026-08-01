#include "Simulation_output.hpp"

#include <string>

#include <doctest/doctest.h>

using namespace cdt::four_d::output;

TEST_CASE("4D JSON output escapes control characters")
{
  std::string controls;
  controls.push_back('\x01');
  controls.push_back('\x1F');

  CHECK_EQ(json_escape(controls), "\\u0001\\u001F");
  CHECK_EQ(json_escape("\"\\\b\f\n\r\t"), "\\\"\\\\\\b\\f\\n\\r\\t");
}
