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
#include <string>
#include <string_view>
#include <system_error>

using namespace cdt;
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

  struct AbstractTriangulationProbe
  {
    struct DataStructure
    {
      [[nodiscard]] auto is_valid() const noexcept -> bool { return true; }
    };

    int                value{};

    [[nodiscard]] auto tds() const noexcept -> DataStructure { return {}; }
    [[nodiscard]] auto is_valid() const noexcept -> bool { return false; }
  };

  auto operator>>(std::istream& input, AbstractTriangulationProbe& parsed)
      -> std::istream&
  { return input >> parsed.value; }

  void replace_metadata_field(std::filesystem::path const& filename,
                              std::string_view const       field,
                              std::string_view const       replacement)
  {
    std::ifstream input{filename};
    std::string   contents{std::istreambuf_iterator<char>{input},
                           std::istreambuf_iterator<char>{}};
    auto const    prefix = std::string{field} + '=';
    auto const    start  = contents.find(prefix);
    if (start == std::string::npos)
    {
      throw std::runtime_error{"Metadata field not found"};
    }
    auto const value_start = start + prefix.size();
    auto const value_end   = contents.find('\n', value_start);
    contents.replace(value_start, value_end - value_start, replacement);
    std::ofstream output{filename, std::ios::trunc};
    output << contents;
  }

  void corrupt_metadata_hex_field(std::filesystem::path const& filename,
                                  std::string_view const       field)
  {
    std::ifstream input{filename};
    std::string   contents{std::istreambuf_iterator<char>{input},
                           std::istreambuf_iterator<char>{}};
    auto const    prefix = std::string{field} + '=';
    auto const    start  = contents.find(prefix);
    if (start == std::string::npos)
    {
      throw std::runtime_error{"Metadata field not found"};
    }
    auto const value_start = start + prefix.size();
    if (value_start == contents.size() || contents[value_start] == '\n')
    {
      throw std::runtime_error{"Metadata field is empty"};
    }
    contents[value_start] = contents[value_start] == '0' ? '1' : '0';
    std::ofstream output{filename, std::ios::trunc};
    output << contents;
  }
}  // namespace

