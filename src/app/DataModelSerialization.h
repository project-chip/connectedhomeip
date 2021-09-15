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

#include <iterator>
#include <lib/core/CHIPCore.h>
#include <lib/core/CHIPSafeCasts.h>
#include <lib/core/CHIPTLV.h>
#include <lib/core/Optional.h>
#include <lib/support/CodeUtils.h>

namespace chip {
namespace app {
namespace DataModel {

/*
 * @brief
 * Set of overloaded encode methods that based on the type of cluster element passed in,
 * appropriately encodes them to TLV.
 */
template <typename X, typename std::enable_if_t<std::is_integral<X>::value, int> = 0>
CHIP_ERROR Encode(TLV::TLVWriter & writer, uint64_t tag, X x)
{
    return writer.Put(tag, x);
}

template <typename X, typename std::enable_if_t<std::is_enum<X>::value, int> = 0>
CHIP_ERROR Encode(TLV::TLVWriter & writer, uint64_t tag, X x)
{
    return writer.Put(tag, x);
}

CHIP_ERROR Encode(TLV::TLVWriter & writer, uint64_t tag, ByteSpan x)
{
    return writer.Put(tag, x);
}

CHIP_ERROR Encode(TLV::TLVWriter & writer, uint64_t tag, const Span<const char> x)
{
    return writer.PutString(tag, x.data(), static_cast<uint32_t>(x.size()));
}

template <typename X>
CHIP_ERROR Encode(TLV::TLVWriter & writer, uint64_t tag, const Span<X> & x)
{
    TLV::TLVType type;

    ReturnErrorOnFailure(writer.StartContainer(tag, TLV::kTLVType_Array, type));
    for (auto & item : x)
    {
        ReturnErrorOnFailure(Encode(writer, TLV::AnonymousTag, item));
    }
    ReturnErrorOnFailure(writer.EndContainer(type));

    return CHIP_NO_ERROR;
}

/*
 * @brief
 *
 * This specific variant that encodes cluster objects (like structs, commands, events) to TLV
 * depends on the presence of an Encode method on the object to present. The signature of that method
 * is as follows:
 *
 * CHIP_ERROR <Object>::Encode(TLVWriter &writer, uint64_t tag);
 *
 *
 */
template <typename X,
          typename std::enable_if_t<std::is_class<X>::value &&
                                        std::is_same<decltype(&X::Encode), CHIP_ERROR (X::*)(TLV::TLVWriter &, uint64_t)>::value,
                                    X> * = nullptr>
CHIP_ERROR Encode(TLV::TLVWriter & writer, uint64_t tag, X & x)
{
    return x.Encode(writer, tag);
}

//
// Decode
//
template <typename X, typename std::enable_if_t<std::is_integral<X>::value, int> = 0>
CHIP_ERROR Decode(TLV::TLVReader & reader, X & x)
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
CHIP_ERROR Decode(TLV::TLVReader & reader, ByteSpan & x)
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
CHIP_ERROR Decode(TLV::TLVReader & reader, Span<const char> & x)
{
    ByteSpan bs;

    VerifyOrReturnError(reader.GetType() == TLV::kTLVType_UTF8String, CHIP_ERROR_UNEXPECTED_TLV_ELEMENT);
    ReturnErrorOnFailure(reader.Get(bs));
    x = Span<const char>(Uint8::to_const_char(bs.data()), (size_t) bs.size());
    return CHIP_NO_ERROR;
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
template <typename X, typename std::enable_if_t<std::is_class<X>::value, int> = 0>
CHIP_ERROR Decode(TLV::TLVReader & reader, X & x)
{
    return x.Decode(reader);
}

} // namespace DataModel
} // namespace app
} // namespace chip
