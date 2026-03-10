// Cycle evaluation logic that turns enumerated graph cycles into fully
// populated arbitrage opportunities.

#include "domain/opportunity/cycle_evaluator.hpp"

#include "domain/graph/market_graph.hpp"
#include "domain/opportunity/trade_size_optimizer.hpp"
#include "domain/pricing/amm.hpp"
#include "util/number_utils.hpp"
#include "util/token_registry.hpp"

#include <algorithm>
#include <limits>
#include <optional>
#include <vector>

namespace {

std::vector<int> Rotate(const std::vector<int>& values, std::size_t shift) {
  std::vector<int> rotated(values.size());
  for (std::size_t i = 0; i < values.size(); ++i) {
    rotated[i] = values[(i + shift) % values.size()];
  }
  return rotated;
}

std::vector<int> ReverseCycleTokens(const std::vector<int>& tokens) {
  std::vector<int> reversed;
  reversed.reserve(tokens.size());
  reversed.push_back(tokens.front());
  for (std::size_t i = tokens.size(); i-- > 1U;) {
    reversed.push_back(tokens[i]);
  }
  return reversed;
}

std::vector<int> ReverseCyclePools(const std::vector<int>& pools) {
  return std::vector<int>(pools.rbegin(), pools.rend());
}

std::vector<EdgeTraversal> BuildTraversals(const Graph& graph,
                                           const std::vector<int>& tokens,
                                           const std::vector<int>& pools) {
  std::vector<EdgeTraversal> edges;
  edges.reserve(tokens.size());

  for (std::size_t i = 0; i < tokens.size(); ++i) {
    edges.push_back(
        MakeTraversal(graph, tokens[i], tokens[(i + 1) % tokens.size()],
                      pools[i]));
  }

  return edges;
}

long double BottleneckReserveUsd(const Graph& graph,
                                 const std::vector<int>& pools) {
  long double bottleneck = std::numeric_limits<long double>::infinity();
  for (int pool_index : pools) {
    bottleneck = std::min(bottleneck, graph.pools[pool_index].reserveUsd);
  }
  return std::isfinite(static_cast<double>(bottleneck)) ? bottleneck : 0.0L;
}

std::optional<RankedOpportunity> EvaluateDirectedCycle(
    const Graph& graph, const std::vector<int>& ordered_tokens,
    const std::vector<int>& ordered_pools, long double max_input_share) {
  if (ordered_tokens.size() < 3U || ordered_tokens.size() != ordered_pools.size()) {
    return std::nullopt;
  }

  const std::vector<EdgeTraversal> edges =
      BuildTraversals(graph, ordered_tokens, ordered_pools);
  const long double marginal_product = MarginalCycleProduct(edges);
  if (marginal_product <= 1.0L) {
    return std::nullopt;
  }

  const std::optional<OptimizedTrade> optimized_trade =
      OptimizeTradeSize(edges, max_input_share);
  if (!optimized_trade.has_value()) {
    return std::nullopt;
  }
  const OptimizedTrade& trade = *optimized_trade;

  const int start_token = ordered_tokens[0];
  const int start_decimals = graph.decimalsByToken[start_token];
  const unsigned __int128 amount_in_raw =
      HumanToRaw(trade.amountIn, start_decimals);
  const unsigned __int128 amount_out_raw =
      HumanToRaw(trade.amountOut, start_decimals);
  const unsigned __int128 expected_profit_raw =
      amount_out_raw > amount_in_raw ? amount_out_raw - amount_in_raw : 0;
  const unsigned __int128 min_out_raw =
      amount_in_raw + (expected_profit_raw * 9U) / 10U;

  RankedOpportunity opportunity;
  opportunity.cycleLength = ordered_tokens.size();
  opportunity.tokens.reserve(ordered_tokens.size() + 1U);
  opportunity.tokenLabels.reserve(ordered_tokens.size() + 1U);
  opportunity.pools.reserve(ordered_pools.size());

  for (int token_index : ordered_tokens) {
    opportunity.tokens.push_back(graph.tokenByIndex[token_index]);
    opportunity.tokenLabels.push_back(TokenLabel(opportunity.tokens.back()));
  }
  opportunity.tokens.push_back(graph.tokenByIndex[start_token]);
  opportunity.tokenLabels.push_back(TokenLabel(opportunity.tokens.back()));

  for (int pool_index : ordered_pools) {
    opportunity.pools.push_back(graph.pools[pool_index].pool);
  }

  opportunity.startTokenDecimals = start_decimals;
  opportunity.amountInHuman = trade.amountIn;
  opportunity.amountInRaw = Uint128ToString(amount_in_raw);
  opportunity.amountOutHuman = trade.amountOut;
  opportunity.amountOutRaw = Uint128ToString(amount_out_raw);
  opportunity.minOutRaw = Uint128ToString(min_out_raw);
  opportunity.profitHuman = trade.profit;
  opportunity.profitPct = trade.profit / trade.amountIn;
  opportunity.hasTrustedUsdValuation = graph.tokenUsdAnchored[start_token] > 0.0L;
  opportunity.usdValuationHops = graph.tokenPriceHops[start_token];
  opportunity.usdValuationConfidenceUsd =
      graph.tokenPriceConfidenceUsd[start_token];
  opportunity.estimatedProfitUsd =
      opportunity.hasTrustedUsdValuation
          ? trade.profit * graph.tokenUsdAnchored[start_token]
          : 0.0L;
  opportunity.bottleneckReserveUsd =
      BottleneckReserveUsd(graph, ordered_pools);
  opportunity.marginalProduct = marginal_product;
  return opportunity;
}

void AppendDirectionOpportunities(
    const Graph& graph, const std::vector<int>& tokens,
    const std::vector<int>& pools, const SearchOptions& options,
    std::vector<RankedOpportunity>* opportunities) {
  for (std::size_t shift = 0; shift < tokens.size(); ++shift) {
    const std::optional<RankedOpportunity> opportunity = EvaluateDirectedCycle(
        graph, Rotate(tokens, shift), Rotate(pools, shift),
        options.maxInputShare);
    if (opportunity.has_value()) {
      opportunities->push_back(*opportunity);
    }
  }
}

}  // namespace

std::vector<RankedOpportunity> FindOpportunities(
    const Graph& graph, const std::vector<CyclePattern>& cycles,
    const SearchOptions& options) {
  std::vector<RankedOpportunity> opportunities;

  for (const CyclePattern& cycle : cycles) {
    AppendDirectionOpportunities(graph, cycle.tokens, cycle.pools, options,
                                 &opportunities);
    AppendDirectionOpportunities(graph, ReverseCycleTokens(cycle.tokens),
                                 ReverseCyclePools(cycle.pools), options,
                                 &opportunities);
  }

  return opportunities;
}
