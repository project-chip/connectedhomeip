/*
 *    Copyright (c) 2021 Project CHIP Authors
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
 */

#pragma once

#include <lib/support/Span.h>

#include <array>
#include <type_traits>

namespace chip {

template <typename Type, class Enable = void>
class TypeSerializer;

// specialization for arithmetic types
template <typename T>
class TypeSerializer<T, typename std::enable_if<std::is_arithmetic<T>::value>::type>
{
public:
    static constexpr const size_t Space = sizeof(T);

    static T LoadValue(FixedSpan<const char, Space> bytes)
    {
        // The span can pointing to arbitrary memory, probably not aligned, so casting data() to pointer may cause bus
        // error on platform which requires aligned memory read/write. Use memcpy instead
        T result;
        ::memcpy(&result, bytes.data(), Space);
        return result;
    }

    static void SaveValue(FixedSpan<char, Space> bytes, T data)
    {
        // The span can pointing to arbitrary memory, probably not aligned, so casting data() to pointer may cause bus
        // error on platform which requires aligned memory read/write. Use memcpy instead
        ::memcpy(bytes.data(), &data, Space);
    }
};

// Specialization for enum
template <typename T>
class TypeSerializer<T, typename std::enable_if<std::is_enum<T>::value>::type>
{
private:
    using UnderlyingType       = typename std::underlying_type<T>::type;
    using UnderlyingSerializer = TypeSerializer<UnderlyingType>;

public:
    static constexpr const size_t Space = UnderlyingSerializer::Space;

    static T LoadValue(FixedSpan<const char, Space> bytes) { return static_cast<T>(UnderlyingSerializer::LoadValue(bytes)); }

    static void SaveValue(FixedSpan<char, Space> bytes, T data)
    {
        UnderlyingSerializer::SaveValue(bytes, static_cast<UnderlyingType>(data));
    }
};

// specialization for std::array
template <typename Type, size_t N>
class TypeSerializer<std::array<Type, N>>
{
private:
    using SubTypeSerializer = TypeSerializer<Type>;

public:
    static constexpr const size_t Space = N * SubTypeSerializer::Space;

    static std::array<Type, N> LoadValue(FixedSpan<const char, Space> bytes)
    {
        std::array<Type, N> result;
        for (size_t i = 0; i < N; ++i)
        {
            result[i] =
                SubTypeSerializer::LoadValue(bytes.template subspan<SubTypeSerializer::Space>(i * SubTypeSerializer::Space));
        }
        return result;
    }

    static void SaveValue(FixedSpan<char, Space> bytes, const std::array<Type, N> & data)
    {
        for (size_t i = 0; i < N; ++i)
        {
            SubTypeSerializer::SaveValue(bytes.template subspan<SubTypeSerializer::Space>(i * SubTypeSerializer::Space), data[i]);
        }
    }
};

} // namespace chip
