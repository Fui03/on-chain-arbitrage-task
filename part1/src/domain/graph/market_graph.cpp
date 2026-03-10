// Graph construction helpers that translate pool records into adjacency and
// pair lookup structures used by the search layer.

#include "domain/graph/market_graph.hpp"

#include <algorithm>
#include <stdexcept>
#include <string>
#include <unordered_map>

namespace {

int EnsureTokenIndex(Graph* graph, std::unordered_map<std::string, int>* token_to_index,
                     const std::string& token, int decimals) {
  const auto existing = token_to_index->find(token);
  if (existing != token_to_index->end()) {
    const int index = existing->second;
    if ((*graph).decimalsByToken[index] <= 0 && decimals > 0) {
      (*graph).decimalsByToken[index] = decimals;
    }
    return index;
  }

  const int index = static_cast<int>(graph->tokenByIndex.size());
  token_to_index->emplace(token, index);
  graph->tokenByIndex.push_back(token);
  graph->decimalsByToken.push_back(decimals);
  graph->neighbors.emplace_back();
  return index;
}

}  // namespace

std::uint64_t MakePairKey(int left, int right) {
  const std::uint32_t low = static_cast<std::uint32_t>(std::min(left, right));
  const std::uint32_t high = static_cast<std::uint32_t>(std::max(left, right));
  return (static_cast<std::uint64_t>(low) << 32U) | high;
}

Graph BuildMarketGraph(std::vector<PoolRecord> pools) {
  Graph graph;
  graph.pools = std::move(pools);

  std::unordered_map<std::string, int> token_to_index;
  token_to_index.reserve(graph.pools.size() * 2U);

  for (std::size_t i = 0; i < graph.pools.size(); ++i) {
    PoolRecord& pool = graph.pools[i];
    pool.token0Index =
        EnsureTokenIndex(&graph, &token_to_index, pool.token0, pool.decimals0);
    pool.token1Index =
        EnsureTokenIndex(&graph, &token_to_index, pool.token1, pool.decimals1);

    graph.neighbors[pool.token0Index].push_back(pool.token1Index);
    graph.neighbors[pool.token1Index].push_back(pool.token0Index);
    graph.pairToPool.emplace(MakePairKey(pool.token0Index, pool.token1Index),
                             static_cast<int>(i));
  }

  for (std::vector<int>& neighbors : graph.neighbors) {
    // Deduplicating adjacency lists keeps simple-cycle enumeration deterministic
    // and avoids revisiting the same neighbor relationship repeatedly.
    std::sort(neighbors.begin(), neighbors.end());
    neighbors.erase(std::unique(neighbors.begin(), neighbors.end()),
                    neighbors.end());
  }

  return graph;
}

int GetPoolForPair(const Graph& graph, int left, int right) {
  const auto it = graph.pairToPool.find(MakePairKey(left, right));
  if (it == graph.pairToPool.end()) {
    throw std::runtime_error("missing pool for token pair");
  }
  return it->second;
}

EdgeTraversal MakeTraversal(const Graph& graph, int from, int to,
                            int pool_index) {
  const PoolRecord& pool = graph.pools.at(pool_index);
  if (pool.token0Index == from && pool.token1Index == to) {
    return EdgeTraversal{from, to, pool_index, pool.reserve0, pool.reserve1};
  }
  if (pool.token0Index == to && pool.token1Index == from) {
    return EdgeTraversal{from, to, pool_index, pool.reserve1, pool.reserve0};
  }
  throw std::runtime_error("pool does not match requested traversal");
}
