/*
 *    Copyright (c) 2023-2026 Project CHIP Authors
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

#include <app/clusters/temperature-control-server/TemperatureControlCluster.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <clusters/TemperatureControl/Metadata.h>

namespace chip::app::Clusters {

using namespace TemperatureControl;
using namespace TemperatureControl::Attributes;

// According to the spec, absolute minimum value is -27315
constexpr int16_t kMinTemperatureRange = -27315;
// According to the spec, maximum minimum value is 32766
constexpr int16_t kMaxTemperatureRange = 32766;
// According to the spec, minimum step value is 1
constexpr int16_t kMinStep = 1;
// According to the spec, maximum selected temperature level value is 31
constexpr uint8_t kMaxSelectedTemperatureLevel = 31;

TemperatureControlCluster::TemperatureControlCluster(EndpointId endpointId, const BitFlags<Feature> features,
                                                     const StartupConfiguration & config) :
    DefaultServerCluster({ endpointId, TemperatureControl::Id }),
    mFeatures(features), mDelegate(config.delegate)
{
    if (mFeatures.Has(Feature::kTemperatureNumber))
    {
        VerifyOrDie(!mFeatures.Has(Feature::kTemperatureLevel));
        VerifyOrDie(config.minTemperature >= kMinTemperatureRange && config.minTemperature <= kMaxTemperatureRange);
        VerifyOrDie(config.maxTemperature >= config.minTemperature + 1);
        VerifyOrDie(config.temperatureSetpoint >= config.minTemperature && config.temperatureSetpoint <= config.maxTemperature);

        if (mFeatures.Has(Feature::kTemperatureStep))
        {
            VerifyOrDie(config.step >= kMinStep && config.step <= (config.maxTemperature - config.minTemperature));
        }
    }
    if (mFeatures.Has(Feature::kTemperatureLevel))
    {
        VerifyOrDie(config.selectedTemperatureLevel <= kMaxSelectedTemperatureLevel);
    }

    mTemperatureSetpoint      = config.temperatureSetpoint;
    mMinTemperature           = config.minTemperature;
    mMaxTemperature           = config.maxTemperature;
    mStep                     = config.step;
    mSelectedTemperatureLevel = config.selectedTemperatureLevel;
}

DataModel::ActionReturnStatus TemperatureControlCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                                       AttributeValueEncoder & encoder)
{
    switch (request.path.mAttributeId)
    {
    case ClusterRevision::Id:
        return encoder.Encode(TemperatureControl::kRevision);
    case FeatureMap::Id:
        return encoder.Encode(mFeatures);
    case TemperatureSetpoint::Id:
        return encoder.Encode(mTemperatureSetpoint);
    case MinTemperature::Id:
        return encoder.Encode(mMinTemperature);
    case MaxTemperature::Id:
        return encoder.Encode(mMaxTemperature);
    case Step::Id:
        return encoder.Encode(mStep);
    case SelectedTemperatureLevel::Id:
        return encoder.Encode(mSelectedTemperatureLevel);
    case SupportedTemperatureLevels::Id:
        if (mDelegate == nullptr)
        {
            return encoder.EncodeEmptyList();
        }
        mDelegate->Reset(request.path.mEndpointId);
        return encoder.EncodeList([&](const auto & encod) -> CHIP_ERROR {
            constexpr uint8_t kMaxTemperatureLevelStringSize = 32;
            char buffer[kMaxTemperatureLevelStringSize]      = { 0 };
            MutableCharSpan item(buffer);
            while (mDelegate->Next(item) == CHIP_NO_ERROR)
            {
                ReturnErrorOnFailure(encod.Encode(item));
                item = MutableCharSpan(buffer);
            }
            return CHIP_NO_ERROR;
        });
    default:
        return Protocols::InteractionModel::Status::UnsupportedAttribute;
    }
}

CHIP_ERROR TemperatureControlCluster::Attributes(const ConcreteClusterPath & path,
                                                 ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    AttributeListBuilder listBuilder(builder);

    const AttributeListBuilder::OptionalAttributeEntry optionalAttributes[] = {
        { mFeatures.Has(Feature::kTemperatureNumber), TemperatureSetpoint::kMetadataEntry },
        { mFeatures.Has(Feature::kTemperatureNumber), MinTemperature::kMetadataEntry },
        { mFeatures.Has(Feature::kTemperatureNumber), MaxTemperature::kMetadataEntry },
        { mFeatures.Has(Feature::kTemperatureStep), Step::kMetadataEntry },
        { mFeatures.Has(Feature::kTemperatureLevel), SelectedTemperatureLevel::kMetadataEntry },
        { mFeatures.Has(Feature::kTemperatureLevel), SupportedTemperatureLevels::kMetadataEntry },
    };

    return listBuilder.Append(Span(kMandatoryMetadata), Span(optionalAttributes));
}

CHIP_ERROR TemperatureControlCluster::SetTemperatureSetpoint(int16_t temperatureSetpoint)
{
    VerifyOrReturnError(mFeatures.Has(Feature::kTemperatureNumber), CHIP_IM_GLOBAL_STATUS(InvalidInState));
    VerifyOrReturnError(temperatureSetpoint >= mMinTemperature && temperatureSetpoint <= mMaxTemperature,
                        CHIP_IM_GLOBAL_STATUS(ConstraintError));

    if (mFeatures.Has(Feature::kTemperatureStep))
    {
        VerifyOrReturnError((temperatureSetpoint - mMinTemperature) % mStep == 0, CHIP_IM_GLOBAL_STATUS(ConstraintError));
    }

    SetAttributeValue(mTemperatureSetpoint, temperatureSetpoint, TemperatureSetpoint::Id);
    return CHIP_NO_ERROR;
}

CHIP_ERROR TemperatureControlCluster::SetSelectedTemperatureLevel(uint8_t selectedTemperatureLevel)
{
    VerifyOrReturnError(mFeatures.Has(Feature::kTemperatureLevel), CHIP_IM_GLOBAL_STATUS(InvalidInState));
    VerifyOrReturnError(mDelegate != nullptr, CHIP_IM_GLOBAL_STATUS(NotFound));
    VerifyOrReturnError(selectedTemperatureLevel < mDelegate->Size(), CHIP_IM_GLOBAL_STATUS(ConstraintError));

    mDelegate->Reset(mPath.mEndpointId);
    SetAttributeValue(mSelectedTemperatureLevel, selectedTemperatureLevel, SelectedTemperatureLevel::Id);
    return CHIP_NO_ERROR;
}

std::optional<DataModel::ActionReturnStatus> TemperatureControlCluster::InvokeCommand(const DataModel::InvokeRequest & request,
                                                                                      TLV::TLVReader & input_arguments,
                                                                                      CommandHandler * handler)
{
    switch (request.path.mCommandId)
    {
    case Commands::SetTemperature::Id: {
        Commands::SetTemperature::DecodableType data;
        ReturnErrorOnFailure(data.Decode(input_arguments));
        return HandleSetTemperature(handler, request.path, data);
    }
    default:
        return Protocols::InteractionModel::Status::UnsupportedCommand;
    }
}

CHIP_ERROR TemperatureControlCluster::AcceptedCommands(const ConcreteClusterPath & path,
                                                       ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder)
{
    return builder.AppendElements({
        Commands::SetTemperature::kMetadataEntry,
    });
}

std::optional<DataModel::ActionReturnStatus>
TemperatureControlCluster::HandleSetTemperature(CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                                                const TemperatureControl::Commands::SetTemperature::DecodableType & commandData)
{
    auto & targetTemperature      = commandData.targetTemperature;
    auto & targetTemperatureLevel = commandData.targetTemperatureLevel;
    using namespace chip::Protocols::InteractionModel;

    if (mFeatures.Has(Feature::kTemperatureNumber))
    {
        if (targetTemperature.HasValue())
        {
            CHIP_ERROR err = SetTemperatureSetpoint(targetTemperature.Value());
            if (err == CHIP_IM_GLOBAL_STATUS(ConstraintError))
            {
                return Status::ConstraintError;
            }
            else if (err != CHIP_NO_ERROR)
            {
                /**
                 * If the server is unable to execute the command at the time the command is received
                 * by the server (e.g. due to the design of a device it cannot accept a change in its
                 * temperature setting after it has begun operation), then the server SHALL respond
                 * with a status code of INVALID_IN_STATE, and discard the command.
                 **/
                return Status::InvalidInState;
            }
        }
        else
        {
            return Status::InvalidCommand;
        }
    }
    if (mFeatures.Has(Feature::kTemperatureLevel))
    {
        if (targetTemperatureLevel.HasValue())
        {
            CHIP_ERROR err = SetSelectedTemperatureLevel(targetTemperatureLevel.Value());
            if (err == CHIP_IM_GLOBAL_STATUS(NotFound))
            {
                return Status::NotFound;
            }
            else if (err == CHIP_IM_GLOBAL_STATUS(ConstraintError))
            {
                return Status::ConstraintError;
            }
            else if (err != CHIP_NO_ERROR)
            {
                /**
                 * If the server is unable to execute the command at the time the command is received
                 * by the server (e.g. due to the design of a device it cannot accept a change in its
                 * temperature setting after it has begun operation), then the server SHALL respond
                 * with a status code of INVALID_IN_STATE, and discard the command.
                 **/
                return Status::InvalidInState;
            }
        }
        else
        {
            return Status::InvalidCommand;
        }
    }

    return Status::Success;
}

} // namespace chip::app::Clusters
