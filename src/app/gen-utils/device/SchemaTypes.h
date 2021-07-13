/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    All rights reserved.
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

/**
 *    @file
 *      Contains various types and methods used in the generated code for constrained devices.
 *
 */

#pragma once

#include <core/CHIPCore.h>
#include <core/CHIPTLVDebug.hpp>
#include <support/CodeUtils.h>
#include <core/CHIPConfig.h>
#include <support/BitFlags.h>
#include <array>
#include <basic-types.h>

namespace chip {
namespace app {

/*
 * @brief
 *
 * Type of a field in a schema element (attributes, commands, events)
 */
enum class Type: uint8_t
{
    TYPE_UINT8 = (1 << 0),
    TYPE_UINT32 = (1 << 1),
    TYPE_ARRAY = (1 << 2),
    TYPE_STRUCT = (1 << 3),
    TYPE_UINT64 = (1 << 4),
    TYPE_OCTSTR = (1 << 5),
    TYPE_STRING = (1 << 6)
};

/*
 * @brief
 *
 * Qualities of a field in a cluster definition.
 */
enum QualityMasks {
    kNone = 0,
    kOptional = (1 << 0),
    kNullable = (1 << 1),
};

/*
 * @brief
 *
 * This descriptor contains schema information describing a particular field in a schema element.
 *
 * This is emited in generated cluster definitions.
 */
struct FullFieldDescriptor {
    chip::FieldId FieldId;
    BitFlags<Type> FieldType;
    uint32_t Qualities;
    uint64_t FieldGenTag;
    chip::Span<const FullFieldDescriptor> FieldList;
};

/*
 * @brief
 *
 * This descriptor contains schema information describing a particular field in a schema element.
 *
 * This is generated using constexpr functions at compile time from generated descriptor tables of
 * type FullFieldDescriptor
 */
struct CompactFieldDescriptor {
    chip::FieldId FieldId;
    BitFlags<Type> FieldType;
    uint32_t Qualities;
    uint32_t Offset;
    uint32_t TypeSize;
    chip::Span<const CompactFieldDescriptor> StructDef;
};

/*
 * @brief
 *
 * For every field in a generated type definition, this structure contains the offset of those fields
 */
struct TypeOffsetInfo {
    uint32_t Offset;
    uint32_t TypeSize;
};


template <size_t N>
struct BaseType {
    std::array<uint32_t,N> mOffsets;
};

template <size_t N>
struct StructDescriptor {
    std::array<CompactFieldDescriptor, N> FieldList;
};

constexpr bool IsImplemented(uint64_t FieldId)
{
#ifdef ImplementedFields
    for (size_t i = 0; i < ImplementedFields.size(); i++) {
        if (ImplementedFields[i] == FieldId) {
            return true;
        }
    }
#endif

    return true;
}

/*
 * @brief
 *
 * A function that computes the number of implemented fields in a given full field descriptor
 * based on what the product configuration file has defined as being implemented.
 *
 * This macro is helpful for compile-time sizing the compact field descriptors, as well as a number of
 * other constexpr arrays.
 */
template <size_t N>
constexpr int GetNumImplementedFields(const FullFieldDescriptor (&structDescriptor)[N])
{
    int count = 0;

    for (size_t i = 0; i < N; i++) {
        if (structDescriptor[i].Qualities & kOptional) {
            count += !!IsImplemented(structDescriptor[i].FieldGenTag);
        }
        else {
            count++;
        }
    }

    return count;
}

/*
 * @brief
 *
 * The main function that generates the compact field descriptors from the full field descriptor using
 * product configuration information about implemented fields/features, as well as other product configuration defines (in the future)
 */
template <size_t N, size_t M, class ...Args>
constexpr std::array<CompactFieldDescriptor, N> PopulateFieldDescriptors(const FullFieldDescriptor (&schema)[M],
                                                                  std::array<TypeOffsetInfo,N> offsets, const Args& ...args) {
    int index = 0;
    int structIndex = 0;

    using result_t = ::std::array<CompactFieldDescriptor, N>;
    result_t r = {};
    std::array<chip::Span<const CompactFieldDescriptor>, sizeof...(args)> structArgs = {{ args... }};
    const result_t& const_r = r;

    for (size_t i = 0; i < M; i++) {
        if (((schema[i].Qualities & kOptional) && IsImplemented(schema[i].FieldGenTag)) ||
            !(schema[i].Qualities & kOptional)) {
            const_cast<decltype(CompactFieldDescriptor::Offset)&>(const_r[index].Offset) = offsets[index].Offset;
            const_cast<decltype(CompactFieldDescriptor::TypeSize)&>(const_r[index].TypeSize) = offsets[index].TypeSize;
            const_cast<decltype(CompactFieldDescriptor::FieldId)&>(const_r[index].FieldId) = schema[i].FieldId;
            const_cast<decltype(CompactFieldDescriptor::FieldType)&>(const_r[index].FieldType) = schema[i].FieldType;
            const_cast<decltype(CompactFieldDescriptor::Qualities)&>(const_r[index].Qualities) = schema[i].Qualities;

            if (schema[i].FieldType.Has(Type::TYPE_STRUCT)) {
                const_cast<decltype(CompactFieldDescriptor::StructDef)&>(const_r[index].StructDef) = structArgs[structIndex++];
            }

            index++;
        }
    }

    return r;
}

} // namespace app
} // namespace chip
