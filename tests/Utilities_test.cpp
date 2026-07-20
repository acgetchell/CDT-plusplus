/*******************************************************************************
 Causal Dynamical Triangulations in C++ using CGAL

 Copyright © 2017 Adam Getchell
 ******************************************************************************/

/// @file Utilities_test.cpp
/// @brief Tests on utility functions
/// @author Adam Getchell
/// @details Tests for random, conversion, and datetime functions.

#include <doctest/doctest.h>
#include <fmt/format.h>

#include <atomic>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <Manifold.hpp>
#include <stdexcept>
#include <string_view>

using namespace std;
using namespace utilities;

namespace
{
  class TemporaryDirectory
  {
    std::filesystem::path m_path;

   public:
    TemporaryDirectory()
    {
      static std::atomic<std::uint64_t> sequence{};
      auto const base = std::filesystem::temp_directory_path();

      for (std::uint64_t attempt = 0; attempt < 100; ++attempt)
      {
        auto const timestamp =
            std::chrono::steady_clock::now().time_since_epoch().count();
        auto const candidate =
            base / fmt::format("cdt-plusplus-tests-{}-{}-{}", timestamp,
                               sequence.fetch_add(1), attempt);
        std::error_code error;
        if (std::filesystem::create_directory(candidate, error))
        {
          m_path = candidate;
          return;
        }
        if (error)
        {
          throw std::filesystem::filesystem_error{
              "Unable to create test directory", candidate, error};
        }
      }

      throw std::runtime_error{"Unable to create a unique test directory"};
    }

    TemporaryDirectory(TemporaryDirectory const&)                    = delete;
    TemporaryDirectory(TemporaryDirectory&&)                         = delete;
    auto operator=(TemporaryDirectory const&) -> TemporaryDirectory& = delete;
    auto operator=(TemporaryDirectory&&) -> TemporaryDirectory&      = delete;

    ~TemporaryDirectory()
    {
      std::error_code error;
      std::filesystem::remove_all(m_path, error);
    }

    [[nodiscard]] auto file(std::string_view const name) const
        -> std::filesystem::path
    { return m_path / name; }
  };

  struct SerializationFailure
  {};

  auto operator<<(std::ostream& output, SerializationFailure const& /*unused*/)
      -> std::ostream&
  {
    output.setstate(std::ios::badbit);
    return output;
  }

  struct ReentrantSerialization
  {
    std::filesystem::path filename;
  };

  auto operator<<(std::ostream& output, ReentrantSerialization const& value)
      -> std::ostream&
  {
    write_file(value.filename, 42);
    return output;
  }

  struct SingleInteger
  {
    int value{};
  };

  auto operator>>(std::istream& input, SingleInteger& parsed) -> std::istream&
  { return input >> parsed.value; }
}  // namespace

SCENARIO("Various string/stream/time utilities" *
         doctest::test_suite("utilities"))
{
  spdlog::debug("Various string/stream/time utilities.\n");
  GIVEN("A topology_type.")
  {
    auto constexpr this_topology = topology_type::SPHERICAL;
    WHEN("Operator<< is invoked.")
    {
      stringstream buffer;
      buffer << this_topology;
      THEN("The output is correct.")
      {
        CHECK_EQ(buffer.str(), "spherical");
        spdlog::debug("buffer.str() contents: {}.\n", buffer.str());
      }
      WHEN("fmt::format is invoked.")
      {
        THEN("The output is correct.")
        {
          auto result = fmt::format("Topology type is: {}.\n", buffer.str());
          CHECK_EQ(result, "Topology type is: spherical.\n");
          spdlog::debug("Topology type is: {}.\n", buffer.str());
        }
      }
    }
  }
  GIVEN("A running environment.")
  {
    WHEN("The current time is requested.")
    {
      THEN("The output is correct.")
      {
        auto const timestamp     = std::chrono::system_clock::now();
        auto const result        = current_date_time(timestamp);
        auto const expected_year = date::format(
            "%Y", std::chrono::floor<std::chrono::seconds>(timestamp));
        CHECK(result.starts_with(expected_year));
      }
    }
    WHEN("A filename is generated.")
    {
      auto constexpr this_topology = topology_type::SPHERICAL;
      auto constexpr dimensions    = 3;
      auto constexpr simplices     = 6700;
      auto constexpr timeslices    = 16;
      auto const filename =
          make_filename(this_topology, dimensions, simplices, timeslices,
                        INITIAL_RADIUS, FOLIATION_SPACING);
      THEN("The output is correct.")
      {
        auto const topology = filename.string().find("S3");
        CHECK_NE(topology, std::string::npos);
        auto const time = filename.string().find("16");
        CHECK_NE(time, std::string::npos);
        auto const cells = filename.string().find("6700");
        CHECK_NE(cells, std::string::npos);
        auto const initial_radius = filename.string().find("1.0");
        CHECK_NE(initial_radius, std::string::npos);
        auto const file_suffix = filename.string().find("off");
        CHECK_NE(file_suffix, std::string::npos);
        CHECK_EQ(filename.string().find(':'), std::string::npos);
      }
    }
  }
}

