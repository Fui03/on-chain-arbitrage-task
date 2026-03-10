// Graph-building primitives for the filtered market snapshot.

#ifndef ARB_DOMAIN_GRAPH_MARKET_GRAPH_HPP
#define ARB_DOMAIN_GRAPH_MARKET_GRAPH_HPP

#include "domain/model/models.hpp"

#include <cstdint>
#include <vector>

/// Builds the unordered token-pair lookup key used by the graph.
std::uint64_t MakePairKey(int left, int right);

/// Converts filtered pool records into the in-memory token graph.
Graph BuildMarketGraph(std::vector<PoolRecord> pools);

/// Returns the pool index for an unordered token pair.
int GetPoolForPair(const Graph& graph, int left, int right);

/// Builds a directed traversal with reserves aligned to the requested path.
EdgeTraversal MakeTraversal(const Graph& graph, int from, int to, int pool_index);

#endif
