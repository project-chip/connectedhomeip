/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include <EnergyEvseDelegateImpl.h>
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-objects.h>

using namespace chip;
using chip::Protocols::InteractionModel::Status;

using namespace chip::app;
using namespace chip::app::DataModel;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::EnergyEvse;
using namespace chip::app::Clusters::EnergyEvse::Attributes;

/**
 * @brief   Called when EVSE cluster receives Disable command
 */
Status EnergyEvseDelegate::Disable()
{
    ChipLogProgress(AppServer, "EnergyEvseDelegate::Disable()");

    /* update State */
    switch (mHwState)
    {
    case StateEnum::kNotPluggedIn:
        SetState(StateEnum::kNotPluggedIn);
        break;

    case StateEnum::kPluggedInNoDemand:
        SetState(StateEnum::kPluggedInNoDemand);
        break;

    case StateEnum::kPluggedInDemand:
        SetState(StateEnum::kPluggedInDemand);
        break;

    default:
        ChipLogError(AppServer, "Unexpected EVSE hardware state\n");
        SetState(StateEnum::kFault);
        break;
    }

    /* update SupplyState */
    SetSupplyState(SupplyStateEnum::kDisabled);

    /* update ChargingEnabledUntil & DischargingEnabledUntil to show 0 */
    SetChargingEnabledUntil(0);
    SetDischargingEnabledUntil(0);

    /* update MinimumChargeCurrent & MaximumChargeCurrent to 0 */
    SetMinimumChargeCurrent(0);
    SetMaximumChargeCurrent(0);

    /* update MaximumDischargeCurrent to 0 */
    SetMaximumDischargeCurrent(0);

    // TODO: Generate events

    return Status::Success;
}

/**
 * @brief   Called when EVSE cluster receives EnableCharging command
 *
 * @param chargingEnabledUntil (can be null to indefinite charging)
 * @param minimumChargeCurrent (in mA)
 * @param maximumChargeCurrent (in mA)
 */
Status EnergyEvseDelegate::EnableCharging(const chip::app::DataModel::Nullable<uint32_t> & chargingEnabledUntil,
                                          const int64_t & minimumChargeCurrent, const int64_t & maximumChargeCurrent)
{
    ChipLogProgress(AppServer, "EnergyEvseDelegate::EnableCharging()");

    if (maximumChargeCurrent < kMinimumChargeCurrent || maximumChargeCurrent > kMaximumChargeCurrent)
    {
        ChipLogError(NotSpecified, "Maximum Current outside limits");
        return Status::ConstraintError;
    }

    if (minimumChargeCurrent < kMinimumChargeCurrent || minimumChargeCurrent > kMaximumChargeCurrent)
    {
        ChipLogError(NotSpecified, "Maximum Current outside limits");
        return Status::ConstraintError;
    }

    if (minimumChargeCurrent > maximumChargeCurrent)
    {
        ChipLogError(NotSpecified, "Minium Current > Maximum Current!");
        return Status::ConstraintError;
    }

    if (chargingEnabledUntil.IsNull())
    {
        /* Charging enabled indefinitely */
        ChipLogError(NotSpecified, "Charging enabled indefinitely");
    }
    else
    {
        /* check chargingEnabledUntil is in the future */
        ChipLogError(NotSpecified, "Charging enabled until: %lu", static_cast<long unsigned int>(chargingEnabledUntil.Value()));
        // TODO
        // if (checkChargingEnabled)
    }

    /* Check current state isn't already enabled */

    /* If charging is already enabled, check that the parameters may have
       changed, these may override an existing charging command */
    switch (mHwState)
    {
    case StateEnum::kNotPluggedIn:
        SetState(StateEnum::kNotPluggedIn);
        break;

    case StateEnum::kPluggedInNoDemand:
        SetState(StateEnum::kPluggedInNoDemand);
        break;

    case StateEnum::kPluggedInDemand:
        /* If the EVSE is asking for demand then enable charging */
        SetState(StateEnum::kPluggedInCharging);
        break;

    default:
        ChipLogError(AppServer, "Unexpected EVSE hardware state\n");
        SetState(StateEnum::kFault);
        break;
    }

    /* update SupplyState */
    SetSupplyState(SupplyStateEnum::kChargingEnabled);

    /* If it looks ok, store the min & max charging current */
    mMaximumChargingCurrentLimitFromCommand = maximumChargeCurrent;
    SetMinimumChargeCurrent(minimumChargeCurrent);
    // TODO persist these to KVS

    // TODO: Generate events

    return this->ComputeMaxChargeCurrentLimit();
}

