// JSON serialization for the ranked Part 1 output file.

#include "io/result_writer.hpp"

#include "util/number_utils.hpp"

#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>

namespace {

std::string Quote(const std::string& value) {
  return "\"" + value + "\"";
}

}  // namespace

void WriteResults(const std::vector<RankedOpportunity>& opportunities,
                  const SearchOptions& options, const ResultSummary& summary) {
  const std::filesystem::path output_path(options.outputPath);
  if (!output_path.parent_path().empty()) {
    std::filesystem::create_directories(output_path.parent_path());
  }

  std::ofstream output(options.outputPath);
  if (!output) {
    throw std::runtime_error("unable to write output file: " + options.outputPath);
  }

  output << "{\n";
  output << "  \"input\": " << Quote(options.inputPath) << ",\n";
  output << "  \"minReserveUsd\": " << Quote(FormatLongDouble(options.minReserveUsd, 6))
         << ",\n";
  output << "  \"maxInputShare\": " << Quote(FormatLongDouble(options.maxInputShare, 6))
         << ",\n";
  output << "  \"maxCycleLength\": " << options.maxCycleLength << ",\n";
  output << "  \"topN\": " << options.topN << ",\n";
  output << "  \"filteredPoolCount\": " << summary.filteredPoolCount << ",\n";
  output << "  \"filteredTokenCount\": " << summary.filteredTokenCount << ",\n";
  output << "  \"enumeratedCycleCount\": " << summary.enumeratedCycleCount
         << ",\n";
  output << "  \"profitableCandidates\": " << summary.profitableCandidates << ",\n";
  output << "  \"uniqueDirectedCycles\": " << summary.uniqueDirectedCycles << ",\n";
  output << "  \"returnedOpportunities\": " << summary.returnedOpportunities << ",\n";
  output << "  \"opportunities\": [\n";

  for (std::size_t i = 0; i < opportunities.size(); ++i) {
    const RankedOpportunity& opportunity = opportunities[i];
    output << "    {\n";

    output << "      \"tokens\": [";
    for (std::size_t j = 0; j < opportunity.tokens.size(); ++j) {
      output << Quote(opportunity.tokens[j]);
      if (j + 1 != opportunity.tokens.size()) {
        output << ", ";
      }
    }
    output << "],\n";

    output << "      \"tokenLabels\": [";
    for (std::size_t j = 0; j < opportunity.tokenLabels.size(); ++j) {
      output << Quote(opportunity.tokenLabels[j]);
      if (j + 1 != opportunity.tokenLabels.size()) {
        output << ", ";
      }
    }
    output << "],\n";

    output << "      \"pools\": [";
    for (std::size_t j = 0; j < opportunity.pools.size(); ++j) {
      output << Quote(opportunity.pools[j]);
      if (j + 1 != opportunity.pools.size()) {
        output << ", ";
      }
    }
    output << "],\n";

    output << "      \"cycleLength\": " << opportunity.cycleLength << ",\n";

    output << "      \"amountInHuman\": "
           << Quote(FormatLongDouble(opportunity.amountInHuman)) << ",\n";
    output << "      \"amountInRaw\": " << Quote(opportunity.amountInRaw) << ",\n";
    output << "      \"amountOutHuman\": "
           << Quote(FormatLongDouble(opportunity.amountOutHuman)) << ",\n";
    output << "      \"amountOutRaw\": " << Quote(opportunity.amountOutRaw)
           << ",\n";
    output << "      \"minOutRaw\": " << Quote(opportunity.minOutRaw) << ",\n";
    output << "      \"profitHuman\": "
           << Quote(FormatLongDouble(opportunity.profitHuman)) << ",\n";
    output << "      \"profitPct\": "
           << Quote(FormatLongDouble(opportunity.profitPct, 10)) << ",\n";
    output << "      \"estimatedProfitUsd\": "
           << Quote(FormatLongDouble(opportunity.estimatedProfitUsd)) << ",\n";
    output << "      \"hasTrustedUsdValuation\": "
           << (opportunity.hasTrustedUsdValuation ? "true" : "false") << ",\n";
    output << "      \"usdValuationHops\": " << opportunity.usdValuationHops
           << ",\n";
    output << "      \"usdValuationConfidenceUsd\": "
           << Quote(FormatLongDouble(opportunity.usdValuationConfidenceUsd))
           << ",\n";
    output << "      \"bottleneckReserveUsd\": "
           << Quote(FormatLongDouble(opportunity.bottleneckReserveUsd)) << ",\n";
    output << "      \"marginalProduct\": "
           << Quote(FormatLongDouble(opportunity.marginalProduct, 10)) << ",\n";
    output << "      \"startTokenDecimals\": " << opportunity.startTokenDecimals
           << "\n";
    output << "    }";
    if (i + 1 != opportunities.size()) {
      output << ",";
    }
    output << "\n";
  }

  output << "  ]\n";
  output << "}\n";
}
