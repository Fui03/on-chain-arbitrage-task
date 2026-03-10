#ifndef MINI_JSON_HPP
#define MINI_JSON_HPP

#include <cctype>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <string_view>

namespace mini_json {

// Minimal JSON parser for the fixed pool snapshot schema used in this challenge.
class Parser {
 public:
  struct ParsedPool {
    std::string pool;
    std::string reserve0;
    std::string reserve1;
    std::string reserveUsd;
    std::string token0;
    std::string token1;
    int decimals0 = 0;
    int decimals1 = 0;
  };

  explicit Parser(std::string_view input) : input_(input) {}

  template <typename Callback>
  void ParsePoolArray(Callback&& callback) {
    SkipWhitespace();
    Expect('[');
    SkipWhitespace();
    if (Consume(']')) {
      return;
    }
    while (true) {
      callback(ParsePool());
      SkipWhitespace();
      if (Consume(']')) {
        break;
      }
      Expect(',');
      SkipWhitespace();
    }
  }

 private:
  ParsedPool ParsePool() {
    ParsedPool pool;
    Expect('{');
    SkipWhitespace();
    if (Consume('}')) {
      return pool;
    }

    while (true) {
      const std::string key = ParseString();
      SkipWhitespace();
      Expect(':');
      SkipWhitespace();

      if (key == "id") {
        pool.pool = ParseString();
      } else if (key == "reserve0") {
        pool.reserve0 = ParseString();
      } else if (key == "reserve1") {
        pool.reserve1 = ParseString();
      } else if (key == "reserveUSD") {
        pool.reserveUsd = ParseString();
      } else if (key == "token0") {
        ParseTokenObject(pool.token0, pool.decimals0);
      } else if (key == "token1") {
        ParseTokenObject(pool.token1, pool.decimals1);
      } else {
        SkipValue();
      }

      SkipWhitespace();
      if (Consume('}')) {
        break;
      }
      Expect(',');
      SkipWhitespace();
    }

    return pool;
  }

  void ParseTokenObject(std::string& token_id, int& decimals) {
    Expect('{');
    SkipWhitespace();
    if (Consume('}')) {
      return;
    }

    while (true) {
      const std::string key = ParseString();
      SkipWhitespace();
      Expect(':');
      SkipWhitespace();

      if (key == "id") {
        token_id = ParseString();
      } else if (key == "decimals") {
        decimals = std::stoi(ParseString());
      } else {
        SkipValue();
      }

      SkipWhitespace();
      if (Consume('}')) {
        break;
      }
      Expect(',');
      SkipWhitespace();
    }
  }

  std::string ParseString() {
    Expect('"');
    std::string value;
    while (position_ < input_.size()) {
      const char ch = input_[position_++];
      if (ch == '"') {
        return value;
      }
      if (ch != '\\') {
        value.push_back(ch);
        continue;
      }

      if (position_ >= input_.size()) {
        throw std::runtime_error("unterminated escape sequence");
      }
      const char escaped = input_[position_++];
      switch (escaped) {
        case '"':
        case '\\':
        case '/':
          value.push_back(escaped);
          break;
        case 'b':
          value.push_back('\b');
          break;
        case 'f':
          value.push_back('\f');
          break;
        case 'n':
          value.push_back('\n');
          break;
        case 'r':
          value.push_back('\r');
          break;
        case 't':
          value.push_back('\t');
          break;
        case 'u':
          AppendUnicodeEscape(value);
          break;
        default:
          throw std::runtime_error("unsupported JSON escape sequence");
      }
    }
    throw std::runtime_error("unterminated JSON string");
  }

  void AppendUnicodeEscape(std::string& value) {
    std::uint32_t codepoint = 0;
    for (int i = 0; i < 4; ++i) {
      if (position_ >= input_.size()) {
        throw std::runtime_error("truncated unicode escape");
      }
      codepoint <<= 4;
      const char ch = input_[position_++];
      if (ch >= '0' && ch <= '9') {
        codepoint |= static_cast<std::uint32_t>(ch - '0');
      } else if (ch >= 'a' && ch <= 'f') {
        codepoint |= static_cast<std::uint32_t>(10 + ch - 'a');
      } else if (ch >= 'A' && ch <= 'F') {
        codepoint |= static_cast<std::uint32_t>(10 + ch - 'A');
      } else {
        throw std::runtime_error("invalid unicode escape");
      }
    }

    if (codepoint <= 0x7F) {
      value.push_back(static_cast<char>(codepoint));
    } else if (codepoint <= 0x7FF) {
      value.push_back(static_cast<char>(0xC0 | ((codepoint >> 6) & 0x1F)));
      value.push_back(static_cast<char>(0x80 | (codepoint & 0x3F)));
    } else {
      value.push_back(static_cast<char>(0xE0 | ((codepoint >> 12) & 0x0F)));
      value.push_back(static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F)));
      value.push_back(static_cast<char>(0x80 | (codepoint & 0x3F)));
    }
  }

  void SkipValue() {
    SkipWhitespace();
    const char ch = Peek();
    if (ch == '"') {
      static_cast<void>(ParseString());
      return;
    }
    if (ch == '{') {
      SkipObject();
      return;
    }
    if (ch == '[') {
      SkipArray();
      return;
    }
    SkipPrimitive();
  }

  void SkipObject() {
    Expect('{');
    SkipWhitespace();
    if (Consume('}')) {
      return;
    }
    while (true) {
      static_cast<void>(ParseString());
      SkipWhitespace();
      Expect(':');
      SkipWhitespace();
      SkipValue();
      SkipWhitespace();
      if (Consume('}')) {
        break;
      }
      Expect(',');
      SkipWhitespace();
    }
  }

  void SkipArray() {
    Expect('[');
    SkipWhitespace();
    if (Consume(']')) {
      return;
    }
    while (true) {
      SkipValue();
      SkipWhitespace();
      if (Consume(']')) {
        break;
      }
      Expect(',');
      SkipWhitespace();
    }
  }

  void SkipPrimitive() {
    while (position_ < input_.size()) {
      const char ch = input_[position_];
      if (ch == ',' || ch == ']' || ch == '}' ||
          std::isspace(static_cast<unsigned char>(ch)) != 0) {
        return;
      }
      ++position_;
    }
  }

  void SkipWhitespace() {
    while (position_ < input_.size() &&
           std::isspace(static_cast<unsigned char>(input_[position_])) != 0) {
      ++position_;
    }
  }

  char Peek() const {
    if (position_ >= input_.size()) {
      throw std::runtime_error("unexpected end of JSON input");
    }
    return input_[position_];
  }

  bool Consume(char expected) {
    if (position_ < input_.size() && input_[position_] == expected) {
      ++position_;
      return true;
    }
    return false;
  }

  void Expect(char expected) {
    if (!Consume(expected)) {
      throw std::runtime_error(std::string("expected '") + expected + "'");
    }
  }

  std::string_view input_;
  std::size_t position_ = 0;
};

}  // namespace mini_json

#endif
