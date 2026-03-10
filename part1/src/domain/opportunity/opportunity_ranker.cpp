// Ranking and de-duplication logic for evaluated arbitrage opportunities.

#include "domain/opportunity/opportunity_ranker.hpp"

#include <algorithm>
#include <string>
#include <unordered_map>
#include <vector>

namespace {

bool BetterOpportunity(const RankedOpportunity& left,
                       const RankedOpportunity& right) {
  if (left.hasTrustedUsdValuation != right.hasTrustedUsdValuation) {
    return left.hasTrustedUsdValuation;
  }
  if (left.estimatedProfitUsd != right.estimatedProfitUsd) {
    return left.estimatedProfitUsd > right.estimatedProfitUsd;
  }
  if (left.usdValuationHops != right.usdValuationHops) {
    if (left.usdValuationHops < 0) {
      return false;
    }
    if (right.usdValuationHops < 0) {
      return true;
    }
    return left.usdValuationHops < right.usdValuationHops;
  }
  if (left.usdValuationConfidenceUsd != right.usdValuationConfidenceUsd) {
    return left.usdValuationConfidenceUsd > right.usdValuationConfidenceUsd;
  }
  if (left.profitPct != right.profitPct) {
    return left.profitPct > right.profitPct;
  }
  return left.bottleneckReserveUsd > right.bottleneckReserveUsd;
}

std::string DirectedCycleKey(const RankedOpportunity& opportunity) {
  const std::vector<std::string> cycle(opportunity.tokens.begin(),
                                       opportunity.tokens.end() - 1);
  std::string best;

  for (std::size_t shift = 0; shift < cycle.size(); ++shift) {
    std::string candidate;
    for (std::size_t i = 0; i < cycle.size(); ++i) {
      if (i > 0U) {
        candidate += "|";
      }
      candidate += cycle[(shift + i) % cycle.size()];
    }
    if (best.empty() || candidate < best) {
      best = candidate;
    }
  }

  return best;
}

}  // namespace

std::size_t DeduplicateDirectedCycles(
    std::vector<RankedOpportunity>* opportunities) {
  std::unordered_map<std::string, std::size_t> best_by_cycle;
  std::vector<RankedOpportunity> unique;
  unique.reserve(opportunities->size());

  for (const RankedOpportunity& opportunity : *opportunities) {
    const std::string key = DirectedCycleKey(opportunity);
    const auto existing = best_by_cycle.find(key);
    if (existing == best_by_cycle.end()) {
      best_by_cycle.emplace(key, unique.size());
      unique.push_back(opportunity);
      continue;
    }

    // Keep the strongest representative for each directed cycle regardless of
    // which rotation discovered it first.
    RankedOpportunity& current_best = unique[existing->second];
    if (BetterOpportunity(opportunity, current_best)) {
      current_best = opportunity;
    }
  }

  opportunities->swap(unique);
  return opportunities->size();
}

void SortAndTrim(std::vector<RankedOpportunity>* opportunities,
                 std::size_t top_n) {
  std::sort(opportunities->begin(), opportunities->end(), BetterOpportunity);
  if (opportunities->size() > top_n) {
    opportunities->resize(top_n);
  }
}
