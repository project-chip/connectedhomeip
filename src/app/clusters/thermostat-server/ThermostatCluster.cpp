/**
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
#include "PresetStructWithOwnedMembers.h"
#include "ThermostatClusterEvents.h"

#include <app/util/attribute-storage.h>

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/callback.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app/CommandHandler.h>
#include <app/ConcreteAttributePath.h>
#include <app/ConcreteCommandPath.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <app/persistence/AttributePersistence.h>
#include <app/server/Server.h>
#include <app/util/endpoint-config-api.h>
#include <clusters/Thermostat/Metadata.h>
#include <lib/core/CHIPEncoding.h>

#include <algorithm>
#include <cstdlib>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::Thermostat;
using namespace chip::app::Clusters::Thermostat::Structs;
using namespace chip::app::Clusters::Thermostat::Attributes;
using namespace Protocols::InteractionModel;

// IMPORTANT NOTE:
// No Side effects are permitted in emberAfThermostatClusterServerPreAttributeChangedCallback
// If a setpoint changes is required as a result of setpoint limit change
// it does not happen here.  It is the responsibility of the device to adjust the setpoint(s)
// as required in emberAfThermostatClusterServerPostAttributeChangedCallback
// limit change validation assures that there is at least 1 setpoint that will be valid

#define FEATURE_MAP_HEAT 0x01
#define FEATURE_MAP_COOL 0x02
#define FEATURE_MAP_OCC 0x04
#define FEATURE_MAP_SCH 0x08
#define FEATURE_MAP_SB 0x10
#define FEATURE_MAP_AUTO 0x20

#define FEATURE_MAP_DEFAULT FEATURE_MAP_HEAT | FEATURE_MAP_COOL | FEATURE_MAP_AUTO

namespace chip {
namespace app {
namespace Clusters {
namespace Thermostat {

int16_t ThermostatCluster::EnforceHeatingSetpointLimits(int16_t heatingSetpoint) const
{
    int16_t minLimit = mMinHeatSetpointLimit;
    int16_t maxLimit = mMaxHeatSetpointLimit;
    if (minLimit < mAbsMinHeatSetpointLimit)
        minLimit = mAbsMinHeatSetpointLimit;
    if (maxLimit > mAbsMaxHeatSetpointLimit)
        maxLimit = mAbsMaxHeatSetpointLimit;
    if (heatingSetpoint < minLimit)
        heatingSetpoint = minLimit;
    if (heatingSetpoint > maxLimit)
        heatingSetpoint = maxLimit;
    return heatingSetpoint;
}

int16_t ThermostatCluster::EnforceCoolingSetpointLimits(int16_t coolingSetpoint) const
{
    int16_t minLimit = mMinCoolSetpointLimit;
    int16_t maxLimit = mMaxCoolSetpointLimit;
    if (minLimit < mAbsMinCoolSetpointLimit)
        minLimit = mAbsMinCoolSetpointLimit;
    if (maxLimit > mAbsMaxCoolSetpointLimit)
        maxLimit = mAbsMaxCoolSetpointLimit;
    if (coolingSetpoint < minLimit)
        coolingSetpoint = minLimit;
    if (coolingSetpoint > maxLimit)
        coolingSetpoint = maxLimit;
    return coolingSetpoint;
}


ThermostatCluster::ThermostatCluster(EndpointId endpointId, uint32_t featureMap, const StartupConfiguration & config,
                                     const Context & context) :
    DefaultServerCluster({ endpointId, Thermostat::Id }),
    mContext(context), mFeatures(featureMap)
{
    mAbsMinHeatSetpointLimit    = config.absMinHeatSetpointLimit;
    mAbsMaxHeatSetpointLimit    = config.absMaxHeatSetpointLimit;
    mAbsMinCoolSetpointLimit    = config.absMinCoolSetpointLimit;
    mAbsMaxCoolSetpointLimit    = config.absMaxCoolSetpointLimit;
    mMinHeatSetpointLimit       = config.minHeatSetpointLimit;
    mMaxHeatSetpointLimit       = config.maxHeatSetpointLimit;
    mMinCoolSetpointLimit       = config.minCoolSetpointLimit;
    mMaxCoolSetpointLimit       = config.maxCoolSetpointLimit;
    mOccupiedHeatingSetpoint    = config.occupiedHeatingSetpoint;
    mOccupiedCoolingSetpoint    = config.occupiedCoolingSetpoint;
    mUnoccupiedHeatingSetpoint  = config.unoccupiedHeatingSetpoint;
    mUnoccupiedCoolingSetpoint  = config.unoccupiedCoolingSetpoint;
    mMinSetpointDeadBand        = config.minSetpointDeadBand;
    mControlSequenceOfOperation = config.controlSequenceOfOperation;
    mSystemMode                 = config.systemMode;
}

CHIP_ERROR ThermostatCluster::Startup(ServerClusterContext & context)
{
    ReturnErrorOnFailure(DefaultServerCluster::Startup(context));

    // Register as a fabric delegate so open atomic writes can be rolled back when their fabric is removed.
    CHIP_ERROR err = mContext.fabricTable.AddFabricDelegate(this);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "Thermostat: failed to register fabric delegate: %" CHIP_ERROR_FORMAT, err.Format());
    }
    LoadPersistentAttributes();
    return CHIP_NO_ERROR;
}

void ThermostatCluster::LoadPersistentAttributes()
{
    AttributePersistence attrPersistence{ DefaultServerCluster::mContext->attributeStorage };

    const auto defaultLocalTempCalib    = mLocalTemperatureCalibration;
    const auto defaultTsph              = mTemperatureSetpointHold;
    const auto defaultTsphDuration      = mTemperatureSetpointHoldDuration;
    const auto defaultEhd               = mEmergencyHeatDelta;
    const auto defaultACType            = mACType;
    const auto defaultACCapacity        = mACCapacity;
    const auto defaultACRefrigerantType = mACRefrigerantType;
    const auto defaultACCompressorType  = mACCompressorType;
    const auto defaultACLouverPos       = mACLouverPosition;
    const auto defaultACCapacityFmt     = mACCapacityformat;

    attrPersistence.LoadNativeEndianValue({ mPath.mEndpointId, Thermostat::Id, LocalTemperatureCalibration::Id },
                                          mLocalTemperatureCalibration, defaultLocalTempCalib);
    attrPersistence.LoadNativeEndianValue({ mPath.mEndpointId, Thermostat::Id, TemperatureSetpointHold::Id },
                                          mTemperatureSetpointHold, defaultTsph);
    attrPersistence.LoadNativeEndianValue({ mPath.mEndpointId, Thermostat::Id, TemperatureSetpointHoldDuration::Id },
                                          mTemperatureSetpointHoldDuration, defaultTsphDuration);
    attrPersistence.LoadNativeEndianValue({ mPath.mEndpointId, Thermostat::Id, EmergencyHeatDelta::Id },
                                          mEmergencyHeatDelta, defaultEhd);
    attrPersistence.LoadNativeEndianValue({ mPath.mEndpointId, Thermostat::Id, ACType::Id }, mACType, defaultACType);
    attrPersistence.LoadNativeEndianValue({ mPath.mEndpointId, Thermostat::Id, ACCapacity::Id }, mACCapacity, defaultACCapacity);
    attrPersistence.LoadNativeEndianValue({ mPath.mEndpointId, Thermostat::Id, ACRefrigerantType::Id },
                                          mACRefrigerantType, defaultACRefrigerantType);
    attrPersistence.LoadNativeEndianValue({ mPath.mEndpointId, Thermostat::Id, ACCompressorType::Id },
                                          mACCompressorType, defaultACCompressorType);
    attrPersistence.LoadNativeEndianValue({ mPath.mEndpointId, Thermostat::Id, ACLouverPosition::Id },
                                          mACLouverPosition, defaultACLouverPos);
    attrPersistence.LoadNativeEndianValue({ mPath.mEndpointId, Thermostat::Id, ACCapacityformat::Id },
                                          mACCapacityformat, defaultACCapacityFmt);

    // Load binary handle attributes using raw ReadValue (ByteSpan is not an arithmetic/enum type).
    {
        MutableByteSpan buf(mActivePresetHandleBuffer);
        if (DefaultServerCluster::mContext->attributeStorage.ReadValue(
                { mPath.mEndpointId, Thermostat::Id, ActivePresetHandle::Id }, buf) == CHIP_NO_ERROR &&
            buf.size() > 0)
        {
            mActivePresetHandle.SetNonNull(ByteSpan(mActivePresetHandleBuffer, buf.size()));
        }
    }
    {
        MutableByteSpan buf(mActiveScheduleHandleBuffer);
        if (DefaultServerCluster::mContext->attributeStorage.ReadValue(
                { mPath.mEndpointId, Thermostat::Id, ActiveScheduleHandle::Id }, buf) == CHIP_NO_ERROR &&
            buf.size() > 0)
        {
            mActiveScheduleHandle.SetNonNull(ByteSpan(mActiveScheduleHandleBuffer, buf.size()));
        }
    }
}

void ThermostatCluster::Shutdown(ClusterShutdownType type)
{
    // Roll back an open atomic write (which also cancels its pending timeout timer). The timer is only ever
    // scheduled while a write is open, so when none is open there is nothing to cancel.
    if (mAtomicWriteSession.state == AtomicWriteState::Open)
    {
        ResetAtomicWrite(GetEndpointId());
    }
    mContext.fabricTable.RemoveFabricDelegate(this);
    DefaultServerCluster::Shutdown(type);
}

// Encodes the delegate-backed list / preset / schedule / suggestion attributes. The scalar attributes
// (including LocalTemperature, RemoteSensing and ClusterRevision) are handled directly by ReadAttribute.
CHIP_ERROR ThermostatCluster::ReadDelegateAttribute(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    switch (aPath.mAttributeId)
    {
    case PresetTypes::Id: {
        auto delegate = mDelegate;
        VerifyOrReturnError(delegate != nullptr, CHIP_ERROR_INCORRECT_STATE, ChipLogError(Zcl, "Delegate is null"));

        return aEncoder.EncodeList([delegate](const auto & encoder) -> CHIP_ERROR {
            for (uint8_t i = 0; true; i++)
            {
                PresetTypeStruct::Type presetType;
                auto err = delegate->GetPresetTypeAtIndex(i, presetType);
                if (err == CHIP_ERROR_PROVIDER_LIST_EXHAUSTED)
                {
                    return CHIP_NO_ERROR;
                }
                ReturnErrorOnFailure(err);
                ReturnErrorOnFailure(encoder.Encode(presetType));
            }
        });
    }
    break;
    case NumberOfPresets::Id: {
        VerifyOrReturnError(mDelegate != nullptr, CHIP_ERROR_INCORRECT_STATE, ChipLogError(Zcl, "Delegate is null"));

        ReturnErrorOnFailure(aEncoder.Encode(mDelegate->GetNumberOfPresets()));
    }
    break;
    case Presets::Id: {
        auto delegate = mDelegate;
        VerifyOrReturnError(delegate != nullptr, CHIP_ERROR_INCORRECT_STATE, ChipLogError(Zcl, "Delegate is null"));

        auto & subjectDescriptor = aEncoder.GetSubjectDescriptor();
        if (InAtomicWrite(aPath.mEndpointId, subjectDescriptor, MakeOptional(aPath.mAttributeId)))
        {
            return aEncoder.EncodeList([delegate](const auto & encoder) -> CHIP_ERROR {
                for (uint8_t i = 0; true; i++)
                {
                    PresetStructWithOwnedMembers preset;
                    auto err = delegate->GetPendingPresetAtIndex(i, preset);
                    if (err == CHIP_ERROR_PROVIDER_LIST_EXHAUSTED)
                    {
                        return CHIP_NO_ERROR;
                    }
                    ReturnErrorOnFailure(err);
                    ReturnErrorOnFailure(encoder.Encode(preset));
                }
            });
        }
        return aEncoder.EncodeList([delegate](const auto & encoder) -> CHIP_ERROR {
            for (uint8_t i = 0; true; i++)
            {
                PresetStructWithOwnedMembers preset;
                auto err = delegate->GetPresetAtIndex(i, preset);
                if (err == CHIP_ERROR_PROVIDER_LIST_EXHAUSTED)
                {
                    return CHIP_NO_ERROR;
                }
                ReturnErrorOnFailure(err);
                ReturnErrorOnFailure(encoder.Encode(preset));
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

        ReturnErrorOnFailure(aEncoder.Encode(activePresetHandle));
    }
    break;
    case ScheduleTypes::Id: {
        auto delegate = mDelegate;
        VerifyOrReturnError(delegate != nullptr, CHIP_ERROR_INCORRECT_STATE, ChipLogError(Zcl, "Delegate is null"));

        return aEncoder.EncodeList([delegate](const auto & encoder) -> CHIP_ERROR {
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
        return aEncoder.EncodeList([](const auto & encoder) -> CHIP_ERROR { return CHIP_NO_ERROR; });
    }
    break;
    case MaxThermostatSuggestions::Id: {
        VerifyOrReturnError(mDelegate != nullptr, CHIP_ERROR_INCORRECT_STATE, ChipLogError(Zcl, "Delegate is null"));

        ReturnErrorOnFailure(aEncoder.Encode(mDelegate->GetMaxThermostatSuggestions()));
    }
    break;
    case ThermostatSuggestions::Id: {
        auto delegate = mDelegate;
        VerifyOrReturnError(delegate != nullptr, CHIP_ERROR_INCORRECT_STATE, ChipLogError(Zcl, "Delegate is null"));

        return aEncoder.EncodeList([delegate](const auto & encoder) -> CHIP_ERROR {
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
        ReturnErrorOnFailure(aEncoder.Encode(currentThermostatSuggestion));
    }
    break;
    case ThermostatSuggestionNotFollowingReason::Id: {
        VerifyOrReturnError(mDelegate != nullptr, CHIP_ERROR_INCORRECT_STATE, ChipLogError(Zcl, "Delegate is null"));

        ReturnErrorOnFailure(aEncoder.Encode(mDelegate->GetThermostatSuggestionNotFollowingReason()));
    }
    break;
    default:
        break;
    }

    return CHIP_NO_ERROR;
}

// Handles writes to the delegate-backed atomic-write attributes (Presets / Schedules). All other writable
// attributes are handled directly by WriteAttribute.
CHIP_ERROR ThermostatCluster::WriteDelegateAttribute(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder)
{
    EndpointId endpoint      = aPath.mEndpointId;
    auto & subjectDescriptor = aDecoder.GetSubjectDescriptor();

    // Check atomic attributes first
    switch (aPath.mAttributeId)
    {
    case Presets::Id: {

        VerifyOrReturnError(mDelegate != nullptr, CHIP_ERROR_INCORRECT_STATE, ChipLogError(Zcl, "Delegate is null"));

        // Presets are not editable, return INVALID_IN_STATE.
        VerifyOrReturnError(InAtomicWrite(endpoint, MakeOptional(aPath.mAttributeId)), CHIP_IM_GLOBAL_STATUS(InvalidInState),
                            ChipLogError(Zcl, "Presets are not editable"));

        // OK, we're in an atomic write, make sure the requesting node is the same one that started the atomic write,
        // otherwise return BUSY.
        if (!InAtomicWrite(endpoint, subjectDescriptor, MakeOptional(aPath.mAttributeId)))
        {
            ChipLogError(Zcl, "Another node is editing presets. Server is busy. Try again later");
            return CHIP_IM_GLOBAL_STATUS(Busy);
        }

        // If the list operation is replace all, clear the existing pending list, iterate over the new presets list
        // and add to the pending presets list.
        if (!aPath.IsListOperation() || aPath.mListOp == ConcreteDataAttributePath::ListOperation::ReplaceAll)
        {
            // Clear the pending presets list
            mDelegate->ClearPendingPresetList();

            Presets::TypeInfo::DecodableType newPresetsList;
            ReturnErrorOnFailure(aDecoder.Decode(newPresetsList));

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
        if (aPath.mListOp == ConcreteDataAttributePath::ListOperation::AppendItem)
        {
            PresetStruct::Type preset;
            ReturnErrorOnFailure(aDecoder.Decode(preset));
            return AppendPendingPreset(preset);
        }
    }
    break;
    case Schedules::Id: {
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }
    break;
    }

    return CHIP_NO_ERROR;
}

void ThermostatCluster::OnFabricRemoved(const FabricTable & fabricTable, FabricIndex fabricIndex)
{
    if (mAtomicWriteSession.state == AtomicWriteState::Open && mAtomicWriteSession.nodeId.GetFabricIndex() == fabricIndex)
    {
        ResetAtomicWrite(GetEndpointId());
    }
}

DataModel::ActionReturnStatus ThermostatCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                               AttributeValueEncoder & encoder)
{
    switch (request.path.mAttributeId)
    {
    case FeatureMap::Id:
        return encoder.Encode(mFeatures.Raw());
    case ClusterRevision::Id:
        return encoder.Encode(Thermostat::kRevision);
    case LocalTemperature::Id:
        return mFeatures.Has(Feature::kLocalTemperatureNotExposed) ? encoder.EncodeNull() : encoder.Encode(mLocalTemperature);
    case RemoteSensing::Id: {
        auto value = mRemoteSensing;
        if (mFeatures.Has(Feature::kLocalTemperatureNotExposed))
        {
            value.Clear(RemoteSensingBitmap::kLocalTemperature);
        }
        return encoder.Encode(value);
    }
    case OutdoorTemperature::Id:
        return encoder.Encode(mOutdoorTemperature);
    case Occupancy::Id:
        return encoder.Encode(mOccupancy);
    case AbsMinHeatSetpointLimit::Id:
        return encoder.Encode(mAbsMinHeatSetpointLimit);
    case AbsMaxHeatSetpointLimit::Id:
        return encoder.Encode(mAbsMaxHeatSetpointLimit);
    case AbsMinCoolSetpointLimit::Id:
        return encoder.Encode(mAbsMinCoolSetpointLimit);
    case AbsMaxCoolSetpointLimit::Id:
        return encoder.Encode(mAbsMaxCoolSetpointLimit);
    case LocalTemperatureCalibration::Id:
        return encoder.Encode(mLocalTemperatureCalibration);
    case OccupiedCoolingSetpoint::Id:
        return encoder.Encode(mOccupiedCoolingSetpoint);
    case OccupiedHeatingSetpoint::Id:
        return encoder.Encode(mOccupiedHeatingSetpoint);
    case UnoccupiedCoolingSetpoint::Id:
        return encoder.Encode(mUnoccupiedCoolingSetpoint);
    case UnoccupiedHeatingSetpoint::Id:
        return encoder.Encode(mUnoccupiedHeatingSetpoint);
    case MinHeatSetpointLimit::Id:
        return encoder.Encode(mMinHeatSetpointLimit);
    case MaxHeatSetpointLimit::Id:
        return encoder.Encode(mMaxHeatSetpointLimit);
    case MinCoolSetpointLimit::Id:
        return encoder.Encode(mMinCoolSetpointLimit);
    case MaxCoolSetpointLimit::Id:
        return encoder.Encode(mMaxCoolSetpointLimit);
    case MinSetpointDeadBand::Id:
        return encoder.Encode(mMinSetpointDeadBand);
    case ControlSequenceOfOperation::Id:
        return encoder.Encode(mControlSequenceOfOperation);
    case SystemMode::Id:
        return encoder.Encode(mSystemMode);
    case ThermostatRunningMode::Id:
        return encoder.Encode(mThermostatRunningMode);
    case StartOfWeek::Id:
        return encoder.Encode(mStartOfWeek);
    case NumberOfWeeklyTransitions::Id:
        return encoder.Encode(mNumberOfWeeklyTransitions);
    case NumberOfDailyTransitions::Id:
        return encoder.Encode(mNumberOfDailyTransitions);
    case TemperatureSetpointHold::Id:
        return encoder.Encode(mTemperatureSetpointHold);
    case TemperatureSetpointHoldDuration::Id:
        return encoder.Encode(mTemperatureSetpointHoldDuration);
    case ThermostatProgrammingOperationMode::Id:
        return encoder.Encode(mThermostatProgrammingOperationMode);
    case ThermostatRunningState::Id:
        return encoder.Encode(mThermostatRunningState);
    case SetpointChangeSource::Id:
        return encoder.Encode(mSetpointChangeSource);
    case SetpointChangeAmount::Id:
        return encoder.Encode(mSetpointChangeAmount);
    case SetpointChangeSourceTimestamp::Id:
        return encoder.Encode(mSetpointChangeSourceTimestamp);
    case OccupiedSetback::Id:
        return encoder.Encode(mOccupiedSetback);
    case OccupiedSetbackMin::Id:
        return encoder.Encode(mOccupiedSetbackMin);
    case OccupiedSetbackMax::Id:
        return encoder.Encode(mOccupiedSetbackMax);
    case UnoccupiedSetback::Id:
        return encoder.Encode(mUnoccupiedSetback);
    case UnoccupiedSetbackMin::Id:
        return encoder.Encode(mUnoccupiedSetbackMin);
    case UnoccupiedSetbackMax::Id:
        return encoder.Encode(mUnoccupiedSetbackMax);
    case EmergencyHeatDelta::Id:
        return encoder.Encode(mEmergencyHeatDelta);
    case ACType::Id:
        return encoder.Encode(mACType);
    case ACCapacity::Id:
        return encoder.Encode(mACCapacity);
    case ACRefrigerantType::Id:
        return encoder.Encode(mACRefrigerantType);
    case ACCompressorType::Id:
        return encoder.Encode(mACCompressorType);
    case ACErrorCode::Id:
        return encoder.Encode(mACErrorCode);
    case ACLouverPosition::Id:
        return encoder.Encode(mACLouverPosition);
    case ACCoilTemperature::Id:
        return encoder.Encode(mACCoilTemperature);
    case ACCapacityformat::Id:
        return encoder.Encode(mACCapacityformat);
    case NumberOfSchedules::Id:
        return encoder.Encode(mNumberOfSchedules);
    case NumberOfScheduleTransitions::Id:
        return encoder.Encode(mNumberOfScheduleTransitions);
    case NumberOfScheduleTransitionPerDay::Id:
        return encoder.Encode(mNumberOfScheduleTransitionPerDay);
    case ActiveScheduleHandle::Id:
        return encoder.Encode(mActiveScheduleHandle);
    case ActivePresetHandle::Id:
        return encoder.Encode(mActivePresetHandle);
    case SetpointHoldExpiryTimestamp::Id:
        return encoder.Encode(mSetpointHoldExpiryTimestamp);
    // Delegate-backed list / preset / schedule / suggestion attributes are still served by the retained helper.
    case PresetTypes::Id:
    case NumberOfPresets::Id:
    case Presets::Id:
    case ScheduleTypes::Id:
    case Schedules::Id:
    case MaxThermostatSuggestions::Id:
    case ThermostatSuggestions::Id:
    case CurrentThermostatSuggestion::Id:
    case ThermostatSuggestionNotFollowingReason::Id:
        return ReadDelegateAttribute(ConcreteReadAttributePath(GetEndpointId(), Thermostat::Id, request.path.mAttributeId),
                                     encoder);
    default:
        return Status::UnsupportedAttribute;
    }
}

void ThermostatCluster::GenerateScalarChangeEvent(AttributeId attributeId)
{
    if (!mFeatures.Has(Feature::kEvents))
    {
        return;
    }

    const EndpointId endpoint = GetEndpointId();
    switch (attributeId)
    {
    case SystemMode::Id:
        GenerateSystemModeChangeEvent(endpoint, NullOptional, mSystemMode);
        break;
    case OccupiedHeatingSetpoint::Id:
        GenerateSetpointChangeEvent(endpoint, SystemModeEnum::kHeat,
                                    MakeOptional(BitMask<OccupancyBitmap>(OccupancyBitmap::kOccupied)), NullOptional,
                                    mOccupiedHeatingSetpoint);
        break;
    case OccupiedCoolingSetpoint::Id:
        GenerateSetpointChangeEvent(endpoint, SystemModeEnum::kCool,
                                    MakeOptional(BitMask<OccupancyBitmap>(OccupancyBitmap::kOccupied)), NullOptional,
                                    mOccupiedCoolingSetpoint);
        break;
    case UnoccupiedHeatingSetpoint::Id:
        GenerateSetpointChangeEvent(endpoint, SystemModeEnum::kHeat, MakeOptional(BitMask<OccupancyBitmap>(0)), NullOptional,
                                    mUnoccupiedHeatingSetpoint);
        break;
    case UnoccupiedCoolingSetpoint::Id:
        GenerateSetpointChangeEvent(endpoint, SystemModeEnum::kCool, MakeOptional(BitMask<OccupancyBitmap>(0)), NullOptional,
                                    mUnoccupiedCoolingSetpoint);
        break;
    default:
        break;
    }
}

void ThermostatCluster::SetLocalTemperature(DataModel::Nullable<int16_t> value)
{
    if (SetAttributeValue(mLocalTemperature, value, LocalTemperature::Id) && mFeatures.Has(Feature::kEvents))
    {
        GenerateLocalTemperatureChangeEvent(GetEndpointId(), mLocalTemperature);
    }
}

DataModel::Nullable<int16_t> ThermostatCluster::GetLocalTemperature()
{
    return mLocalTemperature;
}

void ThermostatCluster::SetOutdoorTemperature(DataModel::Nullable<int16_t> value)
{
    SetAttributeValue(mOutdoorTemperature, value, OutdoorTemperature::Id);
}

DataModel::Nullable<int16_t> ThermostatCluster::GetOutdoorTemperature()
{
    return mOutdoorTemperature;
}

void ThermostatCluster::SetOccupancy(BitMask<OccupancyBitmap> value)
{
    if (SetAttributeValue(mOccupancy, value, Occupancy::Id) && mFeatures.Has(Feature::kEvents))
    {
        GenerateOccupancyChangeEvent(GetEndpointId(), NullOptional, mOccupancy);
    }
}

BitMask<chip::app::Clusters::Thermostat::OccupancyBitmap> ThermostatCluster::GetOccupancy()
{
    return mOccupancy;
}

void ThermostatCluster::SetLocalTemperatureCalibration(int8_t value)
{
    VerifyOrReturn(value != mLocalTemperatureCalibration);
    NotifyAttributeChangedIfSuccess(LocalTemperatureCalibration::Id, DefaultServerCluster::mContext->attributeStorage.WriteValue(
        { mPath.mEndpointId, Thermostat::Id, LocalTemperatureCalibration::Id }, { reinterpret_cast<const uint8_t *>(&mLocalTemperatureCalibration), sizeof(mLocalTemperatureCalibration) }));
}

int8_t ThermostatCluster::GetLocalTemperatureCalibration()
{
    return mLocalTemperatureCalibration;
}

Protocols::InteractionModel::Status ThermostatCluster::SetOccupiedCoolingSetpoint(int16_t value)
{
    return HandleOccupancyCoolingSetpoint(value, OccupiedCoolingSetpoint::Id);
}

int16_t ThermostatCluster::GetOccupiedCoolingSetpoint()
{
    return mOccupiedCoolingSetpoint;
}

Protocols::InteractionModel::Status ThermostatCluster::SetOccupiedHeatingSetpoint(int16_t value)
{
    return HandleOccupancyHeatingSetpoint(value, OccupiedHeatingSetpoint::Id);
}

int16_t ThermostatCluster::GetOccupiedHeatingSetpoint()
{
    return mOccupiedHeatingSetpoint;
}

Protocols::InteractionModel::Status ThermostatCluster::SetUnoccupiedCoolingSetpoint(int16_t value)
{
    return HandleOccupancyCoolingSetpoint(value, UnoccupiedCoolingSetpoint::Id);
}

int16_t ThermostatCluster::GetUnoccupiedCoolingSetpoint()
{
    return mUnoccupiedCoolingSetpoint;
}

Protocols::InteractionModel::Status ThermostatCluster::SetUnoccupiedHeatingSetpoint(int16_t value)
{
    return HandleOccupancyHeatingSetpoint(value, UnoccupiedHeatingSetpoint::Id);
}

int16_t ThermostatCluster::GetUnoccupiedHeatingSetpoint()
{
    return mUnoccupiedHeatingSetpoint;
}

Protocols::InteractionModel::Status ThermostatCluster::SetMinHeatSetpointLimit(int16_t value)
{
    return HandleMinHeatSetpointLimit(value);
}

int16_t ThermostatCluster::GetMinHeatSetpointLimit()
{
    return mMinHeatSetpointLimit;
}

Protocols::InteractionModel::Status ThermostatCluster::SetMaxHeatSetpointLimit(int16_t value)
{
    return HandleMaxHeatSetpointLimit(value);
}

int16_t ThermostatCluster::GetMaxHeatSetpointLimit()
{
    return mMaxHeatSetpointLimit;
}

Protocols::InteractionModel::Status ThermostatCluster::SetMinCoolSetpointLimit(int16_t value)
{
    return HandleMinCoolSetpointLimit(value);
}

int16_t ThermostatCluster::GetMinCoolSetpointLimit()
{
    return mMinCoolSetpointLimit;
}

Protocols::InteractionModel::Status ThermostatCluster::SetMaxCoolSetpointLimit(int16_t value)
{
    return HandleMaxCoolSetpointLimit(value);
}

int16_t ThermostatCluster::GetMaxCoolSetpointLimit()
{
    return mMaxCoolSetpointLimit;
}

Protocols::InteractionModel::Status ThermostatCluster::SetMinSetpointDeadbanc(int8_t value)
{
    return HandleMinSetpointDeadband(value);
}

int8_t ThermostatCluster::GetMinSetpointDeadbanc()
{
    return mMinSetpointDeadBand;
}

void ThermostatCluster::SetThermostatRunningState(BitMask<RelayStateBitmap> value)
{
    if (SetAttributeValue(mThermostatRunningState, value, ThermostatRunningState::Id) && mFeatures.Has(Feature::kEvents))
    {
        GenerateRunningStateChangeEvent(GetEndpointId(), NullOptional, mThermostatRunningState);
    }
}

void ThermostatCluster::SetThermostatRunningMode(ThermostatRunningModeEnum value)
{
    if (SetAttributeValue(mThermostatRunningMode, value, ThermostatRunningMode::Id) && mFeatures.Has(Feature::kEvents))
    {
        GenerateRunningModeChangeEvent(GetEndpointId(), NullOptional, mThermostatRunningMode);
    }
}

void ThermostatCluster::SetSetpointChangeSource(SetpointChangeSourceEnum value)
{
    SetAttributeValue(mSetpointChangeSource, value, SetpointChangeSource::Id);
}

void ThermostatCluster::SetSetpointChangeAmount(DataModel::Nullable<int16_t> value)
{
    SetAttributeValue(mSetpointChangeAmount, value, SetpointChangeAmount::Id);
}

void ThermostatCluster::SetSetpointChangeSourceTimestamp(uint32_t value)
{
    SetAttributeValue(mSetpointChangeSourceTimestamp, value, SetpointChangeSourceTimestamp::Id);
}

void ThermostatCluster::SetACCoilTemperature(DataModel::Nullable<int16_t> value)
{
    SetAttributeValue(mACCoilTemperature, value, ACCoilTemperature::Id);
}

void ThermostatCluster::SetActivePresetHandle(DataModel::Nullable<ByteSpan> value)
{
    VerifyOrReturn(DefaultServerCluster::mContext != nullptr);

    if (value.IsNull())
    {
        if (mActivePresetHandle.IsNull())
        {
            return; // no-op
        }
        mActivePresetHandle.SetNull();
    }
    else
    {
        VerifyOrReturn(value.Value().size() <= kPresetHandleSize,
                       ChipLogError(Zcl, "SetActivePresetHandle: handle too large (%u bytes)", (unsigned) value.Value().size()));
        if (!mActivePresetHandle.IsNull() && mActivePresetHandle.Value().data_equal(value.Value()))
        {
            return; // no-op: same bytes
        }
        memcpy(mActivePresetHandleBuffer, value.Value().data(), value.Value().size());
        mActivePresetHandle.SetNonNull(ByteSpan(mActivePresetHandleBuffer, value.Value().size()));
    }

    NotifyAttributeChanged(ActivePresetHandle::Id);

    const ByteSpan toStore = mActivePresetHandle.IsNull() ? ByteSpan{} : mActivePresetHandle.Value();
    LogErrorOnFailure(DefaultServerCluster::mContext->attributeStorage.WriteValue(
        { mPath.mEndpointId, Thermostat::Id, ActivePresetHandle::Id }, toStore));
}

void ThermostatCluster::SetActiveScheduleHandle(DataModel::Nullable<ByteSpan> value)
{
    VerifyOrReturn(DefaultServerCluster::mContext != nullptr);

    if (value.IsNull())
    {
        if (mActiveScheduleHandle.IsNull())
        {
            return; // no-op
        }
        mActiveScheduleHandle.SetNull();
    }
    else
    {
        VerifyOrReturn(value.Value().size() <= kPresetHandleSize,
                       ChipLogError(Zcl, "SetActiveScheduleHandle: handle too large (%u bytes)", (unsigned) value.Value().size()));
        if (!mActiveScheduleHandle.IsNull() && mActiveScheduleHandle.Value().data_equal(value.Value()))
        {
            return; // no-op: same bytes
        }
        memcpy(mActiveScheduleHandleBuffer, value.Value().data(), value.Value().size());
        mActiveScheduleHandle.SetNonNull(ByteSpan(mActiveScheduleHandleBuffer, value.Value().size()));
    }

    NotifyAttributeChanged(ActiveScheduleHandle::Id);

    const ByteSpan toStore = mActiveScheduleHandle.IsNull() ? ByteSpan{} : mActiveScheduleHandle.Value();
    LogErrorOnFailure(DefaultServerCluster::mContext->attributeStorage.WriteValue(
        { mPath.mEndpointId, Thermostat::Id, ActiveScheduleHandle::Id }, toStore));
}

bool ThermostatCluster::SetAndPersistSetpoint(int16_t & member, int16_t value, AttributeId attributeId)
{
    if (!SetAttributeValue(member, value, attributeId))
    {
        return false;
    }
    LogErrorOnFailure(DefaultServerCluster::mContext->attributeStorage.WriteValue(
        { mPath.mEndpointId, Thermostat::Id, attributeId },
        { reinterpret_cast<const uint8_t *>(&member), sizeof(member) }));
    return true;
}

Protocols::InteractionModel::Status ThermostatCluster::HandleOccupancyHeatingSetpoint(int16_t value, AttributeId attributeId)
{
    VerifyOrReturnError(value >= mAbsMinHeatSetpointLimit && value >= mMinHeatSetpointLimit &&
                                value <= mAbsMaxHeatSetpointLimit && value <= mMaxHeatSetpointLimit,
                            Status::InvalidValue);
    if (mFeatures.Has(Feature::kAutoMode))
    {
        int16_t maxCooling = std::min(mMaxCoolSetpointLimit, mAbsMaxCoolSetpointLimit);
        VerifyOrReturnError(static_cast<int16_t>(value + DeadBandTemp()) <= maxCooling, Status::InvalidValue);
    }
    if (attributeId == OccupiedHeatingSetpoint::Id)
    {
        SetAndPersistSetpoint(mOccupiedHeatingSetpoint, value, OccupiedHeatingSetpoint::Id);
    }
    else
    {
        SetAndPersistSetpoint(mUnoccupiedHeatingSetpoint, value, UnoccupiedHeatingSetpoint::Id);
    }
    HandleSetpointPostWrite(attributeId);

    return Status::Success;
}

Protocols::InteractionModel::Status ThermostatCluster::HandleOccupancyCoolingSetpoint(int16_t value, AttributeId attributeId)
{
    VerifyOrReturnError(value >= mAbsMinCoolSetpointLimit && value >= mMinCoolSetpointLimit &&
                            value <= mAbsMaxCoolSetpointLimit && value <= mMaxCoolSetpointLimit,
                        Status::InvalidValue);
    if (mFeatures.Has(Feature::kAutoMode))
    {
        int16_t minHeating = std::max(mMinHeatSetpointLimit, mAbsMinHeatSetpointLimit);
        VerifyOrReturnError(static_cast<int16_t>(value - DeadBandTemp()) >= minHeating, Status::InvalidValue);
    }
    if (attributeId == OccupiedCoolingSetpoint::Id)
    {
        SetAndPersistSetpoint(mOccupiedCoolingSetpoint, value, OccupiedCoolingSetpoint::Id);
    }
    else
    {
        SetAndPersistSetpoint(mUnoccupiedCoolingSetpoint, value, UnoccupiedCoolingSetpoint::Id);
    }
    HandleSetpointPostWrite(attributeId);

    return Status::Success;
}

Protocols::InteractionModel::Status ThermostatCluster::HandleMinHeatSetpointLimit(int16_t value)
{
    VerifyOrReturnError(value >= mAbsMinHeatSetpointLimit && value <= mMaxHeatSetpointLimit &&
                            value <= mAbsMaxHeatSetpointLimit,
                        Status::InvalidValue);
    if (mFeatures.Has(Feature::kAutoMode))
    {
        VerifyOrReturnError(value <= static_cast<int16_t>(mMinCoolSetpointLimit - DeadBandTemp()), Status::InvalidValue);
    }
    SetAndPersistSetpoint(mMinHeatSetpointLimit, value, MinHeatSetpointLimit::Id);
    return Status::Success;
}

Protocols::InteractionModel::Status ThermostatCluster::HandleMaxHeatSetpointLimit(int16_t value)
{
    VerifyOrReturnError(value >= mAbsMinHeatSetpointLimit && value >= mMinHeatSetpointLimit &&
                            value <= mAbsMaxHeatSetpointLimit,
                        Status::InvalidValue);
    if (mFeatures.Has(Feature::kAutoMode))
    {
        VerifyOrReturnError(value <= static_cast<int16_t>(mMaxCoolSetpointLimit - DeadBandTemp()), Status::InvalidValue);
    }
    SetAndPersistSetpoint(mMaxHeatSetpointLimit, value, MaxHeatSetpointLimit::Id);
    return Status::Success;
}

Protocols::InteractionModel::Status ThermostatCluster::HandleMinCoolSetpointLimit(int16_t value)
{
    VerifyOrReturnError(value >= mAbsMinCoolSetpointLimit && value <= mMaxCoolSetpointLimit &&
                            value <= mAbsMaxCoolSetpointLimit,
                        Status::InvalidValue);
    if (mFeatures.Has(Feature::kAutoMode))
    {
        VerifyOrReturnError(value >= static_cast<int16_t>(mMinHeatSetpointLimit + DeadBandTemp()), Status::InvalidValue);
    }
    SetAndPersistSetpoint(mMinCoolSetpointLimit, value, MinCoolSetpointLimit::Id);
    return Status::Success;
}

Protocols::InteractionModel::Status ThermostatCluster::HandleMaxCoolSetpointLimit(int16_t value)
{
    VerifyOrReturnError(value >= mAbsMinCoolSetpointLimit && value >= mMinCoolSetpointLimit &&
                            value <= mAbsMaxCoolSetpointLimit,
                        Status::InvalidValue);
    if (mFeatures.Has(Feature::kAutoMode))
    {
        VerifyOrReturnError(value >= static_cast<int16_t>(mMaxHeatSetpointLimit + DeadBandTemp()), Status::InvalidValue);
    }
    SetAndPersistSetpoint(mMaxCoolSetpointLimit, value, MaxCoolSetpointLimit::Id);
    return Status::Success;
}

Protocols::InteractionModel::Status ThermostatCluster::HandleMinSetpointDeadband(int8_t value)
{
    VerifyOrReturnError(mFeatures.Has(Feature::kAutoMode), Status::UnsupportedAttribute);
    VerifyOrReturnError(value >= 0 && value <= 127, Status::InvalidValue);
    SetAttributeValue(mMinSetpointDeadBand, value, MinSetpointDeadBand::Id);
    return Status::Success;
}

DataModel::ActionReturnStatus ThermostatCluster::WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                                AttributeValueDecoder & decoder)
{
    const AttributeId attributeId = request.path.mAttributeId;
    AttributePersistence persistence{ DefaultServerCluster::mContext->attributeStorage };

    // Preset / Schedule writes (and their atomic-write semantics) are still owned by the retained helper.
    if (attributeId == Presets::Id || attributeId == Schedules::Id)
    {
        return WriteDelegateAttribute(request.path, decoder);
    }

    // No non-atomic attribute may be written while this client has an atomic write open.
    if (InAtomicWrite(GetEndpointId(), decoder.GetSubjectDescriptor()))
    {
        return Status::InvalidInState;
    }

    switch (attributeId)
    {
    case OccupiedHeatingSetpoint::Id:
    case UnoccupiedHeatingSetpoint::Id: {
        int16_t requested;
        ReturnErrorOnFailure(decoder.Decode(requested));
        return HandleOccupancyHeatingSetpoint(requested, attributeId);
    }
    case OccupiedCoolingSetpoint::Id:
    case UnoccupiedCoolingSetpoint::Id: {
        int16_t requested;
        ReturnErrorOnFailure(decoder.Decode(requested));
        return HandleOccupancyCoolingSetpoint(requested, attributeId);
    }
    case MinHeatSetpointLimit::Id: {
        int16_t requested;
        ReturnErrorOnFailure(decoder.Decode(requested));
        return HandleMinHeatSetpointLimit(requested);
    }
    case MaxHeatSetpointLimit::Id: {
        int16_t requested;
        ReturnErrorOnFailure(decoder.Decode(requested));
        return HandleMaxHeatSetpointLimit(requested);
    }
    case MinCoolSetpointLimit::Id: {
        int16_t requested;
        ReturnErrorOnFailure(decoder.Decode(requested));
        return HandleMinCoolSetpointLimit(requested);
    }
    case MaxCoolSetpointLimit::Id: {
        int16_t requested;
        ReturnErrorOnFailure(decoder.Decode(requested));
        return HandleMaxCoolSetpointLimit(requested);
    }
    case MinSetpointDeadBand::Id: {
        int8_t requested;
        ReturnErrorOnFailure(decoder.Decode(requested));
        return HandleMinSetpointDeadband(requested);
    }
    case RemoteSensing::Id: {
        BitMask<RemoteSensingBitmap> requested;
        ReturnErrorOnFailure(decoder.Decode(requested));
        if (mFeatures.Has(Feature::kLocalTemperatureNotExposed))
        {
            VerifyOrReturnError(!requested.Has(RemoteSensingBitmap::kLocalTemperature), Status::ConstraintError);
        }
        SetAttributeValue(mRemoteSensing, requested, RemoteSensing::Id);
        LogErrorOnFailure(DefaultServerCluster::mContext->attributeStorage.WriteValue(request.path, 
            { reinterpret_cast<const uint8_t *>(requested.RawStorage()), sizeof(requested.Raw()) }));
        return CHIP_NO_ERROR;
    }
    case ControlSequenceOfOperation::Id: {
        ControlSequenceOfOperationEnum requested;
        ReturnErrorOnFailure(decoder.Decode(requested));
        VerifyOrReturnError(requested <= ControlSequenceOfOperationEnum::kCoolingAndHeatingWithReheat, Status::InvalidValue);
        SetAttributeValue(mControlSequenceOfOperation, requested, ControlSequenceOfOperation::Id);
        LogErrorOnFailure(DefaultServerCluster::mContext->attributeStorage.WriteValue(request.path,
             { reinterpret_cast<const uint8_t *>(requested), sizeof(requested) }));
        return CHIP_NO_ERROR;
    }
    case SystemMode::Id: {
        SystemModeEnum requested;
        ReturnErrorOnFailure(decoder.Decode(requested));
        VerifyOrReturnError(EnsureKnownEnumValue(mControlSequenceOfOperation) != ControlSequenceOfOperationEnum::kUnknownEnumValue &&
                                EnsureKnownEnumValue(requested) != SystemModeEnum::kUnknownEnumValue,
                            Status::InvalidValue);
        switch (mControlSequenceOfOperation)
        {
        case ControlSequenceOfOperationEnum::kCoolingOnly:
        case ControlSequenceOfOperationEnum::kCoolingWithReheat:
            VerifyOrReturnError(requested != SystemModeEnum::kHeat && requested != SystemModeEnum::kEmergencyHeat,
                                Status::InvalidValue);
            break;
        case ControlSequenceOfOperationEnum::kHeatingOnly:
        case ControlSequenceOfOperationEnum::kHeatingWithReheat:
            VerifyOrReturnError(requested != SystemModeEnum::kCool && requested != SystemModeEnum::kPrecooling,
                                Status::InvalidValue);
            break;
        default:
            break;
        }
        SetAttributeValue(mSystemMode, requested, SystemMode::Id);
        LogErrorOnFailure(DefaultServerCluster::mContext->attributeStorage.WriteValue(request.path,
             { reinterpret_cast<const uint8_t *>(requested), sizeof(requested) }));
        GenerateScalarChangeEvent(SystemMode::Id);
        return CHIP_NO_ERROR;
    }
    // Writable attributes without cross-attribute validation — decode, persist, and notify in one step.
    case LocalTemperatureCalibration::Id:
        return NotifyAttributeChangedIfSuccess(LocalTemperatureCalibration::Id,
            persistence.DecodeAndStoreNativeEndianValue(request.path, decoder, mLocalTemperatureCalibration));
    case TemperatureSetpointHold::Id:
        return NotifyAttributeChangedIfSuccess(TemperatureSetpointHold::Id,
            persistence.DecodeAndStoreNativeEndianValue(request.path, decoder, mTemperatureSetpointHold));
    case TemperatureSetpointHoldDuration::Id:
        return NotifyAttributeChangedIfSuccess(TemperatureSetpointHoldDuration::Id,
            persistence.DecodeAndStoreNativeEndianValue(request.path, decoder, mTemperatureSetpointHoldDuration));
    case EmergencyHeatDelta::Id:
        return NotifyAttributeChangedIfSuccess(EmergencyHeatDelta::Id,
            persistence.DecodeAndStoreNativeEndianValue(request.path, decoder, mEmergencyHeatDelta));
    case ACType::Id:
        return NotifyAttributeChangedIfSuccess(ACType::Id,
            persistence.DecodeAndStoreNativeEndianValue(request.path, decoder, mACType));
    case ACCapacity::Id:
        return NotifyAttributeChangedIfSuccess(ACCapacity::Id,
            persistence.DecodeAndStoreNativeEndianValue(request.path, decoder, mACCapacity));
    case ACRefrigerantType::Id:
        return NotifyAttributeChangedIfSuccess(ACRefrigerantType::Id,
            persistence.DecodeAndStoreNativeEndianValue(request.path, decoder, mACRefrigerantType));
    case ACCompressorType::Id:
        return NotifyAttributeChangedIfSuccess(ACCompressorType::Id,
            persistence.DecodeAndStoreNativeEndianValue(request.path, decoder, mACCompressorType));
    case ACErrorCode::Id: {
        Attributes::ACErrorCode::TypeInfo::Type requested;
        ReturnErrorOnFailure(decoder.Decode(requested));
        SetAttributeValue(mACErrorCode, requested, ACErrorCode::Id);
        return CHIP_NO_ERROR;
    }
    case ACLouverPosition::Id:
        return NotifyAttributeChangedIfSuccess(ACLouverPosition::Id,
            persistence.DecodeAndStoreNativeEndianValue(request.path, decoder, mACLouverPosition));
    case ACCapacityformat::Id:
        return NotifyAttributeChangedIfSuccess(ACCapacityformat::Id,
            persistence.DecodeAndStoreNativeEndianValue(request.path, decoder, mACCapacityformat));
    default:
        return Status::UnsupportedWrite;
    }
}

void ThermostatCluster::HandleSetpointPostWrite(AttributeId attributeId)
{
    const int16_t deadband = DeadBandTemp();

    // Maintain the deadband by shifting the paired setpoint (replaces EnsureDeadband). WriteAttribute has
    // already validated that this shift stays within the limits, so it cannot exceed them here.
    if (mFeatures.Has(Feature::kAutoMode))
    {
        switch (attributeId)
        {
        case OccupiedHeatingSetpoint::Id: {
            int16_t minValidCooling = static_cast<int16_t>(mOccupiedHeatingSetpoint + deadband);
            if (mOccupiedCoolingSetpoint < minValidCooling &&
                SetAndPersistSetpoint(mOccupiedCoolingSetpoint, minValidCooling, OccupiedCoolingSetpoint::Id))
            {
                GenerateScalarChangeEvent(OccupiedCoolingSetpoint::Id);
            }
            break;
        }
        case OccupiedCoolingSetpoint::Id: {
            int16_t maxValidHeating = static_cast<int16_t>(mOccupiedCoolingSetpoint - deadband);
            if (mOccupiedHeatingSetpoint > maxValidHeating &&
                SetAndPersistSetpoint(mOccupiedHeatingSetpoint, maxValidHeating, OccupiedHeatingSetpoint::Id))
            {
                GenerateScalarChangeEvent(OccupiedHeatingSetpoint::Id);
            }
            break;
        }
        case UnoccupiedHeatingSetpoint::Id: {
            int16_t minValidCooling = static_cast<int16_t>(mUnoccupiedHeatingSetpoint + deadband);
            if (mUnoccupiedCoolingSetpoint < minValidCooling &&
                SetAndPersistSetpoint(mUnoccupiedCoolingSetpoint, minValidCooling, UnoccupiedCoolingSetpoint::Id))
            {
                GenerateScalarChangeEvent(UnoccupiedCoolingSetpoint::Id);
            }
            break;
        }
        case UnoccupiedCoolingSetpoint::Id: {
            int16_t maxValidHeating = static_cast<int16_t>(mUnoccupiedCoolingSetpoint - deadband);
            if (mUnoccupiedHeatingSetpoint > maxValidHeating &&
                SetAndPersistSetpoint(mUnoccupiedHeatingSetpoint, maxValidHeating, UnoccupiedHeatingSetpoint::Id))
            {
                GenerateScalarChangeEvent(UnoccupiedHeatingSetpoint::Id);
            }
            break;
        }
        default:
            break;
        }
    }

    // Clear the active preset when a setpoint relevant to the current occupancy state changes.
    if (mFeatures.Has(Feature::kPresets))
    {
        const bool occupied = !mFeatures.Has(Feature::kOccupancy) || mOccupancy.Has(OccupancyBitmap::kOccupied);
        bool clear          = false;
        switch (attributeId)
        {
        case OccupiedHeatingSetpoint::Id:
        case OccupiedCoolingSetpoint::Id:
            clear = occupied;
            break;
        case UnoccupiedHeatingSetpoint::Id:
        case UnoccupiedCoolingSetpoint::Id:
            clear = !occupied;
            break;
        default:
            break;
        }
        if (clear)
        {
            SetActivePreset(DataModel::NullNullable);
        }
    }
}

std::optional<DataModel::ActionReturnStatus> ThermostatCluster::InvokeCommand(const DataModel::InvokeRequest & request,
                                                                              chip::TLV::TLVReader & input_arguments,
                                                                              CommandHandler * handler)
{
    switch (request.path.mCommandId)
    {
    case Commands::SetpointRaiseLower::Id: {
        Commands::SetpointRaiseLower::DecodableType commandData;
        ReturnErrorOnFailure(commandData.Decode(input_arguments));
        return HandleSetpointRaiseLower(commandData);
    }
    case Commands::SetActivePresetRequest::Id: {
        Commands::SetActivePresetRequest::DecodableType commandData;
        ReturnErrorOnFailure(commandData.Decode(input_arguments));
        return SetActivePreset(commandData.presetHandle);
    }
    case Commands::AtomicRequest::Id: {
        Commands::AtomicRequest::DecodableType commandData;
        ReturnErrorOnFailure(commandData.Decode(input_arguments));
        switch (commandData.requestType)
        {
        case Globals::AtomicRequestTypeEnum::kBeginWrite:
            BeginAtomicWrite(handler, request.path, commandData);
            break;
        case Globals::AtomicRequestTypeEnum::kCommitWrite:
            CommitAtomicWrite(handler, request.path, commandData);
            break;
        case Globals::AtomicRequestTypeEnum::kRollbackWrite:
            RollbackAtomicWrite(handler, request.path, commandData);
            break;
        default:
            handler->AddStatus(request.path, Status::InvalidCommand);
            break;
        }
        return std::nullopt;
    }
    case Commands::AddThermostatSuggestion::Id: {
        Commands::AddThermostatSuggestion::DecodableType commandData;
        ReturnErrorOnFailure(commandData.Decode(input_arguments));
        HandleAddThermostatSuggestion(handler, request.path, commandData);
        return std::nullopt;
    }
    case Commands::RemoveThermostatSuggestion::Id: {
        Commands::RemoveThermostatSuggestion::DecodableType commandData;
        ReturnErrorOnFailure(commandData.Decode(input_arguments));
        HandleRemoveThermostatSuggestion(handler, request.path, commandData);
        return std::nullopt;
    }
    default:
        // SetWeeklySchedule / GetWeeklySchedule / ClearWeeklySchedule / SetActiveScheduleRequest were never
        // implemented in the legacy server (their Ember callbacks returned false -> UnsupportedCommand).
        return Status::UnsupportedCommand;
    }
}

Protocols::InteractionModel::Status
ThermostatCluster::HandleSetpointRaiseLower(const Commands::SetpointRaiseLower::DecodableType & commandData)
{
    const auto & mode   = commandData.mode;
    const auto & amount = commandData.amount;

    const bool heatSupported = mFeatures.Has(Feature::kHeating);
    const bool coolSupported = mFeatures.Has(Feature::kCooling);
    const bool autoSupported = mFeatures.Has(Feature::kAutoMode);
    const int16_t deadband   = DeadBandTemp();

    Status status = Status::Failure;

    switch (mode)
    {
    case SetpointRaiseLowerModeEnum::kBoth: {
        bool coolOk = !coolSupported;
        bool heatOk = !heatSupported;

        if (heatSupported && coolSupported)
        {
            int16_t desiredCooling = static_cast<int16_t>(mOccupiedCoolingSetpoint + amount * 10);
            int16_t coolLimit      = static_cast<int16_t>(desiredCooling - EnforceCoolingSetpointLimits(desiredCooling));
            int16_t desiredHeating = static_cast<int16_t>(mOccupiedHeatingSetpoint + amount * 10);
            int16_t heatLimit      = static_cast<int16_t>(desiredHeating - EnforceHeatingSetpointLimits(desiredHeating));
            if (coolLimit != 0 || heatLimit != 0)
            {
                int16_t adjust = (abs(coolLimit) <= abs(heatLimit)) ? heatLimit : coolLimit;
                desiredHeating  = static_cast<int16_t>(desiredHeating - adjust);
                desiredCooling  = static_cast<int16_t>(desiredCooling - adjust);
            }
            if (SetAndPersistSetpoint(mOccupiedCoolingSetpoint, desiredCooling, OccupiedCoolingSetpoint::Id))
            {
                GenerateScalarChangeEvent(OccupiedCoolingSetpoint::Id);
            }
            if (SetAndPersistSetpoint(mOccupiedHeatingSetpoint, desiredHeating, OccupiedHeatingSetpoint::Id))
            {
                GenerateScalarChangeEvent(OccupiedHeatingSetpoint::Id);
            }
            coolOk = heatOk = true;
        }
        else if (coolSupported)
        {
            int16_t coolingSetpoint = EnforceCoolingSetpointLimits(static_cast<int16_t>(mOccupiedCoolingSetpoint + amount * 10));
            if (SetAndPersistSetpoint(mOccupiedCoolingSetpoint, coolingSetpoint, OccupiedCoolingSetpoint::Id))
            {
                GenerateScalarChangeEvent(OccupiedCoolingSetpoint::Id);
            }
            coolOk = true;
        }
        else if (heatSupported)
        {
            int16_t heatingSetpoint = EnforceHeatingSetpointLimits(static_cast<int16_t>(mOccupiedHeatingSetpoint + amount * 10));
            if (SetAndPersistSetpoint(mOccupiedHeatingSetpoint, heatingSetpoint, OccupiedHeatingSetpoint::Id))
            {
                GenerateScalarChangeEvent(OccupiedHeatingSetpoint::Id);
            }
            heatOk = true;
        }
        status = (heatOk && coolOk) ? Status::Success : Status::Failure;
        break;
    }
    case SetpointRaiseLowerModeEnum::kCool: {
        if (!coolSupported)
        {
            return Status::InvalidCommand;
        }
        int16_t coolingSetpoint = EnforceCoolingSetpointLimits(static_cast<int16_t>(mOccupiedCoolingSetpoint + amount * 10));
        if (autoSupported && (coolingSetpoint - mOccupiedHeatingSetpoint) < deadband)
        {
            int16_t heatingSetpoint = static_cast<int16_t>(coolingSetpoint - deadband);
            if (heatingSetpoint != EnforceHeatingSetpointLimits(heatingSetpoint))
            {
                return Status::InvalidCommand;
            }
            if (SetAndPersistSetpoint(mOccupiedHeatingSetpoint, heatingSetpoint, OccupiedHeatingSetpoint::Id))
            {
                GenerateScalarChangeEvent(OccupiedHeatingSetpoint::Id);
            }
        }
        if (SetAndPersistSetpoint(mOccupiedCoolingSetpoint, coolingSetpoint, OccupiedCoolingSetpoint::Id))
        {
            GenerateScalarChangeEvent(OccupiedCoolingSetpoint::Id);
        }
        status = Status::Success;
        break;
    }
    case SetpointRaiseLowerModeEnum::kHeat: {
        if (!heatSupported)
        {
            return Status::InvalidCommand;
        }
        int16_t heatingSetpoint = EnforceHeatingSetpointLimits(static_cast<int16_t>(mOccupiedHeatingSetpoint + amount * 10));
        if (autoSupported && (mOccupiedCoolingSetpoint - heatingSetpoint) < deadband)
        {
            int16_t coolingSetpoint = static_cast<int16_t>(heatingSetpoint + deadband);
            if (coolingSetpoint != EnforceCoolingSetpointLimits(coolingSetpoint))
            {
                return Status::InvalidCommand;
            }
            if (SetAndPersistSetpoint(mOccupiedCoolingSetpoint, coolingSetpoint, OccupiedCoolingSetpoint::Id))
            {
                GenerateScalarChangeEvent(OccupiedCoolingSetpoint::Id);
            }
        }
        if (SetAndPersistSetpoint(mOccupiedHeatingSetpoint, heatingSetpoint, OccupiedHeatingSetpoint::Id))
        {
            GenerateScalarChangeEvent(OccupiedHeatingSetpoint::Id);
        }
        status = Status::Success;
        break;
    }
    default:
        return Status::InvalidCommand;
    }

    // A successful SetpointRaiseLower changes occupied setpoints, which clears the active preset (occupied).
    if (status == Status::Success && mFeatures.Has(Feature::kPresets) &&
        (!mFeatures.Has(Feature::kOccupancy) || mOccupancy.Has(OccupancyBitmap::kOccupied)))
    {
        SetActivePreset(DataModel::NullNullable);
    }

    return status;
}

CHIP_ERROR ThermostatCluster::Attributes(const ConcreteClusterPath & path,
                                         ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    const bool heat = mFeatures.Has(Feature::kHeating);
    const bool cool = mFeatures.Has(Feature::kCooling);
    const bool occ  = mFeatures.Has(Feature::kOccupancy);
    const bool sch  = mFeatures.Has(Feature::kScheduleConfiguration);
    const bool autoM = mFeatures.Has(Feature::kAutoMode);
    const bool ltne = mFeatures.Has(Feature::kLocalTemperatureNotExposed);
    const bool msch = mFeatures.Has(Feature::kMatterScheduleConfiguration);
    const bool pres = mFeatures.Has(Feature::kPresets);
    const bool sugg = mFeatures.Has(Feature::kThermostatSuggestions);

    using Entry = AttributeListBuilder::OptionalAttributeEntry;
    // NOTE(3a): deprecated attributes (PI*Demand, HVACSystemTypeConfiguration, ThermostatProgrammingOperationMode,
    // *Setback*) are intentionally not advertised. Plain-optional non-feature attributes are advertised
    // unconditionally for now; matching them exactly to each application's ZAP configuration is a follow-up.
    const Entry optionalAttributes[] = {
        { true, OutdoorTemperature::kMetadataEntry },
        { occ, Occupancy::kMetadataEntry },
        { heat, AbsMinHeatSetpointLimit::kMetadataEntry },
        { heat, AbsMaxHeatSetpointLimit::kMetadataEntry },
        { cool, AbsMinCoolSetpointLimit::kMetadataEntry },
        { cool, AbsMaxCoolSetpointLimit::kMetadataEntry },
        { !ltne, LocalTemperatureCalibration::kMetadataEntry },
        { cool, OccupiedCoolingSetpoint::kMetadataEntry },
        { heat, OccupiedHeatingSetpoint::kMetadataEntry },
        { cool && occ, UnoccupiedCoolingSetpoint::kMetadataEntry },
        { heat && occ, UnoccupiedHeatingSetpoint::kMetadataEntry },
        { heat, MinHeatSetpointLimit::kMetadataEntry },
        { heat, MaxHeatSetpointLimit::kMetadataEntry },
        { cool, MinCoolSetpointLimit::kMetadataEntry },
        { cool, MaxCoolSetpointLimit::kMetadataEntry },
        { autoM, MinSetpointDeadBand::kMetadataEntry },
        { true, RemoteSensing::kMetadataEntry },
        { autoM, ThermostatRunningMode::kMetadataEntry },
        { sch, StartOfWeek::kMetadataEntry },
        { sch, NumberOfWeeklyTransitions::kMetadataEntry },
        { sch, NumberOfDailyTransitions::kMetadataEntry },
        { true, TemperatureSetpointHold::kMetadataEntry },
        { true, TemperatureSetpointHoldDuration::kMetadataEntry },
        { true, ThermostatRunningState::kMetadataEntry },
        { true, SetpointChangeSource::kMetadataEntry },
        { true, SetpointChangeAmount::kMetadataEntry },
        { true, SetpointChangeSourceTimestamp::kMetadataEntry },
        { true, EmergencyHeatDelta::kMetadataEntry },
        { true, ACType::kMetadataEntry },
        { true, ACCapacity::kMetadataEntry },
        { true, ACRefrigerantType::kMetadataEntry },
        { true, ACCompressorType::kMetadataEntry },
        { true, ACErrorCode::kMetadataEntry },
        { true, ACLouverPosition::kMetadataEntry },
        { true, ACCoilTemperature::kMetadataEntry },
        { true, ACCapacityformat::kMetadataEntry },
        { pres, PresetTypes::kMetadataEntry },
        { msch, ScheduleTypes::kMetadataEntry },
        { pres, NumberOfPresets::kMetadataEntry },
        { msch, NumberOfSchedules::kMetadataEntry },
        { msch, NumberOfScheduleTransitions::kMetadataEntry },
        { msch, NumberOfScheduleTransitionPerDay::kMetadataEntry },
        { pres, ActivePresetHandle::kMetadataEntry },
        { msch, ActiveScheduleHandle::kMetadataEntry },
        { pres, Presets::kMetadataEntry },
        { msch, Schedules::kMetadataEntry },
        { true, SetpointHoldExpiryTimestamp::kMetadataEntry },
        { sugg, MaxThermostatSuggestions::kMetadataEntry },
        { sugg, ThermostatSuggestions::kMetadataEntry },
        { sugg, CurrentThermostatSuggestion::kMetadataEntry },
        { sugg, ThermostatSuggestionNotFollowingReason::kMetadataEntry },
    };

    AttributeListBuilder listBuilder(builder);
    return listBuilder.Append(Span(Thermostat::Attributes::kMandatoryMetadata), Span(optionalAttributes));
}

CHIP_ERROR ThermostatCluster::AcceptedCommands(const ConcreteClusterPath & path,
                                               ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder)
{
    static constexpr DataModel::AcceptedCommandEntry kSetpointRaiseLower[] = {
        Commands::SetpointRaiseLower::kMetadataEntry,
    };
    static constexpr DataModel::AcceptedCommandEntry kWeeklyScheduleCommands[] = {
        Commands::SetWeeklySchedule::kMetadataEntry,
        Commands::GetWeeklySchedule::kMetadataEntry,
        Commands::ClearWeeklySchedule::kMetadataEntry,
    };
    static constexpr DataModel::AcceptedCommandEntry kSetActiveScheduleRequest[] = {
        Commands::SetActiveScheduleRequest::kMetadataEntry,
    };
    static constexpr DataModel::AcceptedCommandEntry kSetActivePresetRequest[] = {
        Commands::SetActivePresetRequest::kMetadataEntry,
    };
    static constexpr DataModel::AcceptedCommandEntry kSuggestionCommands[] = {
        Commands::AddThermostatSuggestion::kMetadataEntry,
        Commands::RemoveThermostatSuggestion::kMetadataEntry,
    };
    static constexpr DataModel::AcceptedCommandEntry kAtomicRequest[] = {
        Commands::AtomicRequest::kMetadataEntry,
    };

    ReturnErrorOnFailure(builder.ReferenceExisting(kSetpointRaiseLower));
    if (mFeatures.Has(Feature::kScheduleConfiguration))
    {
        ReturnErrorOnFailure(builder.ReferenceExisting(kWeeklyScheduleCommands));
    }
    if (mFeatures.Has(Feature::kMatterScheduleConfiguration))
    {
        ReturnErrorOnFailure(builder.ReferenceExisting(kSetActiveScheduleRequest));
    }
    if (mFeatures.Has(Feature::kPresets))
    {
        ReturnErrorOnFailure(builder.ReferenceExisting(kSetActivePresetRequest));
    }
    if (mFeatures.Has(Feature::kThermostatSuggestions))
    {
        ReturnErrorOnFailure(builder.ReferenceExisting(kSuggestionCommands));
    }
    if (mFeatures.Has(Feature::kPresets) || mFeatures.Has(Feature::kMatterScheduleConfiguration))
    {
        ReturnErrorOnFailure(builder.ReferenceExisting(kAtomicRequest));
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR ThermostatCluster::GeneratedCommands(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<CommandId> & builder)
{
    static constexpr CommandId kGetWeeklyScheduleResponse[]      = { Commands::GetWeeklyScheduleResponse::Id };
    static constexpr CommandId kAddThermostatSuggestionResponse[] = { Commands::AddThermostatSuggestionResponse::Id };
    static constexpr CommandId kAtomicResponse[]                 = { Commands::AtomicResponse::Id };

    if (mFeatures.Has(Feature::kScheduleConfiguration))
    {
        ReturnErrorOnFailure(builder.ReferenceExisting(kGetWeeklyScheduleResponse));
    }
    if (mFeatures.Has(Feature::kThermostatSuggestions))
    {
        ReturnErrorOnFailure(builder.ReferenceExisting(kAddThermostatSuggestionResponse));
    }
    if (mFeatures.Has(Feature::kPresets) || mFeatures.Has(Feature::kMatterScheduleConfiguration))
    {
        ReturnErrorOnFailure(builder.ReferenceExisting(kAtomicResponse));
    }
    return CHIP_NO_ERROR;
}

} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip
