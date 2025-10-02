/*
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <app/clusters/identify-server/IdentifyCluster.h>

#include <app/InteractionModelEngine.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <clusters/Identify/Commands.h>
#include <clusters/Identify/Metadata.h>
#include <tracing/macros.h>

#include <lib/support/logging/CHIPLogging.h>

namespace chip::app::Clusters {

using namespace Identify::Attributes;

using namespace chip::app::Clusters::Identify;

namespace {
DefaultTimerDelegate sDefaultTimerDelegate;

constexpr DataModel::AcceptedCommandEntry kAcceptedCommands[] = {
    Identify::Commands::Identify::kMetadataEntry,
};

constexpr DataModel::AcceptedCommandEntry kAcceptedCommandsWithTriggerEffect[] = {
    Identify::Commands::Identify::kMetadataEntry,
    Identify::Commands::TriggerEffect::kMetadataEntry,
};

} // namespace

IdentifyCluster::IdentifyCluster(const Config & config) :
    DefaultServerCluster({ config.endpointId, Identify::Id }), mIdentifyTime(0), mPreviousIdentifyTime(0),
    mIdentifyType(config.identifyType), mOnIdentifyStart(config.onIdentifyStart), mOnIdentifyStop(config.onIdentifyStop),
    mOnEffectIdentifier(config.onEffectIdentifier), mCurrentEffectIdentifier(config.effectIdentifier),
    mEffectVariant(config.effectVariant),
    mTimerDelegate(config.timerDelegate == nullptr ? &sDefaultTimerDelegate : config.timerDelegate)
{}

DataModel::ActionReturnStatus IdentifyCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                             AttributeValueEncoder & encoder)
{
    switch (request.path.mAttributeId)
    {
    case Attributes::IdentifyTime::Id:
        return encoder.Encode(mIdentifyTime);
    case Attributes::IdentifyType::Id:
        return encoder.Encode(mIdentifyType);
    case Attributes::ClusterRevision::Id:
        return encoder.Encode(Identify::kRevision);
    case Attributes::FeatureMap::Id:
        return encoder.Encode<uint32_t>(0);
    default:
        return Protocols::InteractionModel::Status::UnsupportedAttribute;
    }
}

DataModel::ActionReturnStatus IdentifyCluster::WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                              AttributeValueDecoder & decoder)
{
    switch (request.path.mAttributeId)
    {
    case Attributes::IdentifyTime::Id: {
        mPreviousIdentifyTime = mIdentifyTime;
        ReturnErrorOnFailure(decoder.Decode(mIdentifyTime));
        IdentifyTimeAttributeChanged(/* isWrittenByClientOrCmd */ true);
        return CHIP_NO_ERROR;
    }
    break;
    // Read-only attributes
    case Attributes::IdentifyType::Id:
        return Protocols::InteractionModel::Status::UnsupportedWrite;
    case Attributes::ClusterRevision::Id:
        return Protocols::InteractionModel::Status::UnsupportedWrite;
    case Attributes::FeatureMap::Id:
        return Protocols::InteractionModel::Status::UnsupportedWrite;
    // Invalid attributes
    default:
        return Protocols::InteractionModel::Status::UnsupportedAttribute;
    }
}

CHIP_ERROR IdentifyCluster::Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    AttributeListBuilder listBuilder(builder);
    return listBuilder.Append(Span(Identify::Attributes::kMandatoryMetadata), {});
}

void IdentifyCluster::TimerFired()
{
    if (mIdentifyTime > 0)
    {
        mPreviousIdentifyTime = mIdentifyTime;
        mIdentifyTime--;
        IdentifyTimeAttributeChanged(false);
    }
}

