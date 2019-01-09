/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright © 2018 Adam Getchell

/// @file MoveGuard.hpp
/// @brief RAII wrapper for moves

#ifndef CDT_PLUSPLUS_MOVEGUARD_HPP
#define CDT_PLUSPLUS_MOVEGUARD_HPP

#include <functional>
#include <optional>

template <typename ManifoldType>
class MoveGuard
{
 public:
  using ValueType    = typename ManifoldType::value_type;
  using FunctionType = std::function<ValueType(ValueType)>;

  MoveGuard(ManifoldType manifold, FunctionType fn)
      : _triangulation{std::make_unique<ManifoldType>(manifold)}, _fn{fn}
  {}

  std::optional<ManifoldType> operator()() { return std::nullopt; }

 private:
  std::unique_ptr<ManifoldType> _triangulation;
  FunctionType                  _fn;
};

#endif  // CDT_PLUSPLUS_MOVEGUARD_HPP
