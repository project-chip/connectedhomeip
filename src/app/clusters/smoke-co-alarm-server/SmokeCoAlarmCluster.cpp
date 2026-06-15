/**
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include "SmokeCoAlarmCluster.h"
#include <app/server-cluster/AttributeListBuilder.h>
#include <clusters/SmokeCoAlarm/Metadata.h>

using namespace chip::app::Clusters::SmokeCoAlarm;
using namespace chip::app::Clusters::SmokeCoAlarm::Attributes;
using chip::Protocols::InteractionModel::Status;

namespace chip {
namespace app {
namespace Clusters {

SmokeCoAlarmCluster::SmokeCoAlarmCluster(EndpointId endpointId) : SmokeCoAlarmCluster(endpointId, Config{}) {}

SmokeCoAlarmCluster::SmokeCoAlarmCluster(EndpointId endpointId, const Config & config) :
    DefaultServerCluster({ endpointId, Id }), mConfig(config), mInoperativeWhenUnmounted(config.inoperativeWhenUnmounted)
{}

bool SmokeCoAlarmCluster::SetSmokeState(AlarmStateEnum newSmokeState)
{
    VerifyOrReturnValue(SupportsSmokeAlarm(), false);
    if (!SetAttributeValue(mSmokeState, newSmokeState, SmokeState::Id))
    {
        return true;
    }

    if (mContext != nullptr && (newSmokeState == AlarmStateEnum::kWarning || newSmokeState == AlarmStateEnum::kCritical))
    {
        Events::SmokeAlarm::Type event{ newSmokeState };
        mContext->interactionContext.eventsGenerator.GenerateEvent(event, mPath.mEndpointId);
    }

    if (newSmokeState == AlarmStateEnum::kCritical)
    {
        SetDeviceMuted(MuteStateEnum::kNotMuted);
    }

    return true;
}

bool SmokeCoAlarmCluster::SetCOState(AlarmStateEnum newCOState)
{
    VerifyOrReturnValue(SupportsCOAlarm(), false);
    if (!SetAttributeValue(mCOState, newCOState, COState::Id))
    {
        return true;
    }

    if (mContext != nullptr && (newCOState == AlarmStateEnum::kWarning || newCOState == AlarmStateEnum::kCritical))
    {
        Events::COAlarm::Type event{ newCOState };
        mContext->interactionContext.eventsGenerator.GenerateEvent(event, mPath.mEndpointId);
    }

    if (newCOState == AlarmStateEnum::kCritical)
    {
        SetDeviceMuted(MuteStateEnum::kNotMuted);
    }

    return true;
}

void SmokeCoAlarmCluster::SetBatteryAlert(AlarmStateEnum newBatteryAlert)
{
    if (!SetAttributeValue(mBatteryAlert, newBatteryAlert, BatteryAlert::Id))
    {
        return;
    }

    if (mContext != nullptr && (newBatteryAlert == AlarmStateEnum::kWarning || newBatteryAlert == AlarmStateEnum::kCritical))
    {
        Events::LowBattery::Type event{ newBatteryAlert };
        mContext->interactionContext.eventsGenerator.GenerateEvent(event, mPath.mEndpointId);
    }

    if (newBatteryAlert == AlarmStateEnum::kCritical)
    {
        SetDeviceMuted(MuteStateEnum::kNotMuted);
    }
}

bool SmokeCoAlarmCluster::SetDeviceMuted(MuteStateEnum newDeviceMuted)
{
    VerifyOrReturnValue(mConfig.optionalAttribs.IsSet(DeviceMuted::Id), false);
    if (mDeviceMuted == newDeviceMuted)
    {
        return true;
    }

    if (newDeviceMuted == MuteStateEnum::kMuted)
    {
        // Cannot mute while any alarm is in Critical state.
        // All alarm state members are always present and default to kNormal
        VerifyOrReturnValue(mSmokeState != AlarmStateEnum::kCritical, false);
        VerifyOrReturnValue(mCOState != AlarmStateEnum::kCritical, false);
        VerifyOrReturnValue(mBatteryAlert != AlarmStateEnum::kCritical, false);
        VerifyOrReturnValue(mInterconnectSmokeAlarm != AlarmStateEnum::kCritical, false);
        VerifyOrReturnValue(mInterconnectCOAlarm != AlarmStateEnum::kCritical, false);
    }

    SetAttributeValue(mDeviceMuted, newDeviceMuted, DeviceMuted::Id);

    if (mContext != nullptr)
    {
        if (newDeviceMuted == MuteStateEnum::kMuted)
        {
            Events::AlarmMuted::Type event{};
            mContext->interactionContext.eventsGenerator.GenerateEvent(event, mPath.mEndpointId);
        }
        else if (newDeviceMuted == MuteStateEnum::kNotMuted)
        {
            Events::MuteEnded::Type event{};
            mContext->interactionContext.eventsGenerator.GenerateEvent(event, mPath.mEndpointId);
        }
    }

    return true;
}

void SmokeCoAlarmCluster::SetTestInProgress(bool newTestInProgress)
{
    if (!SetAttributeValue(mTestInProgress, newTestInProgress, TestInProgress::Id))
    {
        return;
    }

    if (!newTestInProgress && mContext != nullptr)
    {
        Events::SelfTestComplete::Type event{};
        mContext->interactionContext.eventsGenerator.GenerateEvent(event, mPath.mEndpointId);
    }
}

void SmokeCoAlarmCluster::SetHardwareFaultAlert(bool newHardwareFaultAlert)
{
    if (!SetAttributeValue(mHardwareFaultAlert, newHardwareFaultAlert, HardwareFaultAlert::Id))
    {
        return;
    }

    if (newHardwareFaultAlert && mContext != nullptr)
    {
        Events::HardwareFault::Type event{};
        mContext->interactionContext.eventsGenerator.GenerateEvent(event, mPath.mEndpointId);
    }
}

void SmokeCoAlarmCluster::SetEndOfServiceAlert(EndOfServiceEnum newEndOfServiceAlert)
{
    if (!SetAttributeValue(mEndOfServiceAlert, newEndOfServiceAlert, EndOfServiceAlert::Id))
    {
        return;
    }

    if (newEndOfServiceAlert == EndOfServiceEnum::kExpired && mContext != nullptr)
    {
        Events::EndOfService::Type event{};
        mContext->interactionContext.eventsGenerator.GenerateEvent(event, mPath.mEndpointId);
    }
}

bool SmokeCoAlarmCluster::SetInterconnectSmokeAlarm(AlarmStateEnum newInterconnectSmokeAlarm)
{
    VerifyOrReturnValue(mConfig.optionalAttribs.IsSet(InterconnectSmokeAlarm::Id), false);
    if (!SetAttributeValue(mInterconnectSmokeAlarm, newInterconnectSmokeAlarm, InterconnectSmokeAlarm::Id))
    {
        return true;
    }

    if (mContext != nullptr &&
        (newInterconnectSmokeAlarm == AlarmStateEnum::kWarning || newInterconnectSmokeAlarm == AlarmStateEnum::kCritical))
    {
        Events::InterconnectSmokeAlarm::Type event{ newInterconnectSmokeAlarm };
        mContext->interactionContext.eventsGenerator.GenerateEvent(event, mPath.mEndpointId);
    }

    if (newInterconnectSmokeAlarm == AlarmStateEnum::kCritical)
    {
        SetDeviceMuted(MuteStateEnum::kNotMuted);
    }

    return true;
}

bool SmokeCoAlarmCluster::SetInterconnectCOAlarm(AlarmStateEnum newInterconnectCOAlarm)
{
    VerifyOrReturnValue(mConfig.optionalAttribs.IsSet(InterconnectCOAlarm::Id), false);
    if (!SetAttributeValue(mInterconnectCOAlarm, newInterconnectCOAlarm, InterconnectCOAlarm::Id))
    {
        return true;
    }

    if (mContext != nullptr &&
        (newInterconnectCOAlarm == AlarmStateEnum::kWarning || newInterconnectCOAlarm == AlarmStateEnum::kCritical))
    {
        Events::InterconnectCOAlarm::Type event{ newInterconnectCOAlarm };
        mContext->interactionContext.eventsGenerator.GenerateEvent(event, mPath.mEndpointId);
    }

    if (newInterconnectCOAlarm == AlarmStateEnum::kCritical)
    {
        SetDeviceMuted(MuteStateEnum::kNotMuted);
    }

    return true;
}

void SmokeCoAlarmCluster::SetContaminationState(ContaminationStateEnum newContaminationState)
{
    VerifyOrReturn(SupportsSmokeAlarm());
    SetAttributeValue(mContaminationState, newContaminationState, ContaminationState::Id);
}

void SmokeCoAlarmCluster::SetSmokeSensitivityLevel(SensitivityEnum newSmokeSensitivityLevel)
{
    VerifyOrReturn(SupportsSmokeAlarm());
    SetAttributeValue(mSmokeSensitivityLevel, newSmokeSensitivityLevel, SmokeSensitivityLevel::Id);
}

void SmokeCoAlarmCluster::SetExpiryDate(uint32_t newExpiryDate)
{
    VerifyOrReturn(mConfig.optionalAttribs.IsSet(ExpiryDate::Id));
    SetAttributeValue(mExpiryDate, newExpiryDate, ExpiryDate::Id);
}

bool SmokeCoAlarmCluster::SetUnmountedState(bool isUnmounted)
{
    VerifyOrReturnValue(mConfig.optionalAttribs.IsSet(Unmounted::Id), false);
    if (!SetAttributeValue(mUnmounted, isUnmounted, Unmounted::Id))
    {
        return true;
    }

    if (mInoperativeWhenUnmounted)
    {
        if (isUnmounted)
        {
            SetExpressedState(ExpressedStateEnum::kInoperative);
        }
        else if (mExpressedState == ExpressedStateEnum::kInoperative)
        {
            SetExpressedState(ExpressedStateEnum::kNormal);
        }
    }

    return true;
}

void SmokeCoAlarmCluster::SetExpressedStateByPriority(const std::array<ExpressedStateEnum, kPriorityOrderLength> & priorityOrder)
{
    for (ExpressedStateEnum priority : priorityOrder)
    {
        AlarmStateEnum alarmState          = AlarmStateEnum::kNormal;
        EndOfServiceEnum endOfServiceState = EndOfServiceEnum::kNormal;
        bool active                        = false;
        bool unmounted                     = false;

        switch (priority)
        {
        case ExpressedStateEnum::kSmokeAlarm:
            alarmState = GetSmokeState();
            break;
        case ExpressedStateEnum::kCOAlarm:
            alarmState = GetCOState();
            break;
        case ExpressedStateEnum::kBatteryAlert:
            alarmState = GetBatteryAlert();
            break;
        case ExpressedStateEnum::kTesting:
            active = GetTestInProgress();
            break;
        case ExpressedStateEnum::kHardwareFault:
            active = GetHardwareFaultAlert();
            break;
        case ExpressedStateEnum::kEndOfService:
            endOfServiceState = GetEndOfServiceAlert();
            break;
        case ExpressedStateEnum::kInterconnectSmoke:
            alarmState = GetInterconnectSmokeAlarm();
            break;
        case ExpressedStateEnum::kInterconnectCO:
            alarmState = GetInterconnectCOAlarm();
            break;
        case ExpressedStateEnum::kInoperative:
            unmounted = GetUnmountedState();
            break;
        default:
            break;
        }

        if ((alarmState != AlarmStateEnum::kNormal) || (endOfServiceState != EndOfServiceEnum::kNormal) || active || unmounted)
        {
            SetExpressedState(priority);
            return;
        }
    }

    SetExpressedState(ExpressedStateEnum::kNormal);
}

bool SmokeCoAlarmCluster::RequestSelfTest()
{
    if (mExpressedState == ExpressedStateEnum::kSmokeAlarm || mExpressedState == ExpressedStateEnum::kCOAlarm ||
        mExpressedState == ExpressedStateEnum::kTesting || mExpressedState == ExpressedStateEnum::kInterconnectSmoke ||
        mExpressedState == ExpressedStateEnum::kInterconnectCO)
    {
        return false;
    }

    SetTestInProgress(true);
    SetExpressedState(ExpressedStateEnum::kTesting);
    if (mDelegate != nullptr)
    {
        mDelegate->OnSelfTestRequested();
    }
    return true;
}

DataModel::ActionReturnStatus SmokeCoAlarmCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                                 AttributeValueEncoder & encoder)
{
    switch (request.path.mAttributeId)
    {
    case ClusterRevision::Id:
        return encoder.Encode(SmokeCoAlarm::kRevision);
    case FeatureMap::Id:
        return encoder.Encode(mConfig.featureMap);
    case ExpressedState::Id:
        return encoder.Encode(mExpressedState);
    case SmokeState::Id:
        return encoder.Encode(mSmokeState);
    case COState::Id:
        return encoder.Encode(mCOState);
    case BatteryAlert::Id:
        return encoder.Encode(mBatteryAlert);
    case DeviceMuted::Id:
        return encoder.Encode(mDeviceMuted);
    case TestInProgress::Id:
        return encoder.Encode(mTestInProgress);
    case HardwareFaultAlert::Id:
        return encoder.Encode(mHardwareFaultAlert);
    case EndOfServiceAlert::Id:
        return encoder.Encode(mEndOfServiceAlert);
    case InterconnectSmokeAlarm::Id:
        return encoder.Encode(mInterconnectSmokeAlarm);
    case InterconnectCOAlarm::Id:
        return encoder.Encode(mInterconnectCOAlarm);
    case ContaminationState::Id:
        return encoder.Encode(mContaminationState);
    case SmokeSensitivityLevel::Id:
        return encoder.Encode(mSmokeSensitivityLevel);
    case ExpiryDate::Id:
        return encoder.Encode(mExpiryDate);
    case Unmounted::Id:
        return encoder.Encode(mUnmounted);
    default:
        return Status::UnsupportedAttribute;
    }
}

DataModel::ActionReturnStatus SmokeCoAlarmCluster::WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                                  AttributeValueDecoder & decoder)
{
    if (request.path.mAttributeId != SmokeSensitivityLevel::Id)
    {
        return Status::UnsupportedWrite;
    }

    SensitivityEnum value;
    ReturnErrorOnFailure(decoder.Decode(value));
    SetSmokeSensitivityLevel(value);
    return Status::Success;
}

CHIP_ERROR SmokeCoAlarmCluster::Attributes(const ConcreteClusterPath & path,
                                           ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    AttributeListBuilder listBuilder(builder);

    const AttributeListBuilder::OptionalAttributeEntry optionalAttributes[] = {
        { mConfig.featureMap.Has(Feature::kSmokeAlarm), SmokeState::kMetadataEntry },
        { mConfig.featureMap.Has(Feature::kCoAlarm), COState::kMetadataEntry },
        { mConfig.optionalAttribs.IsSet(DeviceMuted::Id), DeviceMuted::kMetadataEntry },
        { mConfig.optionalAttribs.IsSet(InterconnectSmokeAlarm::Id), InterconnectSmokeAlarm::kMetadataEntry },
        { mConfig.optionalAttribs.IsSet(InterconnectCOAlarm::Id), InterconnectCOAlarm::kMetadataEntry },
        { mConfig.featureMap.Has(Feature::kSmokeAlarm), ContaminationState::kMetadataEntry },
        { mConfig.featureMap.Has(Feature::kSmokeAlarm), SmokeSensitivityLevel::kMetadataEntry },
        { mConfig.optionalAttribs.IsSet(ExpiryDate::Id), ExpiryDate::kMetadataEntry },
        { mConfig.optionalAttribs.IsSet(Unmounted::Id), Unmounted::kMetadataEntry },
    };

    return listBuilder.Append(Span(kMandatoryMetadata), Span(optionalAttributes));
}

std::optional<DataModel::ActionReturnStatus> SmokeCoAlarmCluster::InvokeCommand(const DataModel::InvokeRequest & request,
                                                                                chip::TLV::TLVReader & input_arguments,
                                                                                CommandHandler * handler)
{
    switch (request.path.mCommandId)
    {
    case Commands::SelfTestRequest::Id:
        return HandleRemoteSelfTestRequest();
    default:
        return Status::UnsupportedCommand;
    }
}

CHIP_ERROR SmokeCoAlarmCluster::AcceptedCommands(const ConcreteClusterPath & path,
                                                 ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder)
{
    return builder.AppendElements({ Commands::SelfTestRequest::kMetadataEntry });
}

void SmokeCoAlarmCluster::SetExpressedState(ExpressedStateEnum newExpressedState)
{
    if (!SetAttributeValue(mExpressedState, newExpressedState, ExpressedState::Id))
    {
        return;
    }

    if (newExpressedState == ExpressedStateEnum::kNormal && mContext != nullptr)
    {
        Events::AllClear::Type event{};
        mContext->interactionContext.eventsGenerator.GenerateEvent(event, mPath.mEndpointId);
    }
}

Status SmokeCoAlarmCluster::HandleRemoteSelfTestRequest()
{
    if (mExpressedState == ExpressedStateEnum::kSmokeAlarm || mExpressedState == ExpressedStateEnum::kCOAlarm ||
        mExpressedState == ExpressedStateEnum::kTesting || mExpressedState == ExpressedStateEnum::kInterconnectSmoke ||
        mExpressedState == ExpressedStateEnum::kInterconnectCO)
    {
        return Status::Busy;
    }

    SetTestInProgress(true);
    SetExpressedState(ExpressedStateEnum::kTesting);
    if (mDelegate != nullptr)
    {
        mDelegate->OnSelfTestRequested();
    }
    return Status::Success;
}

} // namespace Clusters
} // namespace app
} // namespace chip
