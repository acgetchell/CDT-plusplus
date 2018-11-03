//
// Created by Adam Getchell on 2018-11-02.
//

#ifndef CDT_PLUSPLUS_MOVECOMMAND_HPP
#define CDT_PLUSPLUS_MOVECOMMAND_HPP

#include <Manifold.hpp>

template <std::int_fast64_t dimension>
class MoveCommand;

template <>
class MoveCommand<3>
{
 public:
  /// @brief Default constructor
  MoveCommand() : _manifold{Manifold3{}}, _is_updated{false} {};

  /// @brief Constructor using manifold
  /// @param manifold The manifold on which moves will be performed
  explicit MoveCommand(Manifold3 manifold)
      : _manifold{std::move(manifold)}, _is_updated{false}
  {}

  Manifold3 const& get_manifold() const { return _manifold; }

  bool is_updated() const { return _is_updated; }

  void set_is_updated(bool updated) { MoveCommand::_is_updated = updated; }

 private:
  Manifold3 _manifold;
  bool      _is_updated;
};

using MoveCommand3 = MoveCommand<3>;

#endif  // CDT_PLUSPLUS_MOVECOMMAND_HPP