SCENARIO("Printing Delaunay triangulations" * doctest::test_suite("utilities"))
{
  spdlog::debug("Printing Delaunay triangulations.\n");
  GIVEN("A Delaunay_t<3> triangulation.")
  {
    Delaunay_t<3> triangulation;
    triangulation.insert(Point_t<3>(0, 0, 0));
    triangulation.insert(Point_t<3>(1, 0, 0));
    triangulation.insert(Point_t<3>(0, 1, 0));
    triangulation.insert(Point_t<3>(0, 0, 1));
    WHEN("The triangulation is printed.")
    {
      THEN("No exception is thrown.")
      { CHECK_NOTHROW(print_delaunay(triangulation)); }
    }
  }
}

SCENARIO("Reading and writing Delaunay triangulations to files" *
         doctest::test_suite("utilities"))
{
  spdlog::debug("Reading and writing Delaunay triangulations to files.\n");
  GIVEN("A Manifold3 constructed from a Delaunay_t<3> triangulation")
  {
    Delaunay_t<3> triangulation;
    triangulation.insert(Point_t<3>(0, 0, 0));
    triangulation.insert(Point_t<3>(1, 0, 0));
    triangulation.insert(Point_t<3>(0, 1, 0));
    triangulation.insert(Point_t<3>(0, 0, 1));
    // Construct a manifold from a Delaunay triangulation
    manifolds::Manifold_3 const manifold(
        foliated_triangulations::FoliatedTriangulation_3(triangulation, 0, 1));
    WHEN("A replayable checkpoint filename is generated")
    {
      auto const filename = make_filename(manifold, cdt::Random_seed{92}, 7);
      THEN("The seed and completed pass are recorded before the OFF suffix")
      {
        CHECK_NE(filename.string().find("-seed-92-pass-7.off"),
                 std::string::npos);
      }
    }
    WHEN("The triangulation is round-tripped through a file")
    {
      TemporaryDirectory const directory;
      auto const               filename = directory.file("roundtrip.off");
      write_file(filename, manifold.delaunay_snapshot());
      REQUIRE(std::filesystem::exists(filename));

      auto triangulation_from_file =
          utilities::read_file<Delaunay_t<3>>(filename);
      THEN("The file contains the original triangulation")
      {
        REQUIRE(triangulation_from_file.is_valid(true));
        REQUIRE_EQ(triangulation_from_file.dimension(),
                   manifold.dimensionality());
        REQUIRE_EQ(triangulation_from_file.number_of_finite_cells(),
                   manifold.N3());
        REQUIRE_EQ(triangulation_from_file.number_of_finite_facets(),
                   manifold.N2());
        REQUIRE_EQ(triangulation_from_file.number_of_finite_edges(),
                   manifold.N1());
        REQUIRE_EQ(triangulation_from_file.number_of_vertices(), manifold.N0());
        CHECK_EQ(triangulation_from_file, triangulation);
      }
    }
  }
}

