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
    mDiagnosticDataProvider(config.diagnosticDataProvider)
{}

CHIP_ERROR ModeBaseCluster::Startup(ServerClusterContext & context)
{
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
    if (!IsSupportedMode(aNewMode))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    uint8_t oldMode = mCurrentMode;
    mCurrentMode    = aNewMode;
    if (mCurrentMode != oldMode)
    {
        // Write new value to persistent storage.
        CHIP_ERROR err = mSafeAttributePersistenceProvider.WriteScalarValue(
            ConcreteAttributePath(mPath.mEndpointId, mPath.mClusterId, CurrentMode::Id), mCurrentMode);
        if (err != CHIP_NO_ERROR)
        {
            mCurrentMode = oldMode;
            ChipLogError(Zcl, "ModeBase: Failed to write the CurrentMode to the KVS: %" CHIP_ERROR_FORMAT, err.Format());
            return Protocols::InteractionModel::ClusterStatusCode(err).GetStatus();
        }
        NotifyAttributeChanged(CurrentMode::Id);
    }
    return Status::Success;
}

Status ModeBaseCluster::UpdateStartUpMode(DataModel::Nullable<uint8_t> aNewStartUpMode)
{
    if (!aNewStartUpMode.IsNull() && !IsSupportedMode(aNewStartUpMode.Value()))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    DataModel::Nullable<uint8_t> oldStartUpMode = mStartUpMode;
    mStartUpMode                                = aNewStartUpMode;
    if (mStartUpMode != oldStartUpMode)
    {
        // Write new value to persistent storage.
        CHIP_ERROR err = mSafeAttributePersistenceProvider.WriteScalarValue(
            ConcreteAttributePath(mPath.mEndpointId, mPath.mClusterId, StartUpMode::Id), mStartUpMode);
        if (err != CHIP_NO_ERROR)
        {
            mStartUpMode = oldStartUpMode;
            ChipLogError(Zcl, "ModeBase: Failed to write the StartUpMode to the KVS: %" CHIP_ERROR_FORMAT, err.Format());
            return Protocols::InteractionModel::ClusterStatusCode(err).GetStatus();
        }
        NotifyAttributeChanged(StartUpMode::Id);
    }
    return Protocols::InteractionModel::Status::Success;
}

Status ModeBaseCluster::UpdateOnMode(DataModel::Nullable<uint8_t> aNewOnMode)
{
    if (!aNewOnMode.IsNull() && !IsSupportedMode(aNewOnMode.Value()))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    DataModel::Nullable<uint8_t> oldOnMode = mOnMode;
    mOnMode                                = aNewOnMode;
    if (mOnMode != oldOnMode)
    {
        // Write new value to persistent storage.
        CHIP_ERROR err = mSafeAttributePersistenceProvider.WriteScalarValue(
            ConcreteAttributePath(mPath.mEndpointId, mPath.mClusterId, OnMode::Id), mOnMode);
        if (err != CHIP_NO_ERROR)
        {
            mOnMode = oldOnMode;
            ChipLogError(Zcl, "ModeBase: Failed to write the OnMode to the KVS: %" CHIP_ERROR_FORMAT, err.Format());
            return Protocols::InteractionModel::ClusterStatusCode(err).GetStatus();
        }
        NotifyAttributeChanged(OnMode::Id);
    }
    return Protocols::InteractionModel::Status::Success;
}

void ModeBaseCluster::ReportSupportedModesChange()
{
    NotifyAttributeChanged(SupportedModes::Id);
}

bool ModeBaseCluster::IsSupportedMode(uint8_t modeValue)
{
    uint8_t value;
    for (uint8_t i = 0; mAppDelegate.GetModeValueByIndex(i, value) != CHIP_ERROR_PROVIDER_LIST_EXHAUSTED; i++)
    {
        if (value == modeValue)
        {
            return true;
        }
    }
    ChipLogDetail(Zcl, "Cannot find a mode with value %u", modeValue);
    return false;
}

CHIP_ERROR ModeBaseCluster::GetModeValueByModeTag(uint16_t modeTagValue, uint8_t & value)
{
    ModeTagStructType tagsBuffer[kMaxNumOfModeTags];
    DataModel::List<ModeTagStructType> mTags(tagsBuffer);
    for (uint8_t i = 0; mAppDelegate.GetModeTagsByIndex(i, mTags) != CHIP_ERROR_PROVIDER_LIST_EXHAUSTED; i++)
    {
        for (size_t ii = 0; ii < mTags.size(); ii++)
        {
            if (mTags[ii].value == modeTagValue)
            {
                return mAppDelegate.GetModeValueByIndex(i, value);
            }
        }
        mTags = tagsBuffer;
    }
    ChipLogDetail(Zcl, "Cannot find a mode with mode tag %x", modeTagValue);
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
        return Protocols::InteractionModel::Status::UnsupportedCommand;
    }
}

