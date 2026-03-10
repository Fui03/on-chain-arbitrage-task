// Application-layer API for running one complete arbitrage detection pass.

#ifndef ARB_APP_DETECTOR_HPP
#define ARB_APP_DETECTOR_HPP

#include "domain/model/models.hpp"

#include <vector>

/// Aggregates the ranked output list with the run summary metadata.
struct DetectionRunResult {
  ResultSummary summary;
  std::vector<RankedOpportunity> opportunities;
};

/// Runs the full off-chain detection pipeline for one search configuration.
DetectionRunResult RunDetector(const SearchOptions& options);

#endif
