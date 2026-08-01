/**
 *    Copyright (c) 2024-2025 Project CHIP Authors
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

#include "ThermostatClusterPresets.h"
#include "PresetStructWithOwnedMembers.h"
#include "ThermostatCluster.h"

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <platform/internal/CHIPDeviceLayerInternal.h>

using namespace chip::app::Clusters::Globals::Structs;
using namespace chip::app::Clusters::Thermostat::Attributes;
using namespace chip::app::Clusters::Thermostat::Structs;
using namespace chip::Protocols::InteractionModel;

namespace chip {
namespace app {
namespace Clusters {
namespace Thermostat {

namespace {

bool IsValidPresetEntry(const PresetStructWithOwnedMembers & preset)
{
    if (!preset.GetPresetHandle().IsNull() && preset.GetPresetHandle().Value().size() > kPresetHandleSize)
    {
        return false;
    }
    return (preset.GetPresetScenario() != PresetScenarioEnum::kUnknownEnumValue);
}

bool IsBuiltIn(const PresetStructWithOwnedMembers & preset)
{
    return preset.GetBuiltIn().ValueOr(false);
}

bool PresetHandlesExistAndMatch(const PresetStructWithOwnedMembers & preset, const PresetStructWithOwnedMembers & presetToMatch)
{
    return !preset.GetPresetHandle().IsNull() && !presetToMatch.GetPresetHandle().IsNull() &&
        preset.GetPresetHandle().Value().data_equal(presetToMatch.GetPresetHandle().Value());
}

bool MatchingPendingPresetExists(ThermostatPresets::Delegate * delegate, const PresetStructWithOwnedMembers & presetToMatch)
{
    VerifyOrReturnValue(delegate != nullptr, false);

    for (uint8_t i = 0; true; i++)
    {
        PresetStructWithOwnedMembers preset;
        CHIP_ERROR err = delegate->GetPendingPresetAtIndex(i, preset);

        if (err == CHIP_ERROR_PROVIDER_LIST_EXHAUSTED)
        {
            break;
        }
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(Zcl, "MatchingPendingPresetExists: GetPendingPresetAtIndex failed with error %" CHIP_ERROR_FORMAT,
                         err.Format());
            return false;
        }

        if (PresetHandlesExistAndMatch(preset, presetToMatch))
        {
            return true;
        }
    }
    return false;
}

bool GetMatchingPresetInPresets(ThermostatPresets::Delegate * delegate, const DataModel::Nullable<ByteSpan> & presetHandle,
                                PresetStructWithOwnedMembers & matchingPreset)
{
    VerifyOrReturnValue(delegate != nullptr, false);

    for (uint8_t i = 0; true; i++)
    {
        CHIP_ERROR err = delegate->GetPresetAtIndex(i, matchingPreset);

        if (err == CHIP_ERROR_PROVIDER_LIST_EXHAUSTED)
        {
            break;
        }
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(Zcl, "GetMatchingPresetInPresets: GetPresetAtIndex failed with error %" CHIP_ERROR_FORMAT, err.Format());
            return false;
        }

        if (presetHandle.Value().data_equal(matchingPreset.GetPresetHandle().Value()))
        {
            return true;
        }
    }
    return false;
}

CHIP_ERROR MaximumPresetScenarioCount(ThermostatPresets::Delegate * delegate, PresetScenarioEnum presetScenario, size_t & count)
{
    count = 0;
    for (uint8_t i = 0; true; i++)
    {
        PresetTypeStruct::Type presetType;
        auto err = delegate->GetPresetTypeAtIndex(i, presetType);
        if (err == CHIP_ERROR_PROVIDER_LIST_EXHAUSTED)
        {
            return CHIP_NO_ERROR;
        }
        if (err != CHIP_NO_ERROR)
        {
            return err;
        }
        if (presetType.presetScenario == presetScenario)
        {
            count = presetType.numberOfPresets;
            return CHIP_NO_ERROR;
        }
    }
    return CHIP_NO_ERROR;
}

uint8_t CountPresetsInPendingListWithPresetHandle(ThermostatPresets::Delegate * delegate, const ByteSpan & presetHandleToMatch)
{
    uint8_t count = 0;
    VerifyOrReturnValue(delegate != nullptr, count);

    for (uint8_t i = 0; true; i++)
    {
        PresetStructWithOwnedMembers preset;
        auto err = delegate->GetPendingPresetAtIndex(i, preset);
        if (err != CHIP_NO_ERROR)
        {
            return count;
        }

        DataModel::Nullable<ByteSpan> presetHandle = preset.GetPresetHandle();
        if (!presetHandle.IsNull() && presetHandle.Value().data_equal(presetHandleToMatch))
        {
            count++;
        }
    }
    return count;
}

bool PresetTypeSupportsNames(ThermostatPresets::Delegate * delegate, PresetScenarioEnum scenario)
{
    VerifyOrReturnValue(delegate != nullptr, false);

    for (uint8_t i = 0; true; i++)
    {
        PresetTypeStruct::Type presetType;
        auto err = delegate->GetPresetTypeAtIndex(i, presetType);
        if (err != CHIP_NO_ERROR)
        {
            return false;
        }

        if (presetType.presetScenario == scenario)
        {
            return (presetType.presetTypeFeatures.Has(PresetTypeFeaturesBitmap::kSupportsNames));
        }
    }
    return false;
}

} // namespace

std::optional<DataModel::ActionReturnStatus> ThermostatPresets::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                                              AttributeValueEncoder & encoder)
{
    if (mDelegate == nullptr)
    {
        ChipLogError(Zcl, "ReadAttribute: PresetsDelegate is null");
        return std::nullopt;
    }

    switch (request.path.mAttributeId)
    {
    case PresetTypes::Id: {
        auto & delegate = mDelegate;
        return encoder.EncodeList([delegate](const auto & enc) -> CHIP_ERROR {
            for (uint8_t i = 0; true; i++)
            {
                PresetTypeStruct::Type presetType;
                auto err = delegate->GetPresetTypeAtIndex(i, presetType);
                if (err == CHIP_ERROR_PROVIDER_LIST_EXHAUSTED)
                {
                    return CHIP_NO_ERROR;
                }
                ReturnErrorOnFailure(err);
                ReturnErrorOnFailure(enc.Encode(presetType));
            }
        });
    }
    case NumberOfPresets::Id:
        return encoder.Encode(mDelegate->GetNumberOfPresets());
    case Presets::Id: {
        auto & delegate          = mDelegate;
        auto & subjectDescriptor = encoder.GetSubjectDescriptor();
        if (mCluster != nullptr && mCluster->GetAtomicWriteSession().InAtomicWrite(subjectDescriptor, MakeOptional(request.path.mAttributeId)))
        {
            return encoder.EncodeList([delegate](const auto & enc) -> CHIP_ERROR {
                for (uint8_t i = 0; true; i++)
                {
                    PresetStructWithOwnedMembers preset;
                    auto err = delegate->GetPendingPresetAtIndex(i, preset);
                    if (err == CHIP_ERROR_PROVIDER_LIST_EXHAUSTED)
                    {
                        return CHIP_NO_ERROR;
                    }
                    ReturnErrorOnFailure(err);
                    ReturnErrorOnFailure(enc.Encode(preset));
                }
            });
        }
        return encoder.EncodeList([delegate](const auto & enc) -> CHIP_ERROR {
            for (uint8_t i = 0; true; i++)
            {
                PresetStructWithOwnedMembers preset;
                auto err = delegate->GetPresetAtIndex(i, preset);
                if (err == CHIP_ERROR_PROVIDER_LIST_EXHAUSTED)
                {
                    return CHIP_NO_ERROR;
                }
                ReturnErrorOnFailure(err);
                ReturnErrorOnFailure(enc.Encode(preset));
            }
        });
    }
    case ActivePresetHandle::Id: {
        uint8_t buffer[kPresetHandleSize];
        MutableByteSpan activePresetHandleSpan(buffer);
        auto activePresetHandle = DataModel::MakeNullable(activePresetHandleSpan);
        ReturnErrorOnFailure(mDelegate->GetActivePresetHandle(activePresetHandle));
        return encoder.Encode(activePresetHandle);
    }
    case ScheduleTypes::Id: {
        auto & delegate = mDelegate;
        return encoder.EncodeList([delegate](const auto & enc) -> CHIP_ERROR {
            for (uint8_t i = 0; true; i++)
            {
                ScheduleTypeStruct::Type scheduleType;
                auto err = delegate->GetScheduleTypeAtIndex(i, scheduleType);
                if (err == CHIP_ERROR_PROVIDER_LIST_EXHAUSTED)
                {
                    return CHIP_NO_ERROR;
                }
                ReturnErrorOnFailure(err);
                ReturnErrorOnFailure(enc.Encode(scheduleType));
            }
        });
    }
    default:
        return std::nullopt;
    }
}

std::optional<DataModel::ActionReturnStatus> ThermostatPresets::WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                                               AttributeValueDecoder & decoder)
{
    if (mDelegate == nullptr || request.path.mAttributeId != Presets::Id)
    {
        return std::nullopt;
    }

    auto & subjectDescriptor = decoder.GetSubjectDescriptor();
    VerifyOrReturnError(mCluster != nullptr && mCluster->GetAtomicWriteSession().InAtomicWrite(MakeOptional(request.path.mAttributeId)),
                        CHIP_IM_GLOBAL_STATUS(InvalidInState), ChipLogError(Zcl, "Presets are not editable"));

    if (!mCluster->GetAtomicWriteSession().InAtomicWrite(subjectDescriptor, MakeOptional(request.path.mAttributeId)))
    {
        ChipLogError(Zcl, "Another node is editing presets. Server is busy. Try again later");
        return CHIP_IM_GLOBAL_STATUS(Busy);
    }

    if (!request.path.IsListOperation() || request.path.mListOp == ConcreteDataAttributePath::ListOperation::ReplaceAll)
    {
        mDelegate->ClearPendingPresetList();
        Presets::TypeInfo::DecodableType newPresetsList;
        ReturnErrorOnFailure(decoder.Decode(newPresetsList));
        auto iter = newPresetsList.begin();
        while (iter.Next())
        {
            const PresetStruct::Type & preset = iter.GetValue();
            ReturnErrorOnFailure(AppendPendingPreset(preset));
        }
        return iter.GetStatus();
    }

    if (request.path.mListOp == ConcreteDataAttributePath::ListOperation::AppendItem)
    {
        PresetStruct::Type preset;
        ReturnErrorOnFailure(decoder.Decode(preset));
        return AppendPendingPreset(preset);
    }
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

std::optional<DataModel::ActionReturnStatus> ThermostatPresets::InvokeCommand(const DataModel::InvokeRequest & request,
                                                                              TLV::TLVReader & input_arguments,
                                                                              CommandHandler * handler)
{
    if (mDelegate == nullptr)
    {
        ChipLogError(Zcl, "InvokeCommand: PresetsDelegate is null");
        return std::nullopt;
    }

    if (request.path.mCommandId == Commands::SetActivePresetRequest::Id)
    {
        Commands::SetActivePresetRequest::DecodableType request_data;
        ReturnErrorOnFailure(request_data.Decode(input_arguments));
        return SetActivePreset(request_data.presetHandle);
    }

    return std::nullopt;
}

std::optional<Status> ThermostatPresets::OnAtomicWriteBegin(AttributeId attributeId)
{
    if (attributeId == Presets::Id && mDelegate != nullptr)
    {
        mDelegate->InitializePendingPresets();
        return Status::Success;
    }
    return std::nullopt;
}

std::optional<Status> ThermostatPresets::OnAtomicWritePrecommit(AttributeId attributeId)
{
    if (attributeId == Presets::Id && mDelegate != nullptr)
    {
        return PrecommitPresets();
    }
    return std::nullopt;
}

std::optional<Status> ThermostatPresets::OnAtomicWriteCommit(AttributeId attributeId)
{
    if (attributeId == Presets::Id && mDelegate != nullptr)
    {
        ClusterStatusCode status(mDelegate->CommitPendingPresets());
        if (status.IsSuccess() && mCluster != nullptr)
        {
            mCluster->NotifyAttributeChanged(attributeId);
        }
        return status.GetStatus();
    }
    return std::nullopt;
}

std::optional<Status> ThermostatPresets::OnAtomicWriteRollback(AttributeId attributeId)
{
    if (attributeId == Presets::Id && mDelegate != nullptr)
    {
        mDelegate->ClearPendingPresetList();
        return Status::Success;
    }
    return std::nullopt;
}

std::optional<System::Clock::Milliseconds16> ThermostatPresets::GetMaxAtomicWriteTimeout(AttributeId attributeId)
{
    if (mDelegate != nullptr)
    {
        return mDelegate->GetMaxAtomicWriteTimeout(attributeId);
    }
    ChipLogError(Zcl, "GetMaxAtomicWriteTimeout: PresetsDelegate is null");
    return std::nullopt;
}

bool ThermostatPresets::IsPresetHandlePresentInPresets(const ByteSpan & presetHandleToMatch)
{
    VerifyOrReturnValue(mDelegate != nullptr, false);

    PresetStructWithOwnedMembers matchingPreset;
    for (uint8_t i = 0; true; i++)
    {
        CHIP_ERROR err = mDelegate->GetPresetAtIndex(i, matchingPreset);

        if (err == CHIP_ERROR_PROVIDER_LIST_EXHAUSTED)
        {
            return false;
        }

        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(Zcl, "IsPresetHandlePresentInPresets: GetPresetAtIndex failed with error %" CHIP_ERROR_FORMAT,
                         err.Format());
            return false;
        }

        if (!matchingPreset.GetPresetHandle().IsNull() && matchingPreset.GetPresetHandle().Value().data_equal(presetHandleToMatch))
        {
            return true;
        }
    }
    return false;
}

Status ThermostatPresets::SetActivePreset(DataModel::Nullable<ByteSpan> presetHandle)
{
    if (mDelegate == nullptr)
    {
        ChipLogError(Zcl, "PresetsDelegate is null");
        return Status::InvalidInState;
    }

    if (!presetHandle.IsNull() && !IsPresetHandlePresentInPresets(presetHandle.Value()))
    {
        return Status::InvalidCommand;
    }

    CHIP_ERROR err = mDelegate->SetActivePresetHandle(presetHandle);

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "Failed to set ActivePresetHandle with error %" CHIP_ERROR_FORMAT, err.Format());
        return StatusIB(err).mStatus;
    }

    if (mCluster != nullptr)
    {
        mCluster->NotifyAttributeChanged(ActivePresetHandle::Id);
    }

    return Status::Success;
}

CHIP_ERROR ThermostatPresets::AppendPendingPreset(const PresetStruct::Type & newPreset)
{
    if (mDelegate == nullptr)
    {
        ChipLogError(Zcl, "PresetsDelegate is null");
        return CHIP_ERROR_INCORRECT_STATE;
    }
    PresetStructWithOwnedMembers preset = newPreset;
    if (!IsValidPresetEntry(preset))
    {
        return CHIP_IM_GLOBAL_STATUS(ConstraintError);
    }

    if (preset.GetPresetHandle().IsNull())
    {
        if (IsBuiltIn(preset))
        {
            return CHIP_IM_GLOBAL_STATUS(ConstraintError);
        }
        preset.SetBuiltIn(false);
    }
    else
    {
        PresetStructWithOwnedMembers matchingPreset;
        if (!GetMatchingPresetInPresets(mDelegate, preset.GetPresetHandle().Value(), matchingPreset))
        {
            return CHIP_IM_GLOBAL_STATUS(NotFound);
        }

        if (CountPresetsInPendingListWithPresetHandle(mDelegate, preset.GetPresetHandle().Value()) > 0)
        {
            return CHIP_IM_GLOBAL_STATUS(ConstraintError);
        }

        const auto & presetBuiltIn         = preset.GetBuiltIn();
        const auto & matchingPresetBuiltIn = matchingPreset.GetBuiltIn();
        if (presetBuiltIn.IsNull())
        {
            if (matchingPresetBuiltIn.IsNull())
            {
                return CHIP_IM_GLOBAL_STATUS(InvalidInState);
            }
            preset.SetBuiltIn(matchingPresetBuiltIn.Value());
        }
        else
        {
            if (matchingPresetBuiltIn.IsNull())
            {
                return CHIP_IM_GLOBAL_STATUS(InvalidInState);
            }
            if (presetBuiltIn.Value() != matchingPresetBuiltIn.Value())
            {
                return CHIP_IM_GLOBAL_STATUS(ConstraintError);
            }
        }
    }

    size_t maximumPresetCount         = mDelegate->GetNumberOfPresets();
    size_t maximumPresetScenarioCount = 0;
    if (MaximumPresetScenarioCount(mDelegate, preset.GetPresetScenario(), maximumPresetScenarioCount) != CHIP_NO_ERROR)
    {
        return CHIP_IM_GLOBAL_STATUS(InvalidInState);
    }

    if (maximumPresetScenarioCount == 0)
    {
        return CHIP_IM_GLOBAL_STATUS(ConstraintError);
    }

    if (preset.GetName().HasValue() && !PresetTypeSupportsNames(mDelegate, preset.GetPresetScenario()))
    {
        return CHIP_IM_GLOBAL_STATUS(ConstraintError);
    }

    size_t presetCount         = 1;
    size_t presetScenarioCount = 1;
    for (uint8_t i = 0; true; i++)
    {
        PresetStructWithOwnedMembers otherPreset;
        CHIP_ERROR err = mDelegate->GetPendingPresetAtIndex(i, otherPreset);

        if (err == CHIP_ERROR_PROVIDER_LIST_EXHAUSTED)
        {
            break;
        }
        if (err != CHIP_NO_ERROR)
        {
            return CHIP_IM_GLOBAL_STATUS(InvalidInState);
        }
        presetCount++;
        if (preset.GetPresetScenario() == otherPreset.GetPresetScenario())
        {
            presetScenarioCount++;
        }
    }

    if (presetCount > maximumPresetCount)
    {
        ChipLogError(Zcl, "Preset count exceeded %u: %u ", static_cast<unsigned>(maximumPresetCount),
                     static_cast<unsigned>(presetCount));
        return CHIP_IM_GLOBAL_STATUS(ResourceExhausted);
    }

    if (presetScenarioCount > maximumPresetScenarioCount)
    {
        ChipLogError(Zcl, "Preset scenario count exceeded %u: %u ", static_cast<unsigned>(maximumPresetScenarioCount),
                     static_cast<unsigned>(presetScenarioCount));
        return CHIP_IM_GLOBAL_STATUS(ResourceExhausted);
    }

    return mDelegate->AppendToPendingPresetList(preset);
}

Status ThermostatPresets::PrecommitPresets()
{
    if (mDelegate == nullptr)
    {
        ChipLogError(Zcl, "PresetsDelegate is null");
        return Status::InvalidInState;
    }

    CHIP_ERROR err = CHIP_NO_ERROR;

    for (uint8_t i = 0; true; i++)
    {
        PresetStructWithOwnedMembers preset;
        err = mDelegate->GetPresetAtIndex(i, preset);

        if (err == CHIP_ERROR_PROVIDER_LIST_EXHAUSTED)
        {
            break;
        }
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(Zcl, "PrecommitPresets: GetPresetAtIndex failed with error %" CHIP_ERROR_FORMAT, err.Format());
            return Status::InvalidInState;
        }

        bool found = MatchingPendingPresetExists(mDelegate, preset);

        if (IsBuiltIn(preset) && !found)
        {
            return Status::ConstraintError;
        }
    }

    uint8_t buffer[kPresetHandleSize];
    MutableByteSpan activePresetHandleSpan(buffer);
    auto activePresetHandle = DataModel::MakeNullable(activePresetHandleSpan);

    err = mDelegate->GetActivePresetHandle(activePresetHandle);

    if (err != CHIP_NO_ERROR)
    {
        return Status::InvalidInState;
    }

    if (!activePresetHandle.IsNull())
    {
        uint8_t count = CountPresetsInPendingListWithPresetHandle(mDelegate, activePresetHandle.Value());
        if (count == 0)
        {
            return Status::InvalidInState;
        }
    }

    auto heatLimits = mCluster != nullptr ? mCluster->GetSetpoints().GetLimits(SystemModeEnum::kHeat) : AbsoluteSetpointLimits(AbsoluteSetpoint(kInvalidAttributeId, kDefaultAbsMinHeatSetpointLimit), AbsoluteSetpoint(kInvalidAttributeId, kDefaultAbsMaxHeatSetpointLimit));
    auto coolLimits = mCluster != nullptr ? mCluster->GetSetpoints().GetLimits(SystemModeEnum::kCool) : AbsoluteSetpointLimits(AbsoluteSetpoint(kInvalidAttributeId, kDefaultAbsMinCoolSetpointLimit), AbsoluteSetpoint(kInvalidAttributeId, kDefaultAbsMaxCoolSetpointLimit));

    for (uint8_t i = 0; true; i++)
    {
        PresetStructWithOwnedMembers pendingPreset;
        err = mDelegate->GetPendingPresetAtIndex(i, pendingPreset);

        if (err == CHIP_ERROR_PROVIDER_LIST_EXHAUSTED)
        {
            break;
        }
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(Zcl, "PrecommitPresets: GetPendingPresetAtIndex failed with error %" CHIP_ERROR_FORMAT, err.Format());
            return Status::InvalidInState;
        }

        Optional<int16_t> coolingSetpoint = pendingPreset.GetCoolingSetpoint();
        if (coolingSetpoint.HasValue())
        {
            pendingPreset.SetCoolingSetpoint(MakeOptional(coolLimits.Clamp(coolingSetpoint.Value())));
        }

        Optional<int16_t> heatingSetpoint = pendingPreset.GetHeatingSetpoint();
        if (heatingSetpoint.HasValue())
        {
            pendingPreset.SetHeatingSetpoint(MakeOptional(heatLimits.Clamp(heatingSetpoint.Value())));
        }
    }

    return Status::Success;
}

} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip
