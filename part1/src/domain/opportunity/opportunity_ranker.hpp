// APIs for de-duplicating and ordering evaluated opportunities.

#ifndef ARB_DOMAIN_OPPORTUNITY_OPPORTUNITY_RANKER_HPP
#define ARB_DOMAIN_OPPORTUNITY_OPPORTUNITY_RANKER_HPP

#include "domain/model/models.hpp"

#include <vector>

/// Collapses rotation-equivalent directed cycles to their best representative.
std::size_t DeduplicateDirectedCycles(std::vector<RankedOpportunity>* opportunities);
/// Orders opportunities by valuation quality, profit, and liquidity, then trims.
void SortAndTrim(std::vector<RankedOpportunity>* opportunities,
                 std::size_t top_n);

#endif
