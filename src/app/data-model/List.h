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

#include "Decode.h"
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
 * Dedicated type for list<T> that is at its base, just a Span.
 *
 * Motivated by the need to create distinction between Lists that use Spans
 * vs. other data model types that use Spans (like octetstr). These have different
 * encodings. Consequently, there needs to be an actual C++ type distinction to ensure
 * correct specialization of the Encode/Decode methods.
 *
 */
template <typename T>
struct List : public Span<T>
{
    using Span<T>::Span;
    using Span<T>::operator=;
};

template <typename X>
inline CHIP_ERROR Encode(TLV::TLVWriter & writer, uint64_t tag, List<X> x)
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

template <typename X>
CHIP_ERROR Decode(TLV::TLVReader & reader, List<X> & x)
{
    TLV::TLVType outer1;
    size_t destBufSize = x.size();
    uint32_t i         = 0;
    CHIP_ERROR err;

    ReturnErrorOnFailure(reader.EnterContainer(outer1));

    while ((err = reader.Next()) == CHIP_NO_ERROR)
    {
        VerifyOrReturnError(destBufSize, CHIP_ERROR_BUFFER_TOO_SMALL);
        ReturnErrorOnFailure(DataModel::Decode(reader, x.data()[i]));
        destBufSize--;
    }

    return reader.ExitContainer(outer1);
}

} // namespace DataModel
} // namespace app
} // namespace chip
