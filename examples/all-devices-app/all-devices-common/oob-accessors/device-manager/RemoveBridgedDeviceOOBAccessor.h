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

/**
 * Usage:
 *    Tag(1): EndpointId endpointId
 *       The endpointId of the bridged device to be removed. Must be a `Bridged Node` or a descendant of one.
 */

template <typename DeviceFactoryT>
class RemoveBridgedDeviceOOBAccessor : public OOBAccessor
{
public:
    explicit RemoveBridgedDeviceOOBAccessor(DeviceManager<DeviceFactoryT> & deviceManager) : mDeviceManager(deviceManager) {}

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

        EndpointId endpointId = kInvalidEndpointId;
        bool hasEndpointId = false;
        CHIP_ERROR err     = CHIP_NO_ERROR;
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
        static_cast<void>(endpointId);

        auto device = mDeviceManager.GetDevice(endpointId);
        VerifyOrReturnError(device.has_value(), CHIP_ERROR_NOT_FOUND);
        if (!device->isBridged)
        {
            ChipLogError(AppServer, "Device %s is not a bridged device", device->name.c_str());
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
        const std::string deviceName = device->name;
        mDeviceManager.RemoveDevice(endpointId);
        ChipLogProgress(AppServer, "RemoveBridgedDevice succeeded: name='%s' was removed from endpoint %u", deviceName.c_str(), endpointId);
        return CHIP_NO_ERROR;
    }

private:
    DeviceManager<DeviceFactoryT> & mDeviceManager;
};

} // namespace chip::app
