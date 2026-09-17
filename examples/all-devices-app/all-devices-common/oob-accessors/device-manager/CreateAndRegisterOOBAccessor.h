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
#include <oob-accessors/OOBAccessor.h>

#include <string>
#include <vector>

namespace chip::app {

template <typename DeviceFactoryT>
class CreateAndRegisterOOBAccessor : public OOBAccessor
{
public:
    CreateAndRegisterOOBAccessor(DeviceManager<DeviceFactoryT> & deviceManager, EndpointIdAllocator & endpointIdAllocator) :
        mDeviceManager(deviceManager), mEndpointIdAllocator(endpointIdAllocator)
    {}

    std::optional<CHIP_ERROR> HandleAction(CharSpan action, ByteSpan tlvData) override
    {
        if (!action.data_equal("CreateAndRegister"_span))
        {
            return std::nullopt;
        }

        TLV::TLVReader reader;
        reader.Init(tlvData);
        ReturnErrorOnFailure(reader.Next(TLV::kTLVType_Structure, TLV::AnonymousTag()));

        TLV::TLVType outerType;
        ReturnErrorOnFailure(reader.EnterContainer(outerType));

        EndpointId endpointId = kInvalidEndpointId;
        char device[256]      = {};
        bool hasEndpointId    = false;
        bool hasDevice        = false;
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
            case 1:
                ReturnErrorOnFailure(reader.Get(endpointId));
                hasEndpointId = true;
                break;
            case 2:
                ReturnErrorOnFailure(reader.GetString(device, sizeof(device)));
                hasDevice = true;
                break;
            default:
                break;
            }
        }
        VerifyOrReturnError(err == CHIP_END_OF_TLV, err);
        ReturnErrorOnFailure(reader.ExitContainer(outerType));
        VerifyOrReturnError(hasEndpointId && hasDevice, CHIP_ERROR_INVALID_ARGUMENT);
        static_cast<void>(endpointId);

        DeviceTypeParser temporaryDeviceTypeParser;
        ReturnErrorOnFailure(temporaryDeviceTypeParser.ParseSingleDeviceString(device));

        std::vector<std::string> supportedDeviceTypes;
        for (const auto & deviceType : DeviceFactoryT::GetInstance().SupportedDeviceTypes())
        {
            if (deviceType != "aggregator" && deviceType != "bridged-node")
            {
                supportedDeviceTypes.push_back(deviceType);
            }
        }
        temporaryDeviceTypeParser.ExpandWildcards(supportedDeviceTypes);

        auto & deviceTypeParser = DeviceTypeParser::GetInstance();
        const auto & permanentDeviceTypeEntries = deviceTypeParser.GetDeviceTypeEntries();
        const auto & temporaryDeviceTypeEntries = temporaryDeviceTypeParser.GetDeviceTypeEntries();
        std::vector<DeviceTypeParser::Entry> combinedDeviceTypeEntries;
        combinedDeviceTypeEntries.reserve(permanentDeviceTypeEntries.size() + temporaryDeviceTypeEntries.size());
        combinedDeviceTypeEntries.insert(combinedDeviceTypeEntries.end(), permanentDeviceTypeEntries.begin(),
                                          permanentDeviceTypeEntries.end());
        combinedDeviceTypeEntries.insert(combinedDeviceTypeEntries.end(), temporaryDeviceTypeEntries.begin(),
                                          temporaryDeviceTypeEntries.end());
        ReturnErrorOnFailure(DeviceTypeParser::ValidateConfig(combinedDeviceTypeEntries));

        for (const auto & deviceEntry : temporaryDeviceTypeEntries)
        {
            deviceTypeParser.AddDeviceTypeEntry(deviceEntry);
        }

        for (const auto & deviceEntry : temporaryDeviceTypeEntries)
        {
            if (deviceEntry.endpoint != kInvalidEndpointId)
            {
                auto * dynamicAllocator = static_cast<DynamicEndpointIdAllocator *>(&mEndpointIdAllocator);
                dynamicAllocator->ForceNext(deviceEntry.endpoint);
            }
            VerifyOrReturnError(mDeviceManager.CreateAndRegisterDevice(
                                    deviceEntry.type, mEndpointIdAllocator, deviceEntry.label,
                                    EndpointComposition::WithParent(deviceEntry.parentId))
                                    .has_value(),
                                CHIP_ERROR_INTERNAL);
        }

        return CHIP_NO_ERROR;
    }

private:
    DeviceManager<DeviceFactoryT> & mDeviceManager;
    EndpointIdAllocator & mEndpointIdAllocator;
};

} // namespace chip::app
