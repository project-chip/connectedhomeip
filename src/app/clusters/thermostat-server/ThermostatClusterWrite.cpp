/**
 *
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

#include "ThermostatCluster.h"

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/callback.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/ids/Attributes.h>

#include <clusters/Thermostat/Metadata.h>
#include <lib/core/CHIPEncoding.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::Thermostat;
using namespace chip::app::Clusters::Thermostat::Structs;
using namespace chip::app::Clusters::Thermostat::Attributes;
using namespace Protocols::InteractionModel;

namespace chip {
namespace app {
namespace Clusters {
namespace Thermostat {

DataModel::ActionReturnStatus ThermostatCluster::WriteSetpointAttribute(const DataModel::WriteAttributeRequest & request,
                                                                        AttributeValueDecoder & decoder)
{

    AttributePersistence persistence(mContext->attributeStorage);

    int16_t setpoint;
    ReturnErrorOnFailure(decoder.Decode(setpoint));

    auto supportsHeat = mFeatures.Has(Feature::kHeating);
    auto supportsCool = mFeatures.Has(Feature::kCooling);

    bool occupied          = false;
    bool isHeatingSetpoint = false;
    AttributeId deadbandAttribute;
    switch (request.path.mAttributeId)
    {
    case OccupiedHeatingSetpoint::Id:
        if (!supportsHeat)
        {
            return Status::UnsupportedAttribute;
        }
        isHeatingSetpoint = true;
        occupied          = true;
        deadbandAttribute = OccupiedCoolingSetpoint::Id;
        break;
    case OccupiedCoolingSetpoint::Id:
        if (!supportsCool)
        {
            return Status::UnsupportedAttribute;
        }
        occupied          = true;
        deadbandAttribute = OccupiedHeatingSetpoint::Id;
        break;
    case UnoccupiedHeatingSetpoint::Id:
        if (!supportsHeat)
        {
            return Status::UnsupportedAttribute;
        }
        isHeatingSetpoint = true;
        deadbandAttribute = UnoccupiedCoolingSetpoint::Id;
        break;
    case UnoccupiedCoolingSetpoint::Id:
        if (!supportsCool)
        {
            return Status::UnsupportedAttribute;
        }
        occupied          = true;
        deadbandAttribute = UnoccupiedHeatingSetpoint::Id;
        break;
    default:
        return Status::UnsupportedAttribute;
    }
    bool deadbandShift = false;
    auto result = mSetpoints.ChangeSetpoint(setpoint, isHeatingSetpoint, mFeatures.Has(Feature::kAutoMode), occupied, deadbandShift,
                                            decoder, persistence);
    if (result.IsSuccess())
    {
        if (deadbandShift)
        {
            NotifyAttributeChanged(deadbandAttribute);
        }
    }
    return result;
}

DataModel::ActionReturnStatus ThermostatCluster::WriteSetpointLimitAttribute(const DataModel::WriteAttributeRequest & request,
                                                                             AttributeValueDecoder & decoder)
{

    AttributePersistence persistence(mContext->attributeStorage);

    int16_t setpointLimit;
    ReturnErrorOnFailure(decoder.Decode(setpointLimit));

    auto supportsHeat = mFeatures.Has(Feature::kHeating);
    auto supportsCool = mFeatures.Has(Feature::kCooling);
    switch (request.path.mAttributeId)
    {
    case MinHeatSetpointLimit::Id:
    case MaxHeatSetpointLimit::Id:
        if (!supportsHeat)
        {
            return Status::UnsupportedAttribute;
        }
        break;
    case MinCoolSetpointLimit::Id:
    case MaxCoolSetpointLimit::Id:
        if (!supportsCool)
        {
            return Status::UnsupportedAttribute;
        }
        break;
    }
    return mSetpoints.ChangeSetpointLimit(request.path.mAttributeId, setpointLimit, mFeatures.Has(Feature::kAutoMode), decoder,
                                          persistence);
}

DataModel::ActionReturnStatus ThermostatCluster::WriteNonAtomicAttribute(const DataModel::WriteAttributeRequest & request,
                                                                         AttributeValueDecoder & decoder)
{

    switch (request.path.mAttributeId)
    {
    case OccupiedCoolingSetpoint::Id:
    case OccupiedHeatingSetpoint::Id:
    case UnoccupiedCoolingSetpoint::Id:
    case UnoccupiedHeatingSetpoint::Id:
        return WriteSetpointAttribute(request, decoder);
    case MinHeatSetpointLimit::Id:
    case MaxHeatSetpointLimit::Id:
    case MinCoolSetpointLimit::Id:
    case MaxCoolSetpointLimit::Id:
        return WriteSetpointLimitAttribute(request, decoder);
    // return persistence.DecodeAndStoreNativeEndianValue(request.path, decoder, mLocalConfigDisabled);
    case MinSetpointDeadBand::Id: {
        int16_t deadBand;
        ReturnErrorOnFailure(decoder.Decode(deadBand));
        AttributePersistence persistence(mContext->attributeStorage);
        return mSetpoints.ChangeSetpointDeadBand(deadBand, mFeatures.Has(Feature::kAutoMode), decoder, persistence);
    }
    case RemoteSensing::Id: {
        if (mFeatures.Has(Feature::kLocalTemperatureNotExposed))
        {
            BitMask<RemoteSensingBitmap> valueRemoteSensing;
            ReturnErrorOnFailure(decoder.Decode(valueRemoteSensing));
            if (valueRemoteSensing.Has(RemoteSensingBitmap::kLocalTemperature))
            {
                return Status::ConstraintError;
            }
            auto remoteSensing = valueRemoteSensing.Raw();
            AttributePersistence persistence(mContext->attributeStorage);
            auto result = persistence.DecodeAndStoreNativeEndianValue(
                { request.path.mEndpointId, Thermostat::Id, RemoteSensing::Id }, decoder, remoteSensing);
            if (!result.IsSuccess())
            {
                return result;
            }
            mRemoteSensing = valueRemoteSensing;
            return Status::Success;
        }
        return Status::Success;
    }
    case ControlSequenceOfOperation::Id:
        // Per spec, we silently ignore any attempts to write to this attribute
        return Status::Success;
    case SystemMode::Id: {

        SystemModeEnum requestedSystemMode;
        ReturnErrorOnFailure(decoder.Decode(requestedSystemMode));
        if (EnsureKnownEnumValue(requestedSystemMode) == SystemModeEnum::kUnknownEnumValue)
        {
            return Status::InvalidValue;
        }

        switch (EnsureKnownEnumValue(mControlSequenceOfOperation))
        {
        case ControlSequenceOfOperationEnum::kCoolingOnly:
        case ControlSequenceOfOperationEnum::kCoolingWithReheat:
            if (requestedSystemMode == SystemModeEnum::kHeat || requestedSystemMode == SystemModeEnum::kEmergencyHeat)
            {
                return Status::InvalidValue;
            }
            break;
        case ControlSequenceOfOperationEnum::kHeatingOnly:
        case ControlSequenceOfOperationEnum::kHeatingWithReheat:
            if (requestedSystemMode == SystemModeEnum::kCool || requestedSystemMode == SystemModeEnum::kPrecooling)
            {
                return Status::InvalidValue;
            }
            break;
        case ControlSequenceOfOperationEnum::kCoolingAndHeating:
        case ControlSequenceOfOperationEnum::kCoolingAndHeatingWithReheat:
            break;
        case ControlSequenceOfOperationEnum::kUnknownEnumValue:
            // Feels like this should have been invalid state
            return Status::InvalidValue;
        }
        AttributePersistence persistence(mContext->attributeStorage);
        auto result = persistence.DecodeAndStoreNativeEndianValue({ request.path.mEndpointId, Thermostat::Id, SystemMode::Id },
                                                                  decoder, requestedSystemMode);
        if (!result.IsSuccess())
        {
            return result;
        }
        mSystemMode = requestedSystemMode;
        return Status::Success;
    }
    case TemperatureSetpointHold::Id:
    case TemperatureSetpointHoldDuration::Id:
    default:
        return Protocols::InteractionModel::Status::UnsupportedWrite;
    }
}

DataModel::ActionReturnStatus ThermostatCluster::WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                                AttributeValueDecoder & decoder)
{
    auto attributeId         = request.path.mAttributeId;
    auto & subjectDescriptor = decoder.GetSubjectDescriptor();

    switch (attributeId)
    {
    case Presets::Id: {

        VerifyOrReturnError(mDelegate != nullptr, CHIP_ERROR_INCORRECT_STATE, ChipLogError(Zcl, "Delegate is null"));

        // Presets are not editable, return INVALID_IN_STATE.
        VerifyOrReturnError(mAtomicWriteSession.InAtomicWrite(MakeOptional(attributeId)), CHIP_IM_GLOBAL_STATUS(InvalidInState),
                            ChipLogError(Zcl, "Presets are not editable"));

        // OK, we're in an atomic write, make sure the requesting node is the same one that started the atomic write,
        // otherwise return BUSY.
        if (!mAtomicWriteSession.InAtomicWrite(subjectDescriptor, MakeOptional(attributeId)))
        {
            ChipLogError(Zcl, "Another node is editing presets. Server is busy. Try again later");
            return CHIP_IM_GLOBAL_STATUS(Busy);
        }

        // If the list operation is replace all, clear the existing pending list, iterate over the new presets list
        // and add to the pending presets list.
        if (!request.path.IsListOperation() || request.path.mListOp == ConcreteDataAttributePath::ListOperation::ReplaceAll)
        {
            // Clear the pending presets list
            mDelegate->ClearPendingPresetList();

            Presets::TypeInfo::DecodableType newPresetsList;
            ReturnErrorOnFailure(decoder.Decode(newPresetsList));

            // Iterate over the presets and call the delegate to append to the list of pending presets.
            auto iter = newPresetsList.begin();
            while (iter.Next())
            {
                const PresetStruct::Type & preset = iter.GetValue();
                ReturnErrorOnFailure(AppendPendingPreset(preset));
            }
            return iter.GetStatus();
        }

        // If the list operation is AppendItem, call the delegate to append the item to the list of pending presets.
        if (request.path.mListOp == ConcreteDataAttributePath::ListOperation::AppendItem)
        {
            PresetStruct::Type preset;
            ReturnErrorOnFailure(decoder.Decode(preset));
            return AppendPendingPreset(preset);
        }
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }
    break;
    case Schedules::Id:
        return CHIP_ERROR_NOT_IMPLEMENTED;
    default: {
        if (mAtomicWriteSession.InAtomicWrite(subjectDescriptor, MakeOptional(attributeId)))
        {
            ChipLogError(Zcl, "Can not write to non-atomic attributes during atomic write");
            return Status::InvalidInState;
        }
        auto result = NotifyAttributeChangedIfSuccess(request.path.mAttributeId, WriteNonAtomicAttribute(request, decoder));
        if (result.IsSuccess() && mFeatures.Has(Feature::kPresets))
        {
            bool clearActivePreset = false;
            bool occupied          = true;
            if (mFeatures.Has(Feature::kOccupancy))
            {
                occupied = mOccupancy.Has(OccupancyBitmap::kOccupied);
            }
            switch (attributeId)
            {
            case OccupiedHeatingSetpoint::Id:
            case OccupiedCoolingSetpoint::Id:
                clearActivePreset = occupied;
                break;
            case UnoccupiedHeatingSetpoint::Id:
            case UnoccupiedCoolingSetpoint::Id:
                clearActivePreset = !occupied;
                break;
            }
            if (clearActivePreset)
            {
                ChipLogProgress(Zcl, "Setting active preset to null");
                SetActivePreset(std::nullopt);
            }
        }
        return result;
    }
    }
}

} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip
