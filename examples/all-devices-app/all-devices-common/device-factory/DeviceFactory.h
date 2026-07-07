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
#include <devices/air-purifier/impl/LoggingAirPurifierDevice.h>
#include <devices/air-quality-sensor/AirQualitySensorDevice.h>
#include <devices/boolean-state-sensor/BooleanStateSensorDevice.h>
#include <devices/bridged-node/BridgedNodeDevice.h>
#include <devices/chime/ChimeDevice.h>
#include <devices/cooktop/impl/LoggingCooktopDevice.h>
#include <devices/device-energy-management/DeviceEnergyManagementDevice.h>
#include <devices/dimmable-light/impl/LoggingDimmableLightDevice.h>
#include <devices/dimmable-plug-in-unit/DimmablePlugInUnitDevice.h>
#include <devices/dishwasher/DishwasherDevice.h>
#include <devices/extractor-hood/ExtractorHoodDevice.h>
#include <devices/fan/impl/LoggingFanDevice.h>
#include <devices/flow-sensor/impl/IncreasingFlowSensorDevice.h>
#include <devices/generic-switch/GenericSwitchDevice.h>
#include <devices/humidity-sensor/impl/IncreasingHumiditySensorDevice.h>
#include <devices/laundry-dryer/LaundryDryerDevice.h>
#include <devices/laundry-washer/LaundryWasherDevice.h>
#include <devices/light-sensor/impl/IncreasingLightSensorDevice.h>
#include <devices/microwave-oven/MicrowaveOvenDevice.h>
#include <devices/mounted-dimmable-load-control/MountedDimmableLoadControlDevice.h>
#include <devices/mounted-on-off-control/MountedOnOffControlDevice.h>
#include <devices/network-infrastructure-manager/NetworkInfrastructureManagerDevice.h>
#include <devices/occupancy-sensor/impl/LoggingOccupancySensorDevice.h>
#include <devices/on-off-light-switch/OnOffLightSwitchDevice.h>
#include <devices/on-off-light/impl/LoggingOnOffLightDevice.h>
#include <devices/on-off-plug-in-unit/OnOffPlugInUnitDevice.h>
#include <devices/oven/impl/LoggingOvenDevice.h>
#include <devices/power-source/impl/DecreasingBatteryPowerSourceDevice.h>
#include <devices/pressure-sensor/impl/IncreasingPressureSensorDevice.h>
#include <devices/proximity-ranger/ProximityRangerDevice.h>
#include <devices/proximity-ranger/impl/LoggingProximityRangerDevice.h>
#include <devices/refrigerator/impl/LoggingRefrigeratorDevice.h>
#include <devices/robotic-vacuum-cleaner/RoboticVacuumCleanerDevice.h>
#include <devices/smoke-co-alarm/impl/LoggingOnlySmokeCoAlarmDevice.h>
#include <devices/soil-sensor/impl/IncreasingMoistureSoilSensorDevice.h>
#include <devices/speaker/impl/LoggingSpeakerDevice.h>
#include <devices/temperature-sensor/impl/IncreasingTemperatureSensorDevice.h>
#include <devices/water-valve/WaterValveDevice.h>
#include <lib/core/CHIPError.h>
#include <lib/core/CHIPPersistentStorageDelegate.h>
#include <platform/DefaultTimerDelegate.h>

