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

#include <app/FailSafeContext.h>
#include <app/clusters/bindings/BindingManager.h>
#include <app/clusters/bindings/binding-table.h>
#include <app_config/enabled_devices.h>
#include <device/types/aggregator/Aggregator.h>
#include <device/types/air-purifier/impl/LoggingAirPurifier.h>
#include <device/types/air-quality-sensor/AirQualitySensor.h>
#include <device/types/ambient-context-sensor/impl/LoggingAmbientContextSensor.h>
#include <device/types/boolean-state-sensor/BooleanStateSensor.h>
#include <device/types/bridged-node/BridgedNode.h>
#include <device/types/chime/Chime.h>
#include <device/types/cooktop/impl/LoggingCooktop.h>
#include <device/types/device-energy-management/EnergyManagement.h>
#include <device/types/dimmable-light/impl/LoggingDimmableLight.h>
#include <device/types/dimmable-plug-in-unit/DimmablePlugInUnit.h>
#include <device/types/dishwasher/impl/EmulatedDishwasher.h>
#include <device/types/electrical-sensor/impl/SimulatedElectricalSensor.h>
#include <device/types/extractor-hood/ExtractorHood.h>
#include <device/types/fan/impl/LoggingFan.h>
#include <device/types/flow-sensor/impl/IncreasingFlowSensor.h>
#include <device/types/generic-switch/GenericSwitch.h>
#include <device/types/humidity-sensor/impl/IncreasingHumiditySensor.h>
#include <device/types/laundry-dryer/impl/EmulatedLaundryDryer.h>
#include <device/types/laundry-washer/impl/EmulatedLaundryWasher.h>
#include <device/types/light-sensor/impl/IncreasingLightSensor.h>
#include <device/types/microwave-oven/impl/EmulatedMicrowaveOven.h>
#include <device/types/mounted-dimmable-load-control/MountedDimmableLoadControl.h>
#include <device/types/mounted-on-off-control/MountedOnOffControl.h>
#include <device/types/network-infrastructure-manager/NetworkInfrastructureManager.h>
#include <device/types/occupancy-sensor/impl/LoggingOccupancySensor.h>
#include <device/types/on-off-light-switch/OnOffLightSwitch.h>
#include <device/types/on-off-light/impl/LoggingOnOffLight.h>
#include <device/types/on-off-plug-in-unit/OnOffPlugInUnit.h>
#include <device/types/oven/impl/LoggingOven.h>
#include <device/types/power-source/impl/DecreasingBatteryPowerSource.h>
#include <device/types/pressure-sensor/impl/IncreasingPressureSensor.h>
#include <device/types/proximity-ranger/ProximityRanger.h>
#include <device/types/proximity-ranger/impl/LoggingProximityRanger.h>
#include <device/types/refrigerator/impl/LoggingRefrigerator.h>
#include <device/types/robotic-vacuum-cleaner/RoboticVacuumCleaner.h>
#include <device/types/smoke-co-alarm/impl/LoggingOnlySmokeCoAlarm.h>
#include <device/types/soil-sensor/impl/IncreasingMoistureSoilSensor.h>
#include <device/types/speaker/impl/LoggingSpeaker.h>
#include <device/types/temperature-sensor/impl/IncreasingTemperatureSensor.h>
#include <device/types/water-valve/WaterValve.h>
#include <devices/Types.h>
#include <lib/core/CHIPError.h>
#include <lib/core/CHIPPersistentStorageDelegate.h>
#include <platform/DefaultTimerDelegate.h>
#include <platform/DiagnosticDataProvider.h>
#include <platform/PlatformManager.h>

#include <functional>
#include <map>

