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

#include <app/server-cluster/DefaultServerCluster.h>

#include <clusters/ElectricalProtectionAlarm/Attributes.h>
#include <clusters/ElectricalProtectionAlarm/Enums.h>
#include <clusters/ElectricalProtectionAlarm/Events.h>

#include <lib/support/BitMask.h>

namespace chip::app::Clusters {

/// Server for the Electrical Protection Alarm cluster (0x00A3).
///
/// EPALM is a read-only safety-fault reporter derived from Alarm Base. Its three mandatory
/// attributes (Mask, State, Supported) are AlarmBitmap bitmaps; up to seven feature-gated rating
/// attributes describe the device nameplate. There is no Latch attribute (the RESET feature is
/// disallowed) and the cluster accepts no commands (both Reset and ModifyEnabledAlarms are rejected
/// with UNSUPPORTED_COMMAND). The mandatory Notify event is emitted on every State transition.
///
/// The alarm State is driven by the device's own fault detection; for the example app and
/// conformance testing it is driven through the ActivateAlarms/DeactivateAlarms/ClearAllAlarms
/// mutators (wired to a General Diagnostics test-event-trigger).
class ElectricalProtectionAlarmCluster : public DefaultServerCluster
{
public:
    using Feature      = ElectricalProtectionAlarm::Feature;
    using AlarmBitmap  = ElectricalProtectionAlarm::AlarmBitmap;
    using AlarmBitmask = chip::BitMask<AlarmBitmap>;

    struct StartupConfiguration
    {
        chip::BitMask<Feature> featureMap;
        // Supported must contain exactly the AlarmBitmap bits whose gating feature is set; Mask is
        // the set of enabled alarms (defaults to Supported); State is the initially-active set.
        ElectricalProtectionAlarm::Attributes::Supported::TypeInfo::Type supported;
        ElectricalProtectionAlarm::Attributes::Mask::TypeInfo::Type mask;
        ElectricalProtectionAlarm::Attributes::State::TypeInfo::Type state;
        // Feature-gated rating attributes (nullable; default null == "unknown").
        ElectricalProtectionAlarm::Attributes::ArcCause::TypeInfo::Type arcCause;
        ElectricalProtectionAlarm::Attributes::OverLoadRating::TypeInfo::Type overLoadRating;
        ElectricalProtectionAlarm::Attributes::OverVoltageRating::TypeInfo::Type overVoltageRating;
        ElectricalProtectionAlarm::Attributes::SurgeProtectionRating::TypeInfo::Type surgeProtectionRating;
        ElectricalProtectionAlarm::Attributes::ShortCircuitRating::TypeInfo::Type shortCircuitRating;
        ElectricalProtectionAlarm::Attributes::ResidualCurrentRating::TypeInfo::Type residualCurrentRating;
        ElectricalProtectionAlarm::Attributes::ArcFaultRating::TypeInfo::Type arcFaultRating;
    };

    ElectricalProtectionAlarmCluster(EndpointId endpointId, const StartupConfiguration & config);

    // Server cluster implementation
    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override;
    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override;

    /// Mark the given alarm bits active. Any bit outside Supported is ignored. Emits Notify + an
    /// attribute-changed report for State if the effective State changes.
    CHIP_ERROR ActivateAlarms(AlarmBitmask alarms);
    /// Mark the given alarm bits inactive.
    CHIP_ERROR DeactivateAlarms(AlarmBitmask alarms);
    /// Clear all active alarms (State -> 0).
    CHIP_ERROR ClearAllAlarms();

    AlarmBitmask GetState() const { return mState; }
    AlarmBitmask GetMask() const { return mMask; }
    AlarmBitmask GetSupported() const { return mSupported; }

private:
    /// Set State to newState (restricted to Supported bits), emitting Notify + a State report on change.
    CHIP_ERROR SetState(AlarmBitmask newState);

    const chip::BitMask<Feature> mFeatureMap;
    ElectricalProtectionAlarm::Attributes::Mask::TypeInfo::Type mMask;
    ElectricalProtectionAlarm::Attributes::State::TypeInfo::Type mState;
    ElectricalProtectionAlarm::Attributes::Supported::TypeInfo::Type mSupported;
    ElectricalProtectionAlarm::Attributes::ArcCause::TypeInfo::Type mArcCause;
    ElectricalProtectionAlarm::Attributes::OverLoadRating::TypeInfo::Type mOverLoadRating;
    ElectricalProtectionAlarm::Attributes::OverVoltageRating::TypeInfo::Type mOverVoltageRating;
    ElectricalProtectionAlarm::Attributes::SurgeProtectionRating::TypeInfo::Type mSurgeProtectionRating;
    ElectricalProtectionAlarm::Attributes::ShortCircuitRating::TypeInfo::Type mShortCircuitRating;
    ElectricalProtectionAlarm::Attributes::ResidualCurrentRating::TypeInfo::Type mResidualCurrentRating;
    ElectricalProtectionAlarm::Attributes::ArcFaultRating::TypeInfo::Type mArcFaultRating;
};

} // namespace chip::app::Clusters
