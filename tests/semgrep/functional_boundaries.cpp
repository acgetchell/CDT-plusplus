template <typename Derived>
class Static_polymorphism
{};

// ruleid: cdt.cpp.no-crtp-inheritance
class Concrete_move : public Static_polymorphism<Concrete_move>
{};

// ruleid: cdt.cpp.no-crtp-inheritance
struct Final_move final : private cdt::detail::Static_polymorphism<Final_move>
{};

template <typename Payload>
// ruleid: cdt.cpp.no-crtp-inheritance
class Generic_move : protected Static_polymorphism<Generic_move<Payload>>
{};

// ruleid: cdt.cpp.no-crtp-inheritance
class Layered_move
    : public Move_interface
    , public Static_polymorphism<Layered_move>
{};

// ok: cdt.cpp.no-crtp-inheritance
class Runtime_move : public Move_interface
{};

// ok: cdt.cpp.no-crtp-inheritance
class Policy_container
    : public Move_policy<Static_polymorphism<Policy_container>>
{};

class Composed_move
{
  // ok: cdt.cpp.no-crtp-inheritance
  Static_polymorphism<Composed_move_state> implementation;
};

class Imperative_move_run
{
 public:
  void reset()
  {
    // ruleid: cdt.cpp.move-run-accounting-is-value-oriented
    m_attempted_moves.reset();
    // ruleid: cdt.cpp.move-run-accounting-is-value-oriented
    m_succeeded_moves.reset();
  }

  void consume(Command& command)
  {
    // ruleid: cdt.cpp.move-run-accounting-is-value-oriented
    m_attempted_moves += command.get_attempted();
    // ruleid: cdt.cpp.move-run-accounting-is-value-oriented
    m_failed_moves += command.get_failed();
  }

 private:
  Move_counter m_attempted_moves{};
  Move_counter m_succeeded_moves{};
  Move_counter m_failed_moves{};
};

class Value_oriented_move_run
{
 public:
  void commit(MoveRunResult result)
  {
    // ok: cdt.cpp.move-run-accounting-is-value-oriented
    m_result = std::move(result);
  }

 private:
  MoveRunResult m_result{};
};

auto collect_command_result(Command& command)
{
  Move_counter attempted_moves{};
  // ok: cdt.cpp.move-run-accounting-is-value-oriented
  attempted_moves += command.get_attempted();
  return MoveCommandResults{std::move(attempted_moves), {}, {}};
}

class Raw_cadence_strategy
{
 public:
  void execute()
  {
    // ruleid: cdt.cpp.move-run-cadence-is-parsed-once
    if (m_passes <= 0) { return; }
  }

 private:
  // ruleid: cdt.cpp.move-run-cadence-is-parsed-once
  Int_precision m_passes{1};
  // ruleid: cdt.cpp.move-run-cadence-is-parsed-once
  std::size_t m_checkpoint = 1;
};

class Parsed_cadence_strategy
{
 private:
  // ok: cdt.cpp.move-run-cadence-is-parsed-once
  MoveRunCadence m_cadence{};
};

auto parse_cadence(Int_precision raw_passes, Int_precision raw_checkpoint)
{
  // ok: cdt.cpp.move-run-cadence-is-parsed-once
  return MoveRunCadence::parse(raw_passes, raw_checkpoint);
}
