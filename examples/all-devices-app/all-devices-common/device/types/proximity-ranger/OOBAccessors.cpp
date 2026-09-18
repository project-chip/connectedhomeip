/*
 *    Copyright (c) 2026 Project CHIP Authors
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
#include "OOBAccessors.h"

#include <app/data-model/Decode.h>
#include <clusters/ProximityRanging/Structs.h>
#include <lib/core/TLV.h>
#include <lib/support/CodeUtils.h>

#include <memory>
#include <vector>

namespace chip {
namespace app {

using RangingConstraint = Clusters::ProximityRanging::Structs::RangingConstraintStruct::Type;

// Main OOB dispatcher
std::optional<CHIP_ERROR> ProximityRangerOOBAccessor::HandleAction(CharSpan action, ByteSpan tlvData)
{
    if (action.data_equal("SetRangingConstraints"_span))
        return HandleSetRangingConstraints(tlvData);

    return std::nullopt;
}

// Allows OOB configuration of Rnnging Contraints
std::optional<CHIP_ERROR> ProximityRangerOOBAccessor::HandleSetRangingConstraints(ByteSpan tlvData) const
{
    TLV::TLVReader reader;
    reader.Init(tlvData);
    ReturnErrorOnFailure(reader.Next(TLV::kTLVType_Structure, TLV::AnonymousTag()));

    TLV::TLVType outerType;
    ReturnErrorOnFailure(reader.EnterContainer(outerType));

    EndpointId endpointId = kInvalidEndpointId;
    bool hasEndpointId    = false;
    bool hasConstraints   = false;
    std::vector<RangingConstraint> constraints;

    CHIP_ERROR err = CHIP_NO_ERROR;
    while ((err = reader.Next()) == CHIP_NO_ERROR)
    {
        TLV::Tag tag = reader.GetTag();
        if (!TLV::IsContextTag(tag))
            continue;

        switch (TLV::TagNumFromTag(tag))
        {
        case kTagEndpointId:
            ReturnErrorOnFailure(reader.Get(endpointId));
            hasEndpointId = true;
            break;
        case kTagConstraints: {
            VerifyOrReturnError(reader.GetType() == TLV::kTLVType_Array, CHIP_ERROR_WRONG_TLV_TYPE);
            TLV::TLVType arrayType;
            ReturnErrorOnFailure(reader.EnterContainer(arrayType));
            CHIP_ERROR arrayErr = CHIP_NO_ERROR;
            while ((arrayErr = reader.Next()) == CHIP_NO_ERROR)
            {
                RangingConstraint decoded;
                ReturnErrorOnFailure(DataModel::Decode(reader, decoded));
                constraints.push_back(decoded);
            }
            VerifyOrReturnError(arrayErr == CHIP_END_OF_TLV, arrayErr);
            ReturnErrorOnFailure(reader.ExitContainer(arrayType));
            hasConstraints = true;
            break;
        }
        default:
            break;
        }
    }
    VerifyOrReturnError(err == CHIP_END_OF_TLV, err);
    ReturnErrorOnFailure(reader.ExitContainer(outerType));

    VerifyOrReturnError(hasEndpointId && hasConstraints, CHIP_ERROR_INVALID_ARGUMENT);

    if (endpointId != mEndpointId)
        return std::nullopt;

    return mDevice.SetRangingConstraints(Span<const RangingConstraint>(constraints.data(), constraints.size()));
}

// Add OOB accessor to registry
void RegisterOOBAccessors(LoggingProximityRanger & device, OOBAccessorRegistry & registry)
{
    LogErrorOnFailure(registry.Register(std::make_unique<ProximityRangerOOBAccessor>(device, device.GetEndpointId())));
}

} // namespace app
} // namespace chip
