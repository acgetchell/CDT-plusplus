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
    /// @brief Accept only the default point format specification.
    /// @param ctx Format-specification parser context.
    /// @return Iterator at the first unconsumed format character.
    constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin())
    { return ctx.begin(); }

    /// @brief Format a point using its stream insertion representation.
    /// @tparam FormatContext fmt output context type.
    /// @param point Point to format.
    /// @param ctx Output context.
    /// @return Iterator following the formatted point.
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
