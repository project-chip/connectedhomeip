/*
 *
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

#include <app/clusters/alarm-base-server/AlarmBaseCluster.h>
#include <app/server-cluster/AttributeListBuilder.h>

using namespace chip::app::Clusters::AlarmBase;
using namespace chip::app::Clusters::AlarmBase::Attributes;
using chip::Protocols::InteractionModel::Status;

namespace chip::app::Clusters {

AlarmBaseCluster::AlarmBaseCluster(EndpointId endpointId, ClusterEntry cluster, const Config & config) :
    DefaultServerCluster({ endpointId, cluster.id }), mFeature(config.feature), mClusterRevision(cluster.revision),
    mSupportsModifyEnabledAlarms(config.supportsModifyEnabledAlarms), mDelegate(config.delegate), mLatch(config.latch),
    mSupported(config.supported)
{}

Status AlarmBaseCluster::SetMask(const AlarmMap & mask)
{
    VerifyOrReturnError(mSupported.HasAll(mask), Status::Failure);
    VerifyOrReturnError(SetAttributeValue(mMask, mask, Mask::Id), Status::Success);

    AlarmMap state = mState;
    if (!mask.HasAll(state))
    {
        state = mask & state;
        return SetStateIgnoringLatch(state);
    }
    return Status::Success;
}

Status AlarmBaseCluster::SetState(const AlarmMap & newState)
{
    return SetStateInternal(newState, false);
}

Status AlarmBaseCluster::SetStateIgnoringLatch(const AlarmMap & newState)
{
    return SetStateInternal(newState, true);
}

Status AlarmBaseCluster::SetStateInternal(const AlarmMap & newState, bool ignoreLatchState)
{
    AlarmMap finalNewState = newState;

    VerifyOrReturnError(mSupported.HasAll(finalNewState), Status::Failure);
    VerifyOrReturnError(mMask.HasAll(finalNewState), Status::Failure);

    AlarmMap currentState = mState;

    if (!ignoreLatchState && HasResetFeature())
    {
        auto bitsToKeep = GetLatch() & currentState;
        finalNewState.Set(bitsToKeep);
    }

    VerifyOrReturnError(SetAttributeValue(mState, finalNewState, State::Id), Status::Success);

    AlarmMap becameActive;
    becameActive.Set(finalNewState).Clear(currentState);
    AlarmMap becameInactive;
    becameInactive.Set(currentState).Clear(finalNewState);

    SendNotifyEvent(becameActive, becameInactive, finalNewState, mMask);
    return Status::Success;
}

Status AlarmBaseCluster::ResetLatchedAlarms(const AlarmMap & alarms)
{
    VerifyOrReturnError(mSupported.HasAll(alarms), Status::Failure);

    AlarmMap state = mState;
    state.Clear(alarms);
    return SetStateIgnoringLatch(state);
}

DataModel::ActionReturnStatus AlarmBaseCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                              AttributeValueEncoder & encoder)
{
    switch (request.path.mAttributeId)
    {
    case ClusterRevision::Id:
        return encoder.Encode(mClusterRevision);
    case FeatureMap::Id:
        return encoder.Encode(mFeature);
    case Mask::Id:
        return encoder.Encode(mMask);
    case Latch::Id:
        return encoder.Encode(mLatch);
    case State::Id:
        return encoder.Encode(mState);
    case Supported::Id:
        return encoder.Encode(mSupported);
    default:
        return Status::UnsupportedAttribute;
    }
}

CHIP_ERROR AlarmBaseCluster::Attributes(const ConcreteClusterPath & path,
                                        ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    AttributeListBuilder listBuilder(builder);

    const AttributeListBuilder::OptionalAttributeEntry optionalAttributes[] = {
        { HasResetFeature(), Latch::kMetadataEntry },
    };

    return listBuilder.Append(Span(kMandatoryMetadata), Span(optionalAttributes));
}

CHIP_ERROR AlarmBaseCluster::AcceptedCommands(const ConcreteClusterPath & path,
                                              ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder)
{
    if (HasResetFeature())
    {
        ReturnErrorOnFailure(builder.AppendElements({ Commands::Reset::kMetadataEntry }));
    }
    if (mSupportsModifyEnabledAlarms)
    {
        ReturnErrorOnFailure(builder.AppendElements({ Commands::ModifyEnabledAlarms::kMetadataEntry }));
    }
    return CHIP_NO_ERROR;
}

std::optional<DataModel::ActionReturnStatus> AlarmBaseCluster::InvokeCommand(const DataModel::InvokeRequest & request,
                                                                             TLV::TLVReader & input_arguments,
                                                                             CommandHandler * handler)
{
    switch (request.path.mCommandId)
    {
    case Commands::Reset::Id: {
        Commands::Reset::DecodableType data;
        ReturnErrorOnFailure(data.Decode(input_arguments));
        return HandleReset(AlarmMap(data.alarms.Raw()));
    }
    case Commands::ModifyEnabledAlarms::Id: {
        Commands::ModifyEnabledAlarms::DecodableType data;
        ReturnErrorOnFailure(data.Decode(input_arguments));
        return HandleModifyEnabledAlarms(AlarmMap(data.mask.Raw()));
    }
    default:
        return Status::UnsupportedCommand;
    }
}

DataModel::ActionReturnStatus AlarmBaseCluster::HandleReset(const AlarmMap & alarms)
{
    if (!mSupported.HasAll(alarms))
    {
        return Status::InvalidCommand;
    }

    if (!mDelegate.ResetAlarms(alarms))
    {
        return Status::Failure;
    }

    return ResetLatchedAlarms(alarms);
}

DataModel::ActionReturnStatus AlarmBaseCluster::HandleModifyEnabledAlarms(const AlarmMap & mask)
{
    if (!mSupported.HasAll(mask))
    {
        return Status::InvalidCommand;
    }

    if (!mDelegate.ModifyEnabledAlarms(mask))
    {
        return Status::Failure;
    }

    return SetMask(mask);
}

} // namespace chip::app::Clusters
