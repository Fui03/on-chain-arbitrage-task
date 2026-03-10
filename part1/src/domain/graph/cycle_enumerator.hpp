// APIs for enumerating unique simple cycles from the market graph.

#ifndef ARB_DOMAIN_GRAPH_CYCLE_ENUMERATOR_HPP
#define ARB_DOMAIN_GRAPH_CYCLE_ENUMERATOR_HPP

#include "domain/model/models.hpp"

#include <vector>

/// Enumerates undirected simple cycles up to the configured maximum length.
std::vector<CyclePattern> EnumerateSimpleCycles(
    const Graph& graph, std::size_t max_cycle_length);

#endif
