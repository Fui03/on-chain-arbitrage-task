// Small numeric helpers for formatting and converting token quantities.

#ifndef ARB_UTIL_NUMBER_UTILS_HPP
#define ARB_UTIL_NUMBER_UTILS_HPP

#include <string>

/// Formats a floating-point value while trimming insignificant trailing zeros.
std::string FormatLongDouble(long double value, int precision = 18);

/// Converts a human-readable token amount into raw integer units.
unsigned __int128 HumanToRaw(long double amount, int decimals);

/// Formats an unsigned 128-bit integer as base-10 text.
std::string Uint128ToString(unsigned __int128 value);

#endif
