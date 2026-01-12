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

#ifndef NETWORK_URI_INC
#define NETWORK_URI_INC

/**
 * \file
 * \brief Contains the uri class.
 */

#include <iterator>
#include <algorithm>
#include <functional>
#include <memory>
#include <string>
#include <cstdlib>  // For abort() - patched for -fno-exceptions
#include <network/string_view.hpp>
#include <network/optional.hpp>
#include <network/uri/config.hpp>
#include <network/uri/uri_errors.hpp>
#include <network/uri/detail/uri_parts.hpp>
#include <network/uri/detail/encode.hpp>
#include <network/uri/detail/decode.hpp>
#include <network/uri/detail/translate.hpp>

#ifdef NETWORK_URI_MSVC
#pragma warning(push)
#pragma warning(disable : 4251 4231 4660)
#endif

namespace network {
/**
 * \enum uri_comparison_level
 * \brief Defines the steps on the URI comparison ladder.
 */
enum class uri_comparison_level {
  string_comparison,
  syntax_based,
  scheme_based,
};

/**
 * \ingroup uri
 * \class uri_builder network/uri/uri_builder.hpp network/uri.hpp
 * \brief A class that allows complex uri objects to be constructed.
 * \sa uri
 */
class uri_builder;

/**
 * \ingroup uri
 * \class uri network/uri/uri.hpp network/uri.hpp
 * \brief A class that parses a URI (Uniform Resource Identifier)
 *        into its component parts.
 *
 * This class conforms to a URI as defined by RFC 3986, RFC 3987 and
 * RFC 2732, including scoped IDs. It provides member functions for
 * normalizing, comparing and resolving URIs.
 *
 * A URI has the syntax:
 *
 * \code
 * [scheme:][user_info@][host][:port][path][?query][#fragment]
 * \endcode
 *
 * Example:
 *
 * \code
 * network::uri instance("http://cpp-netlib.org/");
 * assert(instance.is_absolute());
 * assert(!instance.is_opaque());
 * assert(instance.scheme());
 * assert("http" == *instance.scheme());
 * assert(instance.host());
 * assert("cpp-netlib.org" == *instance.host());
 * assert(instance.path());
 * assert("/" == *instance.path());
 * \endcode
 */
class uri {
#if !defined(DOXYGEN_SHOULD_SKIP_THIS)
  friend class uri_builder;
#endif  // !defined(DOXYGEN_SHOULD_SKIP_THIS)

 public:
  /**
   * \brief The URI string_type.
   */
  typedef std::string string_type;

  /**
   * \brief A reference to the underlying string_type parts.
   */
  typedef network::string_view string_view;

  /**
   * \brief The char traits.
   */
  typedef string_view::traits_type traits_type;

  /**
   * \brief The URI const_iterator type.
   */
  typedef string_view::const_iterator const_iterator;

  /**
   * \brief The URI iterator type.
   */
  typedef const_iterator iterator;

  /**
   * \brief The URI value_type.
   */
  typedef std::iterator_traits<iterator>::value_type value_type;

  /**
   *
   */
  class query_iterator {
   public:
    using value_type = std::pair<string_view, string_view>;
    using difference_type = std::ptrdiff_t;
    using pointer = const value_type *;
    using reference = const value_type &;
    using iterator_category = std::forward_iterator_tag;

    query_iterator();
    explicit query_iterator(optional<detail::uri_part>);
    query_iterator(const query_iterator &);
    query_iterator &operator=(const query_iterator &);
    reference operator++() noexcept;
    value_type operator++(int) noexcept;
    reference operator*() const noexcept;
    pointer operator->() const noexcept;
    bool operator==(const query_iterator &) const noexcept;
    inline bool operator!=(const query_iterator &other) const noexcept {
      return !(*this == other);
    }

   private:
    void swap(query_iterator &) noexcept;
    void advance_to_next_kvp() noexcept;
    void assign_kvp() noexcept;
    void increment() noexcept;

    optional<detail::uri_part> query_;
    value_type kvp_;
  };

  /**
   * \brief Default constructor.
   */
  uri();

