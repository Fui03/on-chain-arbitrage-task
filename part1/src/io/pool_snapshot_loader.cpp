// Snapshot loading and basic record validation for the provided pool dataset.

#include "io/pool_snapshot_loader.hpp"

#include "json.hpp"

#include <cerrno>
#include <cstdlib>
#include <fstream>
#include <iterator>
#include <stdexcept>
#include <string>

namespace {

long double ParseLongDouble(const std::string& value) {
  char* end = nullptr;
  errno = 0;
  const long double parsed = std::strtold(value.c_str(), &end);
  if (errno != 0 || end == nullptr || *end != '\0') {
    throw std::runtime_error("failed to parse decimal value: " + value);
  }
  return parsed;
}

}  // namespace

std::vector<PoolRecord> LoadPools(const std::string& path,
                                  long double min_reserve_usd) {
  std::ifstream input(path, std::ios::binary);
  if (!input) {
    throw std::runtime_error("unable to open pool snapshot: " + path);
  }

  const std::string json((std::istreambuf_iterator<char>(input)),
                         std::istreambuf_iterator<char>());
  if (json.empty()) {
    throw std::runtime_error("input file is empty: " + path);
  }

  std::vector<PoolRecord> pools;
  mini_json::Parser parser(json);
  parser.ParsePoolArray([&](const mini_json::Parser::ParsedPool& parsed_pool) {
    if (parsed_pool.pool.empty() || parsed_pool.token0.empty() ||
        parsed_pool.token1.empty()) {
      return;
    }

    const long double reserve0 = ParseLongDouble(parsed_pool.reserve0);
    const long double reserve1 = ParseLongDouble(parsed_pool.reserve1);
    const long double reserve_usd = ParseLongDouble(parsed_pool.reserveUsd);
    if (reserve0 <= 0.0L || reserve1 <= 0.0L ||
        reserve_usd < min_reserve_usd) {
      return;
    }

    PoolRecord record;
    record.pool = parsed_pool.pool;
    record.token0 = parsed_pool.token0;
    record.token1 = parsed_pool.token1;
    record.decimals0 = parsed_pool.decimals0;
    record.decimals1 = parsed_pool.decimals1;
    record.reserve0 = reserve0;
    record.reserve1 = reserve1;
    record.reserveUsd = reserve_usd;
    pools.push_back(std::move(record));
  });

  return pools;
}
