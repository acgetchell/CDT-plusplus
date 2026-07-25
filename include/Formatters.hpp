/*******************************************************************************
 Causal Dynamical Triangulations in C++ using CGAL

 Copyright © 2025 Adam Getchell
 ******************************************************************************/

/// @file Formatters.hpp
/// @brief Formatter specializations for various types
/// @author Adam Getchell
/// @details Formatter specializations for types used in the project

#ifndef CDT_PLUSPLUS_FORMATTERS_HPP
#define CDT_PLUSPLUS_FORMATTERS_HPP

#include <fmt/core.h>
#include <fmt/format.h>

#include <sstream>

#include "Triangulation_traits.hpp"

namespace fmt
{
  /// @brief Formatter specialization for `CGAL::Point_3`.
  template <typename Kernel>
  struct formatter<CGAL::Point_3<Kernel>>
  {
    // Format specification handling - keeping it simple for now
    constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin())
    { return ctx.begin(); }

    // Format the point as a string with coordinates
    template <typename FormatContext>
    auto format(CGAL::Point_3<Kernel> const& point, FormatContext& ctx) const
        -> decltype(ctx.out())
    {
      std::stringstream ss;
      ss << point;
      return fmt::format_to(ctx.out(), "{}", ss.str());
    }
  };
}  // namespace fmt

#endif  // CDT_PLUSPLUS_FORMATTERS_HPP