namespace chip::app {

/**
 * @brief Centralized factory registry for instantiating Matter devices in all-devices-app.
 *
 * This class abstracts away concrete device construction and wires device types (e.g., "on-off-light",
 * "occupancy-sensor") from command-line arguments or configuration strings to concrete C++ implementations.
 *
 * ### Variadic Hook Architecture (`template <typename... Hooks>`)
 *
 * `DeviceFactory` supports zero or more compile-time static hooks. Hooks allow platforms and transports
 * (e.g. Out-of-Band TLV accessors, POSIX named pipes, UI controllers, Pigweed RPC) to attach device-specific
 * capabilities during post-registration without coupling core device implementations to platform code or
 * introducing runtime overhead.
 *
 * - When no hooks are needed (e.g. resource-constrained embedded targets), use @ref SimpleDeviceFactory
 *   (`DeviceFactory<>`). In this mode, `MakeOnDeviceRegisteredCallback` compiles to a no-op returning `nullptr`.
 * - When one or more hooks are supplied (e.g. `DeviceFactory<OOBAccessorHook, NamedPipeHook>`), each hook's
 *   static `OnDeviceRegistered(*device)` method is invoked in order via C++17 fold expressions once the device is created.
 *
 * ### Lifecycle & Data Flow
 *
 * ```
 * +-------------------------------------------------------------------------+
 * | 1. Initialize Context (Main / Startup)                                  |
 * |    using AppFactory = DeviceFactory<OOBAccessorHook, NamedPipeHook>;    |
 * |    AppFactory::GetInstance().Init(context);                             |
 * +-------------------------------------------------------------------------+
 *                                    |
 *                                    v
 * +-------------------------------------------------------------------------+
 * | 2. Instantiate Device                                                   |
 * |    auto created = AppFactory::GetInstance().Create(deviceTypeArg);      |
 * |    // created.device -> std::unique_ptr<DeviceInterface>                |
 * |    // created.onDeviceRegistered -> static hook fold invoker      |
 * +-------------------------------------------------------------------------+
 *                                    |
 *                                    v
 * +-------------------------------------------------------------------------+
 * | 3. Register in Data Model                                               |
 * |    created.device->Register(allocator, dataModelProvider);              |
 * +-------------------------------------------------------------------------+
 *                                    |
 *                                    v
 * +-------------------------------------------------------------------------+
 * | 4. Invoke Post-Registration Hooks                                       |
 * |    if (created.onDeviceRegistered) {                              |
 * |        created.onDeviceRegistered();                              |
 * |        // Calls (Hooks::OnDeviceRegistered(*concreteDevice), ...)                 |
 * |    }                                                                    |
 * +-------------------------------------------------------------------------+
 * ```
 *
 * ### Example Usage
 *
 * Standard (Embedded / No-Hooks):
 * @code
 * using Factory = chip::app::SimpleDeviceFactory;
 * Factory::GetInstance().Init(context);
 * auto created = Factory::GetInstance().Create("on-off-light");
 * created.device->Register(allocator, dataModelProvider);
 * @endcode
 *
 * POSIX (With OOB Accessors and Named Pipes):
 * @code
 * using PosixFactory = chip::app::DeviceFactory<OOBAccessorHook, NamedPipeHook>;
 * PosixFactory::GetInstance().Init(context);
 * auto created = PosixFactory::GetInstance().Create("ambient-context-sensor");
 * created.device->Register(allocator, dataModelProvider);
 * if (created.onDeviceRegistered)
 * {
 *     created.onDeviceRegistered();
 * }
 * @endcode
 *
 * ### Implementing a Custom Hook
 *
 * A hook class must provide a static `OnDeviceRegistered` template function:
 * @code
 * struct CustomUIHook
 * {
 *     template <typename TDevice>
 *     static void OnDeviceRegistered(TDevice & device)
 *     {
 *         if constexpr (detail::HasCustomUI<TDevice>::value)
 *         {
 *             RegisterDeviceUI(device);
 *         }
 *     }
 * };
 * @endcode
 */
template <typename... Hooks>
class DeviceFactory
{
public:
    struct CreatedDevice
    {
        std::unique_ptr<DeviceInterface> device;
        std::function<void()> onDeviceRegistered;
    };

    template <typename TDevice>
    static void ExecuteHooks(TDevice & device)
    {
        if constexpr (sizeof...(Hooks) > 0)
        {
            (Hooks::OnDeviceRegistered(device), ...);
        }
    }

    template <typename TDevice>
    static std::function<void()> MakeOnDeviceRegisteredCallback(TDevice * device)
    {
        if constexpr (sizeof...(Hooks) == 0)
        {
            return nullptr;
        }
        else
        {
            return [device]() {
                if (device != nullptr)
                {
                    ExecuteHooks(*device);
                }
            };
        }
    }

