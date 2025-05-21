/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
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

#pragma once

#include <app/data-model/FabricScoped.h>
#include <app/data-model/Nullable.h>
#include <lib/core/DataModelTypes.h>
#include <lib/core/Optional.h>
#include <lib/core/TLV.h>
#include <protocols/interaction_model/Constants.h>

#include <type_traits>

namespace chip {
namespace app {
namespace DataModel {

namespace detail {
// A way to detect whether an enum has a kUnknownEnumValue value, for use in enable_if.
template <typename Enum, Enum value>
using VoidType = void;

template <typename, typename = void>
inline constexpr bool HasUnknownValue = false;

template <typename T>
inline constexpr bool HasUnknownValue<T, VoidType<T, T::kUnknownEnumValue>> = true;
} // namespace detail

/*
 * @brief
 * Set of overloaded encode methods that based on the type of cluster element passed in,
 * appropriately encodes them to TLV.
 */
template <typename X, typename std::enable_if_t<std::is_integral<X>::value, int> = 0>
CHIP_ERROR Encode(TLV::TLVWriter & writer, TLV::Tag tag, X x)
{
    return writer.Put(tag, x);
}

template <typename X, typename std::enable_if_t<std::is_floating_point<X>::value, int> = 0>
CHIP_ERROR Encode(TLV::TLVWriter & writer, TLV::Tag tag, X x)
{
    return writer.Put(tag, x);
}

template <typename X, typename std::enable_if_t<std::is_enum<X>::value && !detail::HasUnknownValue<X>, int> = 0>
CHIP_ERROR Encode(TLV::TLVWriter & writer, TLV::Tag tag, X x)
{
    return writer.Put(tag, x);
}

template <typename X, typename std::enable_if_t<std::is_enum<X>::value && detail::HasUnknownValue<X>, int> = 0>
CHIP_ERROR Encode(TLV::TLVWriter & writer, TLV::Tag tag, X x)
{
#if !CHIP_CONFIG_IM_ENABLE_ENCODING_SENTINEL_ENUM_VALUES
    if (x == X::kUnknownEnumValue)
    {
        return CHIP_IM_GLOBAL_STATUS(ConstraintError);
    }
#endif // !CHIP_CONFIG_IM_ENABLE_ENCODING_SENTINEL_ENUM_VALUES

    return writer.Put(tag, x);
}

template <typename X>
CHIP_ERROR Encode(TLV::TLVWriter & writer, TLV::Tag tag, BitFlags<X> x)
{
    return writer.Put(tag, x);
}

inline CHIP_ERROR Encode(TLV::TLVWriter & writer, TLV::Tag tag, ByteSpan x)
{
    return writer.Put(tag, x);
}

inline CHIP_ERROR Encode(TLV::TLVWriter & writer, TLV::Tag tag, Span<const char> x)
{
    return writer.PutString(tag, x);
}

/*
 * @brief
 *
 * This specific variant that encodes cluster objects (like non fabric-scoped structs, commands, events) to TLV
 * depends on the presence of an Encode method on the object. The signature of that method
 * is as follows:
 *
 * CHIP_ERROR <Object>::Encode(TLVWriter &writer, TLV::Tag tag) const;
 *
 *
 */
template <typename X,
          typename std::enable_if_t<
              std::is_class<X>::value &&
                  std::is_same<decltype(std::declval<X>().Encode(std::declval<TLV::TLVWriter &>(), std::declval<TLV::Tag>())),
                               CHIP_ERROR>::value,
              X> * = nullptr>
CHIP_ERROR Encode(TLV::TLVWriter & writer, TLV::Tag tag, const X & x)
{
    return x.Encode(writer, tag);
}

/*
 * @brief
 *
 * A way to encode fabric-scoped structs for a write that omits encoding the containing fabric index field.
 */
template <typename X,
          typename std::enable_if_t<std::is_class<X>::value &&
                                        std::is_same<decltype(std::declval<X>().EncodeForWrite(std::declval<TLV::TLVWriter &>(),
                                                                                               std::declval<TLV::Tag>())),
                                                     CHIP_ERROR>::value &&
                                        DataModel::IsFabricScoped<X>::value,
                                    X> * = nullptr>
CHIP_ERROR EncodeForWrite(TLV::TLVWriter & writer, TLV::Tag tag, const X & x)
{
    return x.EncodeForWrite(writer, tag);
}

/*
 * @brief
 *
 * A way to encode fabric-scoped structs for a read that always encodes the containing fabric index field.
 *
 * An accessing fabric index must be passed in to permit including/omitting sensitive fields based on a match with the fabric index
 * associated with the scoped struct.
 */
template <typename X,
          typename std::enable_if_t<
              std::is_class<X>::value &&
                  std::is_same<decltype(std::declval<X>().EncodeForRead(std::declval<TLV::TLVWriter &>(), std::declval<TLV::Tag>(),
                                                                        std::declval<FabricIndex>())),
                               CHIP_ERROR>::value &&
                  DataModel::IsFabricScoped<X>::value,
              X> * = nullptr>
CHIP_ERROR EncodeForRead(TLV::TLVWriter & writer, TLV::Tag tag, FabricIndex accessingFabricIndex, const X & x)
{
    return x.EncodeForRead(writer, tag, accessingFabricIndex);
}

/*
 * @brief
 *
 * Encodes an optional value (struct field, command field, event field).
 */
template <typename X>
CHIP_ERROR Encode(TLV::TLVWriter & writer, TLV::Tag tag, const Optional<X> & x)
{
    if (x.HasValue())
    {
        return Encode(writer, tag, x.Value());
    }
    // If no value, just do nothing.
    return CHIP_NO_ERROR;
}

/*
 * @brief
 *
 * Encodes a nullable value.
 */
template <typename X>
CHIP_ERROR Encode(TLV::TLVWriter & writer, TLV::Tag tag, const Nullable<X> & x)
{
    if (x.IsNull())
    {
        return writer.PutNull(tag);
    }

    // Allow sending invalid values for nullables when
    // CONFIG_BUILD_FOR_HOST_UNIT_TEST is true, so we can test how the other side
    // responds.
#if !CONFIG_BUILD_FOR_HOST_UNIT_TEST
    if (!x.ExistingValueInEncodableRange())
    {
        return CHIP_IM_GLOBAL_STATUS(ConstraintError);
    }
#endif // !CONFIG_BUILD_FOR_HOST_UNIT_TEST

    // The -Wmaybe-uninitialized warning gets confused about the fact
    // that x.mValue is always initialized if x.IsNull() is not
    // true, so suppress it for our access to x.Value().
#pragma GCC diagnostic push
#if !defined(__clang__)
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#endif // !defined(__clang__)
    return Encode(writer, tag, x.Value());
#pragma GCC diagnostic pop
}

/*
 * @brief
 *
 * Encodes a nullable fabric-scoped struct for a write.
 */
template <typename X, std::enable_if_t<DataModel::IsFabricScoped<X>::value, bool> = true>
CHIP_ERROR EncodeForWrite(TLV::TLVWriter & writer, TLV::Tag tag, const Nullable<X> & x)
{
    if (x.IsNull())
    {
        return writer.PutNull(tag);
    }

    // The -Wmaybe-uninitialized warning gets confused about the fact
    // that x.mValue is always initialized if x.IsNull() is not
    // true, so suppress it for our access to x.Value().
#pragma GCC diagnostic push
#if !defined(__clang__)
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#endif // !defined(__clang__)
    return EncodeForWrite(writer, tag, x.Value());
#pragma GCC diagnostic pop
}

/*
 * @brief
 *
 * Encodes a nullable fabric-scoped struct for a read.
 */
template <typename X, std::enable_if_t<DataModel::IsFabricScoped<X>::value, bool> = true>
CHIP_ERROR EncodeForRead(TLV::TLVWriter & writer, TLV::Tag tag, FabricIndex accessingFabricIndex, const Nullable<X> & x)
{
    if (x.IsNull())
    {
        return writer.PutNull(tag);
    }

    // The -Wmaybe-uninitialized warning gets confused about the fact
    // that x.mValue is always initialized if x.IsNull() is not
    // true, so suppress it for our access to x.Value().
#pragma GCC diagnostic push
#if !defined(__clang__)
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#endif // !defined(__clang__)
    return EncodeForRead(writer, tag, accessingFabricIndex, x.Value());
#pragma GCC diagnostic pop
}

} // namespace DataModel
} // namespace app
} // namespace chip
