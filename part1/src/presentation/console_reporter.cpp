// Console output helpers for human-readable run summaries and Top-N results.

#include "presentation/console_reporter.hpp"

#include "util/number_utils.hpp"
#include "util/token_registry.hpp"

#include <iomanip>
#include <iostream>
#include <sstream>

namespace {

std::string JoinPath(const std::vector<std::string>& labels) {
  std::ostringstream output;
  for (std::size_t i = 0; i < labels.size(); ++i) {
    if (i > 0U) {
      output << " -> ";
    }
    output << labels[i];
  }
  return output.str();
}

std::string JoinPools(const std::vector<std::string>& pools) {
  std::ostringstream output;
  for (std::size_t i = 0; i < pools.size(); ++i) {
    if (i > 0U) {
      output << ", ";
    }
    output << ShortenAddress(pools[i]);
  }
  return output.str();
}

}  // namespace

void PrintRunSummary(const ResultSummary& summary) {
  std::cout << "Loaded pools: " << summary.filteredPoolCount << "\n";
  std::cout << "Tokens in filtered graph: " << summary.filteredTokenCount << "\n";
  std::cout << "Enumerated cycles: " << summary.enumeratedCycleCount << "\n";
  std::cout << "Profitable candidates before de-duplication: "
            << summary.profitableCandidates << "\n";
  std::cout << "Unique directed cycles after rotation de-duplication: "
            << summary.uniqueDirectedCycles << "\n";
  std::cout << "Returned opportunities: " << summary.returnedOpportunities << "\n";
}

void PrintOpportunities(const std::vector<RankedOpportunity>& opportunities) {
  if (opportunities.empty()) {
    std::cout << "No profitable cycles found under the current filters.\n";
    return;
  }

  std::cout << "Top opportunities:\n";
  for (std::size_t i = 0; i < opportunities.size(); ++i) {
    const RankedOpportunity& opportunity = opportunities[i];
    std::cout << std::setw(2) << (i + 1) << ". "
              << JoinPath(opportunity.tokenLabels)
              << " | hops=" << opportunity.cycleLength
              << " | profit="
              << FormatLongDouble(opportunity.profitHuman, 8)
              << " | estUSD="
              << FormatLongDouble(opportunity.estimatedProfitUsd, 2)
              << " | input=" << FormatLongDouble(opportunity.amountInHuman, 8)
              << " | usdHops=" << opportunity.usdValuationHops << "\n";
    std::cout << "    pools: " << JoinPools(opportunity.pools) << "\n";
  }
}
