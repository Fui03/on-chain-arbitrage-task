// APIs for evaluating graph cycles as directed arbitrage opportunities.

#ifndef ARB_DOMAIN_OPPORTUNITY_CYCLE_EVALUATOR_HPP
#define ARB_DOMAIN_OPPORTUNITY_CYCLE_EVALUATOR_HPP

#include "domain/model/models.hpp"

#include <vector>

/// Evaluates profitable directed rotations for every enumerated cycle.
std::vector<RankedOpportunity> FindOpportunities(
    const Graph& graph, const std::vector<CyclePattern>& cycles,
    const SearchOptions& options);

#endif
