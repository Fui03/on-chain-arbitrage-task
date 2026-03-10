// Address-label helpers used to make console and JSON output more readable.

#include "util/token_registry.hpp"

#include <string>
#include <unordered_map>

std::string ShortenAddress(const std::string& address) {
  if (address.size() <= 12) {
    return address;
  }
  return address.substr(0, 6) + "..." + address.substr(address.size() - 4);
}

std::string TokenLabel(const std::string& address) {
  static const std::unordered_map<std::string, std::string> known_tokens = {
      {"0xc02aaa39b223fe8d0a0e5c4f27ead9083c756cc2", "WETH"},
      {"0xdac17f958d2ee523a2206206994597c13d831ec7", "USDT"},
      {"0xa0b86991c6218b36c1d19d4a2e9eb0ce3606eb48", "USDC"},
      {"0x6b175474e89094c44da98b954eedeac495271d0f", "DAI"},
      {"0x2260fac5e5542a773aa44fbcfedf7c193bc2c599", "WBTC"},
      {"0x514910771af9ca656af840dff83e8264ecf986ca", "LINK"},
      {"0x9f8f72aa9304c8b593d555f12ef6589cc3a579a2", "MKR"},
      {"0x0bc529c00c6401aef6d220be8c6ea1667f6ad93e", "YFI"},
      {"0xd46ba6d942050d489dbd938a2c909a5d5039a161", "AMPL"},
      {"0x1f9840a85d5af5bf1d1762f925bdaddc4201f984", "UNI"},
  };

  const auto it = known_tokens.find(address);
  if (it != known_tokens.end()) {
    return it->second;
  }
  return ShortenAddress(address);
}
