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

#include <app/MessageDef/StatusIB.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <app/server-cluster/ServerClusterContext.h>
#include <clusters/ElectricalProtectionAlarm/Metadata.h>
#include <lib/support/CodeUtils.h>

namespace chip::app::Clusters::ElectricalProtectionAlarm {

using namespace ElectricalProtectionAlarm::Attributes;

namespace {

AlarmBase::Delegate & GetDefaultDelegate()
{
    static AlarmBase::Delegate delegate;
    return delegate;
}

AlarmBaseCluster::Config MakeAlarmBaseConfig(const ElectricalProtectionAlarmCluster::StartupConfiguration & config)
{
    return {
        .delegate                    = GetDefaultDelegate(),
        .feature                     = {},
        .supported                   = AlarmBase::AlarmMap(config.supported.Raw()),
        .latch                       = {},
        .supportsModifyEnabledAlarms = false,
    };
}

CHIP_ERROR ToChipError(Protocols::InteractionModel::Status status)
{
    return StatusIB(status).ToChipError();
}

} // namespace

ElectricalProtectionAlarmCluster::ElectricalProtectionAlarmCluster(EndpointId endpointId, const StartupConfiguration & config) :
    AlarmBaseCluster(endpointId, { ElectricalProtectionAlarm::Id, ElectricalProtectionAlarm::kRevision },
                     MakeAlarmBaseConfig(config)),
    mFeatureMap(config.featureMap), mArcCause(config.arcCause), mOverLoadRating(config.overLoadRating),
    mOverVoltageRating(config.overVoltageRating), mSurgeProtectionRating(config.surgeProtectionRating),
    mShortCircuitRating(config.shortCircuitRating), mResidualCurrentRating(config.residualCurrentRating),
    mArcFaultRating(config.arcFaultRating)
{
    const AlarmBitmask mask = config.mask & config.supported;
    VerifyOrDie(AlarmBaseCluster::SetMask(AlarmBase::AlarmMap(mask.Raw())) == Protocols::InteractionModel::Status::Success);

    const AlarmBitmask state = config.state & mask;
    VerifyOrDie(SetStateIgnoringLatch(AlarmBase::AlarmMap(state.Raw())) == Protocols::InteractionModel::Status::Success);
}

DataModel::ActionReturnStatus ElectricalProtectionAlarmCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                                              AttributeValueEncoder & encoder)
{
    switch (request.path.mAttributeId)
    {
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
    case FeatureMap::Id:
        return encoder.Encode(mFeatureMap);
    default:
        return AlarmBaseCluster::ReadAttribute(request, encoder);
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
    AlarmBitmask newState = GetState().Set(alarms) & GetSupported() & GetMask();
    return ToChipError(AlarmBaseCluster::SetState(AlarmBase::AlarmMap(newState.Raw())));
}

CHIP_ERROR ElectricalProtectionAlarmCluster::DeactivateAlarms(AlarmBitmask alarms)
{
    AlarmBitmask newState = GetState().Clear(alarms);
    return ToChipError(AlarmBaseCluster::SetState(AlarmBase::AlarmMap(newState.Raw())));
}

CHIP_ERROR ElectricalProtectionAlarmCluster::ClearAllAlarms()
{
    return ToChipError(AlarmBaseCluster::SetState(AlarmBase::AlarmMap{}));
}

void ElectricalProtectionAlarmCluster::SendNotifyEvent(AlarmBase::AlarmMap becameActive, AlarmBase::AlarmMap becameInactive,
                                                       AlarmBase::AlarmMap newState, AlarmBase::AlarmMap mask)
{
    VerifyOrReturn(mContext != nullptr);

    ElectricalProtectionAlarm::Events::Notify::Type event{
        .active   = AlarmBitmask(becameActive.Raw()),
        .inactive = AlarmBitmask(becameInactive.Raw()),
        .state    = AlarmBitmask(newState.Raw()),
        .mask     = AlarmBitmask(mask.Raw()),
    };
    mContext->interactionContext.eventsGenerator.GenerateEvent(event, mPath.mEndpointId);
}

} // namespace chip::app::Clusters::ElectricalProtectionAlarm