#include <functional>
#include <map>
#include <oob-accessors/OOBAccessor.h>
#include <oob-accessors/boolean-state-sensor/BooleanStateSensorAccessor.h>

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
    using DeviceCreator         = std::function<std::unique_ptr<DeviceInterface>(const std::string & nodeLabel)>;
    using DeviceAccessorCreator = std::function<std::unique_ptr<OOBAccessor>(DeviceInterface &)>;

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

    void RegisterAccessorCreator(const std::string & deviceTypeArg, DeviceAccessorCreator && creator)
    {
        mAccessorRegistry[deviceTypeArg] = std::move(creator);
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

    std::unique_ptr<OOBAccessor> CreateAccessor(const std::string & deviceTypeArg, DeviceInterface & device)
    {
        if (IsValidDevice(deviceTypeArg) && mAccessorRegistry.find(deviceTypeArg) != mAccessorRegistry.end())
        {
            return mAccessorRegistry.find(deviceTypeArg)->second(device);
        }
        ChipLogProgress(Support, "No accessor found for device type: %s.\n", deviceTypeArg.c_str());
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
    std::map<std::string, DeviceAccessorCreator> mAccessorRegistry;

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
        if constexpr (ALL_DEVICES_ENABLE_AIR_PURIFIER)
        {
            RegisterCreator("air-purifier", [this]() {
                VerifyOrDie(mContext.has_value());
                return std::make_unique<LoggingAirPurifierDevice>(FanLoadDevice::Context{
                    .groupDataProvider = mContext->groupDataProvider,
                    .fabricTable       = mContext->fabricTable,
                    .timerDelegate     = mContext->timerDelegate,
                });
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
                    mContext->timerDelegate, Span<const DataModel::DeviceTypeEntry>(&Device::Type::kContactSensor, 1));
            });
            RegisterAccessorCreator("contact-sensor", [](DeviceInterface & device) {
                return std::make_unique<BooleanStateSensorAccessor>(static_cast<BooleanStateSensorDevice &>(device));
            });
        }
        if constexpr (ALL_DEVICES_ENABLE_WATER_LEAK_DETECTOR)
        {
            RegisterCreator("water-leak-detector", [this]() {
                VerifyOrDie(mContext.has_value());
                return std::make_unique<BooleanStateSensorDevice>(
                    mContext->timerDelegate, Span<const DataModel::DeviceTypeEntry>(&Device::Type::kWaterLeakDetector, 1));
            });
            RegisterAccessorCreator("water-leak-detector", [](DeviceInterface & device) {
                return std::make_unique<BooleanStateSensorAccessor>(static_cast<BooleanStateSensorDevice &>(device));
            });
        }
        if constexpr (ALL_DEVICES_ENABLE_OCCUPANCY_SENSOR)
        {
            RegisterCreator("occupancy-sensor", [this]() {
                VerifyOrDie(mContext.has_value());
                return std::make_unique<LoggingOccupancySensorDevice>(mContext->timerDelegate);
            });
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
        if constexpr (ALL_DEVICES_ENABLE_COOKTOP)
        {
            RegisterCreator("cooktop", [this]() {
                VerifyOrDie(mContext.has_value());
                return std::make_unique<LoggingCooktopDevice>(mContext->timerDelegate);
            });
        }
        if constexpr (ALL_DEVICES_ENABLE_DEVICE_ENERGY_MANAGEMENT)
        {
            RegisterCreator("device-energy-management", [this]() {
                VerifyOrDie(mContext.has_value());
                return std::make_unique<DeviceEnergyManagementDevice>(mContext->timerDelegate);
            });
        }
        if constexpr (ALL_DEVICES_ENABLE_DIMMABLE_LIGHT)
        {
            RegisterCreator("dimmable-light", [this]() {
                VerifyOrDie(mContext.has_value());
                return std::make_unique<LoggingDimmableLightDevice>(
                    LoggingDimmableLightDevice::Context{
                        .groupDataProvider = mContext->groupDataProvider,
                        .fabricTable       = mContext->fabricTable,
                        .timerDelegate     = mContext->timerDelegate,
                    },
                    DimmableLoadDevice::Config{ .levelControl = DimmableLoadDevice::LevelControlConfig::CiPicsDefaults() });
            });
        }
        if constexpr (ALL_DEVICES_ENABLE_DIMMABLE_PLUG_IN_UNIT)
        {
            RegisterCreator("dimmable-plug-in-unit", [this]() {
                VerifyOrDie(mContext.has_value());
                return std::make_unique<DimmablePlugInUnitDevice>(
                    LoggingDimmableLightDevice::Context{
                        .groupDataProvider = mContext->groupDataProvider,
                        .fabricTable       = mContext->fabricTable,
                        .timerDelegate     = mContext->timerDelegate,
                    },
                    DimmableLoadDevice::Config{ .levelControl = DimmableLoadDevice::LevelControlConfig::CiPicsDefaults() });
            });
        }
        if constexpr (ALL_DEVICES_ENABLE_DISHWASHER)
        {
            RegisterCreator("dishwasher", []() { return std::make_unique<DishwasherDevice>(); });
        }
        if constexpr (ALL_DEVICES_ENABLE_MOUNTED_DIMMABLE_LOAD_CONTROL)
        {
            RegisterCreator("mounted-dimmable-load-control", [this]() {
                VerifyOrDie(mContext.has_value());
                return std::make_unique<MountedDimmableLoadControlDevice>(
                    LoggingDimmableLightDevice::Context{
                        .groupDataProvider = mContext->groupDataProvider,
                        .fabricTable       = mContext->fabricTable,
                        .timerDelegate     = mContext->timerDelegate,
                    },
                    DimmableLoadDevice::Config{ .levelControl = DimmableLoadDevice::LevelControlConfig::CiPicsDefaults() });
            });
        }
        if constexpr (ALL_DEVICES_ENABLE_MOUNTED_ON_OFF_CONTROL)
        {
            RegisterCreator("mounted-on-off-control", [this]() {
                VerifyOrDie(mContext.has_value());
                return std::make_unique<MountedOnOffControlDevice>(LoggingOnOffLightDevice::Context{
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
        if constexpr (ALL_DEVICES_ENABLE_ON_OFF_LIGHT_SWITCH)
        {
            RegisterCreator("on-off-light-switch", [this]() {
                VerifyOrDie(mContext.has_value());
                return std::make_unique<OnOffLightSwitchDevice>(mContext->timerDelegate);
            });
        }
        if constexpr (ALL_DEVICES_ENABLE_ON_OFF_PLUG_IN_UNIT)
        {
            RegisterCreator("on-off-plug-in-unit", [this]() {
                VerifyOrDie(mContext.has_value());
                return std::make_unique<OnOffPlugInUnitDevice>(LoggingOnOffLightDevice::Context{
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
        if constexpr (ALL_DEVICES_ENABLE_OVEN)
        {
            RegisterCreator("oven", [this]() {
                VerifyOrDie(mContext.has_value());
                return std::make_unique<LoggingOvenDevice>(mContext->timerDelegate);
            });
        }
        if constexpr (ALL_DEVICES_ENABLE_REFRIGERATOR)
        {
            RegisterCreator("refrigerator", [this]() {
                VerifyOrDie(mContext.has_value());
                return std::make_unique<LoggingRefrigeratorDevice>(mContext->timerDelegate);
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
        if constexpr (ALL_DEVICES_ENABLE_EXTRACTOR_HOOD)
        {
            RegisterCreator("extractor-hood", [this]() {
                VerifyOrDie(mContext.has_value());
                return std::make_unique<ExtractorHoodDevice>(FanLoadDevice::Context{
                    .groupDataProvider   = mContext->groupDataProvider,
                    .fabricTable         = mContext->fabricTable,
                    .timerDelegate       = mContext->timerDelegate,
                    .includeOnOffCluster = true,
                });
            });
        }
        if constexpr (ALL_DEVICES_ENABLE_FAN)
        {
            RegisterCreator("fan", [this]() {
                VerifyOrDie(mContext.has_value());
                // Tagged with PositionTag::kTop to disambiguate from fan-no-onoff under wildcard allocation (*).
                static const Clusters::Globals::Structs::SemanticTagStruct::Type kFanTag = {
                    .mfgCode     = DataModel::NullNullable,
                    .namespaceID = CommonNamespace::kPositionId,
                    .tag         = static_cast<uint8_t>(Clusters::Globals::PositionTag::kTop),
                };
                return std::make_unique<LoggingFanDevice>(FanLoadDevice::Context{
                    .groupDataProvider   = mContext->groupDataProvider,
                    .fabricTable         = mContext->fabricTable,
                    .timerDelegate       = mContext->timerDelegate,
                    .includeOnOffCluster = true,
                    .tagList             = Span(&kFanTag, 1),
                });
            });
            RegisterCreator("fan-no-onoff", [this]() {
                VerifyOrDie(mContext.has_value());
                // Tagged with PositionTag::kBottom to disambiguate from fan (see comments above).
                static const Clusters::Globals::Structs::SemanticTagStruct::Type kFanNoOnOffTag = {
                    .mfgCode     = DataModel::NullNullable,
                    .namespaceID = CommonNamespace::kPositionId,
                    .tag         = static_cast<uint8_t>(Clusters::Globals::PositionTag::kBottom),
                };
                return std::make_unique<LoggingFanDevice>(FanLoadDevice::Context{
                    .groupDataProvider   = mContext->groupDataProvider,
                    .fabricTable         = mContext->fabricTable,
                    .timerDelegate       = mContext->timerDelegate,
                    .includeOnOffCluster = false,
                    .tagList             = Span(&kFanNoOnOffTag, 1),
                });
            });
        }
        if constexpr (ALL_DEVICES_ENABLE_GENERIC_SWITCH)
        {
            RegisterCreator("generic-switch", [this]() {
                VerifyOrDie(mContext.has_value());
                return std::make_unique<GenericSwitchDevice>(mContext->timerDelegate);
            });
        }

        if constexpr (ALL_DEVICES_ENABLE_PROXIMITY_RANGER)
        {
            RegisterCreator("proximity-ranger", [this]() {
                VerifyOrDie(mContext.has_value());
                return std::make_unique<LoggingProximityRangerDevice>(mContext->timerDelegate, mContext->storageDelegate);
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
                return std::make_unique<LoggingOnlySmokeCoAlarmDevice>(mContext->timerDelegate);
            });
        }

        if constexpr (ALL_DEVICES_ENABLE_RAIN_SENSOR)
        {
            RegisterCreator("rain-sensor", [this]() {
                VerifyOrDie(mContext.has_value());
                return std::make_unique<BooleanStateSensorDevice>(
                    mContext->timerDelegate, Span<const DataModel::DeviceTypeEntry>(&Device::Type::kRainSensor, 1));
            });
        }
        if constexpr (ALL_DEVICES_ENABLE_WATER_FREEZE_DETECTOR)
        {
            RegisterCreator("water-freeze-detector", [this]() {
                VerifyOrDie(mContext.has_value());
                return std::make_unique<BooleanStateSensorDevice>(
                    mContext->timerDelegate, Span<const DataModel::DeviceTypeEntry>(&Device::Type::kWaterFreezeDetector, 1));
            });
        }
        if constexpr (ALL_DEVICES_ENABLE_WATER_VALVE)
        {
            RegisterCreator("water-valve", [this]() {
                VerifyOrDie(mContext.has_value());
                return std::make_unique<WaterValveDevice>(mContext->timerDelegate);
            });
        }
        if constexpr (ALL_DEVICES_ENABLE_HUMIDITY_SENSOR)
        {
            RegisterCreator("humidity-sensor", [this]() {
                VerifyOrDie(mContext.has_value());
                return std::make_unique<IncreasingHumiditySensorDevice>(mContext->timerDelegate);
            });
        }
        if constexpr (ALL_DEVICES_ENABLE_LAUNDRY_DRYER)
        {
            RegisterCreator("laundry-dryer", []() { return std::make_unique<LaundryDryerDevice>(); });
        }
        if constexpr (ALL_DEVICES_ENABLE_LAUNDRY_WASHER)
        {
            RegisterCreator("laundry-washer", []() { return std::make_unique<LaundryWasherDevice>(); });
        }
        if constexpr (ALL_DEVICES_ENABLE_LIGHT_SENSOR)
        {
            RegisterCreator("light-sensor", [this]() {
                VerifyOrDie(mContext.has_value());
                return std::make_unique<IncreasingLightSensorDevice>(mContext->timerDelegate);
            });
        }
        if constexpr (ALL_DEVICES_ENABLE_MICROWAVE_OVEN)
        {
            RegisterCreator("microwave-oven", []() { return std::make_unique<MicrowaveOvenDevice>(); });
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
        if constexpr (ALL_DEVICES_ENABLE_ROBOTIC_VACUUM_CLEANER)
        {
            RegisterCreator("robotic-vacuum-cleaner", []() { return std::make_unique<RoboticVacuumCleanerDevice>(); });
        }

        // at least one device type MUST be enabled
        VerifyOrDie(!mRegistry.empty());
    }
};

} // namespace chip::app
