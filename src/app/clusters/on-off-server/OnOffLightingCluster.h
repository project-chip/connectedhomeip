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

#include <app/clusters/on-off-server/OnOffCluster.h>
#include <app/clusters/on-off-server/OnOffEffectDelegate.h>
#include <app/clusters/scenes-server/ScenesIntegrationDelegate.h>
#include <cstdint>
#include <lib/support/TimerDelegate.h>

namespace chip::app::Clusters {

class OnOffLightingClusterTestAccess;

/// Implements the On/Off cluster with Lighting features.
///
/// This includes:
/// - GlobalSceneControl
/// - OnTime / OffWaitTime
/// - StartUpOnOff
/// - Timed commands
class OnOffLightingCluster : public OnOffCluster, public TimerContext
{
public:
    enum class StartupType
    {
        kRegular,

        // post OTA startup: on/off value must be preserved (i.e. ignore startup on off):
        //
        // Spec on StartupOnOff attribute says:
        //   This behavior does not apply to reboots associated with OTA.
        //   After an OTA restart, the OnOff attribute SHALL return to its value
        //   prior to the restart.
        kOTA,
    };

    // Represents defaults for NVS values if no persistent storage set
    struct Defaults
    {
        bool onOff{};
        DataModel::Nullable<OnOff::StartUpOnOffEnum> startupOnOff{};
    };

    struct Context
    {
        TimerDelegate & timerDelegate;
        OnOffEffectDelegate & effectDelegate;
        chip::scenes::ScenesIntegrationDelegate * scenesIntegrationDelegate = nullptr;
        BitMask<OnOff::Feature> featureMap                                  = OnOff::Feature::kLighting;
        StartupType startupType                                             = StartupType::kRegular;
        Defaults defaults{};
    };

    OnOffLightingCluster(EndpointId endpointId, const Context & context);
    ~OnOffLightingCluster() override;

    uint16_t GetOnTime() const { return mOnTime; }
    void SetOnTime(uint16_t value);

    uint16_t GetOffWaitTime() const { return mOffWaitTime; }
    void SetOffWaitTime(uint16_t value);

    DataModel::Nullable<OnOff::StartUpOnOffEnum> GetStartupOnOff() const { return mStartUpOnOff; }
    CHIP_ERROR SetStartupOnOff(DataModel::Nullable<OnOff::StartUpOnOffEnum> value);

    // ServerClusterInterface implementation
    CHIP_ERROR Startup(ServerClusterContext & context) override;
    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override;
    CHIP_ERROR AcceptedCommands(const ConcreteClusterPath & path,
                                ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder) override;
    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override;
    DataModel::ActionReturnStatus WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                 AttributeValueDecoder & decoder) override;
    std::optional<DataModel::ActionReturnStatus> InvokeCommand(const DataModel::InvokeRequest & request,
                                                               chip::TLV::TLVReader & input_arguments,
                                                               CommandHandler * handler) override;

    /// Sets the OnOff value with special behavior for integration with other clusters.
    ///
    /// In particular, the 1.5.4.1 Lighting feature requires that if the Level Control cluster sets
    /// the OnOff value, then:
    ///   - if value is set to false, OnTime is set to 0
    ///   - if value is set to true, OffWaitTime is set to 0
    CHIP_ERROR SetOnOffWithTimeReset(bool on);

    // TimerContext
    void TimerFired() override;

private:
    OnOffEffectDelegate & mEffectDelegate;
    chip::scenes::ScenesIntegrationDelegate * mScenesIntegrationDelegate;

    // Lighting Attributes
    bool mGlobalSceneControl = true;

    // On/Off times represent delayed on/off timeouts in 1/10th second resolution.
    // They are updated by the underlying timer context (see TimerFired) according to the spec.
    //
    // The cluster may be in the following states:
    //   - ON        - On (no timers)
    //   - OFF       - Off (no timers)
    //   - TIMED_ON  - On with a timer set (mOnTime decrements and will turn off on 0)
    //   - TIMED_OFF - Off with a timer set (mOffWaitTime decrements and will turn on on 0)
    //
    // Values are affected by the following:
    //   - HandleOnWithTimedOff (see spec diagram)
    //   - TimerFired (decreases times)
    //   - SetOnOffFromCommand (needs to transition to on/off correctly)
    uint16_t mOnTime      = 0;
    uint16_t mOffWaitTime = 0;

    // controlling cluster startup - these values are only used at startup however
    // user may modify mStartUpOnOff attribute as it is writable.
    DataModel::Nullable<OnOff::StartUpOnOffEnum> mStartUpOnOff;
    StartupType mStartupType;

    DataModel::ActionReturnStatus WriteImpl(const DataModel::WriteAttributeRequest & request, AttributeValueDecoder & decoder);

    // Timer logic
    void UpdateTimer();

    // Command Handlers
    DataModel::ActionReturnStatus HandleOffWithEffect(const DataModel::InvokeRequest & request,
                                                      chip::TLV::TLVReader & input_arguments);
    DataModel::ActionReturnStatus HandleOnWithRecallGlobalScene(const DataModel::InvokeRequest & request);
    DataModel::ActionReturnStatus HandleOnWithTimedOff(chip::TLV::TLVReader & input_arguments);

    // Wrappers for basic commands to add lighting side effects
    DataModel::ActionReturnStatus HandleOn();
    DataModel::ActionReturnStatus HandleOff();
    DataModel::ActionReturnStatus HandleToggle();

    // Handles transitions for timed on/off. This is to be used by commands EXCEPT
    // HandleOnWithTimedOff to represent the spec `Any command which causes the attribute to be set
    // to TRUE/FALSE` like on/off/toggle/OnWithRecallGlobalScene/OffWithEffect
    CHIP_ERROR SetOnOffFromCommand(bool on);
};

} // namespace chip::app::Clusters
