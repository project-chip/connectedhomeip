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

#include "DynamicLightingCluster.h"
#include <app/server-cluster/AttributeListBuilder.h>
#include <clusters/DynamicLighting/Commands.h>
#include <clusters/DynamicLighting/Metadata.h>
#include <lib/support/CodeUtils.h>

namespace chip::app::Clusters {

using namespace DynamicLighting;
using namespace DynamicLighting::Attributes;
using Protocols::InteractionModel::Status;

namespace {

constexpr size_t kMaxAvailableEffects = 128;
constexpr size_t kMaxLabelLength      = 32;

CHIP_ERROR ValidateAvailableEffects(Span<const Structs::EffectStruct::Type> effects)
{
    VerifyOrReturnError(effects.size() <= kMaxAvailableEffects, CHIP_ERROR_INVALID_ARGUMENT);

    for (size_t i = 0; i < effects.size(); i++)
    {
        const auto & effect = effects[i];
        VerifyOrReturnError(!effect.label.empty() && effect.label.size() <= kMaxLabelLength, CHIP_ERROR_INVALID_ARGUMENT);
        VerifyOrReturnError(effect.maxSpeed >= 1 && effect.defaultSpeed >= 1 && effect.defaultSpeed <= effect.maxSpeed,
                            CHIP_ERROR_INVALID_ARGUMENT);

        for (size_t j = 0; j < i; j++)
        {
            VerifyOrReturnError(effects[j].effectID != effect.effectID, CHIP_ERROR_INVALID_ARGUMENT);
            VerifyOrReturnError(!effects[j].label.data_equal(effect.label), CHIP_ERROR_INVALID_ARGUMENT);
        }
    }

    return CHIP_NO_ERROR;
}

} // namespace

DynamicLightingCluster::DynamicLightingCluster(EndpointId endpointId, DynamicLightingDelegate & delegate) :
    DefaultServerCluster({ endpointId, DynamicLighting::Id }), mDelegate(delegate)
{}

CHIP_ERROR DynamicLightingCluster::Startup(ServerClusterContext & context)
{
    ReturnErrorOnFailure(ValidateAvailableEffects(mDelegate.GetAvailableEffects()));
    return DefaultServerCluster::Startup(context);
}

const Structs::EffectStruct::Type * DynamicLightingCluster::FindEffect(uint16_t effectId)
{
    for (const auto & effect : mDelegate.GetAvailableEffects())
    {
        if (effect.effectID == effectId)
        {
            return &effect;
        }
    }
    return nullptr;
}

DataModel::ActionReturnStatus DynamicLightingCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                                    AttributeValueEncoder & encoder)
{
    switch (request.path.mAttributeId)
    {
    case ClusterRevision::Id:
        return encoder.Encode(DynamicLighting::kRevision);
    case FeatureMap::Id:
        return encoder.Encode(static_cast<uint32_t>(0));
    case AvailableEffects::Id:
        return encoder.EncodeList([this](const auto & listEncoder) -> CHIP_ERROR {
            for (const auto & effect : mDelegate.GetAvailableEffects())
            {
                ReturnErrorOnFailure(listEncoder.Encode(effect));
            }
            return CHIP_NO_ERROR;
        });
    case CurrentEffectID::Id:
        return encoder.Encode(mCurrentEffectID);
    case CurrentSpeed::Id:
        return encoder.Encode(mCurrentSpeed);
    default:
        return Status::UnsupportedAttribute;
    }
}

DataModel::ActionReturnStatus DynamicLightingCluster::WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                                     AttributeValueDecoder & decoder)
{
    switch (request.path.mAttributeId)
    {
    case CurrentSpeed::Id: {
        DataModel::Nullable<uint16_t> speed;
        ReturnErrorOnFailure(decoder.Decode(speed));

        VerifyOrReturnError(!mCurrentEffectID.IsNull(), Status::InvalidInState);
        VerifyOrReturnError(!speed.IsNull() && speed.Value() >= 1, Status::ConstraintError);

        const auto * effect = FindEffect(mCurrentEffectID.Value());
        VerifyOrReturnError(effect != nullptr, Status::Failure);
        VerifyOrReturnError(speed.Value() <= effect->maxSpeed, Status::ConstraintError);

        VerifyOrReturnError(mDelegate.SetEffectSpeed(speed.Value()) == CHIP_NO_ERROR, Status::Failure);
        SetAttributeValue(mCurrentSpeed, speed.Value(), CurrentSpeed::Id);
        return Status::Success;
    }
    default:
        return Status::UnsupportedWrite;
    }
}

CHIP_ERROR DynamicLightingCluster::Attributes(const ConcreteClusterPath & path,
                                              ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    AttributeListBuilder listBuilder(builder);
    return listBuilder.Append(Span(DynamicLighting::Attributes::kMandatoryMetadata), {});
}

DataModel::ActionReturnStatus DynamicLightingCluster::StartEffect(TLV::TLVReader & input_arguments)
{
    Commands::StartEffect::DecodableType commandData;
    ReturnErrorOnFailure(DataModel::Decode(input_arguments, commandData));

    const auto * effect = FindEffect(commandData.effectID);
    VerifyOrReturnError(effect != nullptr, Status::InvalidCommand);
    VerifyOrReturnError(commandData.speed >= 1 && commandData.speed <= effect->maxSpeed, Status::InvalidCommand);

    if (!mCurrentEffectID.IsNull())
    {
        mDelegate.StopEffect();
        SetAttributeValue(mCurrentEffectID, DataModel::NullNullable, CurrentEffectID::Id);
        SetAttributeValue(mCurrentSpeed, DataModel::NullNullable, CurrentSpeed::Id);
    }

    VerifyOrReturnError(mDelegate.StartEffect(*effect, commandData.speed) == CHIP_NO_ERROR, Status::Failure);

    SetAttributeValue(mCurrentEffectID, effect->effectID, CurrentEffectID::Id);
    SetAttributeValue(mCurrentSpeed, commandData.speed, CurrentSpeed::Id);
    return Status::Success;
}

DataModel::ActionReturnStatus DynamicLightingCluster::StopEffect()
{
    VerifyOrReturnValue(!mCurrentEffectID.IsNull(), Status::Success);

    mDelegate.StopEffect();
    SetAttributeValue(mCurrentEffectID, DataModel::NullNullable, CurrentEffectID::Id);
    SetAttributeValue(mCurrentSpeed, DataModel::NullNullable, CurrentSpeed::Id);
    return Status::Success;
}

std::optional<DataModel::ActionReturnStatus> DynamicLightingCluster::InvokeCommand(const DataModel::InvokeRequest & request,
                                                                                   chip::TLV::TLVReader & input_arguments,
                                                                                   CommandHandler * handler)
{
    switch (request.path.mCommandId)
    {
    case Commands::StartEffect::Id:
        return StartEffect(input_arguments);
    case Commands::StopEffect::Id:
        return StopEffect();
    default:
        return Status::UnsupportedCommand;
    }
}

CHIP_ERROR DynamicLightingCluster::AcceptedCommands(const ConcreteClusterPath & path,
                                                    ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder)
{
    return builder.AppendElements({
        Commands::StartEffect::kMetadataEntry,
        Commands::StopEffect::kMetadataEntry,
    });
}

} // namespace chip::app::Clusters
