#include <concepts>
#include <cstdint>
#include <expected>
#include <random>
#include <string>
#include <type_traits>

#include "Ergodic_moves_3.hpp"
#include "Metropolis.hpp"
#include "Move_always.hpp"
#include "Move_command.hpp"
#include "Runtime_config.hpp"
#include "S3Action.hpp"

using Manifold    = cdt::manifolds::Manifold_3;
using Move_result = std::expected<Manifold, std::string>;

static_assert(std::same_as<cdt::Int_precision, std::int32_t>);
static_assert(std::uniform_random_bit_generator<cdt::Random>);
static_assert(std::same_as<cdt::Geometry_3, cdt::Geometry<3>>);
static_assert(
    std::same_as<cdt::foliated_triangulations::FoliatedTriangulation_3,
                 cdt::foliated_triangulations::FoliatedTriangulation<3>>);
static_assert(
    std::same_as<cdt::MoveAlways_3,
                 cdt::MoveStrategy<cdt::Strategies::MOVE_ALWAYS, Manifold>>);
static_assert(
    std::same_as<cdt::Metropolis_3,
                 cdt::MoveStrategy<cdt::Strategies::METROPOLIS, Manifold>>);
static_assert(std::is_constructible_v<cdt::MoveCommand<Manifold>, Manifold>);

static_assert(requires(Manifold const& manifold, cdt::Random& random) {
  { cdt::ergodic_moves::null_move(manifold) } -> std::same_as<Move_result>;
  {
    cdt::ergodic_moves::do_23_move(manifold, random)
  } -> std::same_as<Move_result>;
  {
    cdt::ergodic_moves::do_32_move(manifold, random)
  } -> std::same_as<Move_result>;
  {
    cdt::ergodic_moves::do_26_move(manifold, random)
  } -> std::same_as<Move_result>;
  {
    cdt::ergodic_moves::do_62_move(manifold, random)
  } -> std::same_as<Move_result>;
  {
    cdt::ergodic_moves::do_44_move(manifold, random)
  } -> std::same_as<Move_result>;
  {
    cdt::ergodic_moves::propose_23_move(manifold, random)
  } -> std::same_as<Move_result>;
});

static_assert(requires {
  {
    cdt::s3_action::make_physical_parameters(0.6L, 1.1L, 0.1L)
  } -> std::same_as<cdt::s3_action::PhysicalParameters>;
});
