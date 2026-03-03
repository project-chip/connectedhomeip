/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <devices/Types.h>
#include <devices/boolean-state-sensor/BooleanStateSensorDevice.h>
#include <devices/chime/impl/LoggingChimeDevice.h>
#include <devices/occupancy-sensor/impl/TogglingOccupancySensorDevice.h>
#include <devices/on-off-light/LoggingOnOffLightDevice.h>
#include <devices/speaker/impl/LoggingSpeakerDevice.h>
#include <functional>
#include <lib/core/CHIPError.h>
#include <map>
#include <platform/DefaultTimerDelegate.h>

namespace chip::app {

/**
 * This is a factory class made to be used to create any valid device type as part of the
 * all-devices-app. This class is meant to abstract away some details of device specific code,
 * and to have more generic implementation code being used in main to create a device. The keys
 * in the device registry map are the command line arguments used to start the respective device.
 * Create devices by fetching the instance of this class and passing in the device type argument
 * i.e. DeviceFactory::GetInstance().Create(deviceTypeName)
 */
class DeviceFactory
{
public:
    using DeviceCreator = std::function<std::unique_ptr<DeviceInterface>()>;

    struct Context
    {
        Credentials::GroupDataProvider & groupDataProvider;
        FabricTable & fabricTable;
        TimerDelegate & timerDelegate;
    };

    static DeviceFactory & GetInstance()
    {
        static DeviceFactory instance;
        return instance;
    }

    void Init(const Context & context) { mContext.emplace(context); }

    bool IsValidDevice(const std::string & deviceTypeArg) { return mRegistry.find(deviceTypeArg) != mRegistry.end(); }

    std::unique_ptr<DeviceInterface> Create(const std::string & deviceTypeArg)
    {
        if (IsValidDevice(deviceTypeArg))
        {
            return mRegistry.find(deviceTypeArg)->second();
        }
        ChipLogError(
            Support,
            "INTERNAL ERROR: Invalid device type: %s. Run with the --help argument to view the list of valid device types.\n",
            deviceTypeArg.c_str());
        return nullptr;
    }

    std::vector<std::string> SupportedDeviceTypes() const
    {
        std::vector<std::string> result;
        for (auto & item : mRegistry)
        {
            result.push_back(item.first);
        }
        return result;
    }

private:
    std::map<std::string, DeviceCreator> mRegistry;
    std::optional<Context> mContext;

    DeviceFactory()
    {
        // NOTE: context is set in `::Init`, so each lambda checks its
        //       existence separately. `Init` must be called before mRegistry
        //       factories are usable.
        mRegistry["contact-sensor"] = [this]() {
            VerifyOrDie(mContext.has_value());
            return std::make_unique<BooleanStateSensorDevice>(
                &mContext->timerDelegate, Span<const DataModel::DeviceTypeEntry>(&Device::Type::kContactSensor, 1));
        };
        mRegistry["water-leak-detector"] = [this]() {
            VerifyOrDie(mContext.has_value());
            return std::make_unique<BooleanStateSensorDevice>(
                &mContext->timerDelegate, Span<const DataModel::DeviceTypeEntry>(&Device::Type::kWaterLeakDetector, 1));
        };
        mRegistry["occupancy-sensor"] = []() { return std::make_unique<TogglingOccupancySensorDevice>(); };
        mRegistry["chime"]            = []() { return std::make_unique<LoggingChimeDevice>(); };
        mRegistry["on-off-light"]     = [this]() {
            VerifyOrDie(mContext.has_value());
            return std::make_unique<LoggingOnOffLightDevice>(LoggingOnOffLightDevice::Context{
                    .groupDataProvider = mContext->groupDataProvider,
                    .fabricTable       = mContext->fabricTable,
                    .timerDelegate     = mContext->timerDelegate,
            });
        };
        mRegistry["speaker"] = [this]() {
            VerifyOrDie(mContext.has_value());
            return std::make_unique<LoggingSpeakerDevice>(
                LoggingSpeakerDevice::Context{ .timerDelegate = mContext->timerDelegate });
        };
    }
};

} // namespace chip::app
