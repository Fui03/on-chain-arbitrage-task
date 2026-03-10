// APIs for writing ranked opportunities and summary metadata to disk.

#ifndef ARB_IO_RESULT_WRITER_HPP
#define ARB_IO_RESULT_WRITER_HPP

#include "domain/model/models.hpp"

#include <vector>

/// Serializes the ranked opportunities and summary statistics to JSON.
void WriteResults(const std::vector<RankedOpportunity>& opportunities,
                  const SearchOptions& options, const ResultSummary& summary);

#endif
