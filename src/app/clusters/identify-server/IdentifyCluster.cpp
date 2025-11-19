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
constexpr DataModel::AcceptedCommandEntry kAcceptedCommands[] = {
    Identify::Commands::Identify::kMetadataEntry,
};

constexpr DataModel::AcceptedCommandEntry kAcceptedCommandsWithTriggerEffect[] = {
    Identify::Commands::Identify::kMetadataEntry,
    Identify::Commands::TriggerEffect::kMetadataEntry,
};

} // namespace

IdentifyCluster::IdentifyCluster(const Config & config) :
    DefaultServerCluster({ config.endpointId, Identify::Id }), mIdentifyTime(0), mIdentifyType(config.identifyType),
    mIdentifyDelegate(config.identifyDelegate), mEffectIdentifier(config.effectIdentifier), mEffectVariant(config.effectVariant),
    mTimerDelegate(config.timerDelegate)
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
        uint16_t newIdentifyTime;
        ReturnErrorOnFailure(decoder.Decode(newIdentifyTime));
        return NotifyIfAttributeChanged(request.path.mAttributeId,
                                        SetIdentifyTime(IdentifyTimeChangeSource::kClient, newIdentifyTime));
    }
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
        NotifyIfAttributeChanged(
            Attributes::IdentifyTime::Id,
            SetIdentifyTime(IdentifyTimeChangeSource::kTimer, static_cast<uint16_t>(mIdentifyTime - 1)));
    }
}

// According to the spec, section 5.1 - IdentifyTime Attribute:
// Changes to this attribute SHALL only be marked as reportable in the following cases:
// 1. When it changes from 0 to any other value and vice versa, or
// 2. When it is written by a client, or
// 3. When the value is set by an Identify command.
DataModel::ActionReturnStatus IdentifyCluster::SetIdentifyTime(IdentifyTimeChangeSource source, uint16_t newTime)
{
    if (mIdentifyTime == newTime)
    {
        return DataModel::ActionReturnStatus::FixedStatus::kWriteSuccessNoOp;
    }

    uint16_t previousIdentifyTime = mIdentifyTime;
    mIdentifyTime                 = newTime;

    if (mIdentifyDelegate)
    {
        if (previousIdentifyTime == 0 && mIdentifyTime > 0)
        {
            mIdentifyDelegate->OnIdentifyStart(*this);
        }
        else if (previousIdentifyTime > 0 && mIdentifyTime == 0)
        {
            mIdentifyDelegate->OnIdentifyStop(*this);
        }
    }

    if (mIdentifyTime > 0)
    {
        ReturnErrorOnFailure(mTimerDelegate.StartTimer(this, System::Clock::Seconds16(1)));
    }
    else
    {
        mTimerDelegate.CancelTimer(this);
    }

    // Spec, section 5.1: Report on client/command write, or when transitioning to/from 0.
    if (source == IdentifyTimeChangeSource::kClient || (previousIdentifyTime > 0 && mIdentifyTime == 0) ||
        (previousIdentifyTime == 0 && mIdentifyTime > 0))
    {
        return Protocols::InteractionModel::Status::Success;
    }

    return DataModel::ActionReturnStatus::FixedStatus::kWriteSuccessNoOp;
}

std::optional<DataModel::ActionReturnStatus>
IdentifyCluster::InvokeCommand(const DataModel::InvokeRequest & request, TLV::TLVReader & input_arguments, CommandHandler * handler)
{
    switch (request.path.mCommandId)
    {
    case Identify::Commands::Identify::Id: {
        Identify::Commands::Identify::DecodableType data;
        ReturnErrorOnFailure(data.Decode(input_arguments));
        MATTER_TRACE_SCOPE("IdentifyCommand", "Identify");
        return NotifyIfAttributeChanged(Attributes::IdentifyTime::Id,
                                        SetIdentifyTime(IdentifyTimeChangeSource::kClient, data.identifyTime));
    }
    case Identify::Commands::TriggerEffect::Id: {
        Identify::Commands::TriggerEffect::DecodableType data;
        ReturnErrorOnFailure(data.Decode(input_arguments));
        MATTER_TRACE_SCOPE("TriggerEffect", "Identify");
        mEffectIdentifier = data.effectIdentifier;
        mEffectVariant    = data.effectVariant;

        ChipLogProgress(Zcl, "RX identify:trigger effect identifier 0x%X variant 0x%X", to_underlying(mEffectIdentifier),
                        to_underlying(mEffectVariant));

        // No callbacks, nothing to do.
        if (!mIdentifyDelegate)
        {
            return Protocols::InteractionModel::Status::Success;
        }

        // Not identifying, trigger effect immediately.
        if (mIdentifyTime == 0)
        {
            mIdentifyDelegate->OnTriggerEffect(*this);
            return Protocols::InteractionModel::Status::Success;
        }

        // Currently identifying: handle stop/finish effects, otherwise cancel Identify process and trigger new effect.
        if (mEffectIdentifier == Identify::EffectIdentifierEnum::kFinishEffect)
        {
            return NotifyIfAttributeChanged(Attributes::IdentifyTime::Id,
                                            SetIdentifyTime(IdentifyTimeChangeSource::kClient, 1));
        }

        if (mEffectIdentifier == Identify::EffectIdentifierEnum::kStopEffect)
        {
            return NotifyIfAttributeChanged(Attributes::IdentifyTime::Id,
                                            SetIdentifyTime(IdentifyTimeChangeSource::kClient, 0));
        }

        // Other effects: cancel and trigger new effect.
        auto err = NotifyIfAttributeChanged(
            Attributes::IdentifyTime::Id, SetIdentifyTime(IdentifyTimeChangeSource::kClient, 0)); // This will call onIdentifyStop.
        mIdentifyDelegate->OnTriggerEffect(*this);
        return err;
    }
    default:
        return Protocols::InteractionModel::Status::UnsupportedCommand;
    }
}
CHIP_ERROR IdentifyCluster::AcceptedCommands(const ConcreteClusterPath & path,
                                             ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder)
{
    if (mIdentifyDelegate && mIdentifyDelegate->IsTriggerEffectEnabled())
    {
        return builder.ReferenceExisting(kAcceptedCommandsWithTriggerEffect);
    }
    return builder.ReferenceExisting(kAcceptedCommands);
}

} // namespace chip::app::Clusters
