/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright © 2018 Adam Getchell

/// @file MoveGuard.hpp
/// @brief RAII wrapper for moves

#ifndef CDT_PLUSPLUS_MOVEGUARD_HPP
#define CDT_PLUSPLUS_MOVEGUARD_HPP

#include <memory>

template <typename ManifoldType>
class MoveGuard
{
    MoveGuard(ManifoldType manifold, void (*move)()) : _triangulation{std::make_unique(manifold)} {}

private:
    std::unique_ptr<ManifoldType> _triangulation;
};

#endif //CDT_PLUSPLUS_MOVEGUARD_HPP
