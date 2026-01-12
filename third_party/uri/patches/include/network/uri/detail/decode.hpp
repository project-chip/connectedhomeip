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

#ifndef NETWORK_URI_DECODE_INC
#define NETWORK_URI_DECODE_INC

#include <cassert>
#include <cstdlib>
#include <iterator>
#include <network/uri/uri_errors.hpp>

namespace network {
namespace detail {
template <typename CharT>
CharT letter_to_hex(CharT in)
{
    if ((in >= '0') && (in <= '9'))
    {
        return in - '0';
    }

    if ((in >= 'a') && (in <= 'f'))
    {
        return in + 10 - 'a';
    }

    if ((in >= 'A') && (in <= 'F'))
    {
        return in + 10 - 'A';
    }

    abort();
    return CharT(); // Never reached, but needed for return type
}

template <class InputIterator, class charT>
InputIterator decode_char(InputIterator it, charT * out)
{
    assert(*it == '%');
    ++it;
    auto h0 = *it;
    auto v0 = detail::letter_to_hex(h0);
    ++it;
    auto h1 = *it;
    auto v1 = detail::letter_to_hex(h1);
    ++it;
    *out = static_cast<charT>((0x10 * v0) + v1);
    return it;
}

template <class InputIterator, class OutputIterator>
OutputIterator decode(InputIterator in_begin, InputIterator in_end, OutputIterator out_begin)
{
    auto it  = in_begin;
    auto out = out_begin;
    while (it != in_end)
    {
        if (*it == '%')
        {
            if (std::distance(it, in_end) < 3)
            {
                abort();
            }
            char c = '\0';
            it     = decode_char(it, &c);
            *out   = c;
            ++out;
        }
        else
        {
            *out++ = *it++;
        }
    }
    return out;
}

template <class String>
String decode(const String & source)
{
    String unencoded;
    decode(std::begin(source), std::end(source), std::back_inserter(unencoded));
    return unencoded;
}
} // namespace detail
} // namespace network

#endif // NETWORK_URI_DECODE_INC
