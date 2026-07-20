void routine_move_diagnostics()
{
  // ruleid: cdt.cpp.no-routine-output-in-move-hot-paths
  spdlog::trace("proposal started");
  // ruleid: cdt.cpp.no-routine-output-in-move-hot-paths
  spdlog::debug("proposal rejected");
  // ruleid: cdt.cpp.no-routine-output-in-move-hot-paths
  spdlog::info("move applied");
  // ruleid: cdt.cpp.no-routine-output-in-move-hot-paths
  fmt::print("move applied\n");
}

void invariant_diagnostics()
{
  // ok: cdt.cpp.no-routine-output-in-move-hot-paths
  spdlog::warn("incident cells require repair");
  // ok: cdt.cpp.no-routine-output-in-move-hot-paths
  spdlog::error("move violated a topology invariant");
}
