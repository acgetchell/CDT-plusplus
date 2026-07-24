#include <concepts>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <random>
#include <type_traits>
#include <utility>

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

template <cdt::MoveStrategyKind Strategy>
concept CompleteMoveStrategy =
    requires { sizeof(cdt::MoveStrategy<Strategy, Manifold>); };

template <typename Number>
concept IntegerRandomSample = requires(cdt::Random& random, Number value) {
  cdt::utilities::generate_random_int(random, value, value);
};

template <typename Number>
concept RealRandomSample = requires(cdt::Random& random, Number value) {
  cdt::utilities::generate_random_real(random, value, value);
};

static_assert(std::same_as<cdt::Int_precision, std::int32_t>);
static_assert(std::uniform_random_bit_generator<cdt::Random>);
static_assert(!std::convertible_to<cdt::RandomSeed, cdt::RandomStream>);
static_assert(!std::convertible_to<cdt::RandomStream, cdt::RandomSeed>);
static_assert(
    std::constructible_from<cdt::Random, cdt::RandomSeed, cdt::RandomStream>);
static_assert(
    !std::constructible_from<cdt::Random, cdt::RandomStream, cdt::RandomSeed>);
static_assert(IntegerRandomSample<int>);
static_assert(!IntegerRandomSample<double>);
static_assert(RealRandomSample<double>);
static_assert(!RealRandomSample<int>);
static_assert(std::same_as<cdt::Geometry_3, cdt::Geometry<3>>);
static_assert(
    std::same_as<cdt::foliated_triangulations::FoliatedTriangulation_3,
                 cdt::foliated_triangulations::FoliatedTriangulation<3>>);
static_assert(requires(cdt::Random& random) {
  {
    cdt::foliated_triangulations::make_triangulation<3>(2, 2, 1.0, 1.0, random)
  } -> std::same_as<cdt::Delaunay_t<3>>;
});
static_assert(requires(cdt::Delaunay_t<3>&          triangulation,
                       cdt::Edge_handle_t<3> const& edge) {
  {
    cdt::foliated_triangulations::classify_edge<3>(edge)
  } -> std::same_as<cdt::EdgeType>;
  {
    cdt::foliated_triangulations::fix_cells<3>(triangulation)
  } -> std::same_as<bool>;
});
static_assert(std::same_as<
              cdt::MoveAlways_3,
              cdt::MoveStrategy<cdt::MoveStrategyKind::MOVE_ALWAYS, Manifold>>);
static_assert(std::same_as<
              cdt::Metropolis_3,
              cdt::MoveStrategy<cdt::MoveStrategyKind::METROPOLIS, Manifold>>);
static_assert(!std::default_initializable<cdt::Metropolis_3>);
static_assert(std::is_constructible_v<Move_command, Manifold>);
static_assert(CompleteMoveStrategy<cdt::MoveStrategyKind::MOVE_ALWAYS>);
static_assert(CompleteMoveStrategy<cdt::MoveStrategyKind::METROPOLIS>);
static_assert(!CompleteMoveStrategy<static_cast<cdt::MoveStrategyKind>(255)>);
static_assert(requires(Move_command&       command,
                       Move_command const& const_command) {
  { command.result() } noexcept -> std::same_as<Manifold&>;
  { const_command.result() } noexcept -> std::same_as<Manifold const&>;
  { std::move(command).result() } noexcept -> std::same_as<Manifold>;
});
static_assert(requires {
  {
    cdt::MoveRunCadence::parse(1, 1)
  }
  -> std::same_as<std::expected<cdt::MoveRunCadence, cdt::MoveRunCadenceError>>;
});
static_assert(requires {
  {
    cdt::move_tracker::move_from_index(0)
  } -> std::same_as<std::optional<cdt::move_tracker::MoveType>>;
  {
    cdt::Metropolis_3::reverse_move(cdt::move_tracker::MoveType::TWO_THREE)
  } -> std::same_as<std::optional<cdt::move_tracker::MoveType>>;
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
static_assert(!std::is_aggregate_v<cdt::s3_action::PhysicalParameters>);
static_assert(!std::is_constructible_v<cdt::s3_action::PhysicalParameters,
                                       long double, long double, long double>);
static_assert(requires(cdt::s3_action::PhysicalParameters const& parameters) {
  { parameters.alpha() } noexcept -> std::same_as<long double>;
  { parameters.k() } noexcept -> std::same_as<long double>;
  { parameters.lambda() } noexcept -> std::same_as<long double>;
  {
    cdt::s3_action::s3_bulk_action(1, 1, 1, parameters)
  } -> std::same_as<cdt::mpfr_values::Value>;
});

static_assert(requires {
  {
    cdt::runtime_config::make_triangulation(true, false, 64, 3, 3, 1.0, 1.0,
                                            cdt::RandomSeed{92}, 4)
  } -> std::same_as<cdt::runtime_config::Triangulation>;
});
static_assert(requires(cdt::runtime_config::Triangulation const& config) {
  { config.threads() } noexcept -> std::same_as<std::size_t>;
});
