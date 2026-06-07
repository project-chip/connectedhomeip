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
#include <devices/air-quality-sensor/AirQualitySensorDevice.h>
#include <devices/boolean-state-sensor/BooleanStateSensorDevice.h>
#include <devices/chime/ChimeDevice.h>
#include <devices/dimmable-light/impl/LoggingDimmableLightDevice.h>
#include <devices/network-infrastructure-manager/NetworkInfrastructureManagerDevice.h>
#include <devices/occupancy-sensor/impl/TogglingOccupancySensorDevice.h>
#include <devices/on-off-light/LoggingOnOffLightDevice.h>
#include <devices/proximity-ranger/ProximityRangerDevice.h>
#include <devices/soil-sensor/impl/IncreasingMoistureSoilSensorDevice.h>
#include <devices/speaker/impl/LoggingSpeakerDevice.h>
#include <devices/temperature-sensor/impl/IncreasingTemperatureSensorDevice.h>
#include <functional>
#include <lib/core/CHIPError.h>
#include <lib/core/CHIPPersistentStorageDelegate.h>
#include <map>
#include <platform/DefaultTimerDelegate.h>

#if CHIP_ALL_DEVICES_APP_ENABLE_OOB_ACCESSORS
#include <accessors/common/SingleEndpointDeviceAccessor.h>
#include <devices/boolean-state-sensor/BooleanStateSensorAccessor.h>
#endif // CHIP_ALL_DEVICES_APP_ENABLE_OOB_ACCESSORS

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

#if CHIP_ALL_DEVICES_APP_ENABLE_OOB_ACCESSORS
    using DeviceAccessorCreator = std::function<std::unique_ptr<SingleEndpointDeviceAccessor>(DeviceInterface *)>;
#endif // CHIP_ALL_DEVICES_APP_ENABLE_OOB_ACCESSORS

    struct Context
    {
        Credentials::GroupDataProvider & groupDataProvider;
        FabricTable & fabricTable;
        TimerDelegate & timerDelegate;
        PersistentStorageDelegate & storageDelegate;
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

#if CHIP_ALL_DEVICES_APP_ENABLE_OOB_ACCESSORS
    void RegisterAccessorCreator(const std::string & deviceTypeArg, DeviceAccessorCreator && creator)
    {
        mAccessorRegistry[deviceTypeArg] = std::move(creator);
    }
#endif // CHIP_ALL_DEVICES_APP_ENABLE_OOB_ACCESSOR

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

#if CHIP_ALL_DEVICES_APP_ENABLE_OOB_ACCESSORS
    std::unique_ptr<SingleEndpointDeviceAccessor> CreateAccessor(const std::string & deviceTypeArg, DeviceInterface * device)
    {
        if (IsValidDevice(deviceTypeArg))
        {
            return mAccessorRegistry.find(deviceTypeArg)->second(device);
        }
        ChipLogProgress(Support, "No accessor found for device type: %s.\n", deviceTypeArg.c_str());
        return nullptr;
    }
#endif // CHIP_ALL_DEVICES_APP_ENABLE_OOB_ACCESSOR

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

#if CHIP_ALL_DEVICES_APP_ENABLE_OOB_ACCESSORS
    std::map<std::string, DeviceAccessorCreator> mAccessorRegistry;
#endif // CHIP_ALL_DEVICES_APP_ENABLE_OOB_ACCESSOR

    DeviceFactory()
    {
        // NOTE: context is set in `::Init`, so each lambda checks its
        //       existence separately. `Init` must be called before mRegistry
        //       factories are usable.
        if constexpr (ALL_DEVICES_ENABLE_AIR_QUALITY_SENSOR)
        {
            RegisterCreator("air-quality-sensor", [this]() {
                VerifyOrDie(mContext.has_value());
                using namespace Clusters::ConcentrationMeasurement;
                return std::make_unique<AirQualitySensorDevice>(
                    mContext->timerDelegate,
                    AirQualitySensorDevice::Config{
                        .airQualityFeatures = BitFlags<Clusters::AirQuality::Feature>(
                            Clusters::AirQuality::Feature::kFair, Clusters::AirQuality::Feature::kModerate,
                            Clusters::AirQuality::Feature::kVeryPoor, Clusters::AirQuality::Feature::kExtremelyPoor),
                        .co2Config =
                            ConcentrationMeasurementCluster::Config{
                                .clusterId = Clusters::CarbonDioxideConcentrationMeasurement::Id,
                                .features  = BitFlags<Feature>(Feature::kNumericMeasurement, Feature::kPeakMeasurement,
                                                              Feature::kAverageMeasurement, Feature::kLevelIndication),
                                .medium    = MeasurementMediumEnum::kAir,
                                .unit      = MeasurementUnitEnum::kPpm,
                            },
                    });
            });
        }
        if constexpr (ALL_DEVICES_ENABLE_CONTACT_SENSOR)
        {
            RegisterCreator("contact-sensor", [this]() {
                VerifyOrDie(mContext.has_value());
                return std::make_unique<BooleanStateSensorDevice>(
                    &mContext->timerDelegate, Span<const DataModel::DeviceTypeEntry>(&Device::Type::kContactSensor, 1));
            });
#if CHIP_ALL_DEVICES_APP_ENABLE_OOB_ACCESSORS
            RegisterAccessorCreator("contact-sensor", [this](DeviceInterface * device) {
                return std::make_unique<BooleanStateSensorAccessor>(static_cast<BooleanStateSensorDevice *>(device));
            });
#endif // CHIP_ALL_DEVICES_APP_ENABLE_OOB_ACCESSOR
        }
        if constexpr (ALL_DEVICES_ENABLE_WATER_LEAK_DETECTOR)
        {
            RegisterCreator("water-leak-detector", [this]() {
                VerifyOrDie(mContext.has_value());
                return std::make_unique<BooleanStateSensorDevice>(
                    &mContext->timerDelegate, Span<const DataModel::DeviceTypeEntry>(&Device::Type::kWaterLeakDetector, 1));
            });
#if CHIP_ALL_DEVICES_APP_ENABLE_OOB_ACCESSORS
            RegisterAccessorCreator("water-leak-detector", [this](DeviceInterface * device) {
                return std::make_unique<BooleanStateSensorAccessor>(static_cast<BooleanStateSensorDevice *>(device));
            });
#endif // CHIP_ALL_DEVICES_APP_ENABLE_OOB_ACCESSOR
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
        if constexpr (ALL_DEVICES_ENABLE_NETWORK_INFRASTRUCTURE_MANAGER)
        {
            RegisterCreator("network-infrastructure-manager", [this]() {
                VerifyOrDie(mContext.has_value());
                return std::make_unique<NetworkInfrastructureManagerDevice>(mContext->storageDelegate);
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

        if constexpr (ALL_DEVICES_ENABLE_PROXIMITY_RANGER)
        {
            RegisterCreator("proximity-ranger", [this]() {
                VerifyOrDie(mContext.has_value());
                return std::make_unique<ProximityRangerDevice>(mContext->timerDelegate,
                                                               Span<Clusters::ProximityRanging::RangingAdapter * const>());
            });
        }

        // at least one device type MUST be enabled
        VerifyOrDie(!mRegistry.empty());
    }
};

} // namespace chip::app
