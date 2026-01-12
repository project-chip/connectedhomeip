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

#ifndef NETWORK_STRING_VIEW_INC
#define NETWORK_STRING_VIEW_INC

/**
 * \file
 * \brief Contains an implementation of C++17 string_view (n3921).
 */

#include <algorithm>
#include <cassert>
#include <cstdlib> // For abort() - patched for -fno-exceptions
#include <iterator>
#include <stdexcept>
#include <string>

namespace network {
/**
 * \class basic_string_view string_view.hpp network/uri/string_view.hpp
 * \brief An implementation of C++17 string_view (n3921)
 */
template <class charT, class traits = std::char_traits<charT>>
class basic_string_view
{
public:
    typedef traits traits_type;
    typedef charT value_type;
    typedef charT * pointer;
    typedef const charT * const_pointer;
    typedef charT & reference;
    typedef const charT & const_reference;
    typedef const charT * const_iterator;
    typedef const_iterator iterator;
    typedef std::reverse_iterator<const_iterator> const_reverse_iterator;
    typedef const_reverse_iterator reverse_iterator;
    typedef std::size_t size_type;
    typedef std::ptrdiff_t difference_type;
    static constexpr size_type npos = size_type(-1);

    /**
     * \brief Constructor.
     */
    constexpr basic_string_view() noexcept : data_(nullptr), size_(0) {}

    /**
     * \brief Copy constructor.
     */
    constexpr basic_string_view(const basic_string_view &) noexcept = default;

    /**
     * \brief Assignment operator.
     */
    basic_string_view & operator=(const basic_string_view &) noexcept = default;

    /**
     * \brief Constructor.
     */
    template <class Allocator>
    basic_string_view(const std::basic_string<charT, traits, Allocator> & str) noexcept : data_(str.data()), size_(str.size())
    {}

    /**
     * \brief Constructor.
     */
    constexpr basic_string_view(const charT * str) : data_(str), size_(traits::length(str)) {}

    /**
     * \brief Constructor.
     */
    constexpr basic_string_view(const charT * str, size_type len) : data_(str), size_(len) {}

    constexpr const_iterator begin() const noexcept { return data_; }

    constexpr const_iterator end() const noexcept { return data_ + size_; }

    constexpr const_iterator cbegin() const noexcept { return begin(); }

    constexpr const_iterator cend() const noexcept { return end(); }

    const_reverse_iterator rbegin() const noexcept { return const_reverse_iterator(end()); }
    const_reverse_iterator rend() const noexcept { return const_reverse_iterator(begin()); }
    const_reverse_iterator crbegin() const noexcept { return rbegin(); }

    const_reverse_iterator crend() const noexcept { return rend(); }

    constexpr size_type size() const noexcept { return size_; }

    constexpr size_type length() const noexcept { return size_; }

    constexpr size_type max_size() const noexcept { return size_; }

    constexpr bool empty() const noexcept { return size_ == 0; }

    constexpr const_reference operator[](size_type pos) const { return data_[pos]; }

    const_reference at(size_type pos) const
    {
        if (pos >= size_)
        {
            abort(); // Patched: replaced throw with abort() for -fno-exceptions
        }
        return data_[pos];
    }

    const_reference front() const { return *begin(); }

    const_reference back() const
    {
        auto last = (end()) - 1;
        return *last;
    }

    constexpr const_pointer data() const noexcept { return data_; }

    void clear() noexcept
    {
        data_ = nullptr;
        size_ = 0;
    }

    void remove_prefix(size_type n)
    {
        data_ += n;
        size_ -= n;
    }

    void remove_suffix(size_type n) { size_ -= n; }

    void swap(basic_string_view & s) noexcept
    {
        std::swap(data_, s.data_);
        std::swap(size_, s.size_);
    }

    template <class Allocator>
    explicit operator std::basic_string<charT, traits, Allocator>() const
    {
        return to_string<charT, traits, Allocator>();
    }

