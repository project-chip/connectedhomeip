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
#include <clusters/DishwasherAlarm/Events.h>
#include <clusters/RefrigeratorAlarm/Events.h>
#include <clusters/RefrigeratorAlarm/Metadata.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::AlarmBase;
using namespace chip::app::Clusters::AlarmBase::Attributes;
using chip::Protocols::InteractionModel::Status;

AlarmBaseCluster::AlarmBaseCluster(EndpointId endpointId, ClusterId clusterId, const Config & config) :
    DefaultServerCluster({ endpointId, clusterId }), mFeature(config.feature), mClusterRevision(config.clusterRevision),
    mSupportsModifyEnabledAlarms(config.supportsModifyEnabledAlarms), mDelegate(config.delegate), mLatch(config.latch),
    mSupported(config.supported)
{}

Status AlarmBaseCluster::GetMaskValue(AlarmMap * mask) const
{
    VerifyOrReturnError(mask != nullptr, Status::Failure);
    *mask = mMask;
    return Status::Success;
}

Status AlarmBaseCluster::GetLatchValue(AlarmMap * latch) const
{
    VerifyOrReturnError(HasResetFeature(), Status::UnsupportedAttribute);
    VerifyOrReturnError(latch != nullptr, Status::Failure);
    *latch = mLatch;
    return Status::Success;
}

Status AlarmBaseCluster::GetStateValue(AlarmMap * state) const
{
    VerifyOrReturnError(state != nullptr, Status::Failure);
    *state = mState;
    return Status::Success;
}

Status AlarmBaseCluster::GetSupportedValue(AlarmMap * supported) const
{
    VerifyOrReturnError(supported != nullptr, Status::Failure);
    *supported = mSupported;
    return Status::Success;
}

Status AlarmBaseCluster::SetSupportedValue(const AlarmMap supported)
{
    VerifyOrReturnError(SetAttributeValue(mSupported, supported, Supported::Id), Status::Success);

    AlarmMap latch;
    if (GetLatchValue(&latch) == Status::Success && !supported.HasAll(latch))
    {
        latch         = latch & supported;
        Status status = SetLatchValue(latch);
        if (status != Status::Success)
        {
            return status;
        }
    }

    AlarmMap mask;
    if (GetMaskValue(&mask) != Status::Success)
    {
        return Status::Failure;
    }
    if (!supported.HasAll(mask))
    {
        mask          = supported & mask;
        Status status = SetMaskValue(mask);
        if (status != Status::Success)
        {
            return status;
        }
    }
    return Status::Success;
}

Status AlarmBaseCluster::SetMaskValue(const AlarmMap mask)
{
    VerifyOrReturnError(mSupported.HasAll(mask), Status::Failure);
    VerifyOrReturnError(SetAttributeValue(mMask, mask, Mask::Id), Status::Success);

    AlarmMap state;
    if (GetStateValue(&state) != Status::Success)
    {
        return Status::Failure;
    }
    if (!mask.HasAll(state))
    {
        state         = mask & state;
        Status status = SetStateValue(state, true);
        if (status != Status::Success)
        {
            return status;
        }
    }
    return Status::Success;
}

Status AlarmBaseCluster::SetLatchValue(const AlarmMap latch)
{
    VerifyOrReturnError(HasResetFeature(), Status::UnsupportedAttribute);
    VerifyOrReturnError(mSupported.HasAll(latch), Status::Failure);
    VerifyOrReturnError(SetAttributeValue(mLatch, latch, Latch::Id), Status::Success);
    return Status::Success;
}

