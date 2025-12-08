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

DataModel::ActionReturnStatus ThermostatCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                               AttributeValueEncoder & encoder)
{

    uint32_t ourFeatureMap;
    bool localTemperatureNotExposedSupported = (FeatureMap::Get(request.path.mEndpointId, &ourFeatureMap) == Status::Success) &&
        ((ourFeatureMap & to_underlying(Feature::kLocalTemperatureNotExposed)) != 0);

    switch (request.path.mAttributeId)
    {
    case ClusterRevision::Id:
        return encoder.Encode(Thermostat::kRevision);
    case Attributes::FeatureMap::Id:
        return encoder.Encode(ourFeatureMap);
    case LocalTemperature::Id:
        if (localTemperatureNotExposedSupported)
        {
            return encoder.EncodeNull();
        }
        break;
    case RemoteSensing::Id:
        if (localTemperatureNotExposedSupported)
        {
            BitMask<RemoteSensingBitmap> valueRemoteSensing = mRemoteSensing;
            valueRemoteSensing.Clear(RemoteSensingBitmap::kLocalTemperature);
            return encoder.Encode(valueRemoteSensing);
        }
        break;
    case ControlSequenceOfOperation::Id:
        return encoder.Encode(mControlSequenceOfOperation);
    case OccupiedHeatingSetpoint::Id:
        return encoder.Encode(mSetpoints.mOccupiedHeatingSetpoint);
    case OccupiedCoolingSetpoint::Id:
        return encoder.Encode(mSetpoints.mOccupiedCoolingSetpoint);
    case UnoccupiedHeatingSetpoint::Id:
        return encoder.Encode(mSetpoints.mUnoccupiedHeatingSetpoint);
    case UnoccupiedCoolingSetpoint::Id:
        return encoder.Encode(mSetpoints.mUnoccupiedCoolingSetpoint);
    case AbsMinHeatSetpointLimit::Id:
        return encoder.Encode(mSetpoints.mAbsMinHeatSetpointLimit);
    case AbsMaxHeatSetpointLimit::Id:
        return encoder.Encode(mSetpoints.mAbsMaxHeatSetpointLimit);
    case AbsMinCoolSetpointLimit::Id:
        return encoder.Encode(mSetpoints.mAbsMinCoolSetpointLimit);
    case AbsMaxCoolSetpointLimit::Id:
        return encoder.Encode(mSetpoints.mAbsMaxCoolSetpointLimit);
    case MinHeatSetpointLimit::Id:
        return encoder.Encode(mSetpoints.mMinHeatSetpointLimit);
    case MaxHeatSetpointLimit::Id:
        return encoder.Encode(mSetpoints.mMaxHeatSetpointLimit);
    case MinCoolSetpointLimit::Id:
        return encoder.Encode(mSetpoints.mMinCoolSetpointLimit);
    case MaxCoolSetpointLimit::Id:
        return encoder.Encode(mSetpoints.mMaxCoolSetpointLimit);
    case MinSetpointDeadBand::Id: {
        auto deadband = static_cast<uint8_t>(mSetpoints.mDeadBand / 10);
        return encoder.Encode(deadband);
    }
    case PresetTypes::Id: {
        auto & delegate = mDelegate;
        VerifyOrReturnError(mDelegate != nullptr, CHIP_ERROR_INCORRECT_STATE, ChipLogError(Zcl, "Delegate is null"));

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
    break;
    case NumberOfPresets::Id: {
        VerifyOrReturnError(mDelegate != nullptr, CHIP_ERROR_INCORRECT_STATE, ChipLogError(Zcl, "Delegate is null"));

        ReturnErrorOnFailure(encoder.Encode(mDelegate->GetNumberOfPresets()));
    }
    break;
    case Presets::Id: {
        VerifyOrReturnError(mDelegate != nullptr, CHIP_ERROR_INCORRECT_STATE, ChipLogError(Zcl, "Delegate is null"));

        auto & delegate          = mDelegate;
        auto & subjectDescriptor = encoder.GetSubjectDescriptor();
        if (mAtomicWriteSession.InAtomicWrite(subjectDescriptor, MakeOptional(request.path.mAttributeId)))
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
    break;
    case ActivePresetHandle::Id: {
        VerifyOrReturnError(mDelegate != nullptr, CHIP_ERROR_INCORRECT_STATE, ChipLogError(Zcl, "Delegate is null"));

        uint8_t buffer[kPresetHandleSize];
        MutableByteSpan activePresetHandleSpan(buffer);
        auto activePresetHandle = DataModel::MakeNullable(activePresetHandleSpan);

        CHIP_ERROR err = mDelegate->GetActivePresetHandle(activePresetHandle);
        ReturnErrorOnFailure(err);

        ReturnErrorOnFailure(encoder.Encode(activePresetHandle));
    }
    break;
    case ScheduleTypes::Id: {
        auto & delegate = mDelegate;
        VerifyOrReturnError(delegate != nullptr, CHIP_ERROR_INCORRECT_STATE, ChipLogError(Zcl, "Delegate is null"));

        return encoder.EncodeList([delegate](const auto & encoder) -> CHIP_ERROR {
            for (uint8_t i = 0; true; i++)
            {
                ScheduleTypeStruct::Type scheduleType;
                auto err = delegate->GetScheduleTypeAtIndex(i, scheduleType);
                if (err == CHIP_ERROR_PROVIDER_LIST_EXHAUSTED)
                {
                    return CHIP_NO_ERROR;
                }
                ReturnErrorOnFailure(err);
                ReturnErrorOnFailure(encoder.Encode(scheduleType));
            }
        });
    }
    break;
    case Schedules::Id: {
        return encoder.EncodeList([](const auto & encoder) -> CHIP_ERROR { return CHIP_NO_ERROR; });
    }
    break;
    case MaxThermostatSuggestions::Id: {
        VerifyOrReturnError(mDelegate != nullptr, CHIP_ERROR_INCORRECT_STATE, ChipLogError(Zcl, "Delegate is null"));

        ReturnErrorOnFailure(encoder.Encode(mDelegate->GetMaxThermostatSuggestions()));
    }
    break;
    case ThermostatSuggestions::Id: {
        auto & delegate = mDelegate;
        VerifyOrReturnError(delegate != nullptr, CHIP_ERROR_INCORRECT_STATE, ChipLogError(Zcl, "Delegate is null"));

        return encoder.EncodeList([delegate](const auto & encoder) -> CHIP_ERROR {
            for (size_t i = 0; true; i++)
            {
                ThermostatSuggestionStructWithOwnedMembers thermostatSuggestion;
                auto err = delegate->GetThermostatSuggestionAtIndex(i, thermostatSuggestion);
                if (err == CHIP_ERROR_PROVIDER_LIST_EXHAUSTED)
                {
                    return CHIP_NO_ERROR;
                }
                ReturnErrorOnFailure(err);
                ReturnErrorOnFailure(encoder.Encode(thermostatSuggestion));
            }
        });
    }
    break;
    case CurrentThermostatSuggestion::Id: {
        VerifyOrReturnError(mDelegate != nullptr, CHIP_ERROR_INCORRECT_STATE, ChipLogError(Zcl, "Delegate is null"));

        DataModel::Nullable<ThermostatSuggestionStructWithOwnedMembers> currentThermostatSuggestion;

        mDelegate->GetCurrentThermostatSuggestion(currentThermostatSuggestion);
        ReturnErrorOnFailure(encoder.Encode(currentThermostatSuggestion));
    }
    break;
    case ThermostatSuggestionNotFollowingReason::Id: {
        VerifyOrReturnError(mDelegate != nullptr, CHIP_ERROR_INCORRECT_STATE, ChipLogError(Zcl, "Delegate is null"));
        ReturnErrorOnFailure(encoder.Encode(mDelegate->GetThermostatSuggestionNotFollowingReason()));
    }
    break;
    default:
        return Status::UnsupportedAttribute;
    }
    return Status::Success;
}

} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip
