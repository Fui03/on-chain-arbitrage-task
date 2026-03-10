// APIs for selecting a profitable input size for a directed arbitrage cycle.

#ifndef ARB_DOMAIN_OPPORTUNITY_TRADE_SIZE_OPTIMIZER_HPP
#define ARB_DOMAIN_OPPORTUNITY_TRADE_SIZE_OPTIMIZER_HPP

#include "domain/model/models.hpp"

#include <optional>
#include <vector>

/// The best trade size found for one directed arbitrage path.
struct OptimizedTrade {
  long double amountIn = 0.0L;
  long double amountOut = 0.0L;
  long double profit = 0.0L;
};

/// Searches a bounded input range and returns the most profitable trade size.
std::optional<OptimizedTrade> OptimizeTradeSize(
    const std::vector<EdgeTraversal>& edges, long double max_input_share);

#endif
