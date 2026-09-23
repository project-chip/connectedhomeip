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

#include <app_options/DeviceTypeParser.h>
#include <device-factory/DeviceManager.h>
#include <device/api/allocator/DynamicEndpointIdAllocator.h>
#include <lib/core/TLV.h>
#include <lib/support/logging/CHIPLogging.h>
#include <oob-accessors/OOBAccessor.h>

#include <string>
#include <vector>

namespace chip::app {

/**
 * Usage:
 *    Tag(1): EndpointId parentEndpointId
 *        Optional. An endpoint id of an existing device.
 *        If the device is `Bridged Node`, the new device will be added as a child of that device.
 *            `Bridged Node` at parentEndpointId
 *                    └── the new device
 *        If the device is `Aggregator`, a `Bridged Node` will be created as child, and the new device will be added as a child of
 * the latter device. `Aggregator` at parentEndpontId └── a new `Bridged Node` └── the new device If not a `Bridged Node` or
 * `Aggregator`, following layout will be created Some device at parentEndpointId └── a new `Aggregator` └── a new `Bridged Node`
 *                                    └── the new device
 *        If not present, will be same as if `kRootEndpointId` was specified.
 *        Otherwise the command will fail.
 *
 *    Tag(2): String device
 *        The device type name, e.g. "electrical-sensor". See `examples/all-devices-app/README.md` for the list of supported device
 * types.
 */

template <typename DeviceFactoryT>
class AddBridgedDeviceOOBAccessor : public OOBAccessor
{
public:
    AddBridgedDeviceOOBAccessor(DeviceManager<DeviceFactoryT> & deviceManager) : mDeviceManager(deviceManager) {}

    std::optional<CHIP_ERROR> HandleAction(CharSpan action, ByteSpan tlvData) override
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

        EndpointId parentEndpointId = kRootEndpointId;
        char deviceType[256]        = {};
        bool hasDeviceType          = false;
        CHIP_ERROR err              = CHIP_NO_ERROR;
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
                ReturnErrorOnFailure(reader.Get(parentEndpointId));
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

        auto VerifyDeviceWasAddedSuccessfully = [](const auto & device, const char * deviceType,
                                                   EndpointId parentEndpointId = kInvalidEndpointId) -> CHIP_ERROR {
            if (!device.has_value())
            {
                ChipLogError(AppServer, "Failed to add device %s", deviceType);
                return CHIP_ERROR_INCORRECT_STATE;
            }
            if (device->device.GetEndpointId() == kInvalidEndpointId)
            {
                ChipLogError(AppServer, "Possibly failed to register the device %s", deviceType);
                return CHIP_ERROR_INCORRECT_STATE;
            }
            ChipLogProgress(AppServer, "Device %s added successfully, endpoint: 0x%04X, parent: 0x%04X", deviceType,
                            device->device.GetEndpointId(), parentEndpointId);
            return CHIP_NO_ERROR;
        };

        auto device = mDeviceManager.GetDevice(parentEndpointId);
        if (parentEndpointId != kRootEndpointId && !device.has_value())
        {
            ChipLogError(AppServer, "Parent device with endpoint 0x%04X not found", parentEndpointId);
            return CHIP_ERROR_NOT_FOUND;
        }

        if (!device.has_value() || (device->name != "bridged-node" && device->name != "aggregator"))
        {
            // Need to create aggregator
            auto aggregatorDevice = mDeviceManager.AddDevice("aggregator", "", EndpointComposition::WithParent(parentEndpointId));
            ReturnErrorOnFailure(VerifyDeviceWasAddedSuccessfully(aggregatorDevice, "aggregator", parentEndpointId));
            device.emplace(*aggregatorDevice);
            parentEndpointId = device->device.GetEndpointId();
        }

        // the `device` now can hold only either `aggregator` or `bridged-node` device

        if (device->name != "bridged-node")
        {
            // Need to create bridged-node
            auto bridgedNodeDevice =
                mDeviceManager.AddDevice("bridged-node", "", EndpointComposition(parentEndpointId, DataModel::EndpointCompositionPattern::kFullFamily));
            ReturnErrorOnFailure(VerifyDeviceWasAddedSuccessfully(bridgedNodeDevice, "bridged-node", parentEndpointId));
            device.emplace(*bridgedNodeDevice);
            parentEndpointId = device->device.GetEndpointId();
        }

        auto finalDevice = mDeviceManager.AddDevice(deviceType, "", EndpointComposition::WithParent(parentEndpointId));
        ReturnErrorOnFailure(VerifyDeviceWasAddedSuccessfully(finalDevice, deviceType, parentEndpointId));

        return CHIP_NO_ERROR;
    }

private:
    DeviceManager<DeviceFactoryT> & mDeviceManager;
};

} // namespace chip::app
