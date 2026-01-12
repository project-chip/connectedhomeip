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

#include <cassert>
#include <cstdlib>
#include <locale>
#include <algorithm>
#include <functional>
#include "network/uri/uri.hpp"
#include "detail/uri_parse.hpp"
#include "detail/uri_advance_parts.hpp"
#include "detail/uri_percent_encode.hpp"
#include "detail/uri_normalize.hpp"
#include "detail/uri_resolve.hpp"
#include "detail/algorithm.hpp"

namespace network {
namespace {
// With the parser, we use string_views, which are mutable. However,
// there are times (e.g. during normalization), when we want a part
// to be mutable. This function returns a pair of
// std::string::iterators in the same range as the URI part.
//
inline std::pair<std::string::iterator, std::string::iterator> mutable_part(
    std::string &str, detail::uri_part part) {
  auto view = string_view(str);

  auto first_index = std::distance(std::begin(view), std::begin(part));
  auto first = std::begin(str);
  std::advance(first, first_index);

  auto last_index = std::distance(std::begin(view), std::end(part));
  auto last = std::begin(str);
  std::advance(last, last_index);

  return std::make_pair(first, last);
}

// This is a convenience function that converts a part of a
// std::string to a string_view.
inline string_view to_string_view(const std::string &uri,
                                  detail::uri_part part) {
  if (!part.empty()) {
    const char *c_str = uri.c_str();
    const char *part_begin = &(*(std::begin(part)));
    std::advance(c_str, std::distance(c_str, part_begin));
    return string_view(c_str, std::distance(std::begin(part), std::end(part)));
  }
  return string_view();
}

inline optional<std::string> make_arg(optional<string_view> view) {
  if (view) {
    return view->to_string();
  }
  return nullopt;
}

template <class T>
inline void ignore(T) {}
}  // namespace

void uri::initialize(optional<string_type> scheme,
                     optional<string_type> user_info,
                     optional<string_type> host, optional<string_type> port,
                     optional<string_type> path, optional<string_type> query,
                     optional<string_type> fragment) {
  if (scheme) {
    uri_.append(*scheme);
  }

  if (user_info || host || port) {
    if (scheme) {
      uri_.append("://");
    }

    if (user_info) {
      uri_.append(*user_info);
      uri_.append("@");
    }

    if (host) {
      uri_.append(*host);
    } else {
      abort();  // Patched: replaced throw uri_builder_error() with abort() for -fno-exceptions
    }

    if (port) {
      uri_.append(":");
      uri_.append(*port);
    }
  } else {
    if (scheme) {
      if (path || query || fragment) {
        uri_.append(":");
      } else {
        abort();  // Patched: replaced throw uri_builder_error() with abort() for -fno-exceptions
      }
    }
  }

  if (path) {
    // if the URI is not opaque and the path is not already prefixed
    // with a '/', add one.
    if (host && (!path->empty() && path->front() != '/')) {
      path = "/" + *path;
    }
    uri_.append(*path);
  }

  if (query) {
    uri_.append("?");
    uri_.append(*query);
  }

  if (fragment) {
    uri_.append("#");
    uri_.append(*fragment);
  }

  uri_view_ = string_view(uri_);

  auto it = std::begin(uri_view_);
  if (scheme) {
    uri_parts_.scheme = detail::copy_part(*scheme, it);
    // ignore : and ://
    if (*it == ':') {
      ++it;
    }
    if (*it == '/' && *(it + 1) == '/') {
      it += 2;
    }
  }

  if (user_info) {
    uri_parts_.hier_part.user_info = detail::copy_part(*user_info, it);
    ++it;  // ignore @
  }

  if (host) {
    uri_parts_.hier_part.host = detail::copy_part(*host, it);
  }

  if (port) {
    ++it;  // ignore :
    uri_parts_.hier_part.port = detail::copy_part(*port, it);
  }

  if (path) {
    uri_parts_.hier_part.path = detail::copy_part(*path, it);
  }

  if (query) {
    ++it;  // ignore ?
    uri_parts_.query = detail::copy_part(*query, it);
  }

  if (fragment) {
    ++it;  // ignore #
    uri_parts_.fragment = detail::copy_part(*fragment, it);
  }
}

uri::uri() : uri_view_(uri_) {}

uri::uri(const uri &other) : uri_(other.uri_), uri_view_(uri_) {
  detail::advance_parts(uri_view_, uri_parts_, other.uri_parts_);
}

uri::uri(const uri_builder &builder) {
  initialize(builder.scheme_, builder.user_info_, builder.host_, builder.port_,
             builder.path_, builder.query_, builder.fragment_);
}

uri::uri(uri &&other) noexcept
    : uri_(std::move(other.uri_)),
      uri_view_(uri_),
      uri_parts_(std::move(other.uri_parts_)) {
  detail::advance_parts(uri_view_, uri_parts_, other.uri_parts_);
  other.uri_.clear();
  other.uri_view_ = string_view(other.uri_);
  other.uri_parts_ = detail::uri_parts();
}

uri::~uri() {}

uri &uri::operator=(uri other) {
  other.swap(*this);
  return *this;
}

void uri::swap(uri &other) noexcept {
  uri_.swap(other.uri_);
  uri_view_ = uri_;
  other.uri_view_ = other.uri_;

  const auto this_parts = uri_parts_;
  uri_parts_.clear();
  detail::advance_parts(uri_view_, uri_parts_, other.uri_parts_);
  other.uri_parts_.clear();
  detail::advance_parts(other.uri_view_, other.uri_parts_, this_parts);
}

uri::const_iterator uri::begin() const noexcept { return uri_view_.begin(); }

uri::const_iterator uri::end() const noexcept { return uri_view_.end(); }

bool uri::has_scheme() const noexcept {
  return static_cast<bool>(uri_parts_.scheme);
}

uri::string_view uri::scheme() const noexcept {
  return has_scheme() ? to_string_view(uri_, *uri_parts_.scheme)
                      : string_view{};
}

bool uri::has_user_info() const noexcept {
  return static_cast<bool>(uri_parts_.hier_part.user_info);
}

uri::string_view uri::user_info() const noexcept {
  return has_user_info() ? to_string_view(uri_, *uri_parts_.hier_part.user_info)
                         : string_view{};
}

bool uri::has_host() const noexcept {
  return static_cast<bool>(uri_parts_.hier_part.host);
}

uri::string_view uri::host() const noexcept {
  return has_host() ? to_string_view(uri_, *uri_parts_.hier_part.host)
                    : string_view{};
}

bool uri::has_port() const noexcept {
  return static_cast<bool>(uri_parts_.hier_part.port);
}

uri::string_view uri::port() const noexcept {
  return has_port() ? to_string_view(uri_, *uri_parts_.hier_part.port)
                    : string_view{};
}

bool uri::has_path() const noexcept {
  return static_cast<bool>(uri_parts_.hier_part.path);
}

uri::string_view uri::path() const noexcept {
  return has_path() ? to_string_view(uri_, *uri_parts_.hier_part.path)
                    : string_view{};
}

bool uri::has_query() const noexcept {
  return static_cast<bool>(uri_parts_.query);
}

uri::string_view uri::query() const noexcept {
  return has_query() ? to_string_view(uri_, *uri_parts_.query) : string_view{};
}

uri::query_iterator::query_iterator() : query_{}, kvp_{} {}

uri::query_iterator::query_iterator(optional<detail::uri_part> query)
    : query_(query), kvp_{} {
  if (query_ && query_->empty()) {
    query_ = nullopt;
  } else {
    assign_kvp();
  }
}

uri::query_iterator::query_iterator(const query_iterator &other)
    : query_(other.query_), kvp_(other.kvp_) {}

uri::query_iterator &uri::query_iterator::operator=(
    const query_iterator &other) {
  auto tmp(other);
  swap(tmp);
  return *this;
}

uri::query_iterator::reference uri::query_iterator::operator++() noexcept {
  increment();
  return kvp_;
}

uri::query_iterator::value_type uri::query_iterator::operator++(int) noexcept {
  auto original = kvp_;
  increment();
  return original;
}

uri::query_iterator::reference uri::query_iterator::operator*() const noexcept {
  return kvp_;
}

uri::query_iterator::pointer uri::query_iterator::operator->() const noexcept {
  return std::addressof(kvp_);
}

bool uri::query_iterator::operator==(const query_iterator &other) const
    noexcept {
  if (!query_ && !other.query_) {
    return true;
  } else if (query_ && other.query_) {
    // since we're comparing substrings, the address of the first
    // element in each iterator must be the same
    return std::addressof(kvp_.first) == std::addressof(other.kvp_.first);
  }
  return false;
}

void uri::query_iterator::swap(query_iterator &other) noexcept {
  std::swap(query_, other.query_);
  std::swap(kvp_, other.kvp_);
}

void uri::query_iterator::advance_to_next_kvp() noexcept {
  auto first = std::begin(*query_), last = std::end(*query_);

  auto sep_it = std::find_if(
      first, last, [](char c) -> bool { return c == '&' || c == ';'; });

  if (sep_it != last) {
    ++sep_it;  // skip next separator
  }

  // reassign query to the next element
  query_ = detail::uri_part(sep_it, last);
}

void uri::query_iterator::assign_kvp() noexcept {
  auto first = std::begin(*query_), last = std::end(*query_);

  auto sep_it = std::find_if(
      first, last, [](char c) -> bool { return c == '&' || c == ';'; });
  auto eq_it =
      std::find_if(first, sep_it, [](char c) -> bool { return c == '='; });

  kvp_.first = string_view(std::addressof(*first), std::distance(first, eq_it));
  if (eq_it != sep_it) {
    ++eq_it;  // skip '=' symbol
  }
  kvp_.second =
      string_view(std::addressof(*eq_it), std::distance(eq_it, sep_it));
}

void uri::query_iterator::increment() noexcept {
  assert(query_);

  if (!query_->empty()) {
    advance_to_next_kvp();
    assign_kvp();
  }

  if (query_->empty()) {
    query_ = nullopt;
  }
}

uri::query_iterator uri::query_begin() const noexcept {
  return has_query() ? uri::query_iterator{uri_parts_.query}
                     : uri::query_iterator{};
}

uri::query_iterator uri::query_end() const noexcept {
  return uri::query_iterator{};
}

bool uri::has_fragment() const noexcept {
  return static_cast<bool>(uri_parts_.fragment);
}

uri::string_view uri::fragment() const noexcept {
  return has_fragment() ? to_string_view(uri_, *uri_parts_.fragment)
                        : string_view{};
}

bool uri::has_authority() const noexcept { return has_host(); }

uri::string_view uri::authority() const noexcept {
  if (!has_host()) {
    return string_view{};
  }

  auto host = this->host();

  auto user_info = string_view{};
  if (has_user_info()) {
    user_info = this->user_info();
  }

  auto port = string_view{};
  if (has_port()) {
    port = this->port();
  }

  auto first = std::begin(host), last = std::end(host);
  if (has_user_info() && !user_info.empty()) {
    first = std::begin(user_info);
  } else if (host.empty() && has_port() && !port.empty()) {
    first = std::begin(port);
    --first;  // include ':' before port
  }

  if (host.empty()) {
    if (has_port() && !port.empty()) {
      last = std::end(port);
    } else if (has_user_info() && !user_info.empty()) {
      last = std::end(user_info);
      ++last;  // include '@'
    }
  } else if (has_port()) {
    if (port.empty()) {
      ++last;  // include ':' after host
    } else {
      last = std::end(port);
    }
  }

  return string_view(first, std::distance(first, last));
}

std::string uri::string() const { return uri_; }

std::wstring uri::wstring() const {
  return std::wstring(std::begin(*this), std::end(*this));
}

std::u16string uri::u16string() const {
  return std::u16string(std::begin(*this), std::end(*this));
}

std::u32string uri::u32string() const {
  return std::u32string(std::begin(*this), std::end(*this));
}

uri::string_view uri::view() const noexcept { return uri_view_; }

bool uri::empty() const noexcept { return uri_.empty(); }

bool uri::is_absolute() const noexcept { return has_scheme(); }

bool uri::is_opaque() const noexcept {
  return (is_absolute() && !has_authority());
}

uri uri::normalize(uri_comparison_level level) const {
  string_type normalized(uri_);
  string_view normalized_view(normalized);
  detail::uri_parts parts;
  detail::advance_parts(normalized_view, parts, uri_parts_);

  if (uri_comparison_level::syntax_based == level) {
    // All alphabetic characters in the scheme and host are
    // lower-case...
    if (parts.scheme) {
      std::string::iterator first, last;
      std::tie(first, last) = mutable_part(normalized, *parts.scheme);
      std::transform(first, last, first,
                     [](char ch) { return std::tolower(ch, std::locale()); });
    }

    // if (parts.hier_part.host) {
    //   std::string::iterator first, last;
    //   std::tie(first, last) = mutable_part(normalized,
    //   *parts.hier_part.host); std::transform(first, last, first,
    //                  [](char ch) { return std::tolower(ch, std::locale());
    //                  });
    // }

    // ...except when used in percent encoding
    detail::for_each(normalized,
                     detail::percent_encoded_to_upper<std::string>());

    // parts are invalidated here
    // there's got to be a better way of doing this that doesn't
    // mean parsing again (twice!)
    normalized.erase(detail::decode_encoded_unreserved_chars(
                         std::begin(normalized), std::end(normalized)),
                     std::end(normalized));
    normalized_view = string_view(normalized);

    // need to parse the parts again as the underlying string has changed
    const_iterator it = std::begin(normalized_view),
                   last = std::end(normalized_view);
    bool is_valid = detail::parse(it, last, parts);
    ignore(is_valid);
    assert(is_valid);

    if (parts.hier_part.path) {
      uri::string_type path = detail::normalize_path_segments(
          to_string_view(normalized, *parts.hier_part.path));

      // put the normalized path back into the uri
      optional<string_type> query, fragment;
      if (parts.query) {
        query = parts.query->to_string();
      }

      if (parts.fragment) {
        fragment = parts.fragment->to_string();
      }

      auto path_begin = std::begin(normalized);
      auto path_range = mutable_part(normalized, *parts.hier_part.path);
      std::advance(path_begin, std::distance(path_begin, path_range.first));
      normalized.erase(path_begin, std::end(normalized));
      normalized.append(path);

      if (query) {
        normalized.append("?");
        normalized.append(*query);
      }

      if (fragment) {
        normalized.append("#");
        normalized.append(*fragment);
      }
    }
  }

  return uri(normalized);
}

uri uri::make_relative(const uri &other) const {
  if (is_opaque() || other.is_opaque()) {
    return other;
  }

  if ((!has_scheme() || !other.has_scheme()) ||
      !detail::equal(scheme(), other.scheme())) {
    return other;
  }

  if ((!has_authority() || !other.has_authority()) ||
      !detail::equal(authority(), other.authority())) {
    return other;
  }

  if (!has_path() || !other.has_path()) {
    return other;
  }

  auto path =
      detail::normalize_path(this->path(), uri_comparison_level::syntax_based);
  auto other_path =
      detail::normalize_path(other.path(), uri_comparison_level::syntax_based);

  optional<string_type> query, fragment;
  if (other.has_query()) {
    query = other.query().to_string();
  }

  if (other.has_fragment()) {
    fragment = other.fragment().to_string();
  }

  network::uri result;
  result.initialize(optional<string_type>(), optional<string_type>(),
                    optional<string_type>(), optional<string_type>(),
                    other_path, query, fragment);
  return result;
}

uri uri::resolve(const uri &base) const {
  // This implementation uses the psuedo-code given in
  // http://tools.ietf.org/html/rfc3986#section-5.2.2

  if (is_absolute() && !is_opaque()) {
    // throw an exception ?
    return *this;
  }

  if (is_opaque()) {
    // throw an exception ?
    return *this;
  }

  optional<uri::string_type> user_info, host, port, path, query, fragment;

  if (has_authority()) {
    // g -> http://g
    if (has_user_info()) {
      user_info = make_arg(this->user_info());
    }

    if (has_host()) {
      host = make_arg(this->host());
    }

    if (has_port()) {
      port = make_arg(this->port());
    }

    if (has_path()) {
      path = detail::remove_dot_segments(this->path());
    }

    if (has_query()) {
      query = make_arg(this->query());
    }
  } else {
    if (!has_path() || this->path().empty()) {
      if (base.has_path()) {
        path = make_arg(base.path());
      }

      if (has_query()) {
        query = make_arg(this->query());
      } else if (base.has_query()) {
        query = make_arg(base.query());
      }
    } else {
      if (this->path().front() == '/') {
        path = detail::remove_dot_segments(this->path());
      } else {
        path = detail::merge_paths(base, *this);
      }

      if (has_query()) {
        query = make_arg(this->query());
      }
    }

    if (base.has_user_info()) {
      user_info = make_arg(base.user_info());
    }

    if (base.has_host()) {
      host = make_arg(base.host());
    }

    if (base.has_port()) {
      port = make_arg(base.port());
    }
  }

  if (has_fragment()) {
    fragment = make_arg(this->fragment());
  }

  network::uri result;
  result.initialize(make_arg(base.scheme()), std::move(user_info),
                    std::move(host), std::move(port), std::move(path),
                    std::move(query), std::move(fragment));
  return result;
}

int uri::compare(const uri &other, uri_comparison_level level) const noexcept {
  // if both URIs are empty, then we should define them as equal
  // even though they're still invalid.
  if (empty() && other.empty()) {
    return 0;
  }

  if (empty()) {
    return -1;
  }

  if (other.empty()) {
    return 1;
  }

  return normalize(level).uri_.compare(other.normalize(level).uri_);
}

bool uri::initialize(const string_type &uri) {
  uri_ = detail::trim_copy(uri);
  if (!uri_.empty()) {
    uri_view_ = string_view(uri_);
    const_iterator it = std::begin(uri_view_), last = std::end(uri_view_);
    bool is_valid = detail::parse(it, last, uri_parts_);
    return is_valid;
  }
  return true;
}

void swap(uri &lhs, uri &rhs) noexcept { lhs.swap(rhs); }

bool operator==(const uri &lhs, const uri &rhs) noexcept {
  return lhs.view() == rhs.view();
}

bool operator==(const uri &lhs, const char *rhs) noexcept {
  return lhs.view() == string_view{rhs};
}

bool operator<(const uri &lhs, const uri &rhs) noexcept {
  return lhs.view() < rhs.view();
}

}  // namespace network
