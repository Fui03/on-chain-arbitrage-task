// Stablecoin-anchored USD price propagation for ranking cycle profitability.

#include "domain/pricing/price_estimator.hpp"

#include <cmath>
#include <limits>
#include <string>
#include <unordered_set>

namespace {

const std::unordered_set<std::string>& StableTokenAddresses() {
  static const std::unordered_set<std::string> stable_tokens = {
      "0xa0b86991c6218b36c1d19d4a2e9eb0ce3606eb48",  // USDC
      "0xdac17f958d2ee523a2206206994597c13d831ec7",  // USDT
      "0x6b175474e89094c44da98b954eedeac495271d0f",  // DAI
      "0x0000000000085d4780b73119b644ae5ecd22b376",  // TUSD
      "0x956f47f50a910163d8bf957cf5846d573e7f87ca",  // FEI
      "0x4fabb145d64652a948d72533023f6e7a623c7c53"   // BUSD
  };
  return stable_tokens;
}

void SeedStablecoinAnchors(Graph* graph) {
  graph->tokenUsdAnchored.assign(graph->tokenByIndex.size(), 0.0L);
  graph->tokenPriceConfidenceUsd.assign(graph->tokenByIndex.size(), 0.0L);
  graph->tokenPriceHops.assign(graph->tokenByIndex.size(),
                               std::numeric_limits<int>::max());

  const auto& stable_tokens = StableTokenAddresses();
  for (std::size_t token_index = 0; token_index < graph->tokenByIndex.size();
       ++token_index) {
    if (stable_tokens.count(graph->tokenByIndex[token_index]) == 0) {
      continue;
    }
    graph->tokenUsdAnchored[token_index] = 1.0L;
    graph->tokenPriceConfidenceUsd[token_index] =
        std::numeric_limits<long double>::infinity();
    graph->tokenPriceHops[token_index] = 0;
  }
}

void TryPropagatePrice(const Graph& graph, int hop, int priced_token,
                       int unpriced_token, long double reserve_priced,
                       long double reserve_unpriced, long double reserve_usd,
                       std::vector<long double>* next_prices,
                       std::vector<long double>* next_confidence,
                       std::vector<int>* next_hops) {
  if (graph.tokenPriceHops[priced_token] != hop || reserve_priced <= 0.0L ||
      reserve_unpriced <= 0.0L) {
    return;
  }

  const long double priced_usd = graph.tokenUsdAnchored[priced_token];
  if (priced_usd <= 0.0L) {
    return;
  }

  const int candidate_hops = hop + 1;
  const long double candidate_price =
      priced_usd * reserve_priced / reserve_unpriced;
  if (candidate_price <= 0.0L ||
      !std::isfinite(static_cast<double>(candidate_price))) {
    return;
  }

  if (candidate_hops < (*next_hops)[unpriced_token] ||
      (candidate_hops == (*next_hops)[unpriced_token] &&
       reserve_usd > (*next_confidence)[unpriced_token])) {
    (*next_hops)[unpriced_token] = candidate_hops;
    (*next_prices)[unpriced_token] = candidate_price;
    (*next_confidence)[unpriced_token] = reserve_usd;
  }
}

void FinalizeUnknownHops(Graph* graph) {
  for (std::size_t token_index = 0; token_index < graph->tokenPriceHops.size();
       ++token_index) {
    if (graph->tokenPriceHops[token_index] == std::numeric_limits<int>::max()) {
      graph->tokenPriceHops[token_index] = -1;
    }
  }
}

}  // namespace

void EstimateAnchoredUsdPrices(Graph* graph) {
  SeedStablecoinAnchors(graph);

  constexpr int kMaxPricingHops = 2;
  for (int hop = 0; hop < kMaxPricingHops; ++hop) {
    std::vector<long double> next_prices = graph->tokenUsdAnchored;
    std::vector<long double> next_confidence = graph->tokenPriceConfidenceUsd;
    std::vector<int> next_hops = graph->tokenPriceHops;

    // Each pass expands one graph hop away from already trusted anchors.
    for (const PoolRecord& pool : graph->pools) {
      TryPropagatePrice(*graph, hop, pool.token0Index, pool.token1Index,
                        pool.reserve0, pool.reserve1, pool.reserveUsd,
                        &next_prices, &next_confidence, &next_hops);
      TryPropagatePrice(*graph, hop, pool.token1Index, pool.token0Index,
                        pool.reserve1, pool.reserve0, pool.reserveUsd,
                        &next_prices, &next_confidence, &next_hops);
    }

    graph->tokenUsdAnchored.swap(next_prices);
    graph->tokenPriceConfidenceUsd.swap(next_confidence);
    graph->tokenPriceHops.swap(next_hops);
  }

  FinalizeUnknownHops(graph);
}
