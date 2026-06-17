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
#include <devices/aggregator/AggregatorDevice.h>
#include <devices/air-quality-sensor/AirQualitySensorDevice.h>
#include <devices/boolean-state-sensor/BooleanStateSensorDevice.h>
#include <devices/bridged-node/BridgedNodeDevice.h>
#include <devices/chime/ChimeDevice.h>
#include <devices/dimmable-light/impl/LoggingDimmableLightDevice.h>
#include <devices/fan/impl/LoggingFanDevice.h>
#include <devices/flow-sensor/impl/IncreasingFlowSensorDevice.h>
#include <devices/humidity-sensor/impl/IncreasingHumiditySensorDevice.h>
#include <devices/light-sensor/impl/IncreasingLightSensorDevice.h>
#include <devices/network-infrastructure-manager/NetworkInfrastructureManagerDevice.h>
#include <devices/occupancy-sensor/impl/TogglingOccupancySensorDevice.h>
#include <devices/on-off-light/LoggingOnOffLightDevice.h>
#include <devices/power-source/impl/DecreasingBatteryPowerSourceDevice.h>
#include <devices/pressure-sensor/impl/IncreasingPressureSensorDevice.h>
#include <devices/proximity-ranger/ProximityRangerDevice.h>
#include <devices/smoke-co-alarm/SmokeCoAlarmDevice.h>
#include <devices/soil-sensor/impl/IncreasingMoistureSoilSensorDevice.h>
#include <devices/speaker/impl/LoggingSpeakerDevice.h>
#include <devices/temperature-sensor/impl/IncreasingTemperatureSensorDevice.h>
#include <lib/core/CHIPError.h>
#include <lib/core/CHIPPersistentStorageDelegate.h>
#include <platform/DefaultTimerDelegate.h>