/**
 * @brief   Called when EVSE cluster receives EnableDischarging command
 *
 * @param dischargingEnabledUntil (can be null to indefinite discharging)
 * @param maximumChargeCurrent (in mA)
 */
Status EnergyEvseDelegate::EnableDischarging(const chip::app::DataModel::Nullable<uint32_t> & dischargingEnabledUntil,
                                             const int64_t & maximumDischargeCurrent)
{
    ChipLogProgress(AppServer, "EnergyEvseDelegate::EnableDischarging() called.");

    /* update SupplyState */
    SetSupplyState(SupplyStateEnum::kDischargingEnabled);

    // TODO: Generate events

    return Status::Success;
}

/**
 * @brief    Called when EVSE cluster receives StartDiagnostics command
 */
Status EnergyEvseDelegate::StartDiagnostics()
{
    /* For EVSE manufacturers to customize */
    ChipLogProgress(AppServer, "EnergyEvseDelegate::StartDiagnostics()");

    /* update SupplyState */
    SetSupplyState(SupplyStateEnum::kDisabledDiagnostics);

    // TODO: Generate events

    return Status::Success;
}

/* ---------------------------------------------------------------------------
 * FUNCTIONS BELOW:
 *    - EVSE Hardware interface
 *
 *  RegisterEvseHardwareCallback( callbackType, callbackFnc )
 *  SetMaxHardwareCurrentLimit( currentmA )
 *  SetCircuitCapacity( currentmA )
 *  SetCableAssemblyLimit( currentmA )
 *  SetState( EVSEStateEnum )
 *  SetFault
 *
 */

/**
 * @brief    Called by EVSE Hardware to register a callback
 *
 * @param    Callback function
 */
#if 0
Status EnergyEvseDelegate::HwRegisterEvseHardwareCallback(int Callback) // TODO
{
    // TODO
    return CHIP_NO_ERROR;
}
#endif
/**
 * @brief    Called by EVSE Hardware to notify the delegate of the maximum
 *           current limit supported by the hardware.
 *
 *           This is normally called at start-up.
 *
 * @param    currentmA
 */
Status EnergyEvseDelegate::HwSetMaxHardwareCurrentLimit(int64_t currentmA)
{
    if (currentmA < kMinimumChargeCurrent || currentmA > kMaximumChargeCurrent)
    {
        return Status::ConstraintError;
    }

    /* there is no attribute to store this so store in private variable */
    mMaxHardwareCurrentLimit = currentmA;

    return this->ComputeMaxChargeCurrentLimit();
}

/**
 * @brief    Called by EVSE Hardware to notify the delegate of maximum electrician
 *           set current limit.
 *
 *           This is normally called at start-up when reading from DIP-switch
 *           settings.
 *
 * @param    currentmA
 */
Status EnergyEvseDelegate::HwSetCircuitCapacity(int64_t currentmA)
{
    if (currentmA < kMinimumChargeCurrent || currentmA > kMaximumChargeCurrent)
    {
        return Status::ConstraintError;
    }

    mCircuitCapacity = currentmA;
    MatterReportingAttributeChangeCallback(mEndpointId, EnergyEvse::Id, CircuitCapacity::Id);

    return this->ComputeMaxChargeCurrentLimit();
}

/**
 * @brief    Called by EVSE Hardware to notify the delegate of the cable assembly
 *           current limit.
 *
 *           This is normally called when the EV is plugged into the EVSE and the
 *           PP voltage is measured by the EVSE. A pull-up resistor in the cable
 *           causes a voltage drop. Different current limits can be indicated
 *           using different resistors, which results in different voltages
 *           measured by the EVSE.
 *
 * @param    currentmA
 */
