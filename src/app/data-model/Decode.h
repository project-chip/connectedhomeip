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

#include <app-common/zap-generated/cluster-enums-check.h>
#include <app/ConcreteAttributePath.h>
#include <app/data-model/Nullable.h>
#include <lib/core/CHIPError.h>
#include <lib/core/CHIPSafeCasts.h>
#include <lib/core/Optional.h>
#include <lib/core/TLV.h>
#include <protocols/interaction_model/Constants.h>

namespace chip {
namespace app {
namespace Clusters {
static auto __attribute__((unused)) EnsureKnownEnumValue(chip::VendorId val)
{
    return val;
}
} // namespace Clusters

namespace DataModel {

//
// Decode
//
template <typename X, typename std::enable_if_t<std::is_integral<X>::value, int> = 0>
CHIP_ERROR Decode(TLV::TLVReader & reader, X & x)
{
    return reader.Get(x);
}

template <typename X, typename std::enable_if_t<std::is_floating_point<X>::value, int> = 0>
CHIP_ERROR Decode(TLV::TLVReader & reader, X & x)
{
    return reader.Get(x);
}

template <typename X, typename std::enable_if_t<std::is_enum<X>::value, int> = 0>
CHIP_ERROR Decode(TLV::TLVReader & reader, X & x)
{
    ReturnErrorOnFailure(reader.Get(x));
    x = Clusters::EnsureKnownEnumValue(x);
    return CHIP_NO_ERROR;
}

template <typename X>
CHIP_ERROR Decode(TLV::TLVReader & reader, BitFlags<X> & x)
{
    return reader.Get(x);
}

//
// @brief
//
// Decodes an octet string that is expected at the positioned reader.
//
// The passed in ByteSpan is ignored and updated to point directly into
// the buffer backing the reader.
//
inline CHIP_ERROR Decode(TLV::TLVReader & reader, ByteSpan & x)
{
    VerifyOrReturnError(reader.GetType() == TLV::kTLVType_ByteString, CHIP_ERROR_UNEXPECTED_TLV_ELEMENT);
    return reader.Get(x);
}

//
// @brief
//
// Decodes a UTF-8 string that is expected at the positioned reader.
//
// The passed in char Span is ignored and updated to point directly into
// the buffer backing the reader.
//
inline CHIP_ERROR Decode(TLV::TLVReader & reader, Span<const char> & x)
{
    return reader.Get(x);
}

/*
 * @brief
 *
 * This specific variant that decodes cluster objects (like structs, commands, events) from TLV
 * depends on the presence of a Decode method on the object to present. The signature of that method
 * is as follows:
 *
 * CHIP_ERROR <Object>::Decode(TLVReader &reader);
 *
 */
template <typename X,
          typename std::enable_if_t<
              std::is_class<X>::value &&
                  std::is_same<decltype(std::declval<X>().Decode(std::declval<TLV::TLVReader &>())), CHIP_ERROR>::value,
              X> * = nullptr>
CHIP_ERROR Decode(TLV::TLVReader & reader, X & x)
{
    return x.Decode(reader);
}

/*
 * @brief
 *
 * This specific variant decodes from TLV a cluster object that contains all attributes encapsulated within a single, monolithic
 * cluster object.
 *
 * Each attribute in the cluster is decoded based on the provided ConcreteAttributePath. The TLVReader is to be positioned right on
 * the data value for the specified attribute.
 *
 * This API depends on the presence of a Decode method on the object. The signature of that method
 * is as follows:
 *
 * CHIP_ERROR <Object>::Decode(TLVReader &reader, ConcreteAttributePath &path);
 *
 */
template <
    typename X,
    typename std::enable_if_t<std::is_class<X>::value &&
                                  std::is_same<decltype(std::declval<X>().Decode(std::declval<TLV::TLVReader &>(),
                                                                                 std::declval<const ConcreteAttributePath &>())),
                                               CHIP_ERROR>::value,
                              X> * = nullptr>
CHIP_ERROR Decode(TLV::TLVReader & reader, const ConcreteAttributePath & path, X & x)
{
    return x.Decode(reader, path);
}

/*
 * @brief
 *
 * Decodes an optional value (struct field, command field, event field).
 */
template <typename X>
CHIP_ERROR Decode(TLV::TLVReader & reader, Optional<X> & x)
{
    // If we are calling this, it means we found the right tag, so just decode
    // the item.
    return Decode(reader, x.Emplace());
}

/*
 * @brief
 *
 * Decodes a nullable value.
 */
template <typename X>
CHIP_ERROR Decode(TLV::TLVReader & reader, Nullable<X> & x)
{
    if (reader.GetType() == TLV::kTLVType_Null)
    {
        x.SetNull();
        return CHIP_NO_ERROR;
    }

    // We have a value; decode it.
    ReturnErrorOnFailure(Decode(reader, x.SetNonNull()));
    if (!x.ExistingValueInEncodableRange())
    {
        return CHIP_IM_GLOBAL_STATUS(ConstraintError);
    }
    return CHIP_NO_ERROR;
}

} // namespace DataModel
} // namespace app
} // namespace chip
