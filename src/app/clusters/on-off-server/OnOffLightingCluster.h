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

#include <app/clusters/on-off-server/OnOffCluster.h>
#include <app/clusters/on-off-server/OnOffEffectDelegate.h>
#include <app/clusters/scenes-server/ScenesIntegrationDelegate.h>
#include <lib/support/TimerDelegate.h>

namespace chip::app::Clusters::OnOff {

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
    OnOffLightingCluster(EndpointId endpointId, TimerDelegate & timerDelegate, OnOffEffectDelegate & effectDelegate,
                         chip::scenes::ScenesIntegrationDelegate * scenesIntegrationDelegate = nullptr,
                         BitMask<Feature> featureMap                                         = Feature::kLighting);

    ~OnOffLightingCluster() override;

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

    /// Sets the on off value, however it is specific for needs other integrations.
    ///
    /// In particular 1.5.4.1 Lighting feature requires that if level control cluster sets
    /// then onoff value then:
    ///   - if value is set to false, OnTime is set to 0
    ///   - if value is set to true, OffWaitTime is set to 0
    CHIP_ERROR SetOnOffWithTimeReset(bool on);

    // TimerContext
    void TimerFired() override;

private:
    friend class OnOffLightingClusterTestAccess;

    OnOffEffectDelegate & mEffectDelegate;
    chip::scenes::ScenesIntegrationDelegate * mScenesIntegrationDelegate;

    // Lighting Attributes
    bool mGlobalSceneControl = true;
    uint16_t mOnTime         = 0;
    uint16_t mOffWaitTime    = 0;
    DataModel::Nullable<StartUpOnOffEnum> mStartUpOnOff;

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
};

class OnOffLightingClusterTestAccess
{
public:
    OnOffLightingClusterTestAccess(OnOffLightingCluster & cluster) : mCluster(cluster) {}

    void SetOnTime(uint16_t onTime) { mCluster.mOnTime = onTime; }

    void SetOffWaitTime(uint16_t offWaitTime) { mCluster.mOffWaitTime = offWaitTime; }

private:
    OnOffLightingCluster & mCluster;
};

} // namespace chip::app::Clusters::OnOff