Status EnergyEvseDelegate::HwSetCableAssemblyLimit(int64_t currentmA)
{
    if (currentmA < kMinimumChargeCurrent || currentmA > kMaximumChargeCurrent)
    {
        return Status::ConstraintError;
    }

    /* there is no attribute to store this so store in private variable */
    mCableAssemblyCurrentLimit = currentmA;

    return this->ComputeMaxChargeCurrentLimit();
}

/**
 * @brief    Called by EVSE Hardware to indicate a fault
 *
 * @param    StateEnum
 */
Status EnergyEvseDelegate::HwSetState(StateEnum state)
{
    /* the only allowed states that the EVSE hardware can set are:
     *  kNotPluggedIn
     *  kPluggedInNoDemand
     *  kPluggedInDemand
     */
    switch (state)
    {
    case StateEnum::kNotPluggedIn:
        // TODO - work out logic here
        mHwState = state;
        break;
    case StateEnum::kPluggedInNoDemand:
        // TODO - work out logic here
        mHwState = state;
        break;
    case StateEnum::kPluggedInDemand:
        // TODO - work out logic here
        mHwState = state;
        break;

    default:
        /* All other states should be managed by the Delegate */
        // TODO (assert?)
        break;
    }

    return Status::Success;
}

/**
 * @brief    Called by EVSE Hardware to indicate a fault
 *
 * @param    FaultStateEnum
 */
Status EnergyEvseDelegate::HwSetFault(FaultStateEnum fault)
{
    ChipLogProgress(AppServer, "EnergyEvseDelegate::Fault()");

    if (fault == FaultStateEnum::kNoError)
    {
        /* Update State to previous state */
        // TODO: need to work out the logic here!

        /* Update SupplyState to previous state */
    }
    else
    {
        /* Update State & SupplyState */
        SetState(StateEnum::kFault);
        SetSupplyState(SupplyStateEnum::kDisabledError);
    }

    /* Update FaultState */
    SetFaultState(fault);

    // TODO: Generate events

    return Status::Success;
}

Status EnergyEvseDelegate::HwSetVehicleID(chip::CharSpan newValue)
{
    DataModel::Nullable<chip::CharSpan> oldValue = mVehicleID;

    mVehicleID = MakeNullable(newValue);
    if ((oldValue.IsNull()) || (strcmp(mVehicleID.Value().data(), oldValue.Value().data())))
    {
        ChipLogDetail(AppServer, "VehicleID updated to %s", mVehicleID.Value().data());
        MatterReportingAttributeChangeCallback(mEndpointId, EnergyEvse::Id, VehicleID::Id);
    }

    return Status::Success;
}

/* ---------------------------------------------------------------------------
 * Functions below are private helper functions internal to the delegate
 */

/**
 *  @brief   Called to compute the safe charging current limit
 */
Status EnergyEvseDelegate::ComputeMaxChargeCurrentLimit()
{
    int64_t oldValue;
    /* mActualChargingCurrentLimit is the minimum of:
     *   - MaxHardwareCurrentLimit (of the hardware)
     *   - CircuitCapacity (set by the electrician - less than the hardware)
     *   - CableAssemblyLimit (detected when the cable is inserted)
     *   - MaximumChargeCurrent (from charging command)
     *   - UserMaximumChargeCurrent (could dynamically change)
     */

    oldValue                    = mActualChargingCurrentLimit;
    mActualChargingCurrentLimit = mMaxHardwareCurrentLimit;
    mActualChargingCurrentLimit = min(mActualChargingCurrentLimit, mCircuitCapacity);
    mActualChargingCurrentLimit = min(mActualChargingCurrentLimit, mCableAssemblyCurrentLimit);
    mActualChargingCurrentLimit = min(mActualChargingCurrentLimit, mMaximumChargingCurrentLimitFromCommand);
    mActualChargingCurrentLimit = min(mActualChargingCurrentLimit, mUserMaximumChargeCurrent);

    /* Set the actual max charging current attribute */
    mMaximumChargeCurrent = mActualChargingCurrentLimit;

    if (oldValue != mMaximumChargeCurrent)
    {
        ChipLogDetail(AppServer, "MaximumChargeCurrent updated to %ld", static_cast<long>(mMaximumChargeCurrent));
        MatterReportingAttributeChangeCallback(mEndpointId, EnergyEvse::Id, MaximumChargeCurrent::Id);

        /* Call the EV Charger hardware current limit callback */
        // TODO
    }
    return Status::Success;
}