#include <functional>
#include <map>

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
    using DeviceCreator = std::function<std::unique_ptr<DeviceInterface>(const std::string & nodeLabel)>;

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

    /**
     * Convenience overload to support making the label optional for creator registrations
     * that do not care about the label (i.e. most cases).
     */
    void RegisterCreator(const std::string & deviceTypeArg, std::function<std::unique_ptr<DeviceInterface>()> && creator)
    {
        RegisterCreator(deviceTypeArg, [c = std::move(creator)](const std::string &) { return c(); });
    }

    const std::string & GetDefaultDevice() const { return mDefaultDevice; }

    bool IsValidDevice(const std::string & deviceTypeArg) { return mRegistry.find(deviceTypeArg) != mRegistry.end(); }

    std::unique_ptr<DeviceInterface> Create(const std::string & deviceTypeArg, const std::string & nodeLabel = "")
    {
        auto it = mRegistry.find(deviceTypeArg);
        if (it != mRegistry.end())
        {
            return it->second(nodeLabel);
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
        if constexpr (ALL_DEVICES_ENABLE_AGGREGATOR)
        {
            RegisterCreator("aggregator", [this]() {
                VerifyOrDie(mContext.has_value());
                return std::make_unique<AggregatorDevice>(mContext->timerDelegate);
            });
        }
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
        if constexpr (ALL_DEVICES_ENABLE_BRIDGED_NODE)
        {
            RegisterCreator("bridged-node", [this](const std::string & nodeLabel) {
                VerifyOrDie(mContext.has_value());
                static int sBridgedNodeCount = 0;
                sBridgedNodeCount++;
                std::string label = nodeLabel.empty() ? "Bridged Node " + std::to_string(sBridgedNodeCount) : nodeLabel;
                return std::make_unique<BridgedNodeDevice>(mContext->timerDelegate,
                                                           "bridged-node-unique-id-" + std::to_string(sBridgedNodeCount), label);
            });
        }
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
        if constexpr (ALL_DEVICES_ENABLE_FAN)
        {
            RegisterCreator("fan", [this]() {
                VerifyOrDie(mContext.has_value());
                return std::make_unique<LoggingFanDevice>(LoggingFanDevice::Context{
                    .groupDataProvider   = mContext->groupDataProvider,
                    .fabricTable         = mContext->fabricTable,
                    .timerDelegate       = mContext->timerDelegate,
                    .includeOnOffCluster = true,
                });
            });
            RegisterCreator("fan-no-onoff", [this]() {
                VerifyOrDie(mContext.has_value());
                return std::make_unique<LoggingFanDevice>(LoggingFanDevice::Context{
                    .groupDataProvider   = mContext->groupDataProvider,
                    .fabricTable         = mContext->fabricTable,
                    .timerDelegate       = mContext->timerDelegate,
                    .includeOnOffCluster = false,
                });
            });
        }

        if constexpr (ALL_DEVICES_ENABLE_PROXIMITY_RANGER)
        {
            RegisterCreator("proximity-ranger", [this]() {
                VerifyOrDie(mContext.has_value());
                return std::make_unique<ProximityRangerDevice>(mContext->timerDelegate,
                                                               Span<Clusters::ProximityRanging::RangingAdapter * const>());
            });
        }
        if constexpr (ALL_DEVICES_ENABLE_POWER_SOURCE)
        {
            RegisterCreator("power-source", []() { return std::make_unique<DecreasingBatteryPowerSourceDevice>(); });
        }
        if constexpr (ALL_DEVICES_ENABLE_SMOKE_CO_ALARM)
        {
            RegisterCreator("smoke-co-alarm", [this]() {
                VerifyOrDie(mContext.has_value());
                return std::make_unique<SmokeCoAlarmDevice>(
                    mContext->timerDelegate,
                    SmokeCoAlarmDevice::ConcentrationCluster::Config{
                        .clusterId = Clusters::CarbonMonoxideConcentrationMeasurement::Id,
                        .features  = BitFlags<Clusters::ConcentrationMeasurement::Feature>(
                            Clusters::ConcentrationMeasurement::Feature::kNumericMeasurement,
                            Clusters::ConcentrationMeasurement::Feature::kLevelIndication),
                        .medium = Clusters::ConcentrationMeasurement::MeasurementMediumEnum::kAir,
                        .unit   = Clusters::ConcentrationMeasurement::MeasurementUnitEnum::kPpm,
                    });
            });
        }

        if constexpr (ALL_DEVICES_ENABLE_RAIN_SENSOR)
        {
            RegisterCreator("rain-sensor", [this]() {
                VerifyOrDie(mContext.has_value());
                return std::make_unique<BooleanStateSensorDevice>(
                    &mContext->timerDelegate, Span<const DataModel::DeviceTypeEntry>(&Device::Type::kRainSensor, 1));
            });
        }
        if constexpr (ALL_DEVICES_ENABLE_WATER_FREEZE_DETECTOR)
        {
            RegisterCreator("water-freeze-detector", [this]() {
                VerifyOrDie(mContext.has_value());
                return std::make_unique<BooleanStateSensorDevice>(
                    &mContext->timerDelegate, Span<const DataModel::DeviceTypeEntry>(&Device::Type::kWaterFreezeDetector, 1));
            });
        }
        if constexpr (ALL_DEVICES_ENABLE_HUMIDITY_SENSOR)
        {
            RegisterCreator("humidity-sensor", [this]() {
                VerifyOrDie(mContext.has_value());
                return std::make_unique<IncreasingHumiditySensorDevice>(mContext->timerDelegate);
            });
        }
        if constexpr (ALL_DEVICES_ENABLE_LIGHT_SENSOR)
        {
            RegisterCreator("light-sensor", [this]() {
                VerifyOrDie(mContext.has_value());
                return std::make_unique<IncreasingLightSensorDevice>(mContext->timerDelegate);
            });
        }
        if constexpr (ALL_DEVICES_ENABLE_PRESSURE_SENSOR)
        {
            RegisterCreator("pressure-sensor", [this]() {
                VerifyOrDie(mContext.has_value());
                return std::make_unique<IncreasingPressureSensorDevice>(mContext->timerDelegate);
            });
        }
        if constexpr (ALL_DEVICES_ENABLE_FLOW_SENSOR)
        {
            RegisterCreator("flow-sensor", [this]() {
                VerifyOrDie(mContext.has_value());
                return std::make_unique<IncreasingFlowSensorDevice>(mContext->timerDelegate);
            });
        }

        // at least one device type MUST be enabled
        VerifyOrDie(!mRegistry.empty());
    }
};

} // namespace chip::app
