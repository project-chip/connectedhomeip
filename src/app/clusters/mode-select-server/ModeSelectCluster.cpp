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

#include <app/persistence/AttributePersistence.h>
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

namespace {

class ModeSelectSceneValidator : public scenes::AttributeValuePairValidator
{
public:
    CHIP_ERROR Validate(const app::ConcreteClusterPath & clusterPath,
                        AttributeValuePairValidator::AttributeValuePairType & value) override
    {
        VerifyOrReturnError(clusterPath.mClusterId == ModeSelect::Id, CHIP_ERROR_INVALID_ARGUMENT);
        VerifyOrReturnError(value.attributeID == CurrentMode::Id, CHIP_ERROR_INVALID_ARGUMENT);
        return CHIP_NO_ERROR;
    }
};

ModeSelectSceneValidator & GlobalValidator()
{
    static ModeSelectSceneValidator sValidator;
    return sValidator;
}

} // namespace

ModeSelectCluster::ModeSelectCluster(EndpointId endpointId, Delegate & delegate, const Config & config) :
    DefaultServerCluster({ endpointId, ModeSelect::Id }), DefaultSceneHandlerImpl(GlobalValidator()), mDelegate(delegate),
    mFeatureMap(config.featureMap), mOptionalAttributeSet(config.optionalAttributeSet), mDescription(config.description),
    mStandardNamespace(config.standardNamespace), mOnOffValueForStartUp(config.onOffValueForStartUp),
    mDiagnosticDataProvider(config.diagnosticDataProvider), mStartUpMode(config.initialStartUpMode), mOnMode(config.initialOnMode)
{}

CHIP_ERROR ModeSelectCluster::Startup(ServerClusterContext & context)
{
    ReturnErrorOnFailure(DefaultServerCluster::Startup(context));
    LoadPersistentAttributes(context.attributeStorage);
    // Modes may not be available yet if setSupportedModesManager() hasn't been called
    // (it's called in ApplicationInit, after Server::Init). ApplyStartupModeLogic()
    // will be called again from setSupportedModesManager() in that case.
    ApplyStartupModeLogic();
    return CHIP_NO_ERROR;
}

void ModeSelectCluster::ApplyStartupModeLogic()
{
    if (mContext == nullptr)
    {
        return; // Startup() has not run yet
    }

    auto modes = mDelegate.GetSupportedModes();
    if (modes.empty())
    {
        return;
    }

    if (!IsSupportedMode(mCurrentMode))
    {
        // First boot or stale persisted value: initialize to first available mode.
        SetAttributeValue(mCurrentMode, modes[0].mode, CurrentMode::Id);
        LogErrorOnFailure(mContext->attributeStorage.WriteValue(
            ConcreteAttributePath(mPath.mEndpointId, mPath.mClusterId, CurrentMode::Id),
            ByteSpan(reinterpret_cast<const uint8_t *>(&mCurrentMode), sizeof(mCurrentMode))));
    }

    if (!mStartUpMode.IsNull())
    {
        BootReasonType bootReason = BootReasonType::kUnspecified;
        CHIP_ERROR err            = mDiagnosticDataProvider.GetBootReason(bootReason);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(Zcl, "ModeSelect: Unable to retrieve boot reason: %" CHIP_ERROR_FORMAT ". Assuming non-OTA reboot",
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
                ChipLogError(Zcl, "ModeSelect: StartUpMode %u not supported, ignoring.", mStartUpMode.Value());
            }
        }
    }

    if (mOnOffValueForStartUp && !mOnMode.IsNull() && mOnMode.Value() != mCurrentMode)
    {
        ChipLogProgress(Zcl, "ModeSelect: Applying OnMode %u to CurrentMode.", mOnMode.Value());
        Status status = UpdateCurrentMode(mOnMode.Value());
        if (status != Status::Success)
        {
            ChipLogError(Zcl, "ModeSelect: OnMode %u not supported, ignoring.", mOnMode.Value());
        }
    }
}

void ModeSelectCluster::LoadPersistentAttributes(AttributePersistenceProvider & provider)
{
    AttributePersistence persistence{ provider };
    persistence.LoadNativeEndianValue({ mPath.mEndpointId, mPath.mClusterId, CurrentMode::Id }, mCurrentMode, mCurrentMode);

    if (mOptionalAttributeSet.IsSet(StartUpMode::Id))
    {
        persistence.LoadNativeEndianValue({ mPath.mEndpointId, mPath.mClusterId, StartUpMode::Id }, mStartUpMode, mStartUpMode);
    }

    if (mFeatureMap.Has(Feature::kOnOff))
    {
        persistence.LoadNativeEndianValue({ mPath.mEndpointId, mPath.mClusterId, OnMode::Id }, mOnMode, mOnMode);
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
        mContext->attributeStorage.WriteValue(ConcreteAttributePath(mPath.mEndpointId, mPath.mClusterId, CurrentMode::Id),
                                              ByteSpan(reinterpret_cast<const uint8_t *>(&mCurrentMode), sizeof(mCurrentMode))));
    return Status::Success;
}