/**
 * Attribute methods
 */
/* State */
StateEnum EnergyEvseDelegate::GetState()
{
    return mState;
}
CHIP_ERROR EnergyEvseDelegate::SetState(StateEnum newValue)
{
    StateEnum oldValue = mState;
    if (newValue >= StateEnum::kUnknownEnumValue)
    {
        return CHIP_IM_GLOBAL_STATUS(ConstraintError);
    }

    mState = newValue;
    if (oldValue != mState)
    {
        ChipLogDetail(AppServer, "State updated to %d", (int) mState);
        MatterReportingAttributeChangeCallback(mEndpointId, EnergyEvse::Id, State::Id);
    }

    return CHIP_NO_ERROR;
}

/* SupplyState */
SupplyStateEnum EnergyEvseDelegate::GetSupplyState()
{
    return mSupplyState;
}
CHIP_ERROR EnergyEvseDelegate::SetSupplyState(SupplyStateEnum newValue)
{
    SupplyStateEnum oldValue = mSupplyState;

    if (newValue >= SupplyStateEnum::kUnknownEnumValue)
    {
        return CHIP_IM_GLOBAL_STATUS(ConstraintError);
    }

    mSupplyState = newValue;
    if (oldValue != mSupplyState)
    {
        ChipLogDetail(AppServer, "SupplyState updated to %d", (int) mSupplyState);
        MatterReportingAttributeChangeCallback(mEndpointId, EnergyEvse::Id, SupplyState::Id);
    }
    return CHIP_NO_ERROR;
}

/* FaultState */
FaultStateEnum EnergyEvseDelegate::GetFaultState()
{
    return mFaultState;
}
CHIP_ERROR EnergyEvseDelegate::SetFaultState(FaultStateEnum newValue)
{
    FaultStateEnum oldValue = mFaultState;

    if (newValue >= FaultStateEnum::kUnknownEnumValue)
    {
        return CHIP_IM_GLOBAL_STATUS(ConstraintError);
    }

    mFaultState = newValue;
    if (oldValue != mFaultState)
    {
        ChipLogDetail(AppServer, "FaultState updated to %d", (int) mFaultState);
        MatterReportingAttributeChangeCallback(mEndpointId, EnergyEvse::Id, FaultState::Id);
    }
    return CHIP_NO_ERROR;
}

/* ChargingEnabledUntil */
DataModel::Nullable<uint32_t> EnergyEvseDelegate::GetChargingEnabledUntil()
{
    return mChargingEnabledUntil;
}
CHIP_ERROR EnergyEvseDelegate::SetChargingEnabledUntil(uint32_t newValue)
{
    DataModel::Nullable<uint32_t> oldValue = mChargingEnabledUntil;

    mChargingEnabledUntil = MakeNullable(newValue);
    if ((oldValue.IsNull()) || (oldValue.Value() != newValue))
    {
        ChipLogDetail(AppServer, "ChargingEnabledUntil updated to %lu",
                      static_cast<unsigned long int>(mChargingEnabledUntil.Value()));
        MatterReportingAttributeChangeCallback(mEndpointId, EnergyEvse::Id, ChargingEnabledUntil::Id);
    }
    return CHIP_NO_ERROR;
}
/* DischargingEnabledUntil */
DataModel::Nullable<uint32_t> EnergyEvseDelegate::GetDischargingEnabledUntil()
{
    return mDischargingEnabledUntil;
}
CHIP_ERROR EnergyEvseDelegate::SetDischargingEnabledUntil(uint32_t newValue)
{
    DataModel::Nullable<uint32_t> oldValue = mDischargingEnabledUntil;

    mDischargingEnabledUntil = MakeNullable(newValue);
    if ((oldValue.IsNull()) || (oldValue.Value() != newValue))
    {
        ChipLogDetail(AppServer, "DischargingEnabledUntil updated to %lu",
                      static_cast<unsigned long int>(mDischargingEnabledUntil.Value()));
        MatterReportingAttributeChangeCallback(mEndpointId, EnergyEvse::Id, DischargingEnabledUntil::Id);
    }
    return CHIP_NO_ERROR;
}
/* CircuitCapacity */
int64_t EnergyEvseDelegate::GetCircuitCapacity()
{
    return mCircuitCapacity;
}
CHIP_ERROR EnergyEvseDelegate::SetCircuitCapacity(int64_t newValue)
{
    int64_t oldValue = mCircuitCapacity;

    if (newValue >= kMaximumChargeCurrent)
    {
        return CHIP_IM_GLOBAL_STATUS(ConstraintError);
    }

    mCircuitCapacity = newValue;
    if (oldValue != mCircuitCapacity)
    {
        ChipLogDetail(AppServer, "CircuitCapacity updated to %ld", static_cast<long>(mCircuitCapacity));
        MatterReportingAttributeChangeCallback(mEndpointId, EnergyEvse::Id, CircuitCapacity::Id);
    }
    return CHIP_NO_ERROR;
}
/* MinimumChargeCurrent */
int64_t EnergyEvseDelegate::GetMinimumChargeCurrent()
{
    return mMinimumChargeCurrent;
}
CHIP_ERROR EnergyEvseDelegate::SetMinimumChargeCurrent(int64_t newValue)
{
    int64_t oldValue = mMinimumChargeCurrent;

    if (newValue >= kMaximumChargeCurrent)
    {
        return CHIP_IM_GLOBAL_STATUS(ConstraintError);
    }

    mMinimumChargeCurrent = newValue;
    if (oldValue != mMinimumChargeCurrent)
    {
        ChipLogDetail(AppServer, "MinimumChargeCurrent updated to %ld", static_cast<long>(mMinimumChargeCurrent));
        MatterReportingAttributeChangeCallback(mEndpointId, EnergyEvse::Id, MinimumChargeCurrent::Id);
    }
    return CHIP_NO_ERROR;
}