    template <class Allocator = std::allocator<charT>>
    std::basic_string<charT, traits, Allocator> to_string(const Allocator & a = Allocator()) const
    {
        return std::basic_string<charT, traits, Allocator>(begin(), end(), a);
    }

    size_type copy(charT * s, size_type n, size_type pos = 0) const
    {
        size_type rlen = std::min(n, size() - pos);
        std::copy_n(begin() + pos, rlen, s);
        return rlen;
    }

    constexpr basic_string_view substr(size_type pos = 0, size_type n = npos) const
    {
        return basic_string_view(data() + pos, std::min(n, size() - pos));
    }

    constexpr int compare(basic_string_view s) const noexcept
    {
        return size() == s.size() ? traits::compare(data(), s.data(), size())
                                  : (size() < s.size() ? (traits::compare(data(), s.data(), size()) > 0 ? 1 : -1)
                                                       : (traits::compare(data(), s.data(), size()) < 0 ? -1 : 1));
    }

    constexpr int compare(size_type pos1, size_type n1, basic_string_view s) const { return substr(pos1, n1).compare(s); }

    constexpr int compare(size_type pos1, size_type n1, basic_string_view s, size_type pos2, size_type n2) const
    {
        return substr(pos1, n1).compare(s.substr(pos2, n2));
    }

    constexpr int compare(const charT * s) const { return compare(basic_string_view(s)); }

    constexpr int compare(size_type pos1, size_type n1, const charT * s) const
    {
        return substr(pos1, n1).compare(basic_string_view(s));
    }

    constexpr int compare(size_type pos1, size_type n1, const charT * s, size_type n2) const
    {
        return substr(pos1, n1).compare(basic_string_view(s, n2));
    }

private:
    const_pointer data_;
    size_type size_;
};

/**
 * \brief Equality operator.
 * \returns <tt>lhs.compare(rhs) == 0</tt>.
 */
template <class charT, class traits>
constexpr bool operator==(basic_string_view<charT, traits> lhs, basic_string_view<charT, traits> rhs) noexcept
{
    return lhs.compare(rhs) == 0;
}

/**
 * \brief Inequality operator.
 * \returns <tt>!(lhs == rhs)</tt>.
 */
template <class charT, class traits>
constexpr bool operator!=(basic_string_view<charT, traits> lhs, basic_string_view<charT, traits> rhs) noexcept
{
    return !(lhs == rhs);
}

/**
 * \brief Comparison operator.
 * \returns <tt>lhs.compare(rhs) < 0</tt>.
 */
template <class charT, class traits>
constexpr bool operator<(basic_string_view<charT, traits> lhs, basic_string_view<charT, traits> rhs) noexcept
{
    return lhs.compare(rhs) < 0;
}

/**
 * \brief Comparison operator.
 * \returns <tt>rhs < lhs</tt>.
 */
template <class charT, class traits>
constexpr bool operator>(basic_string_view<charT, traits> lhs, basic_string_view<charT, traits> rhs) noexcept
{
    return rhs < lhs;
}

/**
 * \brief Comparison operator.
 * \returns <tt>!(lhs > rhs)</tt>.
 */
template <class charT, class traits>
constexpr bool operator<=(basic_string_view<charT, traits> lhs, basic_string_view<charT, traits> rhs) noexcept
{
    return !(lhs > rhs);
}

/**
 * \brief Comparison operator.
 * \returns <tt>!(lhs < rhs)</tt>.
 */
template <class charT, class traits>
constexpr bool operator>=(basic_string_view<charT, traits> lhs, basic_string_view<charT, traits> rhs) noexcept
{
    return !(lhs < rhs);
}

/**
 * \brief Output stream operator.
 */
template <class charT, class traits>
std::basic_ostream<charT, traits> & operator<<(std::basic_ostream<charT, traits> & os, basic_string_view<charT, traits> str)
{
    return os << str.to_string();
}

typedef basic_string_view<char> string_view;
} // namespace network

#endif // NETWORK_STRING_VIEW_INC
