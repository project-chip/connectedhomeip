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

#include <device-factory/DeviceManager.h>
#include <lib/core/TLV.h>
#include <lib/support/logging/CHIPLogging.h>
#include <oob-accessors/OOBAccessor.h>

namespace chip::app {

template <typename DeviceFactoryT>
class UnregisterAndDestroyOOBAccessor : public OOBAccessor
{
public:
    explicit UnregisterAndDestroyOOBAccessor(DeviceManager<DeviceFactoryT> & deviceManager) : mDeviceManager(deviceManager) {}

    std::optional<CHIP_ERROR> HandleAction(CharSpan action, ByteSpan tlvData) override
    {
        if (!action.data_equal("UnregisterAndDestroy"_span))
        {
            return std::nullopt;
        }

        TLV::TLVReader reader;
        reader.Init(tlvData);
        ReturnErrorOnFailure(reader.Next(TLV::kTLVType_Structure, TLV::AnonymousTag()));

        TLV::TLVType outerType;
        ReturnErrorOnFailure(reader.EnterContainer(outerType));

        EndpointId endpointId = kInvalidEndpointId;
        DeviceId deviceId;
        uint16_t deviceIdValue = 0;
        bool hasEndpointId     = false;
        bool hasDeviceId       = false;
        CHIP_ERROR err         = CHIP_NO_ERROR;
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
            case 2:
                ReturnErrorOnFailure(reader.Get(deviceIdValue));
                deviceId    = DeviceId(deviceIdValue);
                hasDeviceId = true;
                break;
            default:
                break;
            }
        }
        VerifyOrReturnError(err == CHIP_END_OF_TLV, err);
        ReturnErrorOnFailure(reader.ExitContainer(outerType));
        VerifyOrReturnError(hasEndpointId && hasDeviceId, CHIP_ERROR_INVALID_ARGUMENT);
        static_cast<void>(endpointId);

        auto device = mDeviceManager.GetDevice(deviceId);
        VerifyOrReturnError(device.has_value(), CHIP_ERROR_NOT_FOUND);
        const std::string deviceName = device->name;
        const bool wasRegistered     = device->isRegistered;
        mDeviceManager.UnregisterAndDestroyDevice(deviceId);
        ChipLogProgress(AppServer, "UnregisterAndDestroy succeeded: deviceId=%u name='%s' wasRegistered=%s", deviceId.value,
                        deviceName.c_str(), wasRegistered ? "true" : "false");
        return CHIP_NO_ERROR;
    }

private:
    using DeviceId = typename DeviceManager<DeviceFactoryT>::DeviceId;

    DeviceManager<DeviceFactoryT> & mDeviceManager;
};

} // namespace chip::app
