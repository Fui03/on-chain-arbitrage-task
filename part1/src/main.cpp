// Part 1 executable entrypoint that wires CLI parsing, detection, reporting,
// and JSON serialization together.

#include "app/cli.hpp"
#include "app/detector.hpp"
#include "io/result_writer.hpp"
#include "presentation/console_reporter.hpp"

#include <exception>
#include <iostream>

int main(int argc, char** argv) {
  try {
    const SearchOptions options = ParseCliArgs(argc, argv);
    const DetectionRunResult detection = RunDetector(options);
    WriteResults(detection.opportunities, options, detection.summary);

    PrintRunSummary(detection.summary);
    PrintOpportunities(detection.opportunities);
    std::cout << "Results written to: " << options.outputPath << "\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << "Error: " << error.what() << "\n";
    PrintUsage(std::cerr);
    return 1;
  }
}