SCENARIO("Various string/stream/time utilities" *
         doctest::test_suite("utilities"))
{
  spdlog::debug("Various string/stream/time utilities.\n");
  GIVEN("A Topology.")
  {
    constexpr auto this_topology = Topology::SPHERICAL;
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
    CHECK_FALSE(cdt::BUILD_CONFIGURATION.empty());
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
      constexpr auto this_topology = Topology::SPHERICAL;
      constexpr auto dimensions    = 3;
      constexpr auto simplices     = 6700;
      constexpr auto timeslices    = 16;
      auto const     filename =
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
      auto const filename = make_filename(manifold, cdt::RandomSeed{92}, 7);
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
    WHEN("Causal vertex and cell metadata are round-tripped")
    {
      TemporaryDirectory const directory;
      auto const               filename  = directory.file("causal-info.off");
      auto                     annotated = manifold.delaunay_snapshot();
      Int_precision            vertex_info{10};
      for (auto vertex = annotated.finite_vertices_begin();
           vertex != annotated.finite_vertices_end(); ++vertex)
      {
        vertex->info() = vertex_info++;
      }
      for (auto cell = annotated.finite_cells_begin();
           cell != annotated.finite_cells_end(); ++cell)
      {
        cell->info() = 31;
      }

      write_file(filename, annotated);
      auto const restored = read_file<Delaunay_t<3>>(filename);

      THEN("The versioned payload trailer restores the complete causal state")
      {
        CHECK_EQ(utilities::detail::canonical_topology_fingerprint(restored),
                 utilities::detail::canonical_topology_fingerprint(annotated));
      }
    }
    WHEN("A stochastic artifact is written with reproducibility metadata")
    {
      TemporaryDirectory const directory;
      auto const               filename = directory.file("checkpoint.off");
      auto                     metadata = make_reproducibility_metadata(
          manifold, cdt::RandomSeed{92}, ArtifactKind::CHECKPOINT);
      metadata.desired_simplices   = 64;
      metadata.desired_timeslices  = 3;
      metadata.alpha               = 0.6L;
      metadata.k                   = 1.1L;
      metadata.lambda              = 0.1L;
      metadata.configured_passes   = 10;
      metadata.checkpoint_interval = 2;
      metadata.completed_passes    = 4;
      metadata.max_threads         = 4;
      metadata.transition_trace    = 0x1234;
      metadata.transition_count    = 17;

      write_file(filename, manifold.delaunay_snapshot(), metadata);
      auto const sidecar = metadata_filename(filename);

      THEN("The payload and portable provenance sidecar round-trip together")
      {
        REQUIRE(std::filesystem::exists(filename));
        REQUIRE(std::filesystem::exists(sidecar));
        std::ifstream     metadata_input{sidecar};
        std::string const contents{
            std::istreambuf_iterator<char>{metadata_input},
            std::istreambuf_iterator<char>{}};
        CHECK_NE(contents.find("cdt-plusplus-metadata-v1"), std::string::npos);
        CHECK_NE(contents.find("artifact=checkpoint"), std::string::npos);
        CHECK_NE(contents.find("resume_supported=false"), std::string::npos);
        CHECK_NE(contents.find("fresh_topology_replay_supported=false"),
                 std::string::npos);
        CHECK_NE(
            contents.find("transition_replay_requires_identical_start=true"),
            std::string::npos);
        CHECK_NE(contents.find("random.seed=92"), std::string::npos);
        CHECK_NE(contents.find("random.initialization_stream=0"),
                 std::string::npos);
        CHECK_NE(contents.find("random.transition_stream=1"),
                 std::string::npos);
        CHECK_NE(contents.find("desired.simplices=64"), std::string::npos);
        CHECK_NE(contents.find("alpha=0.6"), std::string::npos);
        CHECK_NE(contents.find("completed_passes=4"), std::string::npos);
        CHECK_NE(contents.find("parallel.max_threads=4"), std::string::npos);
        CHECK_NE(contents.find("transition_trace.fnv1a64=0000000000001234"),
                 std::string::npos);
        CHECK_NE(contents.find("placement.fnv1a64="), std::string::npos);
        CHECK_NE(contents.find("topology.fnv1a64="), std::string::npos);
        CHECK_NOTHROW(static_cast<void>(read_file<Delaunay_t<3>>(filename)));
        auto payload_temporary = filename;
        payload_temporary += ".tmp";
        auto metadata_temporary = sidecar;
        metadata_temporary += ".tmp";
        CHECK_FALSE(std::filesystem::exists(payload_temporary));
        CHECK_FALSE(std::filesystem::exists(metadata_temporary));
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
        CHECK_THROWS_AS(static_cast<void>(read_file<SingleInteger>(filename)),
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
        CHECK_THROWS_AS(static_cast<void>(read_file<SingleInteger>(filename)),
                        std::filesystem::filesystem_error);
      }
    }
  }

  GIVEN("A valid triangulation payload with a reproducibility sidecar.")
  {
    Delaunay_t<3> triangulation;
    triangulation.insert(Point_t<3>(0, 0, 0));
    triangulation.insert(Point_t<3>(1, 0, 0));
    triangulation.insert(Point_t<3>(0, 1, 0));
    triangulation.insert(Point_t<3>(0, 0, 1));
    manifolds::Manifold_3 const manifold(
        foliated_triangulations::FoliatedTriangulation_3(triangulation, 0, 1));

    WHEN("The payload is truncated after it was committed.")
    {
      auto const filename = directory.file("truncated.off");
      auto const metadata = make_reproducibility_metadata(
          manifold, cdt::RandomSeed{92}, ArtifactKind::CHECKPOINT);
      auto checkpoint_metadata             = metadata;
      checkpoint_metadata.completed_passes = 2;
      write_file(filename, triangulation, checkpoint_metadata);
      auto const original_size = std::filesystem::file_size(filename);
      REQUIRE_GT(original_size, 2);
      std::filesystem::resize_file(filename, original_size / 2);

      THEN("The checksum mismatch is diagnosed before topology is accepted.")
      {
        CHECK_THROWS_AS(static_cast<void>(read_file<Delaunay_t<3>>(filename)),
                        std::filesystem::filesystem_error);
      }
    }

    WHEN("The provenance sidecar is malformed.")
    {
      auto const filename = directory.file("malformed-metadata.off");
      auto const metadata = make_reproducibility_metadata(
          manifold, cdt::RandomSeed{92}, ArtifactKind::CHECKPOINT);
      auto checkpoint_metadata             = metadata;
      checkpoint_metadata.completed_passes = 2;
      write_file(filename, triangulation, checkpoint_metadata);
      {
        std::ofstream output{metadata_filename(filename), std::ios::trunc};
        output << "not-valid-metadata\n";
      }

      THEN("The artifact is rejected rather than treated as legacy data.")
      {
        CHECK_THROWS_AS(static_cast<void>(read_file<Delaunay_t<3>>(filename)),
                        std::filesystem::filesystem_error);
      }
    }

    WHEN("A payload-derived topology fingerprint is changed in the sidecar.")
    {
      auto const filename = directory.file("changed-topology-fingerprint.off");
      auto       metadata = make_reproducibility_metadata(
          manifold, cdt::RandomSeed{92}, ArtifactKind::CHECKPOINT);
      metadata.completed_passes = 2;
      write_file(filename, triangulation, metadata);
      corrupt_metadata_hex_field(metadata_filename(filename),
                                 "topology.fnv1a64");

      THEN("The semantic manifest/payload mismatch is rejected.")
      {
        CHECK_THROWS_AS(static_cast<void>(read_file<Delaunay_t<3>>(filename)),
                        std::filesystem::filesystem_error);
      }
    }

    WHEN("A payload-derived incidence count is changed in the sidecar.")
    {
      auto const filename = directory.file("changed-incidence-count.off");
      auto       metadata = make_reproducibility_metadata(
          manifold, cdt::RandomSeed{92}, ArtifactKind::CHECKPOINT);
      metadata.completed_passes = 2;
      write_file(filename, triangulation, metadata);
      replace_metadata_field(metadata_filename(filename), "actual.simplices",
                             "999");

      THEN("The semantic manifest/payload mismatch is rejected.")
      {
        CHECK_THROWS_AS(static_cast<void>(read_file<Delaunay_t<3>>(filename)),
                        std::filesystem::filesystem_error);
      }
    }

    WHEN("A typed checkpoint field contains non-numeric data.")
    {
      auto const filename = directory.file("invalid-completed-passes.off");
      auto       metadata = make_reproducibility_metadata(
          manifold, cdt::RandomSeed{92}, ArtifactKind::CHECKPOINT);
      metadata.completed_passes = 2;
      write_file(filename, triangulation, metadata);
      replace_metadata_field(metadata_filename(filename), "completed_passes",
                             "not-a-number");

      THEN("The malformed typed field is rejected.")
      {
        CHECK_THROWS_AS(static_cast<void>(read_file<Delaunay_t<3>>(filename)),
                        std::filesystem::filesystem_error);
      }
    }

    WHEN("A configured thread limit is zero.")
    {
      auto const filename = directory.file("invalid-thread-limit.off");
      auto       metadata = make_reproducibility_metadata(
          manifold, cdt::RandomSeed{92}, ArtifactKind::CHECKPOINT);
      metadata.completed_passes = 2;
      metadata.max_threads      = 1;
      write_file(filename, triangulation, metadata);
      replace_metadata_field(metadata_filename(filename),
                             "parallel.max_threads", "0");

      THEN("The invalid resource provenance is rejected.")
      {
        try
        {
          static_cast<void>(read_file<Delaunay_t<3>>(filename));
          FAIL_CHECK("A zero thread limit was accepted.");
        }
        catch (std::filesystem::filesystem_error const& error)
        {
          CHECK_EQ(error.code(),
                   std::make_error_code(std::errc::illegal_byte_sequence));
          CHECK(
              std::string_view{error.what()}.contains("invalid thread limit"));
        }
      }
    }

    WHEN("Caller-supplied payload-derived provenance is stale.")
    {
      auto const filename = directory.file("reconciled-provenance.off");
      auto       metadata = make_reproducibility_metadata(
          manifold, cdt::RandomSeed{92}, ArtifactKind::CHECKPOINT);
      metadata.completed_passes      = 2;
      metadata.actual_simplices      = 999;
      metadata.topology_fingerprint  = 0;
      metadata.placement_fingerprint = 0;

      THEN("The writer derives those fields from the serialized state.")
      {
        CHECK_NOTHROW(write_file(filename, triangulation, metadata));
        CHECK_NOTHROW(static_cast<void>(read_file<Delaunay_t<3>>(filename)));
        std::ifstream     input{metadata_filename(filename)};
        std::string const contents{std::istreambuf_iterator<char>{input},
                                   std::istreambuf_iterator<char>{}};
        CHECK_EQ(contents.find("actual.simplices=999"), std::string::npos);
      }
    }
  }

  GIVEN(
      "A TDS-valid abstract triangulation that is not geometrically Delaunay.")
  {
    auto const filename = directory.file("abstract-triangulation.off");
    {
      std::ofstream output{filename};
      output << 42;
    }

    WHEN("The abstract triangulation is parsed.")
    {
      THEN("TDS integrity does not impose the Delaunay empty-sphere property.")
      {
        CHECK_NOTHROW(
            static_cast<void>(read_file<AbstractTriangulationProbe>(filename)));
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
    constexpr Int_precision                VECTOR_TEST_SIZE = 100;
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
      constexpr auto min    = 64;
      constexpr auto max    = 6400;
      auto const     value1 = generate_random_int(generator, min, max);
      auto const     value2 = generate_random_int(generator, min, max);
      auto const     value3 = generate_random_int(generator, min, max);
      auto const     value4 = generate_random_int(generator, min, max);
      auto const     value5 = generate_random_int(generator, min, max);
      auto const     value6 = generate_random_int(generator, min, max);
      array container       = {value1, value2, value3, value4, value5, value6};
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
      // Keep this mutable to cover ordinary named lvalue bounds.
      auto       max       = 256;
      auto const value1    = generate_random_timeslice(generator, max);
      auto const value2    = generate_random_timeslice(generator, max);
      auto const value3    = generate_random_timeslice(generator, max);
      auto const value4    = generate_random_timeslice(generator, max);
      auto const value5    = generate_random_timeslice(generator, max);
      auto const value6    = generate_random_timeslice(generator, max);
      array      container = {value1, value2, value3, value4, value5, value6};
      THEN("They should all fall within the range.")
      {
        constexpr auto min = 1;
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
      constexpr auto min   = 0.0L;
      constexpr auto max   = 1.0L;
      auto const     value = generate_random_real(generator, min, max);
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
        REQUIRE_THROWS_AS(
            static_cast<void>(expected_points_per_timeslice(4, 640000, 64)),
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
      auto const converted_value = gmpzf_to_double(TEST_VALUE);
      THEN("It should be exact when converted back from double to Gmpzf.")
      { REQUIRE_EQ(TEST_VALUE, Gmpzf(converted_value)); }
    }
  }
}
