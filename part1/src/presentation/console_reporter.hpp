// Public reporting helpers for stdout-based presentation of detector output.

#ifndef ARB_PRESENTATION_CONSOLE_REPORTER_HPP
#define ARB_PRESENTATION_CONSOLE_REPORTER_HPP

#include "domain/model/models.hpp"

#include <vector>

/// Prints the aggregate run statistics to stdout.
void PrintRunSummary(const ResultSummary& summary);

/// Prints the human-readable Top-N opportunity summary to stdout.
void PrintOpportunities(const std::vector<RankedOpportunity>& opportunities);

#endif
