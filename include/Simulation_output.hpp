/*******************************************************************************
 Causal Dynamical Triangulations in C++ using CGAL
*******************************************************************************/

/// @file Simulation_output.hpp
/// @brief Versioned machine-readable simulation output.

#ifndef CDT_PLUSPLUS_SIMULATION_OUTPUT_HPP
#define CDT_PLUSPLUS_SIMULATION_OUTPUT_HPP

#include <cstddef>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <limits>
#include <ostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "Metropolis_4.hpp"
#include "Phase_analysis.hpp"
#include "Utilities.hpp"

namespace cdt::four_d::output
{
  struct RunManifest
  {
    std::string run_id{"run"};
    std::string git_commit{"unknown"};
    std::string build_type{"unknown"};
    std::string compiler{"unknown"};
  };

  inline void set_numeric_precision(std::ostream& file)
  {
    file << std::setprecision(std::numeric_limits<long double>::max_digits10);
  }

  [[nodiscard]] inline auto json_escape(std::string_view const text)
      -> std::string
  {
    std::string escaped;
    escaped.reserve(text.size());
    for (auto const character : text)
    {
      switch (character)
      {
        case '"': escaped += "\\\""; break;
        case '\\': escaped += "\\\\"; break;
        case '\b': escaped += "\\b"; break;
        case '\f': escaped += "\\f"; break;
        case '\n': escaped += "\\n"; break;
        case '\r': escaped += "\\r"; break;
        case '\t': escaped += "\\t"; break;
        default:
          if (static_cast<unsigned char>(character) < 0x20U)
          {
            auto const byte =
                static_cast<unsigned>(static_cast<unsigned char>(character));
            auto constexpr hex = std::string_view{"0123456789ABCDEF"};
            escaped += "\\u00";
            escaped += hex[(byte >> 4U) & 0xFU];
            escaped += hex[byte & 0xFU];
          }
          else
          {
            escaped += character;
          }
          break;
      }
    }
    return escaped;
  }

  [[nodiscard]] inline auto open_output_file(std::filesystem::path const& path)
      -> std::ofstream
  {
    std::ofstream file(path);
    if (!file)
    {
      throw std::runtime_error("Failed to open output file: " + path.string());
    }
    set_numeric_precision(file);
    return file;
  }

  inline void ensure_wrote(std::ostream const& file,
                           std::filesystem::path const& path)
  {
    if (!file)
    {
      throw std::runtime_error("Failed to write output file: " +
                               path.string());
    }
  }

  inline void write_counts_csv_header(std::ostream& file)
  {
    file << "step,N0,N1,N2,N3,N4,N41,N32,N23,N14\n";
  }

  inline void write_counts_csv_row(std::ostream& file, Int_precision step,
                                   S4Counts const& counts)
  {
    file << step << ',' << counts.N0 << ',' << counts.N1 << ',' << counts.N2
         << ',' << counts.N3 << ',' << counts.N4 << ',' << counts.N41 << ','
         << counts.N32 << ',' << counts.N23 << ',' << counts.N14 << '\n';
  }

