/*
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <oob-accessors/clusters/ElectricalEnergyMeasurementOOBAccessor.h>

#include <lib/core/TLV.h>
#include <lib/support/CodeUtils.h>

namespace chip::app {

std::optional<CHIP_ERROR> ElectricalEnergyMeasurementOOBAccessor::HandleAction(CharSpan action, ByteSpan tlvData)
{
    if (!action.data_equal("GenerateElectricalEnergyMeasurementSnapshots"_span))
    {
        return std::nullopt;
    }

    TLV::TLVReader reader;
    reader.Init(tlvData);
    ReturnErrorOnFailure(reader.Next(TLV::kTLVType_Structure, TLV::AnonymousTag()));

    TLV::TLVType outerType;
    ReturnErrorOnFailure(reader.EnterContainer(outerType));

    EndpointId endpointId = kInvalidEndpointId;
    bool hasEndpointId    = false;

    CHIP_ERROR err = CHIP_NO_ERROR;
    while ((err = reader.Next()) == CHIP_NO_ERROR)
    {
        TLV::Tag tag = reader.GetTag();
        if (!TLV::IsContextTag(tag))
        {
            continue;
        }
        switch (TLV::TagNumFromTag(tag))
        {
        case 1:
            ReturnErrorOnFailure(reader.Get(endpointId));
            hasEndpointId = true;
            break;
        default:
            break;
        }
    }
    VerifyOrReturnError(err == CHIP_END_OF_TLV, err);
    ReturnErrorOnFailure(reader.ExitContainer(outerType));

    VerifyOrReturnError(hasEndpointId, CHIP_ERROR_INVALID_ARGUMENT);

    if (endpointId != mEndpointId)
    {
        return std::nullopt;
    }

    mCluster.GenerateSnapshots();
    return CHIP_NO_ERROR;
}

} // namespace chip::app
