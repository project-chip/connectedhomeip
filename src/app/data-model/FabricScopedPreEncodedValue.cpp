/*
 *    Copyright (c) 2024 Project CHIP Authors
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

#include "FabricScopedPreEncodedValue.h"
#include <lib/core/TLVReader.h>
#include <lib/support/CodeUtils.h>

#include <optional>

namespace chip {
namespace app {
namespace DataModel {

FabricScopedPreEncodedValue::FabricScopedPreEncodedValue(const ByteSpan & aData) : mData(aData) {}

CHIP_ERROR FabricScopedPreEncodedValue::EncodeForRead(TLV::TLVWriter & aWriter, TLV::Tag aTag, FabricIndex aFabricIndex) const
{
    TLV::TLVReader reader;
    reader.Init(mData);

    ReturnErrorOnFailure(reader.Next());

    return aWriter.CopyElement(aTag, reader);
}

FabricIndex FabricScopedPreEncodedValue::GetFabricIndex() const
{
    TLV::TLVReader reader;
    reader.Init(mData);
    CHIP_ERROR err = reader.Next();
    if (err != CHIP_NO_ERROR)
    {
        return kUndefinedFabricIndex;
    }

    // We must have a struct here.
    if (reader.GetType() != TLV::kTLVType_Structure)
    {
        return kUndefinedFabricIndex;
    }

    TLV::TLVType structType;
    err = reader.EnterContainer(structType);
    if (err != CHIP_NO_ERROR)
    {
        return kUndefinedFabricIndex;
    }

    // Now look for a single field with the right tag.
    std::optional<FabricIndex> foundFabricIndex;
    constexpr TLV::Tag kFabricIndexTag = TLV::ContextTag(254);
    while ((err = reader.Next()) == CHIP_NO_ERROR)
    {
        if (reader.GetTag() != kFabricIndexTag)
        {
            continue;
        }

        if (foundFabricIndex.has_value())
        {
            // Two fabric indices?  Just give up.  Note that this will lead to
            // errors encoding our value.
            return kUndefinedFabricIndex;
        }

        err = reader.Get(foundFabricIndex.emplace());
        if (err != CHIP_NO_ERROR)
        {
            return kUndefinedFabricIndex;
        }
    }

    if (err != CHIP_ERROR_END_OF_TLV)
    {
        return kUndefinedFabricIndex;
    }

    err = reader.ExitContainer(structType);
    if (err != CHIP_NO_ERROR)
    {
        return kUndefinedFabricIndex;
    }

    return foundFabricIndex.value_or(kUndefinedFabricIndex);
}

} // namespace DataModel
} // namespace app
} // namespace chip
