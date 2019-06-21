/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright © 2018-2019 Adam Getchell

/// @file Move_guard.hpp
/// @brief RAII wrapper for moves

#ifndef CDT_PLUSPLUS_MOVEGUARD_HPP
#define CDT_PLUSPLUS_MOVEGUARD_HPP

#include <functional>
#include <optional>

template <typename ManifoldType,
          typename FunctionType = std::function<ManifoldType(ManifoldType&)>>
class Move_guard
{
 public:
  Move_guard(ManifoldType manifold, FunctionType function)
      : manifold_{manifold}, function_{function}
  {}

  std::optional<ManifoldType> operator()()
  try
  {
    std::cout << "Perform the move ...\n";
    return function_(manifold_);
  }
  catch (...)
  {
    return std::nullopt;
  }

 private:
  ManifoldType manifold_;
  FunctionType function_;
};

#endif  // CDT_PLUSPLUS_MOVEGUARD_HPP
