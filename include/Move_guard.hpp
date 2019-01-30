/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright © 2018-2019 Adam Getchell

/// @file Move_guard.hpp
/// @brief RAII wrapper for moves

#ifndef CDT_PLUSPLUS_MOVEGUARD_HPP
#define CDT_PLUSPLUS_MOVEGUARD_HPP

#include <functional>
#include <optional>

template <typename ManifoldType>
class Move_guard
{
 public:
  using FunctionType = std::function<ManifoldType(ManifoldType const&)>;

  Move_guard(ManifoldType manifold, FunctionType function)
      : triangulation_{std::move(manifold)}, function_{function}
  {}

  std::optional<ManifoldType> operator()()
  {
    try
    {
      return function_(triangulation_);
    }
    catch (...)
    {
      return std::nullopt;
    }
  }

  ManifoldType get_triangulation() const { return triangulation_; }

 private:
  ManifoldType triangulation_;
  FunctionType function_;
};

#endif  // CDT_PLUSPLUS_MOVEGUARD_HPP
