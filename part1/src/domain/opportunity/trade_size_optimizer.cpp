// Trade-size search utilities for one directed cycle path.

#include "domain/opportunity/trade_size_optimizer.hpp"

#include "domain/pricing/amm.hpp"

#include <algorithm>
#include <cmath>

namespace {

long double RefineBestAmount(const std::vector<EdgeTraversal>& edges,
                             long double lower, long double upper) {
  const long double phi = (1.0L + std::sqrt(5.0L)) / 2.0L;
  long double a = lower;
  long double b = upper;
  long double c = b - (b - a) / phi;
  long double d = a + (b - a) / phi;

  auto profit = [&](long double amount) { return CycleProfit(edges, amount); };

  // Golden-section search is a cheap local refinement once the coarse grid
  // already identified a promising interval.
  for (int i = 0; i < 60; ++i) {
    if (profit(c) < profit(d)) {
      a = c;
      c = d;
      d = a + (b - a) / phi;
    } else {
      b = d;
      d = c;
      c = b - (b - a) / phi;
    }
  }

  const long double mid = (a + b) / 2.0L;
  const std::array<long double, 5> candidates = {lower, c, d, mid, upper};
  long double best_amount = lower;
  long double best_profit = profit(lower);
  for (long double candidate : candidates) {
    const long double candidate_profit = profit(candidate);
    if (candidate_profit > best_profit) {
      best_profit = candidate_profit;
      best_amount = candidate;
    }
  }
  return best_amount;
}

}  // namespace

std::optional<OptimizedTrade> OptimizeTradeSize(
    const std::vector<EdgeTraversal>& edges, long double max_input_share) {
  if (edges.empty()) {
    return std::nullopt;
  }

  const long double first_reserve = edges[0].reserveIn;
  const long double min_amount = std::max(first_reserve * 1e-9L, 1e-18L);
  const long double max_amount = first_reserve * max_input_share;
  if (max_amount <= min_amount) {
    return std::nullopt;
  }

  constexpr int kGridSamples = 48;
  std::array<long double, kGridSamples> amounts{};
  std::array<long double, kGridSamples> profits{};
  const long double log_min = std::log(min_amount);
  const long double log_max = std::log(max_amount);
  int best_index = 0;
  long double best_profit = -1.0L;

  // Log spacing gives useful coverage across several orders of magnitude.
  for (int i = 0; i < kGridSamples; ++i) {
    const long double t = static_cast<long double>(i) / (kGridSamples - 1);
    amounts[i] = std::exp(log_min + (log_max - log_min) * t);
    profits[i] = CycleProfit(edges, amounts[i]);
    if (profits[i] > best_profit) {
      best_profit = profits[i];
      best_index = i;
    }
  }

  if (best_profit <= 0.0L) {
    return std::nullopt;
  }

  const int lower_index = std::max(0, best_index - 1);
  const int upper_index = std::min(kGridSamples - 1, best_index + 1);
  const long double best_amount =
      RefineBestAmount(edges, amounts[lower_index], amounts[upper_index]);
  const long double amount_out = CycleOutput(edges, best_amount);
  const long double profit = amount_out - best_amount;
  if (profit <= 0.0L) {
    return std::nullopt;
  }

  return OptimizedTrade{best_amount, amount_out, profit};
}