SCENARIO("File serialization reports complete failures" *
         doctest::test_suite("utilities"))
{
  TemporaryDirectory const directory;

  GIVEN("A serializer that marks its output stream bad.")
  {
    auto const filename  = directory.file("write-failure.off");
    auto       temporary = filename;
    temporary += ".tmp";
    {
      std::ofstream existing{filename};
      existing << "previous-checkpoint";
    }

    WHEN("Writing the value is attempted.")
    {
      THEN("The failure is reported without replacing the existing file.")
      {
        CHECK_THROWS_AS(write_file(filename, SerializationFailure{}),
                        std::filesystem::filesystem_error);
        std::ifstream     preserved{filename};
        std::string const contents{std::istreambuf_iterator<char>{preserved},
                                   std::istreambuf_iterator<char>{}};
        CHECK_EQ(contents, "previous-checkpoint");
        CHECK_FALSE(std::filesystem::exists(temporary));
      }
    }
  }

  GIVEN("A serializer that recursively starts another file write.")
  {
    auto const filename  = directory.file("reentrant-write.off");
    auto       temporary = filename;
    temporary += ".tmp";
    {
      std::ofstream existing{filename};
      existing << "previous-checkpoint";
    }

    WHEN("Writing the value is attempted.")
    {
      THEN("Reentrancy is rejected without replacing the existing file.")
      {
        CHECK_THROWS_AS(write_file(filename, ReentrantSerialization{filename}),
                        std::logic_error);
        std::ifstream     preserved{filename};
        std::string const contents{std::istreambuf_iterator<char>{preserved},
                                   std::istreambuf_iterator<char>{}};
        CHECK_EQ(contents, "previous-checkpoint");
        CHECK_FALSE(std::filesystem::exists(temporary));
      }
    }
  }

  GIVEN("A serialized value followed by unexpected trailing input.")
  {
    auto const filename = directory.file("trailing-input.off");
    {
      std::ofstream output{filename};
      output << "42 trailing-data";
    }

    WHEN("The file is parsed.")
    {
      THEN("The trailing input is reported.")
      {
        CHECK_THROWS_AS(read_file<SingleInteger>(filename),
                        std::filesystem::filesystem_error);
      }
    }
  }

  GIVEN("A file containing malformed input.")
  {
    auto const filename = directory.file("malformed-input.off");
    {
      std::ofstream output{filename};
      output << "not-an-integer";
    }

    WHEN("The file is parsed.")
    {
      THEN("The malformed input is reported.")
      {
        CHECK_THROWS_AS(read_file<SingleInteger>(filename),
                        std::filesystem::filesystem_error);
      }
    }
  }
}

SCENARIO("Randomizing functions" * doctest::test_suite("utilities"))
{
  spdlog::debug("Randomizing functions.\n");
  GIVEN("A PCG die roller")
  {
    WHEN("We roll a die twice.")
    {
      cdt::Random generator{92};
      CAPTURE(generator.seed());
      auto const roll1 = die_roll(generator);
      auto const roll2 = die_roll(generator);
      THEN("Both results are valid die values.")
      {
        CHECK_GE(roll1, 1);
        CHECK_LE(roll1, 6);
        CHECK_GE(roll2, 1);
        CHECK_LE(roll2, 6);
      }
    }
  }
  GIVEN("A container of ints")
  {
    Int_precision constexpr VECTOR_TEST_SIZE = 100;
    array<Int_precision, VECTOR_TEST_SIZE> container{};
    iota(container.begin(), container.end(), 0);
    WHEN("The container is shuffled.")
    {
      cdt::Random generator{92};
      CAPTURE(generator.seed());
      ranges::shuffle(container, generator);
      THEN("The shuffled result remains a permutation of the input.")
      {
        ranges::sort(container);
        for (Int_precision i = 0; i < VECTOR_TEST_SIZE; ++i)
        {
          CHECK_EQ(container[static_cast<std::size_t>(i)], i);
        }
      }
    }
  }
  GIVEN("A test range of integers")
  {
    WHEN("We generate six random integers within the range.")
    {
      cdt::Random generator{92};
      CAPTURE(generator.seed());
      auto constexpr min   = 64;
      auto constexpr max   = 6400;
      auto const value1    = generate_random_int(generator, min, max);
      auto const value2    = generate_random_int(generator, min, max);
      auto const value3    = generate_random_int(generator, min, max);
      auto const value4    = generate_random_int(generator, min, max);
      auto const value5    = generate_random_int(generator, min, max);
      auto const value6    = generate_random_int(generator, min, max);
      array      container = {value1, value2, value3, value4, value5, value6};
      THEN("They should all fall within the range.")
      {
        // All elements are >= min
        CHECK_GE(*ranges::min_element(container), min);

        // All elements are <= max
        CHECK_LE(*ranges::max_element(container), max);
      }
    }
  }
  GIVEN("A test range of timeslices")
  {
    WHEN("We generate six timeslices within the range.")
    {
      cdt::Random generator{92};
      CAPTURE(generator.seed());
      auto constexpr max   = 256;
      auto const value1    = generate_random_timeslice(generator, max);
      auto const value2    = generate_random_timeslice(generator, max);
      auto const value3    = generate_random_timeslice(generator, max);
      auto const value4    = generate_random_timeslice(generator, max);
      auto const value5    = generate_random_timeslice(generator, max);
      auto const value6    = generate_random_timeslice(generator, max);
      array      container = {value1, value2, value3, value4, value5, value6};
      THEN("They should all fall within the range.")
      {
        auto constexpr min = 1;
        // All elements are >= min
        CHECK_GE(*ranges::min_element(container), min);

        // All elements are <= max
        CHECK_LE(*ranges::max_element(container), max);
      }
    }
  }
  GIVEN("The range between 0 and 1, inclusive")
  {
    WHEN("We generate a random real number.")
    {
      cdt::Random generator{92};
      CAPTURE(generator.seed());
      auto constexpr min = 0.0L;
      auto constexpr max = 1.0L;
      auto const value   = generate_random_real(generator, min, max);
      THEN("The real number should lie within that range.")
      {
        REQUIRE_LE(min, value);
        REQUIRE_LE(value, max);
      }
    }
  }
  GIVEN("A probability generator")
  {
    WHEN("We generate six probabilities.")
    {
      cdt::Random generator{92};
      CAPTURE(generator.seed());
      auto const value1    = generate_probability(generator);
      auto const value2    = generate_probability(generator);
      auto const value3    = generate_probability(generator);
      auto const value4    = generate_probability(generator);
      auto const value5    = generate_probability(generator);
      auto const value6    = generate_probability(generator);
      array      container = {value1, value2, value3, value4, value5, value6};

      THEN("They should all be valid probabilities.")
      {
        CHECK_GE(*ranges::min_element(container), 0.0L);
        CHECK_LE(*ranges::max_element(container), 1.0L);
      }
    }
  }
}

