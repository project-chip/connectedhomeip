/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <app/data-model/Encode.h>
#include <app/data-model/List.h> // So we can encode lists
#include <lib/core/TLV.h>
#include <lib/support/CodeUtils.h>

/**
 * Class that encapsulates a TLVWriter and tag that can be provided to a
 * consumer so that the consumer can just call Encode and have the tagging
 * handled for them.
 */
namespace chip {
namespace app {

class TagBoundEncoder
{
public:
    // Initialization with a null TLVWriter is allowed, but attempts to Encode()
    // will fail.
    TagBoundEncoder(TLV::TLVWriter * aWriter, TLV::Tag aTag) : mWriter(aWriter), mTag(aTag) {}

    template <typename... Ts>
    CHIP_ERROR Encode(Ts &&... aArgs) const
    {
        VerifyOrReturnError(mWriter != nullptr, CHIP_ERROR_INCORRECT_STATE);
        return DataModel::Encode(*mWriter, mTag, std::forward<Ts>(aArgs)...);
    }

    /**
     * aCallback is expected to take a const TagBoundEncoder& argument and
     * Encode() on it as many times as needed to encode all the list elements
     * one by one.  If any of those Encode() calls returns failure, aCallback
     * must stop encoding and return failure.  When all items are encoded
     * aCallback is expected to return success.
     *
     * aCallback may not be called.  Consumers must not assume it will be
     * called.
     */
    template <typename ListGenerator>
    CHIP_ERROR EncodeList(ListGenerator aCallback)
    {
        VerifyOrReturnError(mWriter != nullptr, CHIP_ERROR_INCORRECT_STATE);
        TLV::TLVType outerType;
        ReturnErrorOnFailure(mWriter->StartContainer(mTag, TLV::kTLVType_Array, outerType));
        ReturnErrorOnFailure(aCallback(TagBoundEncoder(mWriter, TLV::AnonymousTag())));
        return mWriter->EndContainer(outerType);
    }

protected:
    TLV::TLVWriter * const mWriter;

private:
    const TLV::Tag mTag;
};

} // namespace app
} // namespace chip
