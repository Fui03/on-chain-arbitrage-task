// Regression tests for the Part 1 pricing, graph, and opportunity logic.

#include "../src/domain/graph/cycle_enumerator.hpp"
#include "../src/domain/graph/market_graph.hpp"
#include "../src/domain/opportunity/cycle_evaluator.hpp"
#include "../src/domain/opportunity/opportunity_ranker.hpp"
#include "../src/domain/pricing/amm.hpp"
#include "../src/domain/pricing/price_estimator.hpp"
#include "../src/util/number_utils.hpp"

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <vector>

namespace {

void Expect(bool condition, const char* message) {
  if (!condition) {
    throw std::runtime_error(message);
  }
}

void ExpectNear(long double actual, long double expected, long double tolerance,
                const char* message) {
  if (std::fabsl(actual - expected) > tolerance) {
    throw std::runtime_error(message);
  }
}

Graph BuildSyntheticGraph(const std::vector<PoolRecord>& pools) {
  Graph graph =
      BuildMarketGraph(std::vector<PoolRecord>(pools.begin(), pools.end()));
  EstimateAnchoredUsdPrices(&graph);
  return graph;
}

void TestSwapFormula() {
  const long double out = SwapOutput(100.0L, 1000.0L, 1000.0L);
  ExpectNear(out, 90.661089388014913L, 1e-12L,
             "swap output does not match constant product formula");
  ExpectNear(SwapOutput(100.0L, 1000.0L, 1000.0L), out, 1e-12L,
             "symmetric pool should have symmetric quote");
}

void TestTriangleEnumerationDedup() {
  const std::vector<PoolRecord> pools = {
      {"poolAB", "A", "B", 18, 18, 100.0L, 100.0L, 5000.0L},
      {"poolBC", "B", "C", 18, 18, 100.0L, 100.0L, 5000.0L},
      {"poolCA", "C", "A", 18, 18, 100.0L, 100.0L, 5000.0L},
  };
  const Graph graph = BuildSyntheticGraph(pools);
  const std::vector<CyclePattern> cycles = EnumerateSimpleCycles(graph, 4);
  Expect(cycles.size() == 1,
         "cycle enumeration should deduplicate triangle rotations");
  Expect(cycles[0].tokens.size() == 3U,
         "triangle should still be represented as a 3-token cycle");
}

void TestFourCycleEnumeration() {
  const std::vector<PoolRecord> pools = {
      {"poolAB", "A", "B", 18, 18, 100.0L, 100.0L, 5000.0L},
      {"poolBC", "B", "C", 18, 18, 100.0L, 100.0L, 5000.0L},
      {"poolCD", "C", "D", 18, 18, 100.0L, 100.0L, 5000.0L},
      {"poolDA", "D", "A", 18, 18, 100.0L, 100.0L, 5000.0L},
  };
  const Graph graph = BuildSyntheticGraph(pools);
  const std::vector<CyclePattern> cycles = EnumerateSimpleCycles(graph, 4);
  Expect(cycles.size() == 1,
         "expected one canonical 4-token cycle in the synthetic graph");
  Expect(cycles[0].tokens.size() == 4U,
         "4-cycle should be represented with four distinct tokens");
}

void TestProfitableVsUnprofitable() {
  SearchOptions options;
  options.maxCycleLength = 4;

  const std::vector<PoolRecord> profitable_pools = {
      {"poolAB", "A", "B", 18, 18, 100.0L, 100.0L, 10000.0L},
      {"poolBC", "B", "C", 18, 18, 100.0L, 100.0L, 10000.0L},
      {"poolCA", "C", "A", 18, 18, 80.0L, 100.0L, 10000.0L},
  };
  const Graph profitable_graph = BuildSyntheticGraph(profitable_pools);
  const std::vector<CyclePattern> profitable_cycles =
      EnumerateSimpleCycles(profitable_graph, options.maxCycleLength);
  const std::vector<RankedOpportunity> profitable =
      FindOpportunities(profitable_graph, profitable_cycles, options);
  Expect(!profitable.empty(), "expected a profitable synthetic triangle");

  const std::vector<PoolRecord> flat_pools = {
      {"poolAB", "A", "B", 18, 18, 100.0L, 100.0L, 10000.0L},
      {"poolBC", "B", "C", 18, 18, 100.0L, 100.0L, 10000.0L},
      {"poolCA", "C", "A", 18, 18, 100.0L, 100.0L, 10000.0L},
  };
  const Graph flat_graph = BuildSyntheticGraph(flat_pools);
  const std::vector<CyclePattern> flat_cycles =
      EnumerateSimpleCycles(flat_graph, options.maxCycleLength);
  const std::vector<RankedOpportunity> unprofitable =
      FindOpportunities(flat_graph, flat_cycles, options);
  Expect(unprofitable.empty(),
         "equal-price triangle should not be profitable after fees");
}

void TestProfitableFourCycle() {
  SearchOptions options;
  options.maxCycleLength = 4;

  const std::vector<PoolRecord> profitable_pools = {
      {"poolAB", "A", "B", 18, 18, 100.0L, 100.0L, 10000.0L},
      {"poolBC", "B", "C", 18, 18, 100.0L, 100.0L, 10000.0L},
      {"poolCD", "C", "D", 18, 18, 100.0L, 100.0L, 10000.0L},
      {"poolDA", "D", "A", 18, 18, 80.0L, 100.0L, 10000.0L},
  };
  const Graph graph = BuildSyntheticGraph(profitable_pools);
  const std::vector<CyclePattern> cycles =
      EnumerateSimpleCycles(graph, options.maxCycleLength);
  const std::vector<RankedOpportunity> opportunities =
      FindOpportunities(graph, cycles, options);

  Expect(!opportunities.empty(),
         "expected a profitable synthetic 4-token cycle");
  Expect(opportunities.front().cycleLength == 4U,
         "expected evaluator to preserve 4-cycle length");
}

void TestRawConversion() {
  Expect(Uint128ToString(HumanToRaw(1.234567L, 6)) == "1234567",
         "6-decimal raw conversion failed");
  Expect(Uint128ToString(HumanToRaw(0.000000000000000123L, 18)) == "123",
         "18-decimal raw conversion failed");
}

void TestDirectedCycleDeduplication() {
  SearchOptions options;
  options.maxCycleLength = 4;

  const std::vector<PoolRecord> profitable_pools = {
      {"poolAB", "A", "B", 18, 18, 100.0L, 100.0L, 10000.0L},
      {"poolBC", "B", "C", 18, 18, 100.0L, 100.0L, 10000.0L},
      {"poolCA", "C", "A", 18, 18, 80.0L, 100.0L, 10000.0L},
  };

  const Graph graph = BuildSyntheticGraph(profitable_pools);
  const std::vector<CyclePattern> cycles =
      EnumerateSimpleCycles(graph, options.maxCycleLength);
  std::vector<RankedOpportunity> opportunities =
      FindOpportunities(graph, cycles, options);
  Expect(opportunities.size() == 3U,
         "profitable directed triangle should appear once per rotation before de-duplication");

  const std::size_t unique_cycles = DeduplicateDirectedCycles(&opportunities);
  Expect(unique_cycles == 1U,
         "rotation-equivalent directed cycles should collapse to one result");
}

void TestAnchoredPricing() {
  const std::vector<PoolRecord> pools = {
      {"poolUsdcWeth",
       "0xa0b86991c6218b36c1d19d4a2e9eb0ce3606eb48",
       "0xc02aaa39b223fe8d0a0e5c4f27ead9083c756cc2",
       6,
       18,
       2000.0L,
       1.0L,
       4000.0L},
      {"poolWethToken", "0xc02aaa39b223fe8d0a0e5c4f27ead9083c756cc2",
       "0x1111111111111111111111111111111111111111", 18, 18, 1.0L, 100.0L,
       4000.0L},
  };

  const Graph graph = BuildSyntheticGraph(pools);

  int usdc_index = -1;
  int weth_index = -1;
  int token_index = -1;
  for (int i = 0; i < static_cast<int>(graph.tokenByIndex.size()); ++i) {
    if (graph.tokenByIndex[i] == "0xa0b86991c6218b36c1d19d4a2e9eb0ce3606eb48") {
      usdc_index = i;
    }
    if (graph.tokenByIndex[i] == "0xc02aaa39b223fe8d0a0e5c4f27ead9083c756cc2") {
      weth_index = i;
    }
    if (graph.tokenByIndex[i] == "0x1111111111111111111111111111111111111111") {
      token_index = i;
    }
  }

  Expect(usdc_index >= 0, "expected USDC index");
  Expect(weth_index >= 0, "expected WETH index");
  Expect(token_index >= 0, "expected custom token index");
  ExpectNear(graph.tokenUsdAnchored[usdc_index], 1.0L, 1e-12L,
             "USDC should remain at 1 USD");
  ExpectNear(graph.tokenUsdAnchored[weth_index], 2000.0L, 1e-9L,
             "WETH anchor price should be propagated from USDC pool");
  ExpectNear(graph.tokenUsdAnchored[token_index], 20.0L, 1e-9L,
             "token anchor price should be propagated from WETH pool");
  Expect(graph.tokenPriceHops[weth_index] == 1,
         "WETH should be one pricing hop from stablecoins");
  Expect(graph.tokenPriceHops[token_index] == 2,
         "secondary token should be two pricing hops from stablecoins");
}

}  // namespace

int main() {
  try {
    TestSwapFormula();
    TestTriangleEnumerationDedup();
    TestFourCycleEnumeration();
    TestProfitableVsUnprofitable();
    TestProfitableFourCycle();
    TestRawConversion();
    TestDirectedCycleDeduplication();
    TestAnchoredPricing();
    std::cout << "All Part 1 tests passed.\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << "Test failure: " << error.what() << "\n";
    return 1;
  }
}
