// Numeric formatting and token unit conversion helpers shared across Part 1.

#include "util/number_utils.hpp"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <string>

std::string FormatLongDouble(long double value, int precision) {
  if (!std::isfinite(static_cast<double>(value))) {
    return "0";
  }

  std::ostringstream stream;
  stream << std::fixed << std::setprecision(precision) << value;
  std::string text = stream.str();

  const std::size_t minus_offset = text[0] == '-' ? 1U : 0U;
  if (text.find('.') != std::string::npos) {
    while (text.size() > (minus_offset + 1) && text.back() == '0') {
      text.pop_back();
    }
    if (!text.empty() && text.back() == '.') {
      text.pop_back();
    }
  }

  if (text == "-0") {
    return "0";
  }
  return text;
}

unsigned __int128 HumanToRaw(long double amount, int decimals) {
  if (amount <= 0.0L) {
    return 0;
  }
  if (decimals < 0 || decimals > 38) {
    throw std::runtime_error("unsupported token decimals");
  }

  long double scale = 1.0L;
  for (int i = 0; i < decimals; ++i) {
    scale *= 10.0L;
  }
  const long double scaled = std::floor(amount * scale + 1e-12L);
  if (scaled < 0.0L) {
    return 0;
  }
  return static_cast<unsigned __int128>(scaled);
}

std::string Uint128ToString(unsigned __int128 value) {
  if (value == 0) {
    return "0";
  }

  std::string digits;
  while (value > 0) {
    const int digit = static_cast<int>(value % 10);
    digits.push_back(static_cast<char>('0' + digit));
    value /= 10;
  }
  std::reverse(digits.begin(), digits.end());
  return digits;
}
