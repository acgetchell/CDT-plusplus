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
#include <string>

#include "Metropolis_4.hpp"

namespace cdt::four_d::output
{
  struct RunManifest
  {
    std::string run_id{"run"};
    std::string git_commit{"unknown"};
    std::string build_type{"unknown"};
    std::string compiler{"unknown"};
  };

  inline void write_counts_csv_header(std::ofstream& file)
  {
    file << "step,N0,N1,N2,N3,N4,N41,N32,N23,N14\n";
  }

  inline void write_counts_csv_row(std::ofstream& file, Int_precision step,
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
    auto const run_dir = root / manifest.run_id;
    std::filesystem::create_directories(run_dir / "checkpoint");

    {
      std::ofstream file(run_dir / "manifest.json");
      file << "{\n";
      file << "  \"version\": 1,\n";
      file << "  \"run_id\": \"" << manifest.run_id << "\",\n";
      file << "  \"git_commit\": \"" << manifest.git_commit << "\",\n";
      file << "  \"build_type\": \"" << manifest.build_type << "\",\n";
      file << "  \"compiler\": \"" << manifest.compiler << "\",\n";
      file << "  \"seed\": " << config.seed << ",\n";
      file << "  \"chain_id\": \"" << config.chain_id << "\",\n";
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
    }

    {
      std::ofstream file(run_dir / "measurements.jsonl");
      for (auto const& measurement : result.measurements)
      {
        file << "{\"step\":" << measurement.step << ",\"action\":"
             << measurement.action << ",\"N4\":" << measurement.counts.N4
             << ",\"max_vertex_order\":" << measurement.max_vertex_order
             << ",\"valid\":" << (measurement.valid ? "true" : "false")
             << "}\n";
      }
    }

    {
      std::ofstream file(run_dir / "spatial_volume.csv");
      file << "timeslice,N3\n";
      auto const profile = result.triangulation.spatial_volume_profile();
      for (std::size_t index = 0; index < profile.size(); ++index)
      {
        file << index << ',' << profile[index] << '\n';
      }
    }

    {
      std::ofstream file(run_dir / "simplex_counts.csv");
      write_counts_csv_header(file);
      for (auto const& measurement : result.measurements)
      {
        write_counts_csv_row(file, measurement.step, measurement.counts);
      }
      if (result.measurements.empty())
      {
        write_counts_csv_row(file, 0, result.triangulation.counts());
      }
    }

    {
      std::ofstream file(run_dir / "move_statistics.csv");
      file << "move,attempted,accepted,invalid\n";
      for (std::size_t index = 0; index < result.move_stats.size(); ++index)
      {
        auto const& stat = result.move_stats[index];
        file << index << ',' << stat.attempted << ',' << stat.accepted << ','
             << stat.invalid << '\n';
      }
    }

    {
      std::ofstream file(run_dir / "action_trace.csv");
      file << "step,action,N4\n";
      for (std::size_t index = 0; index < result.action_trace.size(); ++index)
      {
        file << index + 1 << ',' << result.action_trace[index] << ','
             << result.volume_trace[index] << '\n';
      }
    }

    {
      std::ofstream file(run_dir / "summary.json");
      auto const    counts = result.triangulation.counts();
      auto const    report = result.triangulation.validate();
      file << "{\n";
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
      file << "  \"restricted_ensemble_only\": "
           << (report.restricted_ensemble_only ? "true" : "false") << "\n";
      file << "}\n";
    }
  }
}  // namespace cdt::four_d::output

#endif  // CDT_PLUSPLUS_SIMULATION_OUTPUT_HPP