Status ModeSelectCluster::UpdateStartUpMode(DataModel::Nullable<uint8_t> newStartUpMode)
{
    VerifyOrReturnValue(newStartUpMode.IsNull() || IsSupportedMode(newStartUpMode.Value()), Status::ConstraintError);
    VerifyOrReturnValue(SetAttributeValue(mStartUpMode, newStartUpMode, StartUpMode::Id), Status::Success);

    // Null sentinel for nullable uint8 is 0xFF per Matter spec.
    uint8_t storageVal = mStartUpMode.IsNull() ? static_cast<uint8_t>(0xFF) : mStartUpMode.Value();
    LogErrorOnFailure(mContext->attributeStorage.WriteValue(
        ConcreteAttributePath(mPath.mEndpointId, mPath.mClusterId, StartUpMode::Id), ByteSpan(&storageVal, sizeof(storageVal))));
    return Status::Success;
}

Status ModeSelectCluster::UpdateOnMode(DataModel::Nullable<uint8_t> newOnMode)
{
    VerifyOrReturnValue(newOnMode.IsNull() || IsSupportedMode(newOnMode.Value()), Status::ConstraintError);
    VerifyOrReturnValue(SetAttributeValue(mOnMode, newOnMode, OnMode::Id), Status::Success);

    // Null sentinel for nullable uint8 is 0xFF per Matter spec.
    uint8_t storageVal = mOnMode.IsNull() ? static_cast<uint8_t>(0xFF) : mOnMode.Value();
    LogErrorOnFailure(mContext->attributeStorage.WriteValue(ConcreteAttributePath(mPath.mEndpointId, mPath.mClusterId, OnMode::Id),
                                                            ByteSpan(&storageVal, sizeof(storageVal))));
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

std::optional<DataModel::ActionReturnStatus> ModeSelectCluster::InvokeCommand(const DataModel::InvokeRequest & request,
                                                                              TLV::TLVReader & input_arguments,
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

    static constexpr DataModel::AttributeEntry kMandatory[] = {
        Description::kMetadataEntry,
        StandardNamespace::kMetadataEntry,
        SupportedModes::kMetadataEntry,
        CurrentMode::kMetadataEntry,
    };

    const AttributeListBuilder::OptionalAttributeEntry optionalAttributes[] = {
        { mOptionalAttributeSet.IsSet(StartUpMode::Id), StartUpMode::kMetadataEntry },
        { mFeatureMap.Has(Feature::kOnOff), OnMode::kMetadataEntry },
    };

    return listBuilder.Append(Span(kMandatory), Span(optionalAttributes));
}

CHIP_ERROR ModeSelectCluster::AcceptedCommands(const ConcreteClusterPath & path,
                                               ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder)
{
    return builder.AppendElements({ Commands::ChangeToMode::kMetadataEntry });
}

bool ModeSelectCluster::SupportsCluster(EndpointId endpoint, ClusterId cluster)
{
    return (cluster == ModeSelect::Id) && (endpoint == mPath.mEndpointId);
}

CHIP_ERROR ModeSelectCluster::SerializeSave(EndpointId endpoint, ClusterId cluster, MutableByteSpan & serializedBytes)
{
    using AttributeValuePair = ScenesManagement::Structs::AttributeValuePairStruct::Type;

    VerifyOrReturnError(SupportsCluster(endpoint, cluster), CHIP_ERROR_INVALID_ARGUMENT);

    AttributeValuePair pairs[1];
    pairs[0].attributeID = CurrentMode::Id;
    pairs[0].valueUnsigned8.SetValue(mCurrentMode);

    app::DataModel::List<AttributeValuePair> attributeValueList(pairs);
    return EncodeAttributeValueList(attributeValueList, serializedBytes);
}

CHIP_ERROR ModeSelectCluster::ApplyScene(EndpointId endpoint, ClusterId cluster, const ByteSpan & serializedBytes,
                                         scenes::TransitionTimeMs timeMs)
{
    VerifyOrReturnError(SupportsCluster(endpoint, cluster), CHIP_ERROR_INVALID_ARGUMENT);

    app::DataModel::DecodableList<ScenesManagement::Structs::AttributeValuePairStruct::DecodableType> attributeValueList;
    ReturnErrorOnFailure(DecodeAttributeValueList(serializedBytes, attributeValueList));

    auto pair_iterator = attributeValueList.begin();
    while (pair_iterator.Next())
    {
        auto & decodePair = pair_iterator.GetValue();
        VerifyOrReturnError(decodePair.attributeID == CurrentMode::Id, CHIP_ERROR_INVALID_ARGUMENT);
        VerifyOrReturnError(decodePair.valueUnsigned8.HasValue(), CHIP_ERROR_INVALID_ARGUMENT);

        Status status = UpdateCurrentMode(decodePair.valueUnsigned8.Value());
        VerifyOrReturnError(status == Status::Success, StatusIB(status).ToChipError());
    }
    return pair_iterator.GetStatus();
}