  /**
   * \brief Constructor.
   * \param first The first element in a string sequence.
   * \param last The end + 1th element in a string sequence.
   * \throws uri_syntax_error if the sequence is not a valid URI.
   * \throws std::bad_alloc If the underlying string cannot be allocated.
   */
  template <class InputIter>
  uri(InputIter first, InputIter last) {
    if (!initialize(string_type(first, last))) {
      abort();  // Patched: replaced throw uri_syntax_error() with abort() for -fno-exceptions
    }
  }

#if !defined(DOXYGEN_SHOULD_SKIP_THIS)
  template <class InputIter>
  explicit uri(InputIter first, InputIter last, std::error_code &ec) {
    if (!initialize(string_type(first, last))) {
      ec = make_error_code(uri_error::invalid_syntax);
    }
  }
#endif  // !defined(DOXYGEN_SHOULD_SKIP_THIS)

  /**
   * \brief Constructor.
   * \param source A source string that is to be parsed as a URI.
   * \throws uri_syntax_error if the source is not a valid URI.
   * \throws std::bad_alloc If the underlying string cannot be allocated.
   */
  template <class Source>
  explicit uri(const Source &source) {
    if (!initialize(detail::translate(source))) {
      abort();  // Patched: replaced throw uri_syntax_error() with abort() for -fno-exceptions
    }
  }

#if !defined(DOXYGEN_SHOULD_SKIP_THIS)
  template <class Source>
  explicit uri(const Source &source, std::error_code &ec) {
    if (!initialize(detail::translate(source))) {
      ec = make_error_code(uri_error::invalid_syntax);
    }
  }
#endif  // !defined(DOXYGEN_SHOULD_SKIP_THIS)

#if !defined(DOXYGEN_SHOULD_SKIP_THIS)
  explicit uri(const uri_builder &builder);
#endif  // !defined(DOXYGEN_SHOULD_SKIP_THIS)

  /**
   * \brief Copy constructor.
   * \throws std::bad_alloc If the underlying string cannot be allocated.
   */
  uri(const uri &other);

  /**
   * \brief Move constructor.
   */
  uri(uri &&other) noexcept;

  /**
   * \brief Destructor.
   */
  ~uri();

  /**
   * \brief Assignment operator.
   * \throws std::bad_alloc If the underlying string cannot be allocated.
   */
  uri &operator=(uri other);

  /**
   * \brief Swaps one uri object with another.
   * \param other The other uri object.
   */
  void swap(uri &other) noexcept;

  /**
   * \brief Returns an iterator at the first element in the
   *        underlying sequence.
   * \return An iterator starting at the first element.
   */
  const_iterator begin() const noexcept;

  /**
   * \brief Returns an iterator at the end + 1th element in the
   *        underlying sequence.
   * \return An iterator starting at the end + 1th element.
   */
  const_iterator end() const noexcept;

  /**
   * \brief Tests whether this URI has a scheme component.
   * \return \c true if the URI has a scheme, \c false otherwise.
   */
  bool has_scheme() const noexcept;

  /**
   * \brief Returns the URI scheme.
   * \return The scheme.
   * \pre has_scheme()
   */
  string_view scheme() const noexcept;

  /**
   * \brief Tests whether this URI has a user info component.
   * \return \c true if the URI has a user info, \c false otherwise.
   */
  bool has_user_info() const noexcept;

  /**
   * \brief Returns the URI user info.
   * \return The user info.
   * \pre has_user_info()
   */
  string_view user_info() const noexcept;

  /**
   * \brief Tests whether this URI has a host component.
   * \return \c true if the URI has a host, \c false otherwise.
   */
  bool has_host() const noexcept;

  /**
   * \brief Returns the URI host.
   * \return The host.
   * \pre has_host()
   */
  string_view host() const noexcept;

  /**
   * \brief Tests whether this URI has a port component.
   * \return \c true if the URI has a port, \c false otherwise.
   */
  bool has_port() const noexcept;

  /**
   * \brief Returns the URI port.
   * \return The port.
   * \pre has_port()
   */
  string_view port() const noexcept;

  /**
   * \brief Returns the URI port as an integer.
   * \return The port number.
   * \pre has_port()
   */
  template <typename intT>
  intT port(typename std::is_integral<intT>::type * = 0) const {
    assert(has_port());
    auto p = port();
    const char *port_first = std::addressof(*p.begin());
    char *port_last = 0;
    return static_cast<intT>(std::strtoul(port_first, &port_last, 10));
  }

