// Simple-cycle enumeration over the undirected token graph.

#include "domain/graph/cycle_enumerator.hpp"

#include "domain/graph/market_graph.hpp"

#include <algorithm>
#include <cstddef>
#include <vector>

namespace {

void AppendCycle(const Graph& graph, const std::vector<int>& path,
                 std::vector<CyclePattern>* cycles) {
  CyclePattern cycle;
  cycle.tokens = path;
  cycle.pools.reserve(path.size());

  for (std::size_t i = 0; i < path.size(); ++i) {
    cycle.pools.push_back(
        GetPoolForPair(graph, path[i], path[(i + 1) % path.size()]));
  }

  cycles->push_back(std::move(cycle));
}

void DepthFirstSearch(const Graph& graph, int start, int current,
                      std::size_t max_cycle_length, std::vector<int>* path,
                      std::vector<bool>* visited,
                      std::vector<CyclePattern>* cycles) {
  for (int next : graph.neighbors[current]) {
    if (next == start) {
      // With the smallest token fixed as the root, `path[1] < current`
      // eliminates the reverse traversal of the same undirected cycle.
      if (path->size() >= 3U && (*path)[1] < current) {
        AppendCycle(graph, *path, cycles);
      }
      continue;
    }

    if (path->size() >= max_cycle_length || next < start || (*visited)[next]) {
      continue;
    }

    (*visited)[next] = true;
    path->push_back(next);
    DepthFirstSearch(graph, start, next, max_cycle_length, path, visited,
                     cycles);
    path->pop_back();
    (*visited)[next] = false;
  }
}

}  // namespace

std::vector<CyclePattern> EnumerateSimpleCycles(const Graph& graph,
                                                std::size_t max_cycle_length) {
  if (max_cycle_length < 3U) {
    return {};
  }

  std::vector<CyclePattern> cycles;
  std::vector<bool> visited(graph.tokenByIndex.size(), false);
  std::vector<int> path;

  for (int start = 0; start < static_cast<int>(graph.tokenByIndex.size());
       ++start) {
    visited[start] = true;
    path.assign(1, start);

    for (int next : graph.neighbors[start]) {
      if (next <= start) {
        continue;
      }

      visited[next] = true;
      path.push_back(next);
      DepthFirstSearch(graph, start, next, max_cycle_length, &path, &visited,
                       &cycles);
      path.pop_back();
      visited[next] = false;
    }

    visited[start] = false;
  }

  return cycles;
}
