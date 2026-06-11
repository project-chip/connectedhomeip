/*
 *
 *    Copyright (c) 2023-2026 Project CHIP Authors
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

#include <app/clusters/mode-base-server/ModeBaseCluster.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <platform/PlatformManager.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ModeBase;
using namespace chip::app::Clusters::ModeBase::Attributes;
using chip::Protocols::InteractionModel::Status;
using BootReasonType       = GeneralDiagnostics::BootReasonEnum;
using ModeOptionStructType = chip::app::Clusters::detail::Structs::ModeOptionStruct::Type;
using ModeTagStructType    = chip::app::Clusters::detail::Structs::ModeTagStruct::Type;

namespace chip::app::Clusters {

namespace {

// Cluster constants, from the spec.
constexpr uint8_t kMaxModeLabelSize = 64;
constexpr uint8_t kMaxNumOfModeTags = 8;

} // namespace

ModeBaseCluster::ModeBaseCluster(EndpointId endpointId, ClusterId aClusterId, const Config & config) :
    DefaultServerCluster({ endpointId, aClusterId }), mFeature(config.feature), mOptionalAttributeSet(config.optionalAttributeSet),
    mAppDelegate(config.appDelegate), mOnOffValueForStartUp(config.onOffValueForStartUp),
    mSafeAttributePersistenceProvider(config.safeAttributePersistenceProvider),
    mDiagnosticDataProvider(config.diagnosticDataProvider), mClusterRevision(config.clusterRevision)
{}

CHIP_ERROR ModeBaseCluster::Startup(ServerClusterContext & context)
{
    ReturnErrorOnFailure(DefaultServerCluster::Startup(context));

    // Initialise the current mode with the value of the first mode. This ensures that it is representing a valid mode.
    ReturnErrorOnFailure(mAppDelegate.GetModeValueByIndex(0, mCurrentMode));

    LoadPersistentAttributes();

    // If the StartUpMode is set, the CurrentMode attribute SHALL be set to the StartUpMode value, when the server is powered up.
    if (!mStartUpMode.IsNull())
    {
        // This behavior does not apply to reboots associated with OTA.
        // After an OTA restart, the CurrentMode attribute SHALL return to its value prior to the restart.
        // todo this only works for matter OTAs. According to the spec, this should also work for general OTAs.
        BootReasonType bootReason = BootReasonType::kUnspecified;
        CHIP_ERROR err            = mDiagnosticDataProvider.GetBootReason(bootReason);

        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(
                Zcl, "Unable to retrieve boot reason: %" CHIP_ERROR_FORMAT ". Assuming that we did not reboot because of an OTA",
                err.Format());
            bootReason = BootReasonType::kUnspecified;
        }

        if (bootReason == BootReasonType::kSoftwareUpdateCompleted)
        {
            ChipLogDetail(Zcl, "ModeBase: StartUpMode is ignored for OTA reboot.");
        }
        else
        {
            // Set CurrentMode to StartUpMode
            if (mStartUpMode.Value() != mCurrentMode)
            {
                ChipLogProgress(Zcl, "ModeBase: Changing CurrentMode to the StartUpMode value.");
                Status status = UpdateCurrentMode(mStartUpMode.Value());
                if (status != Status::Success)
                {
                    ChipLogError(Zcl, "ModeBase: Failed to change the CurrentMode to the StartUpMode value: %u",
                                 to_underlying(status));
                    return StatusIB(status).ToChipError();
                }

                ChipLogProgress(Zcl, "ModeBase: Successfully initialized CurrentMode to the StartUpMode value %u",
                                mStartUpMode.Value());
            }
        }
    }

    if (mOnOffValueForStartUp && !mOnMode.IsNull())
    {
        // Set CurrentMode to OnMode
        if (mOnMode.Value() != mCurrentMode)
        {
            ChipLogProgress(Zcl, "ModeBase: Changing CurrentMode to the OnMode value.");
            Status status = UpdateCurrentMode(mOnMode.Value());
            if (status != Status::Success)
            {
                ChipLogError(Zcl, "ModeBase: Failed to change the CurrentMode to the OnMode value: %u", to_underlying(status));
                return StatusIB(status).ToChipError();
            }

            ChipLogProgress(Zcl, "ModeBase: Successfully initialized CurrentMode to the OnMode value %u", mOnMode.Value());
        }
    }

    return CHIP_NO_ERROR;
}

Status ModeBaseCluster::UpdateCurrentMode(uint8_t aNewMode)
{
    VerifyOrReturnValue(IsSupportedMode(aNewMode), Status::ConstraintError);
    VerifyOrReturnValue(SetAttributeValue(mCurrentMode, aNewMode, CurrentMode::Id), Status::Success);

    // Write new value to persistent storage.
    LogErrorOnFailure(
        mSafeAttributePersistenceProvider.WriteScalarValue({ mPath.mEndpointId, mPath.mClusterId, CurrentMode::Id }, mCurrentMode));
    return Status::Success;
}

Status ModeBaseCluster::UpdateStartUpMode(DataModel::Nullable<uint8_t> aNewStartUpMode)
{
    VerifyOrReturnValue(aNewStartUpMode.IsNull() || IsSupportedMode(aNewStartUpMode.Value()), Status::ConstraintError);
    VerifyOrReturnValue(SetAttributeValue(mStartUpMode, aNewStartUpMode, StartUpMode::Id), Status::Success);

    // Write new value to persistent storage.
    LogErrorOnFailure(
        mSafeAttributePersistenceProvider.WriteScalarValue({ mPath.mEndpointId, mPath.mClusterId, StartUpMode::Id }, mStartUpMode));
    return Status::Success;
}

Status ModeBaseCluster::UpdateOnMode(DataModel::Nullable<uint8_t> aNewOnMode)
{
    VerifyOrReturnValue(aNewOnMode.IsNull() || IsSupportedMode(aNewOnMode.Value()), Status::ConstraintError);
    VerifyOrReturnValue(SetAttributeValue(mOnMode, aNewOnMode, OnMode::Id), Status::Success);

    // Write new value to persistent storage.
    LogErrorOnFailure(
        mSafeAttributePersistenceProvider.WriteScalarValue({ mPath.mEndpointId, mPath.mClusterId, OnMode::Id }, mOnMode));
    return Status::Success;
}

void ModeBaseCluster::ReportSupportedModesChange()
{
    NotifyAttributeChanged(SupportedModes::Id);
}

bool ModeBaseCluster::IsSupportedMode(uint8_t modeValue)
{
    uint8_t value;
    CHIP_ERROR err;
    for (uint8_t i = 0; (err = mAppDelegate.GetModeValueByIndex(i, value)) != CHIP_ERROR_PROVIDER_LIST_EXHAUSTED; i++)
    {
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(Zcl, "ModeBase: Failed to get the mode value by index %u: %" CHIP_ERROR_FORMAT, i, err.Format());
            return false;
        }
        if (value == modeValue)
        {
            return true;
        }
    }
    ChipLogDetail(Zcl, "ModeBase: Cannot find a mode with value %u", modeValue);
    return false;
}

CHIP_ERROR ModeBaseCluster::GetModeValueByModeTag(uint16_t modeTagValue, uint8_t & value)
{
    ModeTagStructType tagsBuffer[kMaxNumOfModeTags];
    DataModel::List<ModeTagStructType> mTags(tagsBuffer);
    CHIP_ERROR err;
    for (uint8_t i = 0; (err = mAppDelegate.GetModeTagsByIndex(i, mTags)) != CHIP_ERROR_PROVIDER_LIST_EXHAUSTED; i++)
    {
        ReturnErrorOnFailure(err);
        for (size_t ii = 0; ii < mTags.size(); ii++)
        {
            if (mTags[ii].value == modeTagValue)
            {
                return mAppDelegate.GetModeValueByIndex(i, value);
            }
        }
        mTags = tagsBuffer;
    }
    ChipLogDetail(Zcl, "ModeBase: Cannot find a mode with mode tag %x", modeTagValue);
    return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
}

std::optional<DataModel::ActionReturnStatus>
ModeBaseCluster::InvokeCommand(const DataModel::InvokeRequest & request, TLV::TLVReader & input_arguments, CommandHandler * handler)
{
    switch (request.path.mCommandId)
    {
    case Commands::ChangeToMode::Id: {
        Commands::ChangeToMode::DecodableType data;
        ReturnErrorOnFailure(data.Decode(input_arguments));
        return HandleChangeToMode(*handler, request.path, data);
    }
    default:
        return Status::UnsupportedCommand;
    }
}

DataModel::ActionReturnStatus ModeBaseCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                             AttributeValueEncoder & encoder)
{
    switch (request.path.mAttributeId)
    {
    case ClusterRevision::Id:
        return encoder.Encode(mClusterRevision);
    case SupportedModes::Id:
        return encoder.EncodeList([this](const auto & encod) -> CHIP_ERROR { return EncodeSupportedModes(encod); });
    case CurrentMode::Id:
        return encoder.Encode(mCurrentMode);
    case StartUpMode::Id:
        return encoder.Encode(mStartUpMode);
    case OnMode::Id:
        return encoder.Encode(mOnMode);
    case FeatureMap::Id:
        return encoder.Encode(mFeature);
    default:
        return Status::UnsupportedAttribute;
    }
}

DataModel::ActionReturnStatus ModeBaseCluster::WriteAttribute(const DataModel::WriteAttributeRequest & request,
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

CHIP_ERROR ModeBaseCluster::Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    AttributeListBuilder listBuilder(builder);

    const AttributeListBuilder::OptionalAttributeEntry optionalAttributes[] = {
        { mOptionalAttributeSet.IsSet(StartUpMode::Id), StartUpMode::kMetadataEntry },
        { mFeature.Has(Feature::kOnOff), OnMode::kMetadataEntry },
    };

    return listBuilder.Append(Span(kMandatoryMetadata), Span(optionalAttributes));
}

CHIP_ERROR ModeBaseCluster::AcceptedCommands(const ConcreteClusterPath & path,
                                             ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder)
{
    // MicrowaveOvenMode is a special case. It does not support the ChangeToMode command.
    if (mPath.mClusterId != MicrowaveOvenMode::Id)
    {
        return builder.AppendElements({ Commands::ChangeToMode::kMetadataEntry });
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR ModeBaseCluster::GeneratedCommands(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<CommandId> & builder)
{
    // MicrowaveOvenMode is a special case. It does not support the ChangeToModeResponse command.
    if (mPath.mClusterId != MicrowaveOvenMode::Id)
    {
        return builder.AppendElements({ Commands::ChangeToModeResponse::Id });
    }
    return CHIP_NO_ERROR;
}

std::optional<DataModel::ActionReturnStatus>
ModeBaseCluster::HandleChangeToMode(CommandHandler & commandObj, const ConcreteCommandPath & commandPath,
                                    const Commands::ChangeToMode::DecodableType & commandData)
{
    uint8_t newMode = commandData.newMode;

    Commands::ChangeToModeResponse::Type response;

    // If the NewMode field doesn't match the Mode field of any entry of the SupportedModes list,
    // the ChangeToModeResponse command's Status field SHALL indicate UnsupportedMode and
    // the StatusText field SHALL be included and MAY be used to indicate the issue, with a human readable string,
    // or include an empty string.
    // We are leaving the StatusText empty since the Status is descriptive enough.
    if (!IsSupportedMode(newMode))
    {
        ChipLogError(Zcl, "ModeBase: Failed to find the option with mode %u", newMode);
        response.status = to_underlying(StatusCode::kUnsupportedMode);
        commandObj.AddResponse(commandPath, response);
        return std::nullopt;
    }

    // If the NewMode field is the same as the value of the CurrentMode attribute
    // the ChangeToModeResponse command SHALL have the Status field set to Success and
    // the StatusText field MAY be supplied with a human readable string or include an empty string.
    // We are leaving the StatusText empty since the Status is descriptive enough.
    if (newMode == GetCurrentMode())
    {
        response.status = to_underlying(StatusCode::kSuccess);
        commandObj.AddResponse(commandPath, response);
        return std::nullopt;
    }

    mAppDelegate.HandleChangeToMode(newMode, response);

    if (response.status == to_underlying(StatusCode::kSuccess))
    {
        Status status = UpdateCurrentMode(newMode);
        if (status != Status::Success)
        {
            response.status = to_underlying(StatusCode::kGenericFailure);
        }
        else
        {
            ChipLogProgress(Zcl, "ModeBase: HandleChangeToMode changed to mode %u", newMode);
        }
    }

    commandObj.AddResponse(commandPath, response);
    return std::nullopt;
}

void ModeBaseCluster::LogStatus(Status status, const uint8_t & value, const char * attributeName)
{
    if (status == Status::Success)
    {
        ChipLogDetail(Zcl, "ModeBase: Loaded %s as %u", attributeName, value);
    }
    else
    {
        ChipLogError(Zcl, "ModeBase: Could not update %s to %u: %u", attributeName, value, to_underlying(status));
    }
}

void ModeBaseCluster::LogStatus(Status status, const DataModel::Nullable<uint8_t> & value, const char * attributeName)
{
    if (status == Status::Success)
    {
        if (!value.IsNull())
        {
            ChipLogDetail(Zcl, "ModeBase: Loaded %s as %u", attributeName, value.Value());
        }
        else
        {
            ChipLogDetail(Zcl, "ModeBase: Loaded %s as null", attributeName);
        }
    }
    else
    {
        if (!value.IsNull())
        {
            ChipLogError(Zcl, "ModeBase: Could not update %s to %u: %u", attributeName, value.Value(), to_underlying(status));
        }
        else
        {
            ChipLogError(Zcl, "ModeBase: Could not update %s to null: %u", attributeName, to_underlying(status));
        }
    }
}

void ModeBaseCluster::LoadPersistentAttributes()
{
    uint8_t currentMode = 0;
    DataModel::Nullable<uint8_t> startUpMode;
    DataModel::Nullable<uint8_t> onMode;

    CHIP_ERROR err =
        mSafeAttributePersistenceProvider.ReadScalarValue({ mPath.mEndpointId, mPath.mClusterId, CurrentMode::Id }, currentMode);
    if (err == CHIP_NO_ERROR)
    {
        LogStatus(UpdateCurrentMode(currentMode), currentMode, "CurrentMode");
    }
    else
    {
        ChipLogError(Zcl, "ModeBase: Unable to load the CurrentMode from the KVS. Assuming zero.");
    }

    if (mOptionalAttributeSet.IsSet(StartUpMode::Id))
    {
        err = mSafeAttributePersistenceProvider.ReadScalarValue({ mPath.mEndpointId, mPath.mClusterId, StartUpMode::Id },
                                                                startUpMode);
        if (err == CHIP_NO_ERROR)
        {
            LogStatus(UpdateStartUpMode(startUpMode), startUpMode, "StartUpMode");
        }
        else
        {
            ChipLogError(Zcl, "ModeBase: Unable to load the StartUpMode from the KVS. Assuming null.");
        }
    }

    if (mFeature.Has(Feature::kOnOff))
    {
        err = mSafeAttributePersistenceProvider.ReadScalarValue({ mPath.mEndpointId, mPath.mClusterId, OnMode::Id }, onMode);
        if (err == CHIP_NO_ERROR)
        {
            LogStatus(UpdateOnMode(onMode), onMode, "OnMode");
        }
        else
        {
            ChipLogError(Zcl, "ModeBase: Unable to load the OnMode from the KVS. Assuming null.");
        }
    }
}

CHIP_ERROR ModeBaseCluster::EncodeSupportedModes(const AttributeValueEncoder::ListEncodeHelper & encoder)
{
    for (uint8_t i = 0; true; i++)
    {
        ModeOptionStructType mode;

        // Get the mode label
        char buffer[kMaxModeLabelSize];
        MutableCharSpan label(buffer);
        auto err = mAppDelegate.GetModeLabelByIndex(i, label);
        if (err == CHIP_ERROR_PROVIDER_LIST_EXHAUSTED)
        {
            return CHIP_NO_ERROR;
        }
        ReturnErrorOnFailure(err);

        mode.label = label;

        // Get the mode value
        ReturnErrorOnFailure(mAppDelegate.GetModeValueByIndex(i, mode.mode));

        // Get the mode tags
        ModeTagStructType tagsBuffer[kMaxNumOfModeTags];
        DataModel::List<ModeTagStructType> tags(tagsBuffer);
        ReturnErrorOnFailure(mAppDelegate.GetModeTagsByIndex(i, tags));
        mode.modeTags = tags;

        ReturnErrorOnFailure(encoder.Encode(mode));
    }
    return CHIP_NO_ERROR;
}

} // namespace chip::app::Clusters
