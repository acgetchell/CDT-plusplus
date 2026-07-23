#include <concepts>
#include <cstdint>
#include <expected>
#include <random>
#include <string>
#include <type_traits>

#include "Ergodic_moves_3.hpp"
#include "Foliated_triangulation.hpp"
#include "Metropolis.hpp"
#include "Move_always.hpp"
#include "Move_command.hpp"
#include "Move_run.hpp"
#include "Runtime_config.hpp"
#include "S3Action.hpp"

using Manifold     = cdt::manifolds::Manifold_3;
using Move_command = cdt::MoveCommand<Manifold>;
using Move_result  = cdt::ergodic_moves::MoveResult<Manifold>;

template <typename Result>
concept Supported_move_command_result =
    requires { typename cdt::MoveCommand<Manifold, Result>; };

static_assert(std::same_as<cdt::Int_precision, std::int32_t>);
static_assert(std::uniform_random_bit_generator<cdt::Random>);
static_assert(std::same_as<cdt::Geometry_3, cdt::Geometry<3>>);
static_assert(
    std::same_as<cdt::foliated_triangulations::FoliatedTriangulation_3,
                 cdt::foliated_triangulations::FoliatedTriangulation<3>>);
static_assert(requires(cdt::Random& random) {
  {
    cdt::foliated_triangulations::make_triangulation<3>(2, 2, 1.0, 1.0, random)
  } -> std::same_as<cdt::Delaunay_t<3>>;
});
static_assert(
    std::same_as<cdt::MoveAlways_3,
                 cdt::MoveStrategy<cdt::Strategies::MOVE_ALWAYS, Manifold>>);
static_assert(
    std::same_as<cdt::Metropolis_3,
                 cdt::MoveStrategy<cdt::Strategies::METROPOLIS, Manifold>>);
static_assert(std::is_constructible_v<Move_command, Manifold>);
static_assert(
    std::same_as<Move_command, cdt::MoveCommand<Manifold, Move_result>>);
static_assert(Supported_move_command_result<Move_result>);
static_assert(
    !Supported_move_command_result<std::expected<Manifold, std::string>>);
static_assert(requires {
  {
    cdt::MoveRunCadence::parse(1, 1)
  }
  -> std::same_as<std::expected<cdt::MoveRunCadence, cdt::MoveRunCadenceError>>;
});

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
  {
    cdt::ergodic_moves::propose_32_move(manifold, random)
  } -> std::same_as<Move_result>;
  {
    cdt::ergodic_moves::propose_26_move(manifold, random)
  } -> std::same_as<Move_result>;
  {
    cdt::ergodic_moves::propose_62_move(manifold, random)
  } -> std::same_as<Move_result>;
  {
    cdt::ergodic_moves::propose_44_move(manifold, random)
  } -> std::same_as<Move_result>;
});

static_assert(requires {
  {
    cdt::s3_action::make_physical_parameters(0.6L, 1.1L, 0.1L)
  } -> std::same_as<cdt::s3_action::PhysicalParameters>;
});
