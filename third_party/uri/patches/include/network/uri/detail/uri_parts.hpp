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

#ifndef NETWORK_DETAIL_URI_PARTS_INC
#define NETWORK_DETAIL_URI_PARTS_INC

#include <string>
#include <utility>
#include <iterator>
#include <network/optional.hpp>
#include <network/string_view.hpp>

namespace network {
namespace detail {
class uri_part {
 public:
  typedef string_view::value_type value_type;
  typedef string_view::iterator iterator;
  typedef string_view::const_iterator const_iterator;
  typedef string_view::const_pointer const_pointer;
  typedef string_view::size_type size_type;
  typedef string_view::difference_type difference_type;

  uri_part() noexcept = default;

  uri_part(const_iterator first_arg, const_iterator last_arg) noexcept
      : first(first_arg), last(last_arg) {}

  const_iterator begin() const noexcept { return first; }

  const_iterator end() const noexcept { return last; }

  bool empty() const noexcept { return first == last; }

  std::string to_string() const { return std::string(first, last); }

  const_pointer ptr() const noexcept {
    assert(first != last);
    return first;
  }

  difference_type length() const noexcept { return last - first; }

  string_view to_string_view() const noexcept {
    return string_view(ptr(), length());
  }

 private:
  const_iterator first, last;
};

struct hierarchical_part {
  hierarchical_part() = default;

  optional<uri_part> user_info;
  optional<uri_part> host;
  optional<uri_part> port;
  optional<uri_part> path;

  void clear() {
    user_info = nullopt;
    host = nullopt;
    port = nullopt;
    path = nullopt;
  }
};

struct uri_parts {
  uri_parts() = default;

  optional<uri_part> scheme;
  hierarchical_part hier_part;
  optional<uri_part> query;
  optional<uri_part> fragment;

  void clear() {
    scheme = nullopt;
    hier_part.clear();
    query = nullopt;
    fragment = nullopt;
  }
};
}  // namespace detail
}  // namespace network

#endif  // NETWORK_DETAIL_URI_PARTS_INC
