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

#include <app/TestEventTriggerDelegate.h>
#include <app/clusters/device-energy-management-server/DeviceEnergyManagementCluster.h>
#include <app/server-cluster/ServerClusterInterfaceRegistry.h>
#include <device/api/SingleEndpoint.h>
#include <lib/support/TimerDelegate.h>
#include <system/SystemClock.h>

#include <optional>

namespace chip::app {

/// Device Energy Management device type (0x050D) with a simulated power adjustment.
///
/// Power adjustment states (ESAState):
///
///   Online --PowerAdjustRequest--> PowerAdjustActive      (PowerAdjustStart event)
///   PowerAdjustActive --PowerAdjustRequest--> PowerAdjustActive  (duration restarts, no event)
///   PowerAdjustActive --> Online                          (PowerAdjustEnd event), on:
///       duration expiry             cause NormalCompletion
///       CancelPowerAdjustRequest    cause Cancelled
///       user opt-out of the cause   cause UserOptOut
///
/// Device Energy Management test event triggers for power adjustment and user opt-out are handled while the
/// device is registered.
class EnergyManagement : public SingleEndpoint,
                         public Clusters::DeviceEnergyManagement::Delegate,
                         public TimerContext,
                         public TestEventTriggerHandler
{
public:
    using SingleEndpoint::GetEndpointId;
    using Feature = Clusters::DeviceEnergyManagement::Feature;

    struct Config
    {
        BitMask<Feature> featureFlags;
        Clusters::DeviceEnergyManagement::ESATypeEnum esaType = Clusters::DeviceEnergyManagement::ESATypeEnum::kEvse;
        bool esaCanGenerate                                   = false;
    };

    struct Context
    {
        TimerDelegate & timerDelegate;
        TestEventTriggerDelegate & testEventTriggerDelegate;
    };

    explicit EnergyManagement(const Context & context);
    EnergyManagement(const Context & context, Config config);
    ~EnergyManagement() override;

    CHIP_ERROR Register(EndpointId endpoint, CodeDrivenDataModelProvider & provider, EndpointComposition composition = {}) override;
    void Unregister(CodeDrivenDataModelProvider & provider) override;

    // Clusters::DeviceEnergyManagement::Delegate
    Protocols::InteractionModel::Status PowerAdjustRequest(const int64_t power, const uint32_t duration,
                                                           Clusters::DeviceEnergyManagement::AdjustmentCauseEnum cause) override;
    Protocols::InteractionModel::Status CancelPowerAdjustRequest() override;
    Protocols::InteractionModel::Status
    StartTimeAdjustRequest(const uint32_t requestedStartTime, Clusters::DeviceEnergyManagement::AdjustmentCauseEnum cause) override;
    Protocols::InteractionModel::Status PauseRequest(const uint32_t duration,
                                                     Clusters::DeviceEnergyManagement::AdjustmentCauseEnum cause) override;
    Protocols::InteractionModel::Status ResumeRequest() override;
    Protocols::InteractionModel::Status ModifyForecastRequest(
        const uint32_t forecastID,
        const DataModel::DecodableList<Clusters::DeviceEnergyManagement::Structs::SlotAdjustmentStruct::Type> & slotAdjustments,
        Clusters::DeviceEnergyManagement::AdjustmentCauseEnum cause) override;
    Protocols::InteractionModel::Status RequestConstraintBasedForecast(
        const DataModel::DecodableList<Clusters::DeviceEnergyManagement::Structs::ConstraintsStruct::Type> & constraints,
        Clusters::DeviceEnergyManagement::AdjustmentCauseEnum cause) override;
    Protocols::InteractionModel::Status CancelRequest() override;
    Protocols::InteractionModel::Status
    PowerRangeAdjustRequest(const Optional<int64_t> minPower, const Optional<int64_t> maxPower, const uint32_t duration,
                            Clusters::DeviceEnergyManagement::AdjustmentCauseEnum cause) override;
    Protocols::InteractionModel::Status CancelPowerRangeAdjustRequest() override;

    Clusters::DeviceEnergyManagement::ESATypeEnum GetESAType() override;
    bool GetESACanGenerate() override;
    Clusters::DeviceEnergyManagement::ESAStateEnum GetESAState() override;
    int64_t GetAbsMinPower() override;
    int64_t GetAbsMaxPower() override;
    Clusters::DeviceEnergyManagement::OptOutStateEnum GetOptOutState() override;
    const DataModel::Nullable<Clusters::DeviceEnergyManagement::Structs::PowerAdjustCapabilityStruct::Type> &
    GetPowerAdjustmentCapability() override;
    const DataModel::Nullable<Clusters::DeviceEnergyManagement::Structs::ForecastStruct::Type> & GetForecast() override;
    const DataModel::Nullable<Clusters::DeviceEnergyManagement::Structs::PowerRangeAdjustStruct::Type> &
    GetPowerRangeAdjustment() override;
    CHIP_ERROR SetESAState(Clusters::DeviceEnergyManagement::ESAStateEnum state) override;

    /// Sets the user opt-out state. An active power adjustment ends with cause UserOptOut when `state` opts out
    /// of the cause of that adjustment.
    void SetOptOutState(Clusters::DeviceEnergyManagement::OptOutStateEnum state);

    // Public getters for programmatic control
    Clusters::DeviceEnergyManagementCluster & EnergyManagementCluster() { return mDemCluster.Cluster(); }

    // TimerContext
    void TimerFired() override;

    // TestEventTriggerHandler
    CHIP_ERROR HandleEventTrigger(uint64_t eventTrigger) override;

private:
    struct ActivePowerAdjustment
    {
        System::Clock::Timestamp startTime;
        // Start of the interval that uses `power`. A PowerAdjustRequest during an active adjustment starts a new interval.
        System::Clock::Timestamp intervalStartTime;
        int64_t power;                       // mW
        int64_t energyMilliwattMilliseconds; // mW*ms, of the intervals before `intervalStartTime`
    };

    CHIP_ERROR SetPowerAdjustmentCause(Clusters::DeviceEnergyManagement::PowerAdjustReasonEnum cause);
    void SetPowerAdjustEntry(const Clusters::DeviceEnergyManagement::Structs::PowerAdjustStruct::Type & entry);
    void EndPowerAdjustment(Clusters::DeviceEnergyManagement::CauseEnum cause);
    void NotifyAttributeChanged(AttributeId attribute);

    TimerDelegate & mTimerDelegate;
    TestEventTriggerDelegate & mTestEventTriggerDelegate;
    Config mConfig;
    Clusters::DeviceEnergyManagement::ESAStateEnum mESAState       = Clusters::DeviceEnergyManagement::ESAStateEnum::kOnline;
    Clusters::DeviceEnergyManagement::OptOutStateEnum mOptOutState = Clusters::DeviceEnergyManagement::OptOutStateEnum::kNoOptOut;
    const Clusters::DeviceEnergyManagement::Structs::PowerAdjustStruct::Type * mPowerAdjustEntry;
    std::optional<ActivePowerAdjustment> mActivePowerAdjustment;
    DataModel::Nullable<Clusters::DeviceEnergyManagement::Structs::PowerAdjustCapabilityStruct::Type> mPowerAdjustmentCapability;
    DataModel::Nullable<Clusters::DeviceEnergyManagement::Structs::ForecastStruct::Type> mForecast;
    DataModel::Nullable<Clusters::DeviceEnergyManagement::Structs::PowerRangeAdjustStruct::Type> mPowerRangeAdjustment;

    LazyRegisteredServerCluster<Clusters::DeviceEnergyManagementCluster> mDemCluster;
};

} // namespace chip::app
