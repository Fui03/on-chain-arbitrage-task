// APIs for deriving approximate USD anchor prices from stablecoin-connected
// pools in the graph.

#ifndef ARB_DOMAIN_PRICING_PRICE_ESTIMATOR_HPP
#define ARB_DOMAIN_PRICING_PRICE_ESTIMATOR_HPP

#include "domain/model/models.hpp"

/// Seeds stablecoins at $1 and propagates approximate anchor prices outward.
void EstimateAnchoredUsdPrices(Graph* graph);

#endif