SCENARIO("Expected points per timeslice" * doctest::test_suite("utilities"))
{
  spdlog::debug("Expected points per timeslice.\n");
  GIVEN("Simplices and timeslices for various foliations")
  {
    WHEN("We request 2 simplices on 2 timeslices.")
    {
      THEN("The results are correct.")
      { REQUIRE_EQ(expected_points_per_timeslice(3, 2, 2), 2); }
    }
    WHEN("We request 500 simplices on 4 timeslices.")
    {
      THEN("The results are correct.")
      { REQUIRE_EQ(expected_points_per_timeslice(3, 500, 4), 50); }
    }
    WHEN("We request 5000 simplices on 8 timeslices.")
    {
      THEN("The results are correct.")
      { REQUIRE_EQ(expected_points_per_timeslice(3, 5000, 8), 125); }
    }
    WHEN("We request 64,000 simplices on 16 timeslices.")
    {
      THEN("The results are correct.")
      { REQUIRE_EQ(expected_points_per_timeslice(3, 64000, 16), 600); }
    }
    WHEN("We request 640,000 simplices on 64 timeslices.")
    {
      THEN("The results are correct.")
      { REQUIRE_EQ(expected_points_per_timeslice(3, 640000, 64), 1000); }
    }
    WHEN("We specify 4 dimensions")
    {
      THEN("A std::invalid_argument exception is thrown.")
      {
        REQUIRE_THROWS_AS(expected_points_per_timeslice(4, 640000, 64),
                          std::invalid_argument);
      }
    }
  }
}

SCENARIO("Exact number (Gmpzf) conversion" * doctest::test_suite("utilities"))
{
  spdlog::debug("Exact number (Gmpzf) conversion.\n");
  GIVEN("A number not exactly representable in binary.")
  {
    Gmpzf const TEST_VALUE = 0.17;
    WHEN("We convert it to double.")
    {
      auto const converted_value = Gmpzf_to_double(TEST_VALUE);
      THEN("It should be exact when converted back from double to Gmpzf.")
      { REQUIRE_EQ(TEST_VALUE, Gmpzf(converted_value)); }
    }
  }
}
