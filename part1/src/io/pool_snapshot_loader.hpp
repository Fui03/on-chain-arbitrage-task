// APIs for loading and filtering the JSON pool snapshot.

#ifndef ARB_IO_POOL_SNAPSHOT_LOADER_HPP
#define ARB_IO_POOL_SNAPSHOT_LOADER_HPP

#include "domain/model/models.hpp"

#include <string>
#include <vector>

/// Loads pools from the snapshot and applies the liquidity filter.
std::vector<PoolRecord> LoadPools(const std::string& path,
                                  long double min_reserve_usd);

#endif
