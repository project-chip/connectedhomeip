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

#pragma once

#include <app_config/enabled_devices.h>
#include <devices/Types.h>
#include <devices/boolean-state-sensor/BooleanStateSensorDevice.h>
#include <devices/chime/ChimeDevice.h>
#include <devices/dimmable-light/impl/LoggingDimmableLightDevice.h>
#include <devices/occupancy-sensor/impl/TogglingOccupancySensorDevice.h>
#include <devices/on-off-light/LoggingOnOffLightDevice.h>
#include <devices/soil-sensor/impl/IncreasingMoistureSoilSensorDevice.h>
#include <devices/speaker/impl/LoggingSpeakerDevice.h>
#include <devices/temperature-sensor/impl/IncreasingTemperatureSensorDevice.h>
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

    void RegisterCreator(const std::string & deviceTypeArg, DeviceCreator && creator)
    {
        if (mDefaultDevice.empty())
        {
            mDefaultDevice = deviceTypeArg;
        }
        mRegistry[deviceTypeArg] = std::move(creator);
    }

    const std::string & GetDefaultDevice() const { return mDefaultDevice; }

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
    std::string mDefaultDevice;

    DeviceFactory()
    {
        // NOTE: context is set in `::Init`, so each lambda checks its
        //       existence separately. `Init` must be called before mRegistry
        //       factories are usable.
        if constexpr (ALL_DEVICES_ENABLE_CONTACT_SENSOR)
        {
            RegisterCreator("contact-sensor", [this]() {
                VerifyOrDie(mContext.has_value());
                return std::make_unique<BooleanStateSensorDevice>(
                    &mContext->timerDelegate, Span<const DataModel::DeviceTypeEntry>(&Device::Type::kContactSensor, 1));
            });
        }
        if constexpr (ALL_DEVICES_ENABLE_WATER_LEAK_DETECTOR)
        {
            RegisterCreator("water-leak-detector", [this]() {
                VerifyOrDie(mContext.has_value());
                return std::make_unique<BooleanStateSensorDevice>(
                    &mContext->timerDelegate, Span<const DataModel::DeviceTypeEntry>(&Device::Type::kWaterLeakDetector, 1));
            });
        }
        if constexpr (ALL_DEVICES_ENABLE_OCCUPANCY_SENSOR)
        {
            RegisterCreator("occupancy-sensor", []() { return std::make_unique<TogglingOccupancySensorDevice>(); });
        }
        if constexpr (ALL_DEVICES_ENABLE_CHIME)
        {
            RegisterCreator("chime", [this]() {
                VerifyOrDie(mContext.has_value());
                static const ChimeDevice::Sound kDefaultSounds[] = {
                    { 0, "Ding Dong"_span },
                    { 1, "Ring Ring"_span },
                };
                return std::make_unique<ChimeDevice>(mContext->timerDelegate, Span<const ChimeDevice::Sound>(kDefaultSounds));
            });
        }
        if constexpr (ALL_DEVICES_ENABLE_DIMMABLE_LIGHT)
        {
            RegisterCreator("dimmable-light", [this]() {
                VerifyOrDie(mContext.has_value());
                return std::make_unique<LoggingDimmableLightDevice>(LoggingDimmableLightDevice::Context{
                    .groupDataProvider = mContext->groupDataProvider,
                    .fabricTable       = mContext->fabricTable,
                    .timerDelegate     = mContext->timerDelegate,
                });
            });
        }
        if constexpr (ALL_DEVICES_ENABLE_ON_OFF_LIGHT)
        {
            RegisterCreator("on-off-light", [this]() {
                VerifyOrDie(mContext.has_value());
                return std::make_unique<LoggingOnOffLightDevice>(LoggingOnOffLightDevice::Context{
                    .groupDataProvider = mContext->groupDataProvider,
                    .fabricTable       = mContext->fabricTable,
                    .timerDelegate     = mContext->timerDelegate,
                });
            });
        }
        if constexpr (ALL_DEVICES_ENABLE_SPEAKER)
        {
            RegisterCreator("speaker", [this]() {
                VerifyOrDie(mContext.has_value());
                return std::make_unique<LoggingSpeakerDevice>(
                    LoggingSpeakerDevice::Context{ .timerDelegate = mContext->timerDelegate });
            });
        }
        if constexpr (ALL_DEVICES_ENABLE_SOIL_SENSOR)
        {
            RegisterCreator("soil-sensor", []() { return std::make_unique<IncreasingMoistureSoilSensorDevice>(); });
        }
        if constexpr (ALL_DEVICES_ENABLE_TEMPERATURE_SENSOR)
        {
            RegisterCreator("temperature-sensor", []() { return std::make_unique<IncreasingTemperatureSensorDevice>(); });
        }

        // at least one device type MUST be enabled
        VerifyOrDie(!mRegistry.empty());
    }
};

} // namespace chip::app
