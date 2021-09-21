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

#include <lib/core/CHIPTLV.h>

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

inline CHIP_ERROR Encode(TLV::TLVWriter & writer, uint64_t tag, ByteSpan x)
{
    return writer.Put(tag, x);
}

inline CHIP_ERROR Encode(TLV::TLVWriter & writer, uint64_t tag, Span<const char> x)
{
    return writer.PutString(tag, x);
}

/*
 * @brief
 *
 * This specific variant that encodes cluster objects (like structs, commands, events) to TLV
 * depends on the presence of an Encode method on the object. The signature of that method
 * is as follows:
 *
 * CHIP_ERROR <Object>::Encode(TLVWriter &writer, uint64_t tag) const;
 *
 *
 */
template <
    typename X,
    typename std::enable_if_t<std::is_class<X>::value &&
                                  std::is_same<decltype(&X::Encode), CHIP_ERROR (X::*)(TLV::TLVWriter &, uint64_t) const>::value,
                              X> * = nullptr>
CHIP_ERROR Encode(TLV::TLVWriter & writer, uint64_t tag, const X & x)
{
    return x.Encode(writer, tag);
}

} // namespace DataModel
} // namespace app
} // namespace chip