/* MaximumChargeCurrent */
int64_t EnergyEvseDelegate::GetMaximumChargeCurrent()
{
    return mMaximumChargeCurrent;
}
CHIP_ERROR EnergyEvseDelegate::SetMaximumChargeCurrent(int64_t newValue)
{
    int64_t oldValue = mMaximumChargeCurrent;

    if (newValue >= kMaximumChargeCurrent)
    {
        return CHIP_IM_GLOBAL_STATUS(ConstraintError);
    }

    mMaximumChargeCurrent = newValue;
    if (oldValue != mMaximumChargeCurrent)
    {
        ChipLogDetail(AppServer, "MaximumChargeCurrent updated to %ld", static_cast<long>(mMaximumChargeCurrent));
        MatterReportingAttributeChangeCallback(mEndpointId, EnergyEvse::Id, MaximumChargeCurrent::Id);
    }
    return CHIP_NO_ERROR;
}
/* MaximumDischargeCurrent */
int64_t EnergyEvseDelegate::GetMaximumDischargeCurrent()
{
    return mMaximumDischargeCurrent;
}
CHIP_ERROR EnergyEvseDelegate::SetMaximumDischargeCurrent(int64_t newValue)
{
    int64_t oldValue = mMaximumDischargeCurrent;

    if (newValue >= kMaximumChargeCurrent)
    {
        return CHIP_IM_GLOBAL_STATUS(ConstraintError);
    }

    mMaximumDischargeCurrent = newValue;
    if (oldValue != mMaximumDischargeCurrent)
    {
        ChipLogDetail(AppServer, "MaximumDischargeCurrent updated to %ld", static_cast<long>(mMaximumDischargeCurrent));
        MatterReportingAttributeChangeCallback(mEndpointId, EnergyEvse::Id, MaximumDischargeCurrent::Id);
    }
    return CHIP_NO_ERROR;
}
/* UserMaximumChargeCurrent */
int64_t EnergyEvseDelegate::GetUserMaximumChargeCurrent()
{
    return mUserMaximumChargeCurrent;
}
CHIP_ERROR EnergyEvseDelegate::SetUserMaximumChargeCurrent(int64_t newValue)
{
    if ((newValue < 0) || (newValue > kMaximumChargeCurrent))
    {
        return CHIP_IM_GLOBAL_STATUS(ConstraintError);
    }

    int64_t oldValue          = mUserMaximumChargeCurrent;
    mUserMaximumChargeCurrent = newValue;
    if (oldValue != newValue)
    {
        ChipLogDetail(AppServer, "UserMaximumChargeCurrent updated to %ld", static_cast<long>(mUserMaximumChargeCurrent));
        MatterReportingAttributeChangeCallback(mEndpointId, EnergyEvse::Id, UserMaximumChargeCurrent::Id);
    }

    return CHIP_NO_ERROR;
}
/* RandomizationDelayWindow */
uint32_t EnergyEvseDelegate::GetRandomizationDelayWindow()
{
    return mRandomizationDelayWindow;
}
CHIP_ERROR EnergyEvseDelegate::SetRandomizationDelayWindow(uint32_t newValue)
{
    uint32_t oldValue = mRandomizationDelayWindow;
    if (newValue > kMaxRandomizationDelayWindow)
    {
        return CHIP_IM_GLOBAL_STATUS(ConstraintError);
    }

    mRandomizationDelayWindow = newValue;
    if (oldValue != newValue)
    {
        ChipLogDetail(AppServer, "RandomizationDelayWindow updated to %lu",
                      static_cast<unsigned long int>(mRandomizationDelayWindow));
        MatterReportingAttributeChangeCallback(mEndpointId, EnergyEvse::Id, RandomizationDelayWindow::Id);
    }
    return CHIP_NO_ERROR;
}