  /**
   * \brief Tests whether this URI has a path component.
   * \return \c true if the URI has a path, \c false otherwise.
   */
  bool has_path() const noexcept;

  /**
   * \brief Returns the URI path.
   * \return The path.
   * \pre has_path()
   */
  string_view path() const noexcept;

  /**
   * \brief Tests whether this URI has a query component.
   * \return \c true if the URI has a query, \c false otherwise.
   */
  bool has_query() const noexcept;

  /**
   * \brief Returns the URI query.
   * \return The query.
   * \pre has_query()
   */
  string_view query() const noexcept;

  /**
   * \brief Returns an iterator to the first key-value pair in the query
   *        component.
   *
   * \return query_iterator.
   */
  query_iterator query_begin() const noexcept;

  /**
   * \brief Returns an iterator to the last key-value pair in the query
   *        component.
   *
   * \return query_iterator.
   */
  query_iterator query_end() const noexcept;

  /**
   * \brief Tests whether this URI has a fragment component.
   * \return \c true if the URI has a fragment, \c false otherwise.
   */
  bool has_fragment() const noexcept;

  /**
   * \brief Returns the URI fragment.
   * \return The fragment.
   * \pre has_fragment()
   */
  string_view fragment() const noexcept;

  /**
   * \brief Tests whether this URI has a valid authority.
   * \return \c true if the URI has an authority, \c false otherwise.
   */
  bool has_authority() const noexcept;

  /**
   * \brief Returns the URI authority.
   * \return The authority.
   */
  string_view authority() const noexcept;

  /**
   * \brief Returns the URI as a std::basic_string object.
   * \return A URI string.
   */
  template <class charT, class traits = std::char_traits<charT>,
            class Allocator = std::allocator<charT> >
  std::basic_string<charT, traits, Allocator> to_string(
      const Allocator &alloc = Allocator()) const {
    return std::basic_string<charT, traits, Allocator>(begin(), end());
  }

  /**
   * \brief Returns the URI as a std::string object.
   * \returns A URI string.
   */
  std::string string() const;

  /**
   * \brief Returns the URI as a std::wstring object.
   * \returns A URI string.
   */
  std::wstring wstring() const;

  /**
   * \brief Returns the URI as a std::u16string object.
   * \returns A URI string.
   */
  std::u16string u16string() const;

  /**
   * \brief Returns the URI as a std::u32string object.
   * \returns A URI string.
   */
  std::u32string u32string() const;

  /**
   * \brief Returns the URI as a string_view object.
   * \returns A URI string view.
   */
  string_view view() const noexcept;

  /**
   * \brief Checks if the uri object is empty, i.e. it has no parts.
   * \returns \c true if there are no parts, \c false otherwise.
   */
  bool empty() const noexcept;

  /**
   * \brief Checks if the uri is absolute, i.e. it has a scheme.
   * \returns \c true if it is absolute, \c false if it is relative.
   */
  bool is_absolute() const noexcept;

  /**
   * \brief Checks if the uri is opaque, i.e. if it doesn't have an
   *        authority.
   * \returns \c true if it is opaque, \c false if it is hierarchical.
   */
  bool is_opaque() const noexcept;

  /**
   * \brief Normalizes a uri object at a given level in the
   *        comparison ladder.
   * \param level The comparison level.
   * \returns A normalized uri.
   * \post compare(normalize(uri, level), level) == 0
   * \throws std::bad_alloc
   */
  uri normalize(uri_comparison_level level) const;

  /**
   * \brief Returns a relative reference against the base URI.
   * \param base The base URI.
   * \returns A relative reference of this URI against the base.
   * \throws std::bad_alloc
   */
  uri make_relative(const uri &base) const;

  /**
   * \brief Resolves a relative reference against the given URI.
   * \param base The base URI to resolve against.
   * \returns An absolute URI.
   * \throws std::bad_alloc
   */
  uri resolve(const uri &base) const;

  /**
   * \brief Compares this URI against another, corresponding to the
   *        level in the comparison ladder.
   * \param other The other URI.
   * \param level The level in the comparison ladder.
   * \returns \c 0 if the URIs are considered equal, \c -1 if this is
   *         less than other and and 1 if this is greater than
   *         other.
   */
  int compare(const uri &other, uri_comparison_level level) const noexcept;

