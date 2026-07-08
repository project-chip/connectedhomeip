/*
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <app/clusters/mode-select-server/ModeSelectCluster.h>

#include <app/server-cluster/AttributeListBuilder.h>
#include <clusters/ModeSelect/Commands.h>
#include <clusters/ModeSelect/Metadata.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ModeSelect;
using namespace chip::app::Clusters::ModeSelect::Attributes;
using chip::Protocols::InteractionModel::Status;
using BootReasonType = GeneralDiagnostics::BootReasonEnum;

ModeSelectCluster::ModeSelectCluster(EndpointId endpointId, Delegate & delegate, const Config & config) :
    DefaultServerCluster({ endpointId, ModeSelect::Id }),
    mDelegate(delegate),
    mDescription(config.description),
    mStandardNamespace(config.standardNamespace),
    mFeatureMap(config.featureMap),
    mOptionalAttributeSet(config.optionalAttributeSet),
    mOnOffValueForStartUp(config.onOffValueForStartUp),
    mPersistenceProvider(config.persistenceProvider),
    mDiagnosticDataProvider(config.diagnosticDataProvider)
{}

CHIP_ERROR ModeSelectCluster::Startup(ServerClusterContext & context)
{
    ReturnErrorOnFailure(DefaultServerCluster::Startup(context));

    auto modes = mDelegate.GetSupportedModes();
    VerifyOrReturnError(modes.size() > 0, CHIP_ERROR_INVALID_ARGUMENT);
    mCurrentMode = modes[0].mode;

    LoadPersistentAttributes();

    if (!mStartUpMode.IsNull())
    {
        BootReasonType bootReason = BootReasonType::kUnspecified;
        CHIP_ERROR err            = mDiagnosticDataProvider.GetBootReason(bootReason);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(Zcl,
                         "ModeSelect: Unable to retrieve boot reason: %" CHIP_ERROR_FORMAT ". Assuming non-OTA reboot",
                         err.Format());
            bootReason = BootReasonType::kUnspecified;
        }

        if (bootReason == BootReasonType::kSoftwareUpdateCompleted)
        {
            ChipLogDetail(Zcl, "ModeSelect: StartUpMode ignored for OTA reboot.");
        }
        else if (mStartUpMode.Value() != mCurrentMode)
        {
            ChipLogProgress(Zcl, "ModeSelect: Applying StartUpMode %u to CurrentMode.", mStartUpMode.Value());
            Status status = UpdateCurrentMode(mStartUpMode.Value());
            if (status != Status::Success)
            {
                ChipLogError(Zcl, "ModeSelect: Failed to apply StartUpMode: %u", to_underlying(status));
                return StatusIB(status).ToChipError();
            }
        }
    }

    if (mOnOffValueForStartUp && !mOnMode.IsNull() && mOnMode.Value() != mCurrentMode)
    {
        ChipLogProgress(Zcl, "ModeSelect: Applying OnMode %u to CurrentMode.", mOnMode.Value());
        Status status = UpdateCurrentMode(mOnMode.Value());
        if (status != Status::Success)
        {
            ChipLogError(Zcl, "ModeSelect: Failed to apply OnMode: %u", to_underlying(status));
            return StatusIB(status).ToChipError();
        }
    }

    return CHIP_NO_ERROR;
}

void ModeSelectCluster::LoadPersistentAttributes()
{
    CHIP_ERROR err;

    err = mPersistenceProvider.ReadScalarValue({ mPath.mEndpointId, mPath.mClusterId, CurrentMode::Id }, mCurrentMode);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogDetail(Zcl, "ModeSelect: Failed to restore CurrentMode: %" CHIP_ERROR_FORMAT, err.Format());
    }

    if (mOptionalAttributeSet.IsSet(StartUpMode::Id))
    {
        err = mPersistenceProvider.ReadScalarValue({ mPath.mEndpointId, mPath.mClusterId, StartUpMode::Id }, mStartUpMode);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogDetail(Zcl, "ModeSelect: Failed to restore StartUpMode: %" CHIP_ERROR_FORMAT, err.Format());
        }
    }

    if (mFeatureMap.Has(Feature::kOnOff))
    {
        err = mPersistenceProvider.ReadScalarValue({ mPath.mEndpointId, mPath.mClusterId, OnMode::Id }, mOnMode);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogDetail(Zcl, "ModeSelect: Failed to restore OnMode: %" CHIP_ERROR_FORMAT, err.Format());
        }
    }
}

bool ModeSelectCluster::IsSupportedMode(uint8_t mode) const
{
    for (const auto & option : mDelegate.GetSupportedModes())
    {
        if (option.mode == mode)
        {
            return true;
        }
    }
    return false;
}

Status ModeSelectCluster::UpdateCurrentMode(uint8_t newMode)
{
    VerifyOrReturnValue(IsSupportedMode(newMode), Status::ConstraintError);
    VerifyOrReturnValue(SetAttributeValue(mCurrentMode, newMode, CurrentMode::Id), Status::Success);

    LogErrorOnFailure(
        mPersistenceProvider.WriteScalarValue({ mPath.mEndpointId, mPath.mClusterId, CurrentMode::Id }, mCurrentMode));
    return Status::Success;
}

Status ModeSelectCluster::UpdateStartUpMode(DataModel::Nullable<uint8_t> newStartUpMode)
{
    VerifyOrReturnValue(newStartUpMode.IsNull() || IsSupportedMode(newStartUpMode.Value()), Status::ConstraintError);
    VerifyOrReturnValue(SetAttributeValue(mStartUpMode, newStartUpMode, StartUpMode::Id), Status::Success);

    LogErrorOnFailure(
        mPersistenceProvider.WriteScalarValue({ mPath.mEndpointId, mPath.mClusterId, StartUpMode::Id }, mStartUpMode));
    return Status::Success;
}

Status ModeSelectCluster::UpdateOnMode(DataModel::Nullable<uint8_t> newOnMode)
{
    VerifyOrReturnValue(newOnMode.IsNull() || IsSupportedMode(newOnMode.Value()), Status::ConstraintError);
    VerifyOrReturnValue(SetAttributeValue(mOnMode, newOnMode, OnMode::Id), Status::Success);

    LogErrorOnFailure(
        mPersistenceProvider.WriteScalarValue({ mPath.mEndpointId, mPath.mClusterId, OnMode::Id }, mOnMode));
    return Status::Success;
}

DataModel::ActionReturnStatus ModeSelectCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                               AttributeValueEncoder & encoder)
{
    switch (request.path.mAttributeId)
    {
    case ClusterRevision::Id:
        return encoder.Encode(kRevision);
    case FeatureMap::Id:
        return encoder.Encode(mFeatureMap);
    case Description::Id:
        return encoder.Encode(mDescription);
    case StandardNamespace::Id:
        return encoder.Encode(mStandardNamespace);
    case SupportedModes::Id:
        return encoder.EncodeList([this](const auto & encod) -> CHIP_ERROR {
            for (const auto & mode : mDelegate.GetSupportedModes())
            {
                ReturnErrorOnFailure(encod.Encode(mode));
            }
            return CHIP_NO_ERROR;
        });
    case CurrentMode::Id:
        return encoder.Encode(mCurrentMode);
    case StartUpMode::Id:
        return encoder.Encode(mStartUpMode);
    case OnMode::Id:
        return encoder.Encode(mOnMode);
    default:
        return Status::UnsupportedAttribute;
    }
}

DataModel::ActionReturnStatus ModeSelectCluster::WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                                AttributeValueDecoder & decoder)
{
    switch (request.path.mAttributeId)
    {
    case StartUpMode::Id: {
        DataModel::Nullable<uint8_t> newValue;
        ReturnErrorOnFailure(decoder.Decode(newValue));
        return UpdateStartUpMode(newValue);
    }
    case OnMode::Id: {
        DataModel::Nullable<uint8_t> newValue;
        ReturnErrorOnFailure(decoder.Decode(newValue));
        return UpdateOnMode(newValue);
    }
    default:
        return Status::UnsupportedAttribute;
    }
}

std::optional<DataModel::ActionReturnStatus>
ModeSelectCluster::InvokeCommand(const DataModel::InvokeRequest & request, TLV::TLVReader & input_arguments,
                                 CommandHandler * handler)
{
    switch (request.path.mCommandId)
    {
    case Commands::ChangeToMode::Id: {
        Commands::ChangeToMode::DecodableType data;
        ReturnErrorOnFailure(data.Decode(input_arguments));
        if (!IsSupportedMode(data.newMode))
        {
            ChipLogProgress(Zcl, "ModeSelect: Mode %u is not supported", data.newMode);
            return Status::InvalidCommand;
        }
        return UpdateCurrentMode(data.newMode);
    }
    default:
        return Status::UnsupportedCommand;
    }
}

CHIP_ERROR ModeSelectCluster::Attributes(const ConcreteClusterPath & path,
                                          ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    AttributeListBuilder listBuilder(builder);

    const AttributeListBuilder::OptionalAttributeEntry optionalAttributes[] = {
        { mOptionalAttributeSet.IsSet(StartUpMode::Id), StartUpMode::kMetadataEntry },
        { mFeatureMap.Has(Feature::kOnOff), OnMode::kMetadataEntry },
    };

    return listBuilder.Append(Span(kMandatoryMetadata), Span(optionalAttributes));
}

CHIP_ERROR ModeSelectCluster::AcceptedCommands(const ConcreteClusterPath & path,
                                               ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder)
{
    return builder.AppendElements({ Commands::ChangeToMode::kMetadataEntry });
}
