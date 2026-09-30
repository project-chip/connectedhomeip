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

#include "ElectricalProtectionAlarmCluster.h"

#include <app/server-cluster/AttributeListBuilder.h>
#include <app/server-cluster/ServerClusterContext.h>
#include <clusters/ElectricalProtectionAlarm/Metadata.h>

namespace chip::app::Clusters::ElectricalProtectionAlarm {

using namespace ElectricalProtectionAlarm::Attributes;

ElectricalProtectionAlarmCluster::ElectricalProtectionAlarmCluster(EndpointId endpointId, const StartupConfiguration & config) :
    DefaultServerCluster({ endpointId, ElectricalProtectionAlarm::Id }), mFeatureMap(config.featureMap), mMask(config.mask),
    mState(config.state), mSupported(config.supported), mArcCause(config.arcCause), mOverLoadRating(config.overLoadRating),
    mOverVoltageRating(config.overVoltageRating), mSurgeProtectionRating(config.surgeProtectionRating),
    mShortCircuitRating(config.shortCircuitRating), mResidualCurrentRating(config.residualCurrentRating),
    mArcFaultRating(config.arcFaultRating)
{
    // Hold the same invariant SetState() enforces, that State only ever carries bits which are both
    // supported and unmasked. Without this an alarm the device cannot raise would read as active
    // from the first read, and would then appear in the inactive field of the first Notify event
    // despite never having legitimately been active. Applied in the body rather than the
    // initialiser list because mSupported is declared after mState.
    mState.SetRaw(static_cast<uint32_t>(mState.Raw() & mSupported.Raw() & mMask.Raw()));
}

DataModel::ActionReturnStatus ElectricalProtectionAlarmCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                                              AttributeValueEncoder & encoder)
{
    switch (request.path.mAttributeId)
    {
    case Mask::Id:
        return encoder.Encode(mMask);
    case State::Id:
        return encoder.Encode(mState);
    case Supported::Id:
        return encoder.Encode(mSupported);
    case ArcCause::Id:
        return encoder.Encode(mArcCause);
    case OverLoadRating::Id:
        return encoder.Encode(mOverLoadRating);
    case OverVoltageRating::Id:
        return encoder.Encode(mOverVoltageRating);
    case SurgeProtectionRating::Id:
        return encoder.Encode(mSurgeProtectionRating);
    case ShortCircuitRating::Id:
        return encoder.Encode(mShortCircuitRating);
    case ResidualCurrentRating::Id:
        return encoder.Encode(mResidualCurrentRating);
    case ArcFaultRating::Id:
        return encoder.Encode(mArcFaultRating);
    case ClusterRevision::Id:
        return encoder.Encode(ElectricalProtectionAlarm::kRevision);
    case FeatureMap::Id:
        return encoder.Encode(mFeatureMap);
    default:
        return Protocols::InteractionModel::Status::UnsupportedAttribute;
    }
}

CHIP_ERROR ElectricalProtectionAlarmCluster::Attributes(const ConcreteClusterPath & path,
                                                        ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    AttributeListBuilder listBuilder(builder);

    const AttributeListBuilder::OptionalAttributeEntry optionalAttributes[] = {
        { mFeatureMap.Has(Feature::kArcFault), ArcCause::kMetadataEntry },
        { mFeatureMap.Has(Feature::kOverLoad), OverLoadRating::kMetadataEntry },
        { mFeatureMap.Has(Feature::kOverVoltage), OverVoltageRating::kMetadataEntry },
        { mFeatureMap.Has(Feature::kSurgeProtection), SurgeProtectionRating::kMetadataEntry },
        { mFeatureMap.Has(Feature::kShortCircuit), ShortCircuitRating::kMetadataEntry },
        { mFeatureMap.Has(Feature::kResidualCurrent), ResidualCurrentRating::kMetadataEntry },
        { mFeatureMap.Has(Feature::kArcFault), ArcFaultRating::kMetadataEntry },
    };

    return listBuilder.Append(Span(ElectricalProtectionAlarm::Attributes::kMandatoryMetadata), Span(optionalAttributes));
}

CHIP_ERROR ElectricalProtectionAlarmCluster::ActivateAlarms(AlarmBitmask alarms)
{
    AlarmBitmask newState;
    newState.SetRaw(static_cast<uint32_t>(mState.Raw() | alarms.Raw()));
    return SetState(newState);
}

CHIP_ERROR ElectricalProtectionAlarmCluster::DeactivateAlarms(AlarmBitmask alarms)
{
    AlarmBitmask newState;
    newState.SetRaw(static_cast<uint32_t>(mState.Raw() & ~alarms.Raw()));
    return SetState(newState);
}

CHIP_ERROR ElectricalProtectionAlarmCluster::ClearAllAlarms()
{
    return SetState(AlarmBitmask{});
}

CHIP_ERROR ElectricalProtectionAlarmCluster::SetState(AlarmBitmask newState)
{
    // An alarm can only be active if its bit is both supported and enabled by the mask; the Alarm
    // Base cluster requires State to reflect the Mask, so a masked-out (disabled) alarm stays inactive.
    const AlarmBitmask effectiveState = (newState & mSupported & mMask);
    const AlarmBitmask oldState       = mState;

    if (!SetAttributeValue(mState, effectiveState, State::Id))
    {
        return CHIP_NO_ERROR; // no transition: no report, no event
    }

    if (mContext != nullptr)
    {
        ElectricalProtectionAlarm::Events::Notify::Type event;
        event.active   = AlarmBitmask(effectiveState).Clear(oldState);
        event.inactive = AlarmBitmask(oldState).Clear(effectiveState);
        event.state    = effectiveState;
        event.mask     = mMask;
        mContext->interactionContext.eventsGenerator.GenerateEvent(event, mPath.mEndpointId);
    }

    return CHIP_NO_ERROR;
}

} // namespace chip::app::Clusters::ElectricalProtectionAlarm