    template <typename TDevice, typename... Args>
    static CreatedDevice MakeCreatedDevice(Args &&... args)
    {
        auto dev   = std::make_unique<TDevice>(std::forward<Args>(args)...);
        auto * raw = dev.get();
        return CreatedDevice{
            .device             = std::move(dev),
            .onDeviceRegistered = MakeOnDeviceRegisteredCallback(raw),
        };
    }

    using DeviceCreator = std::function<CreatedDevice(const std::string & nodeLabel)>;

    struct Context
    {
        Credentials::GroupDataProvider & groupDataProvider;
        FabricTable & fabricTable;
        TimerDelegate & timerDelegate;
        PersistentStorageDelegate & storageDelegate;
        DeviceLayer::DiagnosticDataProvider & diagnosticDataProvider;
        DeviceLayer::PlatformManager & platformManager;
        FailSafeContext & failSafeContext;
        Clusters::Binding::Table & bindingTable;
        Clusters::Binding::Manager & bindingManager;
        TestEventTriggerDelegate & testEventTriggerDelegate;
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

    void RegisterCreator(const std::string & deviceTypeArg, std::function<CreatedDevice()> && creator)
    {
        RegisterCreator(deviceTypeArg, [c = std::move(creator)](const std::string &) { return c(); });
    }

    void RegisterCreator(const std::string & deviceTypeArg,
                         std::function<std::unique_ptr<DeviceInterface>(const std::string &)> && creator)
    {
        RegisterCreator(deviceTypeArg, [c = std::move(creator)](const std::string & label) {
            return CreatedDevice{ .device = c(label), .onDeviceRegistered = nullptr };
        });
    }

    void RegisterCreator(const std::string & deviceTypeArg, std::function<std::unique_ptr<DeviceInterface>()> && creator)
    {
        RegisterCreator(deviceTypeArg, [c = std::move(creator)](const std::string &) {
            return CreatedDevice{ .device = c(), .onDeviceRegistered = nullptr };
        });
    }

    const std::string & GetDefaultDevice() const { return mDefaultDevice; }

    bool IsValidDevice(const std::string & deviceTypeArg) const { return mRegistry.find(deviceTypeArg) != mRegistry.end(); }