  inline void write_run_directory(std::filesystem::path const& root,
                                  RunManifest const& manifest,
                                  Metropolis4Config const& config,
                                  Metropolis4Result const& result)
  {
    cdt::utilities::validate_path_component("run_id", manifest.run_id);
    auto const run_dir = root / manifest.run_id;
    std::filesystem::create_directories(run_dir / "checkpoint");

    {
      auto const path = run_dir / "manifest.json";
      auto file = open_output_file(path);
      file << "{\n";
      file << "  \"version\": 1,\n";
      file << "  \"run_id\": \"" << json_escape(manifest.run_id) << "\",\n";
      file << "  \"git_commit\": \"" << json_escape(manifest.git_commit)
           << "\",\n";
      file << "  \"build_type\": \"" << json_escape(manifest.build_type)
           << "\",\n";
      file << "  \"compiler\": \"" << json_escape(manifest.compiler)
           << "\",\n";
      file << "  \"seed\": " << config.seed << ",\n";
      file << "  \"chain_id\": \"" << json_escape(config.chain_id)
           << "\",\n";
      file << "  \"kappa_0\": " << config.couplings.kappa_0 << ",\n";
      file << "  \"kappa_4\": " << config.couplings.kappa_4 << ",\n";
      file << "  \"Delta\": " << config.couplings.Delta << ",\n";
      file << "  \"target_N4\": " << config.couplings.target_N4 << ",\n";
      file << "  \"volume_epsilon\": "
           << config.couplings.volume_epsilon << ",\n";
      file << "  \"timeslices\": " << result.triangulation.timeslices()
           << ",\n";
      file << "  \"thermalization_steps\": "
           << config.thermalization_steps << ",\n";
      file << "  \"measurement_interval\": "
           << config.measurement_interval << "\n";
      file << "}\n";
      ensure_wrote(file, path);
    }

    {
      auto const path = run_dir / "measurements.jsonl";
      auto file = open_output_file(path);
      for (auto const& measurement : result.measurements)
      {
        file << "{\"step\":" << measurement.step << ",\"action\":"
             << measurement.action << ",\"N4\":" << measurement.counts.N4
             << ",\"max_vertex_order\":" << measurement.max_vertex_order
             << ",\"valid\":" << (measurement.valid ? "true" : "false")
             << "}\n";
      }
      ensure_wrote(file, path);
    }

    {
      auto const path = run_dir / "spatial_volume.csv";
      auto file = open_output_file(path);
      file << "timeslice,N3\n";
      auto const profile = result.triangulation.spatial_volume_profile();
      for (std::size_t index = 0; index < profile.size(); ++index)
      {
        file << index << ',' << profile[index] << '\n';
      }
      ensure_wrote(file, path);
    }

    {
      auto const path = run_dir / "spatial_volume_profiles.csv";
      auto file = open_output_file(path);
      file << "step,timeslice,N3\n";
      for (auto const& measurement : result.measurements)
      {
        for (std::size_t index = 0; index < measurement.spatial_profile.size();
             ++index)
        {
          file << measurement.step << ',' << index << ','
               << measurement.spatial_profile[index] << '\n';
        }
      }
      ensure_wrote(file, path);
    }

    {
      auto const path = run_dir / "simplex_counts.csv";
      auto file = open_output_file(path);
      write_counts_csv_header(file);
      for (auto const& measurement : result.measurements)
      {
        write_counts_csv_row(file, measurement.step, measurement.counts);
      }
      if (result.measurements.empty())
      {
        write_counts_csv_row(file, 0, result.triangulation.counts());
      }
      ensure_wrote(file, path);
    }

    {
      auto const path = run_dir / "move_statistics.csv";
      auto file = open_output_file(path);
      file << "move,attempted,accepted,invalid\n";
      for (std::size_t index = 0; index < result.move_stats.size(); ++index)
      {
        auto const& stat = result.move_stats[index];
        auto const descriptor =
            move_descriptor_4d(move_tracker::as_move_4d(
                static_cast<int>(index)));
        file << descriptor.name << ',' << stat.attempted << ','
             << stat.accepted << ',' << stat.invalid << '\n';
      }
      ensure_wrote(file, path);
    }

    {
      if (result.action_trace.size() != result.volume_trace.size())
      {
        throw std::runtime_error(
            "Action and volume traces have different lengths.");
      }
      auto const path = run_dir / "action_trace.csv";
      auto file = open_output_file(path);
      file << "step,action,N4\n";
      for (std::size_t index = 0; index < result.action_trace.size(); ++index)
      {
        file << index + 1 << ',' << result.action_trace[index] << ','
             << result.volume_trace[index] << '\n';
      }
      ensure_wrote(file, path);
    }

    std::vector<phase::Profile> measured_profiles;
    for (auto const& measurement : result.measurements)
    {
      phase::Profile profile;
      profile.reserve(measurement.spatial_profile.size());
      for (auto const value : measurement.spatial_profile)
      {
        profile.push_back(static_cast<long double>(value));
      }
      measured_profiles.push_back(std::move(profile));
    }
    if (!measured_profiles.empty())
    {
      auto const profile_mean = phase::mean(measured_profiles);
      auto const covariance = phase::covariance(measured_profiles, profile_mean);
      auto const kernel = phase::effective_action_kernel(measured_profiles);

      auto const covariance_path = run_dir / "covariance.csv";
      auto covariance_file = open_output_file(covariance_path);
      for (auto const& row : covariance)
      {
        for (std::size_t index = 0; index < row.size(); ++index)
        {
          covariance_file << row[index]
                          << (index + 1 == row.size() ? '\n' : ',');
        }
      }
      ensure_wrote(covariance_file, covariance_path);

      auto const effective_path = run_dir / "effective_action.csv";
      auto effective_file = open_output_file(effective_path);
      for (auto const& row : kernel.inverse_covariance_diagonal_regularized)
      {
        for (std::size_t index = 0; index < row.size(); ++index)
        {
          effective_file << row[index]
                         << (index + 1 == row.size() ? '\n' : ',');
        }
      }
      ensure_wrote(effective_file, effective_path);
    }

    {
      auto const path = run_dir / "summary.json";
      auto file = open_output_file(path);
      auto const    counts = result.triangulation.counts();
      auto const    report = result.triangulation.validate();
      auto const diagnostics = measured_profiles.empty()
                                   ? phase::Diagnostics{}
                                   : phase::diagnose(std::move(measured_profiles));
      file << "{\n";
      file << "  \"chain_id\": \"" << json_escape(config.chain_id)
           << "\",\n";
      file << "  \"N0\": " << counts.N0 << ",\n";
      file << "  \"N1\": " << counts.N1 << ",\n";
      file << "  \"N2\": " << counts.N2 << ",\n";
      file << "  \"N3\": " << counts.N3 << ",\n";
      file << "  \"N4\": " << counts.N4 << ",\n";
      file << "  \"N41\": " << counts.N41 << ",\n";
      file << "  \"N32\": " << counts.N32 << ",\n";
      file << "  \"N23\": " << counts.N23 << ",\n";
      file << "  \"N14\": " << counts.N14 << ",\n";
      file << "  \"action\": "
           << S4_bulk_action(counts, config.couplings) << ",\n";
      file << "  \"maximum_vertex_order\": "
           << result.triangulation.max_vertex_order() << ",\n";
      file << "  \"valid\": " << (report.valid() ? "true" : "false")
           << ",\n";
      file << "  \"standard_cdt_candidate\": "
           << (report.valid() && report.standard_cdt_candidate ? "true"
                                                               : "false")
           << ",\n";
      file << "  \"phase_verdict\": \""
           << phase::to_string(diagnostics.verdict) << "\",\n";
      file << "  \"autocorrelation_time\": "
           << diagnostics.autocorrelation_time << ",\n";
      file << "  \"held_out_likelihood\": "
           << diagnostics.held_out_likelihood << ",\n";
      file << "  \"aic\": " << diagnostics.aic << ",\n";
      file << "  \"bic\": " << diagnostics.bic << "\n";
      file << "}\n";
      ensure_wrote(file, path);
    }
  }
}  // namespace cdt::four_d::output

#endif  // CDT_PLUSPLUS_SIMULATION_OUTPUT_HPP
