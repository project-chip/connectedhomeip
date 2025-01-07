/*
 *    Copyright (c) 2021-2024 Project CHIP Authors
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

#include <access/SubjectDescriptor.h>
#include <app/data-model/Decode.h>
#include <app/data-model/FabricScoped.h>
#include <lib/core/TLVReader.h>

#include <type_traits>

namespace chip {
namespace app {

class AttributeValueDecoder
{
public:
    AttributeValueDecoder(TLV::TLVReader & aReader, const Access::SubjectDescriptor & aSubjectDescriptor) :
        mReader(aReader), mSubjectDescriptor(aSubjectDescriptor)
    {}

    template <typename T, typename std::enable_if_t<!DataModel::IsFabricScoped<T>::value, bool> = true>
    CHIP_ERROR Decode(T & aArg)
    {
        mTriedDecode = true;
        return DataModel::Decode(mReader, aArg);
    }

    template <typename T, typename std::enable_if_t<DataModel::IsFabricScoped<T>::value, bool> = true>
    CHIP_ERROR Decode(T & aArg)
    {
        mTriedDecode = true;
        // The WriteRequest comes with no fabric index, this will happen when receiving a write request on a PASE session before
        // AddNOC.
        VerifyOrReturnError(AccessingFabricIndex() != kUndefinedFabricIndex, CHIP_IM_GLOBAL_STATUS(UnsupportedAccess));
        ReturnErrorOnFailure(DataModel::Decode(mReader, aArg));
        aArg.SetFabricIndex(AccessingFabricIndex());
        return CHIP_NO_ERROR;
    }

    bool TriedDecode() const { return mTriedDecode; }

    /**
     * The accessing fabric index for this write interaction.
     */
    FabricIndex AccessingFabricIndex() const { return mSubjectDescriptor.fabricIndex; }

    /**
     * The accessing subject descriptor for this write interaction.
     */
    const Access::SubjectDescriptor & GetSubjectDescriptor() const { return mSubjectDescriptor; }

private:
    friend class TestOnlyAttributeValueDecoderAccessor;

    TLV::TLVReader & mReader;
    bool mTriedDecode = false;
    const Access::SubjectDescriptor mSubjectDescriptor;
};

} // namespace app
} // namespace chip
