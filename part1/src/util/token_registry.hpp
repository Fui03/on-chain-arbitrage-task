// Token labeling helpers for known assets and generic address formatting.

#ifndef ARB_UTIL_TOKEN_REGISTRY_HPP
#define ARB_UTIL_TOKEN_REGISTRY_HPP

#include <string>

/// Shortens an address for console-friendly output.
std::string ShortenAddress(const std::string& address);

/// Maps well-known token addresses to readable labels when available.
std::string TokenLabel(const std::string& address);

#endif
