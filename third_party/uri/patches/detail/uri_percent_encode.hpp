/**
 *
 *    Copyright (c) 2025 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 *
 */
// Patched version for -fno-exceptions compilation
// Original: repo/uri/src/detail/uri_percent_encode.hpp

#ifndef NETWORK_DETAIL_URI_PERCENT_ENCODE_INC
#define NETWORK_DETAIL_URI_PERCENT_ENCODE_INC

#include <string>
#include <vector>
#include <locale>
#include <network/optional.hpp>
// Note: We don't include decode.hpp here because it contains exception-throwing code
// Our patched percent_encode doesn't use decode_char, so we don't need it

namespace network {
namespace detail {

// Patched version without exceptions - returns empty optional on error
inline optional<char> percent_encode(std::string::const_iterator it) {
  // Without exceptions, we check if decode_char would succeed by validating hex chars
  // Note: This is a simplified version that doesn't use decode_char to avoid exceptions
  // We manually decode the percent-encoded character

  // Check if we have enough characters for %XX
  auto it_copy = it;
  if (*it_copy != '%') {
    return optional<char>();
  }

  ++it_copy;
  if (it_copy == std::string::const_iterator()) {
    return optional<char>();
  }
  char h0 = *it_copy;
  ++it_copy;
  if (it_copy == std::string::const_iterator()) {
    return optional<char>();
  }
  char h1 = *it_copy;

  // Validate hex characters
  auto is_hex = [](char c) {
    return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
  };

  if (!is_hex(h0) || !is_hex(h1)) {
    return optional<char>();
  }

  // Safe to decode - calculate hex values
  auto v0 = (h0 >= '0' && h0 <= '9') ? (h0 - '0') :
            (h0 >= 'a' && h0 <= 'f') ? (h0 - 'a' + 10) : (h0 - 'A' + 10);
  auto v1 = (h1 >= '0' && h1 <= '9') ? (h1 - '0') :
            (h1 >= 'a' && h1 <= 'f') ? (h1 - 'a' + 10) : (h1 - 'A' + 10);

  char output = static_cast<char>((0x10 * v0) + v1);
  return optional<char>(output);
}

template <class String>
struct percent_encoded_to_upper {
  percent_encoded_to_upper() : count(0) {}

  void operator()(typename String::value_type &c) {
    if (c == '%') {
      count = 2;
    } else if (count > 0) {
      c = std::toupper(c, std::locale());
      --count;
    }
  }

  unsigned count;
};

template <class Iter>
Iter decode_encoded_unreserved_chars(Iter first, Iter last) {
  // unreserved  = ALPHA / DIGIT / "-" / "." / "_" / "~"
  // clang-format off
  const auto is_unreserved = [](char c) {
      return std::isalnum(c, std::locale())
      || '-' == c
      || '.' == c
      || '_' == c
      || '~' == c;
  };
  // clang-format on

  auto it = first, it2 = first;
  while (it != last) {
    if (*it == '%') {
      const auto sfirst = it;
      const auto opt_char = percent_encode(sfirst);
      if (opt_char && is_unreserved(*opt_char)) {
        *it2 = *opt_char;
        ++it;
        ++it;
      } else {
        *it2 = *it;
      }
    } else {
      *it2 = *it;
    }
    ++it;
    ++it2;
  }
  return it2;
}
}  // namespace detail
}  // namespace network

#endif  // NETWORK_DETAIL_URI_PERCENT_ENCODE_INC
