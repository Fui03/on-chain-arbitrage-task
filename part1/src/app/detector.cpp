// Top-level orchestration for the Part 1 off-chain detection pipeline.

#include "app/detector.hpp"

#include "domain/graph/cycle_enumerator.hpp"
#include "domain/graph/market_graph.hpp"
#include "domain/opportunity/cycle_evaluator.hpp"
#include "domain/opportunity/opportunity_ranker.hpp"
#include "domain/pricing/price_estimator.hpp"
#include "io/pool_snapshot_loader.hpp"

DetectionRunResult RunDetector(const SearchOptions& options) {
  std::vector<PoolRecord> pools = LoadPools(options.inputPath, options.minReserveUsd);
  Graph graph = BuildMarketGraph(std::move(pools));
  EstimateAnchoredUsdPrices(&graph);

  const std::vector<CyclePattern> cycles =
      EnumerateSimpleCycles(graph, options.maxCycleLength);
  std::vector<RankedOpportunity> opportunities =
      FindOpportunities(graph, cycles, options);

  DetectionRunResult result;
  result.summary.filteredPoolCount = graph.pools.size();
  result.summary.filteredTokenCount = graph.tokenByIndex.size();
  result.summary.enumeratedCycleCount = cycles.size();
  result.summary.profitableCandidates = opportunities.size();
  result.summary.uniqueDirectedCycles =
      DeduplicateDirectedCycles(&opportunities);
  SortAndTrim(&opportunities, options.topN);
  result.summary.returnedOpportunities = opportunities.size();
  result.opportunities = std::move(opportunities);
  return result;
}