DataModel::ActionReturnStatus ModeBaseCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                             AttributeValueEncoder & encoder)
{
    switch (request.path.mAttributeId)
    {
    case ClusterRevision::Id:
        // TODO: We need a way to know the current revision of the cluster.
        // For now, we are returning a hardcoded value of 1.
        return encoder.Encode<uint16_t>(1);
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
        return Protocols::InteractionModel::Status::UnsupportedAttribute;
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
        return Protocols::InteractionModel::Status::UnsupportedAttribute;
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
    ReturnErrorOnFailure(builder.AppendElements({ Commands::ChangeToMode::kMetadataEntry }));
    return CHIP_NO_ERROR;
}

CHIP_ERROR ModeBaseCluster::GeneratedCommands(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<CommandId> & builder)
{
    ReturnErrorOnFailure(builder.AppendElements({ Commands::ChangeToModeResponse::Id }));
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
        TEMPORARY_RETURN_IGNORED UpdateCurrentMode(newMode);
        ChipLogProgress(Zcl, "ModeBase: HandleChangeToMode changed to mode %u", newMode);
    }

    commandObj.AddResponse(commandPath, response);
    return std::nullopt;
}

void ModeBaseCluster::LoadPersistentAttributes()
{
    // Load Current Mode
    uint8_t tempCurrentMode;
    CHIP_ERROR err = mSafeAttributePersistenceProvider.ReadScalarValue(
        ConcreteAttributePath(mPath.mEndpointId, mPath.mClusterId, CurrentMode::Id), tempCurrentMode);
    if (err == CHIP_NO_ERROR)
    {
        Status status = UpdateCurrentMode(tempCurrentMode);
        if (status == Status::Success)
        {
            ChipLogDetail(Zcl, "ModeBase: Loaded CurrentMode as %u", GetCurrentMode());
        }
        else
        {
            ChipLogError(Zcl, "ModeBase: Could not update CurrentMode to %u: %u", tempCurrentMode, to_underlying(status));
        }
    }
    else
    {
        // If we cannot find the previous CurrentMode, we will assume it to be the first mode in the
        // list, as was initialised in the constructor.
        ChipLogDetail(Zcl, "ModeBase: Unable to load the CurrentMode from the KVS. Assuming %u", GetCurrentMode());
    }

    // Load Start-Up Mode
    DataModel::Nullable<uint8_t> tempStartUpMode;
    err = mSafeAttributePersistenceProvider.ReadScalarValue(
        ConcreteAttributePath(mPath.mEndpointId, mPath.mClusterId, StartUpMode::Id), tempStartUpMode);
    if (err == CHIP_NO_ERROR)
    {
        Status status = UpdateStartUpMode(tempStartUpMode);
        if (status == Status::Success)
        {
            if (GetStartUpMode().IsNull())
            {
                ChipLogDetail(Zcl, "ModeBase: Loaded StartUpMode as null");
            }
            else
            {
                ChipLogDetail(Zcl, "ModeBase: Loaded StartUpMode as %u", GetStartUpMode().Value());
            }
        }
        else
        {
            ChipLogError(Zcl, "ModeBase: Could not update StartUpMode: %u", to_underlying(status));
        }
    }
    else
    {
        ChipLogDetail(Zcl, "ModeBase: Unable to load the StartUpMode from the KVS. Assuming null");
    }

    // Load On Mode
    DataModel::Nullable<uint8_t> tempOnMode;
    err = mSafeAttributePersistenceProvider.ReadScalarValue(ConcreteAttributePath(mPath.mEndpointId, mPath.mClusterId, OnMode::Id),
                                                            tempOnMode);
    if (err == CHIP_NO_ERROR)
    {
        Status status = UpdateOnMode(tempOnMode);
        if (status == Status::Success)
        {
            if (GetOnMode().IsNull())
            {
                ChipLogDetail(Zcl, "ModeBase: Loaded OnMode as null");
            }
            else
            {
                ChipLogDetail(Zcl, "ModeBase: Loaded OnMode as %u", GetOnMode().Value());
            }
        }
        else
        {
            ChipLogError(Zcl, "ModeBase: Could not update OnMode: %u", to_underlying(status));
        }
    }
    else
    {
        ChipLogDetail(Zcl, "ModeBase: Unable to load the OnMode from the KVS. Assuming null");
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
