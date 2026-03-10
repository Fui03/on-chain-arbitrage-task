// Constant-product pricing helpers used by both the optimizer and evaluator.

#ifndef ARB_DOMAIN_PRICING_AMM_HPP
#define ARB_DOMAIN_PRICING_AMM_HPP

#include "domain/model/models.hpp"

#include <vector>

/// Uniswap V2 fee multiplier expressed in human-scale decimal math.
constexpr long double kFeeMultiplier = 0.997L;

/// Quotes one directed swap against constant-product reserves.
inline long double SwapOutput(long double amount_in, long double reserve_in,
                              long double reserve_out) {
  if (amount_in <= 0.0L || reserve_in <= 0.0L || reserve_out <= 0.0L) {
    return 0.0L;
  }
  const long double amount_in_with_fee = amount_in * kFeeMultiplier;
  return (amount_in_with_fee * reserve_out) /
         (reserve_in + amount_in_with_fee);
}

/// Applies the cycle swap path to a candidate input amount.
inline long double CycleOutput(const std::vector<EdgeTraversal>& edges,
                               long double amount_in) {
  long double running = amount_in;
  for (const EdgeTraversal& edge : edges) {
    running = SwapOutput(running, edge.reserveIn, edge.reserveOut);
    if (running <= 0.0L) {
      return 0.0L;
    }
  }
  return running;
}

/// Computes net cycle profit in start-token units.
inline long double CycleProfit(const std::vector<EdgeTraversal>& edges,
                               long double amount_in) {
  return CycleOutput(edges, amount_in) - amount_in;
}

/// Approximates cycle profitability for an infinitesimal trade.
inline long double MarginalCycleProduct(
    const std::vector<EdgeTraversal>& edges) {
  long double product = 1.0L;
  for (const EdgeTraversal& edge : edges) {
    product *= kFeeMultiplier * edge.reserveOut / edge.reserveIn;
  }
  return product;
}

#endif
