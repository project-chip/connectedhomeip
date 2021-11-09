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
#include <lib/support/TypeSerializer.h>

#include <initializer_list>
#include <numeric>

namespace chip {

/*
 * @brief
 *   Serialize a given field of given type in target class.
 *
 *   @tparam Target the target class.
 *   @tparam FieldType the type of the field.
 *   @tparam Field the field to be serialized.
 */
template <typename Target, typename FieldType, FieldType Target::*Field>
class FieldSerializer
{
public:
    static constexpr const size_t Space = TypeSerializer<FieldType>::Space;

    /// Load the field in the target from the given FixedSpan.
    static void LoadField(Target & target, FixedSpan<const char, Space> bytes)
    {
        target.*Field = TypeSerializer<FieldType>::LoadValue(bytes);
    }

    /// Save the field value in the target into the given FixedSpan.
    static void SaveField(const Target & target, FixedSpan<char, Space> bytes)
    {
        TypeSerializer<FieldType>::SaveValue(bytes, target.*Field);
    }
};

/*
 * @brief
 *   Serialize a given class
 *
 *   @tparam Target the target class.
 *   @tparam FieldSerializer the list of fields to be serialized.
 */
template <typename Target, typename... FieldSerializer>
class Serializer
{
private:
    static constexpr size_t AccumulateSpace(std::initializer_list<size_t> spaces)
    {
        size_t res = 0;
        for (auto i : spaces)
            res += i;
        return res;
    }

public:
    // Only if we have C++17
    // static constexpr const size_t Space = (FieldSerializer::Space + ...);
    static constexpr const size_t Space = AccumulateSpace({ FieldSerializer::Space... });

    /// Load the target from the given FixedSpan.
    static void LoadObject(Target & target, FixedSpan<const char, Space> bytes)
    {
        size_t pos = 0;
        // Expand pack into a initializer list, because fold-expression is a C++17 feature.
        int dummy[sizeof...(FieldSerializer)] = { (
            FieldSerializer::LoadField(target, bytes.template subspan<FieldSerializer::Space>(pos)),
            (pos += FieldSerializer::Space), 0)... };
        (void) (dummy);
    }

    /// Save the target value into the given FixedSpan.
    static void SaveObject(const Target & target, FixedSpan<char, Space> bytes)
    {
        size_t pos = 0;
        // Expand pack into a initializer list, because fold-expression is a C++17 feature.
        int dummy[sizeof...(FieldSerializer)] = { (
            FieldSerializer::SaveField(target, bytes.template subspan<FieldSerializer::Space>(pos)),
            (pos += FieldSerializer::Space), 0)... };
        (void) (dummy);
    }
};

} // namespace chip