  /**
   * \brief Encodes a sequence according to the rules for encoding a
   *        user info part.
   * \param first The iterator at first element in the input
   *        sequence.
   * \param last The iterator at end + 1th element in the input
   *        sequence.
   * \param out The iterator at the first element in the output
   *        sequence.
   * \returns The iterator at the end + 1th in the output sequence.
   */
  template <typename InputIter, typename OutputIter>
  static OutputIter encode_user_info(InputIter first, InputIter last,
                                     OutputIter out) {
    return detail::encode_user_info(first, last, out);
  }

  /**
   * \brief Encodes a sequence according to the rules for encoding a
   *        host part.
   * \param first The iterator at first element in the input
   *        sequence.
   * \param last The iterator at end + 1th element in the input
   *        sequence.
   * \param out The iterator at the first element in the output
   *        sequence.
   * \returns The iterator at the end + 1th in the output sequence.
   */
  template <typename InputIter, typename OutputIter>
  static OutputIter encode_host(InputIter first, InputIter last,
                                OutputIter out) {
    return detail::encode_host(first, last, out);
  }

  /**
   * \brief Encodes a sequence according to the rules for encoding a
   *        port part.
   * \param first The iterator at first element in the input
   *        sequence.
   * \param last The iterator at end + 1th element in the input
   *        sequence.
   * \param out The iterator at the first element in the output
   *        sequence.
   * \returns The iterator at the end + 1th in the output sequence.
   */
  template <typename InputIter, typename OutputIter>
  static OutputIter encode_port(InputIter first, InputIter last,
                                OutputIter out) {
    return detail::encode_port(first, last, out);
  }

  /**
   * \brief Encodes a sequence according to the rules for encoding a
   *        path part.
   * \param first The iterator at first element in the input
   *        sequence.
   * \param last The iterator at end + 1th element in the input
   *        sequence.
   * \param out The iterator at the first element in the output
   *        sequence.
   * \returns The iterator at the end + 1th in the output sequence.
   */
  template <typename InputIter, typename OutputIter>
  static OutputIter encode_path(InputIter first, InputIter last,
                                OutputIter out) {
    return detail::encode_path(first, last, out);
  }

  /**
   * \deprecated Avoid using this function
   * \brief Equivalent to \c encode_query_component
   * \param first The iterator at first element in the input
   *        sequence.
   * \param last The iterator at end + 1th element in the input
   *        sequence.
   * \param out The iterator at the first element in the output
   *        sequence.
   * \returns The iterator at the end + 1th in the output sequence.
   * \sa encode_query_commponent
   * \sa encode_query_key_value_pair
   */
  template <typename InputIter, typename OutputIter>
  static OutputIter encode_query(InputIter first, InputIter last,
                                 OutputIter out) {
    return encode_query_component(first, last, out);
  }

  /**
   * \brief Encodes a sequence according to the rules for encoding a
   *        query component, including the '=' character.
   * \param first The iterator at first element in the input
   *              sequence.
   * \param last The iterator at end + 1th element in the input
   *             sequence.
   * \param out The iterator at the first element in the output
   *            sequence.
   * \returns The iterator at the end + 1th in the output sequence.
   */
  template <typename InputIter, typename OutputIter>
  static OutputIter encode_query_component(InputIter first, InputIter last,
                                           OutputIter out) {
    return detail::encode_query_component(first, last, out);
  }

  /**
   * \brief Encodes a sequence according to the rules for encoding a
   *        query key value pair.
   * \param key_first The iterator at first element in the input
   *                   sequence.
   * \param key_last The iterator at end + 1th element in the input
   *                  sequence.
   * \param out The iterator at the first element in the output
   *            sequence.
   * \returns The iterator at the end + 1th in the output sequence.
   */
  template <typename InputIter, typename OutputIter>
  static OutputIter encode_query_key_value_pair(InputIter key_first,
                                                InputIter key_last,
                                                InputIter value_first,
                                                InputIter value_last,
                                                OutputIter out) {
    out = detail::encode_query_component(key_first, key_last, out);
    out++ = '=';
    return detail::encode_query_component(value_first, value_last, out);
  }

