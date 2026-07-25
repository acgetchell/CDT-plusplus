void streamed_console_output(int const value)
{
  // ruleid: cdt.cpp.use-fmt-print-for-console-output
  std::cout << "value: " << value << '\n';
  // ruleid: cdt.cpp.use-fmt-print-for-console-output
  std::cerr << "error: " << value << '\n';
  // ruleid: cdt.cpp.use-fmt-print-for-console-output
  std::clog << "diagnostic: " << value << '\n';
}

void formatted_console_output(int const value)
{
  // ok: cdt.cpp.use-fmt-print-for-console-output
  fmt::print("value: {}\n", value);
  // ok: cdt.cpp.use-fmt-print-for-console-output
  fmt::print(stderr, "error: {}\n", value);
}

void serialization_contract(std::ostream& output, int const value)
{
  // ok: cdt.cpp.use-fmt-print-for-console-output
  output << value;
}
