// CLI parsing and validation for the Part 1 detector binary.

#include "app/cli.hpp"

#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>

void PrintUsage(std::ostream& output) {
  output
      << "Usage: ./bin/arb_detect --input <path> [--output <path>] [--top <n>]\n"
      << "                      [--min-reserve-usd <value>] [--max-input-share <value>]\n"
      << "                      [--max-cycle-length <n>]\n";
}

SearchOptions ParseCliArgs(int argc, char** argv) {
  SearchOptions options;

  for (int i = 1; i < argc; ++i) {
    const std::string arg = argv[i];
    // Each flag in this CLI takes a single value, so validation is centralized
    // in one local helper rather than repeated in every branch.
    auto require_value = [&](const char* flag) -> std::string {
      if (i + 1 >= argc) {
        throw std::runtime_error(std::string("missing value for ") + flag);
      }
      return argv[++i];
    };

    if (arg == "--input") {
      options.inputPath = require_value("--input");
    } else if (arg == "--output") {
      options.outputPath = require_value("--output");
    } else if (arg == "--top") {
      options.topN = static_cast<std::size_t>(std::stoul(require_value("--top")));
    } else if (arg == "--min-reserve-usd") {
      options.minReserveUsd = std::stold(require_value("--min-reserve-usd"));
    } else if (arg == "--max-input-share") {
      options.maxInputShare = std::stold(require_value("--max-input-share"));
    } else if (arg == "--max-cycle-length") {
      options.maxCycleLength =
          static_cast<std::size_t>(std::stoul(require_value("--max-cycle-length")));
    } else if (arg == "--help" || arg == "-h") {
      PrintUsage(std::cout);
      std::exit(0);
    } else {
      throw std::runtime_error("unknown argument: " + arg);
    }
  }

  if (options.inputPath.empty()) {
    throw std::runtime_error("--input is required");
  }
  if (options.maxInputShare <= 0.0L || options.maxInputShare >= 1.0L) {
    throw std::runtime_error("--max-input-share must be between 0 and 1");
  }
  if (options.maxCycleLength < 3U) {
    throw std::runtime_error("--max-cycle-length must be at least 3");
  }

  return options;
}
