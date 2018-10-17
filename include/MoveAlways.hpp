/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright © 2017 Adam Getchell
///
/// Always picks a random move on the foliated Delaunay triangulations.
/// For testing purposes.
///
/// @file MoveAlways.hpp
/// @brief Always randomly selects moves to perform on Delaunay Triangulations
/// @author Adam Getchell
/// @bug The call operator segfaults in Release mode

#ifndef INCLUDE_MOVE_ALWAYS_HPP_
#define INCLUDE_MOVE_ALWAYS_HPP_

#include <Measurements.hpp>
#include <MoveAlgorithm.hpp>

class MoveAlways : public MoveAlgorithm
{
 public:
  /// @brief Default constructor using default values
  MoveAlways() = default;

  /// @brief Set passes and checkpoint with MoveAlgorithm 2-argument constructor
  /// @param passes Number of passes through triangulation
  /// @param checkpoint Number of passes per checkpoint
  MoveAlways(const std::size_t passes, const std::size_t checkpoint)
      : MoveAlgorithm(passes, checkpoint)
  {
#ifndef NDEBUG
    std::cout << __PRETTY_FUNCTION__ << " called." << std::endl;
#endif
  }

  /// @brief Call operator
  /// @tparam T Type of manifold
  /// @param universe Manifold on which to operate
  /// @return Manifold upon which moves have been completed
  template <typename T>
  auto operator()(T&& universe) -> decltype(universe)
  {
#ifndef NDEBUG
    std::cout << __PRETTY_FUNCTION__ << " called." << std::endl;
#endif
    std::cout << "Starting Move Always algorithm ...\n";
    // Populate member data
    universe_ = std::move(universe);
    N1_TL_    = universe_.geometry->N1_TL();
    N3_31_13_ = universe_.geometry->N3_31_13();
    N3_22_    = universe_.geometry->N3_22();

    std::cout << "Making random moves ..." << std::endl;
    // Loop through passes_
    for (std::size_t pass_number = 1; pass_number <= passes_; ++pass_number)
    {
      auto total_simplices_this_pass = CurrentTotalSimplices();
      // Loop through CurrentTotalSimplices
      for (std::size_t move_attempt = 0;
           move_attempt < total_simplices_this_pass; ++move_attempt)
      {
        // Pick a move to attempt
        auto move_choice = generate_random_int(0, 3);
#ifndef NDEBUG
        std::cout << "Move choice = " << move_choice << std::endl;
#endif

        // Convert std::size_t move_choice to move_type enum
        auto move = static_cast<move_type>(move_choice);
        make_move(move);
      }  // End loop through CurrentTotalSimplices

      // Do stuff on checkpoint_
      if ((pass_number % checkpoint_) == 0)
      {
        std::cout << "Pass " << pass_number << std::endl;
        // write results to a file
        write_file(universe_, topology_type::SPHERICAL, 3,
                   universe_.geometry->number_of_cells(),
                   universe_.geometry->max_timevalue().get());
      }
    }  // End loop through passes_
    // output results
    std::cout << "Run results: " << std::endl;
    print_run();
    return universe_;
  }
};  // MoveAlways

#endif  // INCLUDE_MOVE_ALWAYS_HPP_
