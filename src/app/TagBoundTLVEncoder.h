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
#include <lib/core/CHIPTLV.h>

/**
 * Class that encapsulates a TLVWriter and tag, so consumers can just call
 * Encode and have the tagging handled for them.
 */
namespace chip {
namespace app {

class TagBoundTLVEncoder
{
public:
    // Consumers that initialize a TagBoundTLVEncoder with a null TLVWriter must
    // ensure they don't call Encode on it.
    TagBoundTLVEncoder(TLV::TLVWriter * aWriter, TLV::Tag aTag) : mWriter(aWriter), mTag(aTag) {}

    template <typename... Ts>
    CHIP_ERROR Encode(Ts... aArgs) const
    {
        return DataModel::Encode(*mWriter, mTag, std::forward<Ts>(aArgs)...);
    }

protected:
    TLV::TLVWriter * const mWriter;
    const TLV::Tag mTag;
};

} // namespace app
} // namespace chip
