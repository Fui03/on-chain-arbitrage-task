// Shared domain models for pool snapshots, graph state, search options, and
// ranked arbitrage output.

#ifndef ARB_DOMAIN_MODEL_MODELS_HPP
#define ARB_DOMAIN_MODEL_MODELS_HPP

#include <cstddef>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

/// One validated pool snapshot entry loaded from the JSON dataset.
struct PoolRecord {
  std::string pool;
  std::string token0;
  std::string token1;
  int decimals0 = 0;
  int decimals1 = 0;
  long double reserve0 = 0.0L;
  long double reserve1 = 0.0L;
  long double reserveUsd = 0.0L;
  int token0Index = -1;
  int token1Index = -1;
};

/// User-configurable options that control a single search run.
struct SearchOptions {
  std::string inputPath;
  std::string outputPath = "results/top10_cycles.json";
  std::size_t topN = 10;
  long double minReserveUsd = 1000.0L;
  long double maxInputShare = 0.05L;
  std::size_t maxCycleLength = 4;
};

/// High-level metrics emitted for reporting and result serialization.
struct ResultSummary {
  std::size_t filteredPoolCount = 0;
  std::size_t filteredTokenCount = 0;
  std::size_t enumeratedCycleCount = 0;
  std::size_t profitableCandidates = 0;
  std::size_t uniqueDirectedCycles = 0;
  std::size_t returnedOpportunities = 0;
};

/// A directed traversal through a single pool with reserves aligned to the path.
struct EdgeTraversal {
  int from = -1;
  int to = -1;
  int poolIndex = -1;
  long double reserveIn = 0.0L;
  long double reserveOut = 0.0L;
};

/// A unique undirected simple cycle in the filtered token graph.
struct CyclePattern {
  std::vector<int> tokens;
  std::vector<int> pools;
};

/// A fully evaluated arbitrage opportunity ready for ranking and output.
struct RankedOpportunity {
  std::vector<std::string> tokens;
  std::vector<std::string> tokenLabels;
  std::vector<std::string> pools;
  std::size_t cycleLength = 0;
  int startTokenDecimals = 0;
  long double amountInHuman = 0.0L;
  std::string amountInRaw;
  long double amountOutHuman = 0.0L;
  std::string amountOutRaw;
  std::string minOutRaw;
  long double profitHuman = 0.0L;
  long double profitPct = 0.0L;
  long double estimatedProfitUsd = 0.0L;
  bool hasTrustedUsdValuation = false;
  int usdValuationHops = -1;
  long double usdValuationConfidenceUsd = 0.0L;
  long double bottleneckReserveUsd = 0.0L;
  long double marginalProduct = 0.0L;
};

/// The filtered market graph plus derived pricing metadata.
struct Graph {
  std::vector<PoolRecord> pools;
  std::vector<std::string> tokenByIndex;
  std::vector<int> decimalsByToken;
  std::vector<std::vector<int>> neighbors;
  std::unordered_map<std::uint64_t, int> pairToPool;
  std::vector<long double> tokenUsdAnchored;
  std::vector<long double> tokenPriceConfidenceUsd;
  std::vector<int> tokenPriceHops;
};

#endif