Status AlarmBaseCluster::SetStateValue(const AlarmMap newState, bool ignoreLatchState)
{
    AlarmMap finalNewState = newState;

    VerifyOrReturnError(mSupported.HasAll(finalNewState), Status::Failure);
    VerifyOrReturnError(mMask.HasAll(finalNewState), Status::Failure);

    AlarmMap currentState = mState;

    if (!ignoreLatchState)
    {
        AlarmMap latch;
        if (GetLatchValue(&latch) == Status::Success)
        {
            auto bitsToKeep = latch & currentState;
            finalNewState.Set(bitsToKeep);
        }
    }

    VerifyOrReturnError(SetAttributeValue(mState, finalNewState, State::Id), Status::Success);

    AlarmMap becameActive;
    becameActive.Set(finalNewState).Clear(currentState);
    AlarmMap becameInactive;
    becameInactive.Set(currentState).Clear(finalNewState);

    SendNotifyEvent(becameActive, becameInactive, finalNewState, mMask);
    return Status::Success;
}

Status AlarmBaseCluster::ResetLatchedAlarms(const AlarmMap alarms)
{
    VerifyOrReturnError(mSupported.HasAll(alarms), Status::Failure);

    AlarmMap state;
    if (GetStateValue(&state) != Status::Success)
    {
        return Status::Failure;
    }
    state.Clear(alarms);
    return SetStateValue(state, true);
}

void AlarmBaseCluster::SendNotifyEvent(AlarmMap becameActive, AlarmMap becameInactive, AlarmMap newState, AlarmMap mask)
{
    VerifyOrReturn(mContext != nullptr);

    if (mPath.mClusterId == DishwasherAlarm::Id)
    {
        DishwasherAlarm::Events::Notify::Type event{
            .active   = BitMask<DishwasherAlarm::AlarmBitmap>(becameActive.Raw()),
            .inactive = BitMask<DishwasherAlarm::AlarmBitmap>(becameInactive.Raw()),
            .state    = BitMask<DishwasherAlarm::AlarmBitmap>(newState.Raw()),
            .mask     = BitMask<DishwasherAlarm::AlarmBitmap>(mask.Raw()),
        };
        mContext->interactionContext.eventsGenerator.GenerateEvent(event, mPath.mEndpointId);
        return;
    }

    if (mPath.mClusterId == RefrigeratorAlarm::Id)
    {
        RefrigeratorAlarm::Events::Notify::Type event{
            .active   = BitMask<RefrigeratorAlarm::AlarmBitmap>(becameActive.Raw()),
            .inactive = BitMask<RefrigeratorAlarm::AlarmBitmap>(becameInactive.Raw()),
            .state    = BitMask<RefrigeratorAlarm::AlarmBitmap>(newState.Raw()),
            .mask     = BitMask<RefrigeratorAlarm::AlarmBitmap>(mask.Raw()),
        };
        mContext->interactionContext.eventsGenerator.GenerateEvent(event, mPath.mEndpointId);
    }
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
        if (!HasResetFeature())
        {
            return Status::UnsupportedAttribute;
        }
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

DataModel::ActionReturnStatus AlarmBaseCluster::HandleReset(AlarmMap alarms)
{
    if (!HasResetFeature())
    {
        return Status::UnsupportedCommand;
    }

    if (!mSupported.HasAll(alarms))
    {
        return Status::Failure;
    }

    if (mDelegate != nullptr && !mDelegate->ResetAlarms(alarms))
    {
        return Status::Failure;
    }

    if (ResetLatchedAlarms(alarms) != Status::Success)
    {
        return Status::Failure;
    }

    return Status::Success;
}

DataModel::ActionReturnStatus AlarmBaseCluster::HandleModifyEnabledAlarms(AlarmMap mask)
{
    if (!mSupportsModifyEnabledAlarms)
    {
        return Status::UnsupportedCommand;
    }

    if (!mSupported.HasAll(mask))
    {
        return Status::InvalidCommand;
    }

    if (mDelegate != nullptr && !mDelegate->ModifyEnabledAlarms(mask))
    {
        return Status::Failure;
    }

    if (SetMaskValue(mask) != Status::Success)
    {
        return Status::Failure;
    }

    return Status::Success;
}