/* PREF attributes */
uint8_t EnergyEvseDelegate::GetNumberOfWeeklyTargets()
{
    return mNumberOfWeeklyTargets;
}
uint8_t EnergyEvseDelegate::GetNumberOfDailyTargets()
{
    return mNumberOfDailyTargets;
}
DataModel::Nullable<uint32_t> EnergyEvseDelegate::GetNextChargeStartTime()
{
    return mNextChargeStartTime;
}
DataModel::Nullable<uint32_t> EnergyEvseDelegate::GetNextChargeTargetTime()
{
    return mNextChargeTargetTime;
}
DataModel::Nullable<int64_t> EnergyEvseDelegate::GetNextChargeRequiredEnergy()
{
    return mNextChargeRequiredEnergy;
}
DataModel::Nullable<chip::Percent> EnergyEvseDelegate::GetNextChargeTargetSoC()
{
    return mNextChargeTargetSoC;
}

/* ApproximateEVEfficiency */
DataModel::Nullable<uint16_t> EnergyEvseDelegate::GetApproximateEVEfficiency()
{
    return mApproximateEVEfficiency;
}
CHIP_ERROR EnergyEvseDelegate::SetApproximateEVEfficiency(uint16_t newValue)
{
    DataModel::Nullable<uint16_t> oldValue = mApproximateEVEfficiency;

    mApproximateEVEfficiency = MakeNullable(newValue);
    if ((oldValue.IsNull()) || (oldValue.Value() != newValue))
    {
        ChipLogDetail(AppServer, "ApproximateEVEfficiency updated to %d", mApproximateEVEfficiency.Value());
        MatterReportingAttributeChangeCallback(mEndpointId, EnergyEvse::Id, ApproximateEVEfficiency::Id);
    }

    return CHIP_NO_ERROR;
}

/* SOC attributes */
DataModel::Nullable<chip::Percent> EnergyEvseDelegate::GetStateOfCharge()
{
    return mStateOfCharge;
}
DataModel::Nullable<int64_t> EnergyEvseDelegate::GetBatteryCapacity()
{
    return mBatteryCapacity;
}

/* PNC attributes*/
DataModel::Nullable<chip::CharSpan> EnergyEvseDelegate::GetVehicleID()
{
    return mVehicleID;
}

/* Session SESS attributes */
DataModel::Nullable<uint32_t> EnergyEvseDelegate::GetSessionID()
{
    return mSessionID;
}
DataModel::Nullable<uint32_t> EnergyEvseDelegate::GetSessionDuration()
{
    return mSessionDuration;
}
DataModel::Nullable<int64_t> EnergyEvseDelegate::GetSessionEnergyCharged()
{
    return mSessionEnergyCharged;
}
DataModel::Nullable<int64_t> EnergyEvseDelegate::GetSessionEnergyDischarged()
{
    return mSessionEnergyDischarged;
}
