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

 #include "AddBridgedDeviceOOBAccessor.h"

#include <lib/core/TLV.h>
#include <lib/support/logging/CHIPLogging.h>

namespace chip::app {

AddBridgedDeviceOOBAccessor::AddBridgedDeviceOOBAccessor(BridgedDeviceManager & bridgedDeviceManager) :
    mBridgedDeviceManager(bridgedDeviceManager)
{}

std::optional<CHIP_ERROR> AddBridgedDeviceOOBAccessor::HandleAction(CharSpan action, ByteSpan tlvData)
{
    if (!action.data_equal("AddBridgedDevice"_span))
    {
        return std::nullopt;
    }

    TLV::TLVReader reader;
    reader.Init(tlvData);
    ReturnErrorOnFailure(reader.Next(TLV::kTLVType_Structure, TLV::AnonymousTag()));

    TLV::TLVType outerType;
    ReturnErrorOnFailure(reader.EnterContainer(outerType));

    EndpointId aggregatorEndpointId = kInvalidEndpointId;
    char deviceType[256]            = {};
    bool hasDeviceType              = false;
    CHIP_ERROR err                  = CHIP_NO_ERROR;
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
            ReturnErrorOnFailure(reader.Get(aggregatorEndpointId));
            break;
        case 2:
            ReturnErrorOnFailure(reader.GetString(deviceType, sizeof(deviceType)));
            hasDeviceType = true;
            break;
        default:
            break;
        }
    }
    VerifyOrReturnError(err == CHIP_END_OF_TLV, err);
    ReturnErrorOnFailure(reader.ExitContainer(outerType));
    VerifyOrReturnError(hasDeviceType, CHIP_ERROR_INVALID_ARGUMENT);

    auto optionalDeviceInterfaceId = mBridgedDeviceManager.AddBridgedDevice(deviceType, {}, aggregatorEndpointId);
    if (!optionalDeviceInterfaceId.has_value())
    {
        ChipLogError(AppServer, "Failed to add bridged device: %s", deviceType);
        return CHIP_ERROR_INCORRECT_STATE;
    }

    ChipLogProgress(AppServer, "Successfully added bridged device: %s with DeviceInterfaceId: %u", deviceType,
                    static_cast<uint16_t>(optionalDeviceInterfaceId.value()));
    return CHIP_NO_ERROR;
}

} // namespace chip::app