// According to the spec, section 5.1 - IdentifyTime Attribute:
// Changes to this attribute SHALL only be marked as reportable in the following cases:
// 1. When it changes from 0 to any other value and vice versa, or
// 2. When it is written by a client, or
// 3. When the value is set by an Identify command.
void IdentifyCluster::IdentifyTimeAttributeChanged(bool isWrittenByClientOrCmd)
{
    // Start Identify
    if (mPreviousIdentifyTime == 0 && mIdentifyTime > 0)
    {
        NotifyAttributeChanged(Attributes::IdentifyTime::Id);
        if (mOnIdentifyStart)
        {
            mOnIdentifyStart(this);
        }
    }
    // Stop Identify
    else if (mPreviousIdentifyTime > 0 && mIdentifyTime == 0)
    {
        NotifyAttributeChanged(Attributes::IdentifyTime::Id);
        if (mOnIdentifyStop)
        {
            mOnIdentifyStop(this);
        }
    }
    // Attribute was changed by the client (AttributeWrite or Command), must notify.
    else if (isWrittenByClientOrCmd)
    {
        NotifyAttributeChanged(Attributes::IdentifyTime::Id);
    }

    if (mIdentifyTime > 0)
    {
        mTimerDelegate->StartTimer(this, System::Clock::Seconds16(1));
    }
    else
    {
        mTimerDelegate->CancelTimer(this);
    }
}

std::optional<DataModel::ActionReturnStatus>
IdentifyCluster::InvokeCommand(const DataModel::InvokeRequest & request, TLV::TLVReader & input_arguments, CommandHandler * handler)
{
    switch (request.path.mCommandId)
    {
    case Identify::Commands::Identify::Id: {
        Identify::Commands::Identify::DecodableType data;
        if (data.Decode(input_arguments) != CHIP_NO_ERROR)
        {
            return DataModel::ActionReturnStatus(Protocols::InteractionModel::Status::InvalidCommand);
        }
        MATTER_TRACE_SCOPE("IdentifyCommand", "Identify");
        mPreviousIdentifyTime = mIdentifyTime;
        mIdentifyTime         = data.identifyTime;
        IdentifyTimeAttributeChanged(/* isWrittenByClientOrCmd */ true);
        return DataModel::ActionReturnStatus(Protocols::InteractionModel::Status::Success);
    }
    case Identify::Commands::TriggerEffect::Id: {
        if (!mOnEffectIdentifier)
        {
            return DataModel::ActionReturnStatus(Protocols::InteractionModel::Status::UnsupportedCommand);
        }

        Identify::Commands::TriggerEffect::DecodableType data;
        if (data.Decode(input_arguments) != CHIP_NO_ERROR)
        {
            return DataModel::ActionReturnStatus(Protocols::InteractionModel::Status::InvalidCommand);
        }

        MATTER_TRACE_SCOPE("TriggerEffect", "Identify");

        mCurrentEffectIdentifier = data.effectIdentifier;
        mEffectVariant           = data.effectVariant;

        ChipLogProgress(Zcl, "RX identify:trigger effect identifier 0x%X variant 0x%X", to_underlying(mCurrentEffectIdentifier),
                        to_underlying(mEffectVariant));

        if (mIdentifyTime > 0)
        {
            if (mCurrentEffectIdentifier == Identify::EffectIdentifierEnum::kFinishEffect)
            {
                mPreviousIdentifyTime = mIdentifyTime;
                mIdentifyTime         = 1;
                IdentifyTimeAttributeChanged(true);
            }
            else if (mCurrentEffectIdentifier == Identify::EffectIdentifierEnum::kStopEffect)
            {
                mPreviousIdentifyTime = mIdentifyTime;
                mIdentifyTime         = 0;
                IdentifyTimeAttributeChanged(true);
            }
            else
            {
                // Other effects: cancel and trigger new effect.
                mPreviousIdentifyTime = mIdentifyTime;
                mIdentifyTime         = 0;
                IdentifyTimeAttributeChanged(true); // This will call onIdentifyStop.
                mOnEffectIdentifier(this);
            }
        }
        else
        {
            mOnEffectIdentifier(this);
        }

        return DataModel::ActionReturnStatus(Protocols::InteractionModel::Status::Success);
    }
    default:
        return DefaultServerCluster::InvokeCommand(request, input_arguments, handler);
    }
}
CHIP_ERROR IdentifyCluster::AcceptedCommands(const ConcreteClusterPath & path,
                                             ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder)
{
    if (mOnEffectIdentifier)
    {
        return builder.ReferenceExisting(kAcceptedCommandsWithTriggerEffect);
    }
    return builder.ReferenceExisting(kAcceptedCommands);
}

} // namespace chip::app::Clusters