  /**
   * \brief Encodes a sequence according to the rules for encoding a
   *        fragment part.
   * \param first The iterator at first element in the input
   *        sequence.
   * \param last The iterator at end + 1th element in the input
   *        sequence.
   * \param out The iterator at the first element in the output
   *        sequence.
   * \returns The iterator at the end + 1th in the output sequence.
   */
  template <typename InputIter, typename OutputIter>
  static OutputIter encode_fragment(InputIter first, InputIter last,
                                    OutputIter out) {
    return detail::encode_fragment(first, last, out);
  }

  /**
   * \brief Decodes a sequence according to the percent decoding
   *        rules.
   * \param first The iterator at first element in the input
   *        sequence.
   * \param last The iterator at end + 1th element in the input
   *        sequence.
   * \param out The iterator at the first element in the output
   *        sequence.
   * \returns The iterator at the end + 1th in the output sequence.
   */
  template <typename InputIter, typename OutputIter>
  static OutputIter decode(InputIter first, InputIter last, OutputIter out) {
    return detail::decode(first, last, out);
  }

 private:
  bool initialize(const string_type &uri);

  void initialize(optional<string_type> scheme, optional<string_type> user_info,
                  optional<string_type> host, optional<string_type> port,
                  optional<string_type> path, optional<string_type> query,
                  optional<string_type> fragment);

  string_type uri_;
  string_view uri_view_;
  detail::uri_parts uri_parts_;
};

/**
 * \brief \c uri factory function.
 * \param first The first element in a string sequence.
 * \param last The end + 1th element in a string sequence.
 * \param ec Error code set if the sequence is not a valid URI.
 */
template <class InputIter>
inline uri make_uri(InputIter first, InputIter last, std::error_code &ec) {
  return uri(first, last, ec);
}

/**
 * \brief \c uri factory function.
 * \param source A source string that is to be parsed as a URI.
 * \param ec Error code set if the source is not a valid URI.
 */
template <class Source>
inline uri make_uri(const Source &source, std::error_code &ec) {
  return uri(source, ec);
}

/**
 * \brief Swaps one uri object with another.
 */
void swap(uri &lhs, uri &rhs) noexcept;

/**
 * \brief Equality operator for the \c uri.
 */
bool operator==(const uri &lhs, const uri &rhs) noexcept;

/**
 * \brief Equality operator for the \c uri.
 */
bool operator==(const uri &lhs, const char *rhs) noexcept;

/**
 * \brief Equality operator for the \c uri.
 */
inline bool operator==(const char *lhs, const uri &rhs) noexcept {
  return rhs == lhs;
}

/**
 * \brief Inequality operator for the \c uri.
 */
inline bool operator!=(const uri &lhs, const uri &rhs) noexcept {
  return !(lhs == rhs);
}

/**
 * \brief Less-than operator for the \c uri.
 */
bool operator<(const uri &lhs, const uri &rhs) noexcept;

/**
 * \brief Greater-than operator for the \c uri.
 */
inline bool operator>(const uri &lhs, const uri &rhs) noexcept {
  return rhs < lhs;
}

/**
 * \brief Less-than-or-equal-to operator for the \c uri.
 */
inline bool operator<=(const uri &lhs, const uri &rhs) noexcept {
  return !(rhs < lhs);
}

/**
 * \brief Greater-than-or-equal-to operator for the \c uri.
 */
inline bool operator>=(const uri &lhs, const uri &rhs) noexcept {
  return !(lhs < rhs);
}
}  // namespace network

#if !defined(DOXYGEN_SHOULD_SKIP_THIS)
namespace std {
template <>
struct is_error_code_enum<network::uri_error> : public true_type {};
}  // namespace std

namespace std {
template <>
struct hash<network::uri> {
  std::size_t operator()(const network::uri &uri_) const {
    std::size_t seed = 0;
    std::for_each(std::begin(uri_), std::end(uri_),
                  [&seed](network::uri::value_type v) {
                    std::hash<network::uri::value_type> hasher;
                    seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
                  });
    return seed;
  }
};
}  // namespace std
#endif  // !defined(DOXYGEN_SHOULD_SKIP_THIS)

#ifdef NETWORK_URI_MSVC
#pragma warning(pop)
#endif

#include <network/uri/uri_builder.hpp>

#endif  // NETWORK_URI_INC