    CreatedDevice Create(const std::string & deviceTypeArg, const std::string & nodeLabel = "") const
    {
        auto it = mRegistry.find(deviceTypeArg);
        if (it != mRegistry.end())
        {
            return it->second(nodeLabel);
        }
        ChipLogError(
            AppServer,
            "INTERNAL ERROR: Invalid device type: %s. Run with the --help argument to view the list of valid device types.",
            deviceTypeArg.c_str());
        return CreatedDevice{ nullptr, nullptr };
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
                return MakeCreatedDevice<Aggregator>(mContext->timerDelegate);
            });
        }
        if constexpr (ALL_DEVICES_ENABLE_AIR_PURIFIER)
        {
            RegisterCreator("air-purifier", [this]() {
                VerifyOrDie(mContext.has_value());
                return MakeCreatedDevice<LoggingAirPurifier>(FanLoad::Context{
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
                return MakeCreatedDevice<AirQualitySensor>(
                    mContext->timerDelegate,
                    AirQualitySensor::Config{
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
        if constexpr (ALL_DEVICES_ENABLE_AMBIENT_CONTEXT_SENSOR)
        {
            RegisterCreator("ambient-context-sensor", [this]() {
                VerifyOrDie(mContext.has_value());
                return MakeCreatedDevice<Clusters::AmbientContextSensing::LoggingAmbientContextSensor>(mContext->timerDelegate);
            });
        }
        if constexpr (ALL_DEVICES_ENABLE_BRIDGED_NODE)
        {
            RegisterCreator("bridged-node", [this](const std::string & nodeLabel) {
                VerifyOrDie(mContext.has_value());
                static int sBridgedNodeCount = 0;
                sBridgedNodeCount++;
                std::string label = nodeLabel.empty() ? "Bridged Node " + std::to_string(sBridgedNodeCount) : nodeLabel;
                return MakeCreatedDevice<BridgedNode>(mContext->timerDelegate,
                                                      "bridged-node-unique-id-" + std::to_string(sBridgedNodeCount), label);
            });
        }
        if constexpr (ALL_DEVICES_ENABLE_CONTACT_SENSOR)
        {
            RegisterCreator("contact-sensor", [this]() {
                VerifyOrDie(mContext.has_value());
                return MakeCreatedDevice<BooleanStateSensor>(
                    mContext->timerDelegate, Span<const DataModel::DeviceTypeEntry>(&Device::Type::kContactSensor, 1));
            });
        }
        if constexpr (ALL_DEVICES_ENABLE_WATER_LEAK_DETECTOR)
        {
            RegisterCreator("water-leak-detector", [this]() {
                VerifyOrDie(mContext.has_value());
                return MakeCreatedDevice<BooleanStateSensor>(
                    mContext->timerDelegate, Span<const DataModel::DeviceTypeEntry>(&Device::Type::kWaterLeakDetector, 1));
            });
        }
        if constexpr (ALL_DEVICES_ENABLE_OCCUPANCY_SENSOR)
        {
            RegisterCreator("occupancy-sensor", [this]() {
                VerifyOrDie(mContext.has_value());
                return MakeCreatedDevice<LoggingOccupancySensor>(mContext->timerDelegate);
            });
        }
        if constexpr (ALL_DEVICES_ENABLE_CHIME)
        {
            RegisterCreator("chime", [this]() {
                VerifyOrDie(mContext.has_value());
                static const Chime::Sound kDefaultSounds[] = {
                    { 0, "Ding Dong"_span },
                    { 1, "Ring Ring"_span },
                };
                return MakeCreatedDevice<Chime>(mContext->timerDelegate, Span<const Chime::Sound>(kDefaultSounds));
            });
        }
        if constexpr (ALL_DEVICES_ENABLE_COOKTOP)
        {
            RegisterCreator("cooktop", [this]() {
                VerifyOrDie(mContext.has_value());
                return MakeCreatedDevice<LoggingCooktop>(mContext->timerDelegate);
            });
        }
        if constexpr (ALL_DEVICES_ENABLE_DEVICE_ENERGY_MANAGEMENT)
        {
            RegisterCreator("device-energy-management", [this]() {
                VerifyOrDie(mContext.has_value());
                return MakeCreatedDevice<EnergyManagement>(mContext->timerDelegate);
            });
        }
        if constexpr (ALL_DEVICES_ENABLE_DIMMABLE_LIGHT)
        {
            RegisterCreator("dimmable-light", [this]() {
                VerifyOrDie(mContext.has_value());
                return MakeCreatedDevice<LoggingDimmableLight>(
                    LoggingDimmableLight::Context{
                        .groupDataProvider = mContext->groupDataProvider,
                        .fabricTable       = mContext->fabricTable,
                        .timerDelegate     = mContext->timerDelegate,
                    },
                    DimmableLoad::Config{ .levelControl = DimmableLoad::LevelControlConfig::CiPicsDefaults() });
            });
        }
        if constexpr (ALL_DEVICES_ENABLE_DIMMABLE_PLUG_IN_UNIT)
        {
            RegisterCreator("dimmable-plug-in-unit", [this]() {
                VerifyOrDie(mContext.has_value());
                return MakeCreatedDevice<DimmablePlugInUnit>(
                    LoggingDimmableLight::Context{
                        .groupDataProvider = mContext->groupDataProvider,
                        .fabricTable       = mContext->fabricTable,
                        .timerDelegate     = mContext->timerDelegate,
                    },
                    DimmableLoad::Config{ .levelControl = DimmableLoad::LevelControlConfig::CiPicsDefaults() });
            });
        }
        if constexpr (ALL_DEVICES_ENABLE_DISHWASHER)
        {
            RegisterCreator("dishwasher", [this]() {
                VerifyOrDie(mContext.has_value());
                return MakeCreatedDevice<EmulatedDishwasher>(EmulatedDishwasher::Context{
                    .timerDelegate          = mContext->timerDelegate,
                    .diagnosticDataProvider = mContext->diagnosticDataProvider,
                });
            });
        }
        if constexpr (ALL_DEVICES_ENABLE_MOUNTED_DIMMABLE_LOAD_CONTROL)
        {
            RegisterCreator("mounted-dimmable-load-control", [this]() {
                VerifyOrDie(mContext.has_value());
                return MakeCreatedDevice<MountedDimmableLoadControl>(
                    LoggingDimmableLight::Context{
                        .groupDataProvider = mContext->groupDataProvider,
                        .fabricTable       = mContext->fabricTable,
                        .timerDelegate     = mContext->timerDelegate,
                    },
                    DimmableLoad::Config{ .levelControl = DimmableLoad::LevelControlConfig::CiPicsDefaults() });
            });
        }
        if constexpr (ALL_DEVICES_ENABLE_MOUNTED_ON_OFF_CONTROL)
        {
            RegisterCreator("mounted-on-off-control", [this]() {
                VerifyOrDie(mContext.has_value());
                return MakeCreatedDevice<MountedOnOffControl>(LoggingOnOffLight::Context{
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
                return MakeCreatedDevice<NetworkInfrastructureManager>(mContext->timerDelegate, mContext->storageDelegate,
                                                                       mContext->platformManager, mContext->failSafeContext);
            });
        }
        if constexpr (ALL_DEVICES_ENABLE_ON_OFF_LIGHT)
        {
            RegisterCreator("on-off-light", [this]() {
                VerifyOrDie(mContext.has_value());
                return MakeCreatedDevice<LoggingOnOffLight>(LoggingOnOffLight::Context{
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
                return MakeCreatedDevice<OnOffLightSwitch>(mContext->timerDelegate, mContext->platformManager,
                                                           mContext->bindingTable, mContext->bindingManager);
            });
        }
        if constexpr (ALL_DEVICES_ENABLE_ON_OFF_PLUG_IN_UNIT)
        {
            RegisterCreator("on-off-plug-in-unit", [this]() {
                VerifyOrDie(mContext.has_value());
                return MakeCreatedDevice<OnOffPlugInUnit>(LoggingOnOffLight::Context{
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
                return MakeCreatedDevice<LoggingSpeaker>(LoggingSpeaker::Context{ .timerDelegate = mContext->timerDelegate });
            });
        }
        if constexpr (ALL_DEVICES_ENABLE_OVEN)
        {
            RegisterCreator("oven", [this]() {
                VerifyOrDie(mContext.has_value());
                return MakeCreatedDevice<LoggingOven>(mContext->timerDelegate);
            });
        }
        if constexpr (ALL_DEVICES_ENABLE_REFRIGERATOR)
        {
            RegisterCreator("refrigerator", [this]() {
                VerifyOrDie(mContext.has_value());
                return MakeCreatedDevice<LoggingRefrigerator>(mContext->timerDelegate);
            });
        }
        if constexpr (ALL_DEVICES_ENABLE_SOIL_SENSOR)
        {
            RegisterCreator("soil-sensor", []() { return MakeCreatedDevice<IncreasingMoistureSoilSensor>(); });
        }
        if constexpr (ALL_DEVICES_ENABLE_TEMPERATURE_SENSOR)
        {
            RegisterCreator("temperature-sensor", []() { return MakeCreatedDevice<IncreasingTemperatureSensor>(); });
        }
        if constexpr (ALL_DEVICES_ENABLE_ELECTRICAL_SENSOR)
        {
            RegisterCreator("electrical-sensor", [this]() {
                VerifyOrDie(mContext.has_value());
                return MakeCreatedDevice<SimulatedElectricalSensor>(mContext->timerDelegate, mContext->testEventTriggerDelegate);
            });
        }
        if constexpr (ALL_DEVICES_ENABLE_EXTRACTOR_HOOD)
        {
            RegisterCreator("extractor-hood", [this]() {
                VerifyOrDie(mContext.has_value());
                return MakeCreatedDevice<ExtractorHood>(FanLoad::Context{
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
                return MakeCreatedDevice<LoggingFan>(FanLoad::Context{
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
                return MakeCreatedDevice<LoggingFan>(FanLoad::Context{
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
                return MakeCreatedDevice<GenericSwitch>(mContext->timerDelegate);
            });
        }

        if constexpr (ALL_DEVICES_ENABLE_PROXIMITY_RANGER)
        {
            RegisterCreator("proximity-ranger", [this]() {
                VerifyOrDie(mContext.has_value());
                return MakeCreatedDevice<LoggingProximityRanger>(mContext->timerDelegate, mContext->storageDelegate);
            });
        }
        if constexpr (ALL_DEVICES_ENABLE_POWER_SOURCE)
        {
            RegisterCreator("power-source", []() { return MakeCreatedDevice<DecreasingBatteryPowerSource>(); });
        }
        if constexpr (ALL_DEVICES_ENABLE_SMOKE_CO_ALARM)
        {
            RegisterCreator("smoke-co-alarm", [this]() {
                VerifyOrDie(mContext.has_value());
                return MakeCreatedDevice<LoggingOnlySmokeCoAlarm>(mContext->timerDelegate);
            });
        }

        if constexpr (ALL_DEVICES_ENABLE_RAIN_SENSOR)
        {
            RegisterCreator("rain-sensor", [this]() {
                VerifyOrDie(mContext.has_value());
                return MakeCreatedDevice<BooleanStateSensor>(mContext->timerDelegate,
                                                             Span<const DataModel::DeviceTypeEntry>(&Device::Type::kRainSensor, 1));
            });
        }
        if constexpr (ALL_DEVICES_ENABLE_WATER_FREEZE_DETECTOR)
        {
            RegisterCreator("water-freeze-detector", [this]() {
                VerifyOrDie(mContext.has_value());
                return MakeCreatedDevice<BooleanStateSensor>(
                    mContext->timerDelegate, Span<const DataModel::DeviceTypeEntry>(&Device::Type::kWaterFreezeDetector, 1));
            });
        }
        if constexpr (ALL_DEVICES_ENABLE_WATER_VALVE)
        {
            RegisterCreator("water-valve", [this]() {
                VerifyOrDie(mContext.has_value());
                return MakeCreatedDevice<WaterValve>(mContext->timerDelegate);
            });
        }
        if constexpr (ALL_DEVICES_ENABLE_HUMIDITY_SENSOR)
        {
            RegisterCreator("humidity-sensor", [this]() {
                VerifyOrDie(mContext.has_value());
                return MakeCreatedDevice<IncreasingHumiditySensor>(mContext->timerDelegate);
            });
        }
        if constexpr (ALL_DEVICES_ENABLE_LAUNDRY_DRYER)
        {
            RegisterCreator("laundry-dryer", [this]() {
                VerifyOrDie(mContext.has_value());
                return MakeCreatedDevice<EmulatedLaundryDryer>(mContext->timerDelegate);
            });
        }
        if constexpr (ALL_DEVICES_ENABLE_LAUNDRY_WASHER)
        {
            RegisterCreator("laundry-washer", [this]() {
                VerifyOrDie(mContext.has_value());
                return MakeCreatedDevice<EmulatedLaundryWasher>(EmulatedLaundryWasher::Context{
                    .timerDelegate          = mContext->timerDelegate,
                    .diagnosticDataProvider = mContext->diagnosticDataProvider,
                });
            });
        }
        if constexpr (ALL_DEVICES_ENABLE_LIGHT_SENSOR)
        {
            RegisterCreator("light-sensor", [this]() {
                VerifyOrDie(mContext.has_value());
                return MakeCreatedDevice<IncreasingLightSensor>(mContext->timerDelegate);
            });
        }
        if constexpr (ALL_DEVICES_ENABLE_MICROWAVE_OVEN)
        {
            RegisterCreator("microwave-oven", [this]() {
                VerifyOrDie(mContext.has_value());
                return MakeCreatedDevice<EmulatedMicrowaveOven>(EmulatedMicrowaveOven::Context{
                    .timerDelegate          = mContext->timerDelegate,
                    .diagnosticDataProvider = mContext->diagnosticDataProvider,
                });
            });
        }
        if constexpr (ALL_DEVICES_ENABLE_PRESSURE_SENSOR)
        {
            RegisterCreator("pressure-sensor", [this]() {
                VerifyOrDie(mContext.has_value());
                return MakeCreatedDevice<IncreasingPressureSensor>(mContext->timerDelegate);
            });
        }
        if constexpr (ALL_DEVICES_ENABLE_FLOW_SENSOR)
        {
            RegisterCreator("flow-sensor", [this]() {
                VerifyOrDie(mContext.has_value());
                return MakeCreatedDevice<IncreasingFlowSensor>(mContext->timerDelegate);
            });
        }
        if constexpr (ALL_DEVICES_ENABLE_ROBOTIC_VACUUM_CLEANER)
        {
            RegisterCreator("robotic-vacuum-cleaner", []() { return MakeCreatedDevice<RoboticVacuumCleaner>(); });
        }

        // at least one device type MUST be enabled
        VerifyOrDie(!mRegistry.empty());
    }
};

using SimpleDeviceFactory = DeviceFactory<>;

} // namespace chip::app
