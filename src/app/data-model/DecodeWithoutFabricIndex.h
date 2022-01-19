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

#include <app/data-model/DecodableList.h>
#include <app/data-model/Decode.h>

namespace chip {
namespace app {
namespace DataModel {

/*
 * @brief
 *
 * This function works just like Decode(reader, x), however, for fabric scoped data, decode with this won't overwrite the fabric
 * index field in the payload. This function is expected to be called by client side code when decoding the values from server with
 * a fabric index field with a unified API for both fabric-scoped and non-fabric-scoped values.
 *
 */
template <typename X,
          typename std::enable_if_t<std::is_class<X>::value &&
                                        std::is_same<decltype(Decode(std::declval<TLV::TLVReader &>(), std::declval<X &>(),
                                                                     std::declval<Optional<FabricIndex> &>())),
                                                     CHIP_ERROR>::value,
                                    X> * = nullptr>
CHIP_ERROR DecodeWithoutFabricIndex(TLV::TLVReader & reader, X & x)
{
    return Decode(reader, x, NullOptional);
}

/*
 * @brief
 *
 * This function works just like Decode(reader, x), however, for fabric scoped data, decode with this won't overwrite the fabric
 * index field in the payload. This function is expected to be called by client side code when decoding the values from server with
 * a fabric index field with a unified API for both fabric-scoped and non-fabric-scoped values.
 *
 */
template <
    typename X,
    typename std::enable_if_t<
        std::is_same<decltype(Decode(std::declval<TLV::TLVReader &>(), std::declval<X &>())), CHIP_ERROR>::value, X> * = nullptr>
CHIP_ERROR DecodeWithoutFabricIndex(TLV::TLVReader & reader, X & x)
{
    return Decode(reader, x);
}

} // namespace DataModel
} // namespace app
} // namespace chip
