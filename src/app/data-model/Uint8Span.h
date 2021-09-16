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

#include "Encode.h"
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
 * Span<const uint8_t> serves as the underlying type
 * for both octstr and list<uint8> data model types. However, these two
 * data model types have very different encodings (TLV ByteString and TLV Array respectively).
 *
 * To permit that distinction, this Uint8Span type has been created for list<uint8> to be distinct in type
 * from ByteSpan, which is to be used for octstr.
 */
struct Uint8Span : public Span<const uint8_t>
{
    using Span::Span;
    using Span::operator=;
};

inline CHIP_ERROR Encode(TLV::TLVWriter & writer, uint64_t tag, Uint8Span x)
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

} // namespace DataModel
} // namespace app
} // namespace chip
