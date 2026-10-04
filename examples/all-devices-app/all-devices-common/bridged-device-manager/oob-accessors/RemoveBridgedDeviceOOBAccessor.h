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

#pragma once

#include <bridged-device-manager/BridgedDeviceManager.h>
#include <lib/core/TLV.h>
#include <lib/support/logging/CHIPLogging.h>
#include <oob-accessors/OOBAccessor.h>

namespace chip::app {

/**
 * Usage:
 *    Tag(2): BridgedDeviceManager::DeviceId deviceId (uint16_t)
 *       The deviceId of the bridged device to be removed.
 *
 * Note: Here Tag(2) is used for the deviceId, because Tag(1) is commonly used for endpointId.
 */

class RemoveBridgedDeviceOOBAccessor : public OOBAccessor
{
public:
    explicit RemoveBridgedDeviceOOBAccessor(BridgedDeviceManager & bridgedDeviceManager) : mBridgedDeviceManager(bridgedDeviceManager) {}

    std::optional<CHIP_ERROR> HandleAction(CharSpan action, ByteSpan tlvData) override
    {
        if (!action.data_equal("RemoveBridgedDevice"_span))
        {
            return std::nullopt;
        }

        TLV::TLVReader reader;
        reader.Init(tlvData);
        ReturnErrorOnFailure(reader.Next(TLV::kTLVType_Structure, TLV::AnonymousTag()));

        TLV::TLVType outerType;
        ReturnErrorOnFailure(reader.EnterContainer(outerType));

        uint16_t deviceIdVal = 0;
        bool hasDeviceId    = false;
        CHIP_ERROR err        = CHIP_NO_ERROR;
        while ((err = reader.Next()) == CHIP_NO_ERROR)
        {
            TLV::Tag tag = reader.GetTag();
            if (!TLV::IsContextTag(tag))
            {
                continue;
            }
            switch (TLV::TagNumFromTag(tag))
            {
            case 2:
                ReturnErrorOnFailure(reader.Get(deviceIdVal));
                hasDeviceId = true;
                break;
            default:
                break;
            }
        }
        VerifyOrReturnError(err == CHIP_END_OF_TLV, err);
        ReturnErrorOnFailure(reader.ExitContainer(outerType));
        VerifyOrReturnError(hasDeviceId, CHIP_ERROR_INVALID_ARGUMENT);
        BridgedDeviceManager::DeviceId deviceId(deviceIdVal);

        mBridgedDeviceManager.RemoveDevice(deviceId);
        ChipLogProgress(AppServer, "Removed bridged device with id: %u", static_cast<uint16_t>(deviceId));
        return CHIP_NO_ERROR;
    }

private:
    BridgedDeviceManager & mBridgedDeviceManager;
};

} // namespace chip::app
