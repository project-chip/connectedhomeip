/*
 *
 *    Copyright (c) 2023-2024 Project CHIP Authors
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
#include <app/EventLogging.h>
#include <app/SafeAttributePersistenceProvider.h>
#include <app/clusters/energy-evse-server/CodegenIntegration.h>
#include <app/reporting/reporting.h>
#include <platform/CHIPDeviceLayer.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::DataModel;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::EnergyEvse;
using namespace chip::app::Clusters::EnergyEvse::Attributes;

using chip::app::LogEvent;
using chip::Protocols::InteractionModel::Status;

/**
 * @brief   Called when EVSE cluster receives Disable command
 */
Status EnergyEvseDelegate::Disable()
{
    ChipLogProgress(AppServer, "EnergyEvseDelegate::Disable()");

    VerifyOrReturnValue(mInstance != nullptr, Status::Failure);

    DataModel::Nullable<uint32_t> disableTime(0);
    /* update ChargingEnabledUntil & DischargingEnabledUntil to show 0 */
    TEMPORARY_RETURN_IGNORED mInstance->SetChargingEnabledUntil(disableTime);
    TEMPORARY_RETURN_IGNORED mInstance->SetDischargingEnabledUntil(disableTime);

    /* update MinimumChargeCurrent & MaximumChargeCurrent to 0 */
    TEMPORARY_RETURN_IGNORED mInstance->SetMinimumChargeCurrent(0);

    mMaximumChargingCurrentLimitFromCommand = 0;
    ComputeMaxChargeCurrentLimit();

    /* update MaximumDischargeCurrent to 0 */
    mMaximumDischargingCurrentLimitFromCommand = 0;
    ComputeMaxDischargeCurrentLimit();

    return HandleStateMachineEvent(EVSEStateMachineEvent::DisabledEvent);
}

/**
 * @brief   Called when EVSE cluster receives EnableCharging command
 *
 * @param chargingEnabledUntil (can be null to indefinite charging)
 * @param minimumChargeCurrent (in mA)
 * @param maximumChargeCurrent (in mA)
 */
Status EnergyEvseDelegate::EnableCharging(const DataModel::Nullable<uint32_t> & chargingEnabledUntil,
                                          const int64_t & minimumChargeCurrent, const int64_t & maximumChargeCurrent)
{
    ChipLogProgress(AppServer, "EnergyEvseDelegate::EnableCharging()");

    if (maximumChargeCurrent < kMinimumChargeCurrentLimit)
    {
        ChipLogError(AppServer, "Maximum Current outside limits");
        return Status::ConstraintError;
    }

    if (minimumChargeCurrent < kMinimumChargeCurrentLimit)
    {
        ChipLogError(AppServer, "Minimum Current outside limits");
        return Status::ConstraintError;
    }

    if (minimumChargeCurrent > maximumChargeCurrent)
    {
        ChipLogError(AppServer, "Minium Current > Maximum Current!");
        return Status::ConstraintError;
    }

    VerifyOrReturnValue(mInstance != nullptr, Status::Failure);

    if (chargingEnabledUntil.IsNull())
    {
        /* Charging enabled indefinitely */
        ChipLogProgress(AppServer, "Charging enabled indefinitely");
    }
    else
    {
        /* check chargingEnabledUntil is in the future */
        ChipLogProgress(AppServer, "Charging enabled until: %lu", static_cast<long unsigned int>(chargingEnabledUntil.Value()));
    }
    TEMPORARY_RETURN_IGNORED mInstance->SetChargingEnabledUntil(chargingEnabledUntil);

    /* If it looks ok, store the min & max charging current */
    mMaximumChargingCurrentLimitFromCommand = maximumChargeCurrent;
    TEMPORARY_RETURN_IGNORED mInstance->SetMinimumChargeCurrent(minimumChargeCurrent);
    // TODO persist these to KVS

    ComputeMaxChargeCurrentLimit();

    return HandleStateMachineEvent(EVSEStateMachineEvent::ChargingEnabledEvent);
}

/**
 * @brief   Called when EVSE cluster receives EnableDischarging command
 *
 * @param dischargingEnabledUntil (can be null to indefinite discharging)
 * @param maximumDischargeCurrent (in mA)
 */
Status EnergyEvseDelegate::EnableDischarging(const DataModel::Nullable<uint32_t> & dischargingEnabledUntil,
                                             const int64_t & maximumDischargeCurrent)
{
    ChipLogProgress(AppServer, "EnergyEvseDelegate::EnableDischarging() called.");

    if (maximumDischargeCurrent < kMinimumChargeCurrentLimit)
    {
        ChipLogError(AppServer, "Maximum Discharging Current outside limits - cannot be negative");
        return Status::ConstraintError;
    }

    VerifyOrReturnValue(mInstance != nullptr, Status::Failure);

    if (dischargingEnabledUntil.IsNull())
    {
        /* Discharging enabled indefinitely */
        ChipLogProgress(AppServer, "Discharging enabled indefinitely");
    }
    else
    {
        ChipLogProgress(AppServer, "Discharging enabled until: %lu",
                        static_cast<long unsigned int>(dischargingEnabledUntil.Value()));
    }
    TEMPORARY_RETURN_IGNORED mInstance->SetDischargingEnabledUntil(dischargingEnabledUntil);

    /* If it looks ok, store the max discharging current */
    mMaximumDischargingCurrentLimitFromCommand = maximumDischargeCurrent;
    ComputeMaxDischargeCurrentLimit();

    // TODO persist these to KVS

    return HandleStateMachineEvent(EVSEStateMachineEvent::DischargingEnabledEvent);
}

/**
 * @brief    Helper function to get the earliest time from two Nullable<uint32_t> values
 *
 * If either time is null, it returns the other time. If both are non-null, it returns the earlier one.
 */
static DataModel::Nullable<uint32_t> GetEarliestTime(const DataModel::Nullable<uint32_t> & time1,
                                                     const DataModel::Nullable<uint32_t> & time2)
{
    if (time1.IsNull())
        return time2;
    if (time2.IsNull())
        return time1;
    return (time1.Value() < time2.Value()) ? time1 : time2;
}

/**
 * @brief    Helper function to check if a time value has expired
 *
 * @param timeValue The Nullable<uint32_t> time value to check
 * @param currentTime The current time in seconds since epoch
 * @return true if the time has expired, false otherwise
 */
static bool IsTimeExpired(const DataModel::Nullable<uint32_t> & timeValue, uint32_t currentTime)
{
    return !timeValue.IsNull() && (timeValue.Value() <= currentTime);
}
/**
 * @brief    Helper function to handle timer expiration when in enabled state
 *
 * @param matterEpochSeconds Current time in Matter epoch seconds
 * This function is called when the EVSE is in an enabled state
 * (either charging or discharging) and the timer expires.
 * It checks if the charging or discharging enabled times have expired
 * and updates the EVSE state accordingly.
 * If both charging and discharging have expired or are Zero, it disables the EVSE.
 * If only one has expired, it updates the state to the other enabled state.
 * If both are still valid, it does nothing.
 */
void EnergyEvseDelegate::HandleEnabledStateExpiration(uint32_t matterEpochSeconds)
{
    if (mInstance == nullptr)
    {
        return;
    }

    DataModel::Nullable<uint32_t> chargingEnabledUntil    = GetChargingEnabledUntil();
    DataModel::Nullable<uint32_t> dischargingEnabledUntil = GetDischargingEnabledUntil();

    bool chargingExpired    = IsTimeExpired(chargingEnabledUntil, matterEpochSeconds);
    bool dischargingExpired = IsTimeExpired(dischargingEnabledUntil, matterEpochSeconds);

    if (chargingExpired)
    {
        // set to zero to indicate disabled
        TEMPORARY_RETURN_IGNORED mInstance->SetChargingEnabledUntil(DataModel::Nullable<uint32_t>(0));

        // update MinimumChargeCurrent & MaximumChargeCurrent to 0
        TEMPORARY_RETURN_IGNORED mInstance->SetMinimumChargeCurrent(0);

        mMaximumChargingCurrentLimitFromCommand = 0;
        ComputeMaxChargeCurrentLimit();

        // Change to discharging-only if discharging is still enabled
        if (!dischargingExpired)
        {
            TEMPORARY_RETURN_IGNORED mInstance->SetSupplyState(SupplyStateEnum::kDischargingEnabled);
        }
        else
        {
            // If both charging and discharging have expired, disable the EVSE
            Disable();
        }
    }

    if (dischargingExpired)
    {
        // set to zero to indicate disabled
        TEMPORARY_RETURN_IGNORED mInstance->SetDischargingEnabledUntil(DataModel::Nullable<uint32_t>(0));

        // update MaximumDischargeCurrent to 0
        mMaximumDischargingCurrentLimitFromCommand = 0;
        ComputeMaxDischargeCurrentLimit();

        // Change to charging-only if charging is still enabled
        if (!chargingExpired)
        {
            TEMPORARY_RETURN_IGNORED mInstance->SetSupplyState(SupplyStateEnum::kChargingEnabled);
        }
        else
        {
            // If both charging and discharging have expired, disable the EVSE
            Disable();
        }
    }
}

/**
 * @brief    Routine to help schedule a timer callback to check if the EVSE should go disabled
 *
 * If the clock is sync'd we can work out when to call back to check when to disable the EVSE
 * automatically. If the clock isn't sync'd the we just set a timer to check once every 30s.
 *
 * We first check the SupplyState to check if it is EnabledCharging or EnabledDischarging
 * Then if the EnabledCharging/DischargingUntil is not Null, then we compute a delay to come
 * back and check.
 */
Status EnergyEvseDelegate::ScheduleCheckOnEnabledTimeout()
{
    // Determine the relevant timeout based on current supply state
    DataModel::Nullable<uint32_t> enabledUntilTime;
    SupplyStateEnum currentSupplyState = GetSupplyState();
    switch (currentSupplyState)
    {
    case SupplyStateEnum::kChargingEnabled:
        enabledUntilTime = GetChargingEnabledUntil();
        break;
    case SupplyStateEnum::kDischargingEnabled:
        enabledUntilTime = GetDischargingEnabledUntil();
        break;
    case SupplyStateEnum::kEnabled:
        // For enabled state, use the earliest of charging or discharging timeout
        enabledUntilTime = GetEarliestTime(GetChargingEnabledUntil(), GetDischargingEnabledUntil());
        break;
    default:
        // In all other states the EVSE is disabled, no timer needed
        return Status::Success;
    }

    if (enabledUntilTime.IsNull())
    {
        ChipLogDetail(AppServer, "EVSE is enabled indefinitely, no timer needed");
        return Status::Success;
    }

    uint32_t matterEpochSeconds = 0;
    CHIP_ERROR err              = System::Clock::GetClock_MatterEpochS(matterEpochSeconds);
    if (err == CHIP_ERROR_REAL_TIME_NOT_SYNCED)
    {
        // Real time isn't sync'd - check again in 30 seconds
        TEMPORARY_RETURN_IGNORED DeviceLayer::SystemLayer().StartTimer(
            System::Clock::Seconds32(kPeriodicCheckIntervalRealTimeClockNotSynced_sec), EvseCheckTimerExpiry, this);
        return Status::Success;
    }

    if (err != CHIP_NO_ERROR)
    {
        return Status::Failure; // Can't get time, skip scheduling
    }

    if (enabledUntilTime.Value() > matterEpochSeconds)
    {
        // Timer hasn't expired yet - schedule future check
        uint32_t delta = enabledUntilTime.Value() - matterEpochSeconds;
        ChipLogDetail(AppServer, "Setting EVSE Enable check timer for %lu seconds", static_cast<unsigned long>(delta));
        TEMPORARY_RETURN_IGNORED DeviceLayer::SystemLayer().StartTimer(System::Clock::Seconds32(delta), EvseCheckTimerExpiry, this);
        return Status::Success;
    }

    // Time has expired - handle expiration based on current state
    ChipLogDetail(AppServer, "EVSE enable time expired, processing expiration");

    // Re-check supply state as it may have changed
    SupplyStateEnum currentState = GetSupplyState();
    if (currentState == SupplyStateEnum::kChargingEnabled || currentState == SupplyStateEnum::kDischargingEnabled)
    {
        Disable();
    }
    else if (currentState == SupplyStateEnum::kEnabled)
    {
        HandleEnabledStateExpiration(matterEpochSeconds);
        // Call ourselves again now that one of our 2 timers has expired
        // The other timer expiry may need to be scheduled now
        ScheduleCheckOnEnabledTimeout();
    }

    return Status::Success;
}

void EnergyEvseDelegate::CancelActiveTimers()
{
    // Cancel the EVSE check timer if it is active
    DeviceLayer::SystemLayer().CancelTimer(EvseCheckTimerExpiry, this);
}

void EnergyEvseDelegate::EvseCheckTimerExpiry(System::Layer * systemLayer, void * delegate)
{
    EnergyEvseDelegate * dg = reinterpret_cast<EnergyEvseDelegate *>(delegate);

    dg->ScheduleCheckOnEnabledTimeout();
}

/**
 * @brief    Called when EVSE cluster receives StartDiagnostics command
 *
 *  NOTE: Application code needs to call HwDiagnosticsComplete
 *  once diagnostics have been completed.
 */
Status EnergyEvseDelegate::StartDiagnostics()
{
    /* For EVSE manufacturers to customize */
    ChipLogProgress(AppServer, "EnergyEvseDelegate::StartDiagnostics()");

    if (GetSupplyState() != SupplyStateEnum::kDisabled)
    {
        ChipLogError(AppServer, "EVSE: cannot be put into diagnostics mode if it is not Disabled!");
        return Status::Failure;
    }

    VerifyOrReturnValue(mInstance != nullptr, Status::Failure);

    // Update the SupplyState - this will automatically callback the Application StateChanged callback
    TEMPORARY_RETURN_IGNORED mInstance->SetSupplyState(SupplyStateEnum::kDisabledDiagnostics);

    return Status::Success;
}

/**
 * @brief    Called when EVSE cluster receives SetTargets command
 */
Status EnergyEvseDelegate::SetTargets(
    const DataModel::DecodableList<Structs::ChargingTargetScheduleStruct::DecodableType> & chargingTargetSchedules)
{
    ChipLogProgress(AppServer, "EnergyEvseDelegate::SetTargets()");

    EvseTargetsDelegate * targets = GetEvseTargetsDelegate();
    VerifyOrReturnError(targets != nullptr, Status::Failure);

    CHIP_ERROR err = targets->SetTargets(chargingTargetSchedules);
    if (err == CHIP_ERROR_NO_MEMORY)
    {
        return Status::ResourceExhausted;
    }
    VerifyOrReturnError(err == CHIP_NO_ERROR, StatusIB(err).mStatus);

    /* The Application needs to be told that the Targets have been updated
     * so it can potentially re-optimize the charging start time etc
     */
    NotifyApplicationChargingPreferencesChange();

    return Status::Success;
}

Status EnergyEvseDelegate::LoadTargets()
{
    ChipLogProgress(AppServer, "EnergyEvseDelegate::LoadTargets()");

    EvseTargetsDelegate * targets = GetEvseTargetsDelegate();
    VerifyOrReturnError(targets != nullptr, StatusIB(CHIP_ERROR_UNINITIALIZED).mStatus);

    CHIP_ERROR err = targets->LoadTargets();
    VerifyOrReturnError(err == CHIP_NO_ERROR, StatusIB(err).mStatus);

    return Status::Success;
}

Status EnergyEvseDelegate::GetTargets(DataModel::List<const Structs::ChargingTargetScheduleStruct::Type> & chargingTargetSchedules)
{
    ChipLogProgress(AppServer, "EnergyEvseDelegate::GetTargets()");

    EvseTargetsDelegate * targets = GetEvseTargetsDelegate();
    VerifyOrReturnError(targets != nullptr, StatusIB(CHIP_ERROR_UNINITIALIZED).mStatus);

    chargingTargetSchedules = targets->GetTargets();

    return Status::Success;
}

/**
 * @brief    Called when EVSE cluster receives ClearTargets command
 */
Status EnergyEvseDelegate::ClearTargets()
{
    ChipLogProgress(AppServer, "EnergyEvseDelegate::ClearTargets()");

    CHIP_ERROR err;

    EvseTargetsDelegate * targets = GetEvseTargetsDelegate();
    if (targets == nullptr)
    {
        return StatusIB(CHIP_ERROR_UNINITIALIZED).mStatus;
    }

    err = targets->ClearTargets();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Failed to clear Evse targets: %" CHIP_ERROR_FORMAT, err.Format());
        return Status::Failure;
    }

    /* The Application needs to be told that the Targets have been deleted
     * so it can potentially re-optimize the charging start time etc
     */
    NotifyApplicationChargingPreferencesChange();

    return Status::Success;
}

/* ---------------------------------------------------------------------------
 *  EVSE Hardware interface below
 */

/**
 * @brief    Called by EVSE Hardware to register a callback handler mechanism
 *
 *           This is normally called at start-up.
 *
 * @param    EVSECallbackFunct - function pointer to call
 * @param    intptr_t          - optional context to provide back to callback handler
 */
Status EnergyEvseDelegate::HwRegisterEvseCallbackHandler(EVSECallbackFunc handler, intptr_t arg)
{
    if (mCallbacks.handler != nullptr)
    {
        ChipLogError(AppServer, "Callback handler already initialized");
        return Status::Failure;
    }
    mCallbacks.handler = handler;
    mCallbacks.arg     = arg;

    return Status::Success;
}

/**
 * @brief    Called by EVSE Hardware to notify the delegate of the maximum
 *           current limit supported by the hardware (for charging).
 *
 *           This is normally called at start-up.
 *
 * @param    currentmA - Maximum current limit supported by the hardware
 */
Status EnergyEvseDelegate::HwSetMaxHardwareChargeCurrentLimit(int64_t currentmA)
{
    if (currentmA < kMinimumChargeCurrentLimit)
    {
        return Status::ConstraintError;
    }

    /* there is no attribute to store this so store in private variable */
    mMaxHardwareChargeCurrentLimit = currentmA;

    return ComputeMaxChargeCurrentLimit();
}

/**
 * @brief    Called by EVSE Hardware to notify the delegate of the maximum
 *           current limit supported by the hardware (for discharging).
 *
 *           This is normally called at start-up.
 *
 * @param    currentmA - Maximum current limit supported by the hardware for discharging
 */
Status EnergyEvseDelegate::HwSetMaxHardwareDischargeCurrentLimit(int64_t currentmA)
{
    if (currentmA < kMinimumChargeCurrentLimit)
    {
        return Status::ConstraintError;
    }

    /* there is no attribute to store this so store in private variable */
    mMaxHardwareDischargeCurrentLimit = currentmA;

    return ComputeMaxDischargeCurrentLimit();
}

/**
 * @brief    Called by EVSE Hardware to notify the delegate of the nominal
 *           mains voltage (in mV)
 *
 *           This is normally called at start-up.
 *
 * @param   voltage_mV - nominal mains voltage
 */
Status EnergyEvseDelegate::HwSetNominalMainsVoltage(int64_t voltage_mV)
{
    if (voltage_mV < kMinimumMainsVoltage_mV)
    {
        ChipLogError(AppServer, "Mains voltage looks too low - check value is in mV");
        return Status::ConstraintError;
    }

    mNominalMainsVoltage = voltage_mV;

    return Status::Success;
}

/**
 * @brief    Called by EVSE Hardware to notify the delegate of maximum electrician
 *           set current limit.
 *
 *           This is normally called at start-up when reading from DIP-switch
 *           settings.
 *
 * @param    currentmA - Maximum current limit specified by electrician
 */
Status EnergyEvseDelegate::HwSetCircuitCapacity(int64_t currentmA)
{
    if (currentmA < kMinimumChargeCurrentLimit)
    {
        return Status::ConstraintError;
    }

    VerifyOrReturnValue(mInstance != nullptr, Status::Failure);

    TEMPORARY_RETURN_IGNORED mInstance->SetCircuitCapacity(currentmA);

    return ComputeMaxChargeCurrentLimit();
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
 * @param    currentmA - Maximum current limit detected from Cable assembly
 */
Status EnergyEvseDelegate::HwSetCableAssemblyLimit(int64_t currentmA)
{
    if (currentmA < kMinimumChargeCurrentLimit)
    {
        return Status::ConstraintError;
    }

    /* there is no attribute to store this so store in private variable */
    mCableAssemblyCurrentLimit = currentmA;

    return ComputeMaxChargeCurrentLimit();
}

/**
 * @brief    Called by EVSE Hardware to indicate if EV is detected
 *
 * The only allowed states that the EVSE hardware can tell us about are:
 *  kNotPluggedIn
 *  kPluggedInNoDemand
 *  kPluggedInDemand
 *
 * The actual overall state is more complex and includes faults,
 * enable & disable charging or discharging etc.
 *
 * @param    StateEnum - the state of the EV being plugged in and asking for demand etc
 */
Status EnergyEvseDelegate::HwSetState(StateEnum newState)
{
    switch (newState)
    {
    case StateEnum::kNotPluggedIn:
        switch (mHwState)
        {
        case StateEnum::kNotPluggedIn:
            // No change
            break;
        case StateEnum::kPluggedInNoDemand:
        case StateEnum::kPluggedInDemand:
            /* EVSE has been unplugged now */
            mHwState = newState;
            HandleStateMachineEvent(EVSEStateMachineEvent::EVNotDetectedEvent);
            break;

        default:
            // invalid value for mHwState
            ChipLogError(AppServer, "HwSetState newstate(kNotPluggedIn) - Invalid value for mHwState");
            mHwState = newState; // set it to the new state anyway
            break;
        }
        break;

    case StateEnum::kPluggedInNoDemand:
        switch (mHwState)
        {
        case StateEnum::kNotPluggedIn:
            /* EV was unplugged, now is plugged in */
            mHwState = newState;
            HandleStateMachineEvent(EVSEStateMachineEvent::EVPluggedInEvent);
            break;
        case StateEnum::kPluggedInNoDemand:
            // No change
            break;
        case StateEnum::kPluggedInDemand:
            /* EV was plugged in and wanted demand, now doesn't want demand */
            mHwState = newState;
            HandleStateMachineEvent(EVSEStateMachineEvent::EVNoDemandEvent);
            break;
        default:
            // invalid value for mHwState
            ChipLogError(AppServer, "HwSetState newstate(kPluggedInNoDemand) - Invalid value for mHwState");
            mHwState = newState; // set it to the new state anyway
            break;
        }
        break;
    case StateEnum::kPluggedInDemand:
        switch (mHwState)
        {
        case StateEnum::kNotPluggedIn:
            /* EV was unplugged, now is plugged in and wants demand */
            mHwState = newState;
            HandleStateMachineEvent(EVSEStateMachineEvent::EVPluggedInEvent);
            HandleStateMachineEvent(EVSEStateMachineEvent::EVDemandEvent);
            break;
        case StateEnum::kPluggedInNoDemand:
            /* EV was plugged in and didn't want demand, now does want demand */
            mHwState = newState;
            HandleStateMachineEvent(EVSEStateMachineEvent::EVDemandEvent);
            break;
        case StateEnum::kPluggedInDemand:
            // No change
            break;
        default:
            // invalid value for mHwState
            ChipLogError(AppServer, "HwSetState newstate(kPluggedInDemand) - Invalid value for mHwState");
            mHwState = newState; // set it to the new state anyway
            break;
        }
        break;

    default:
        /* All other states should be managed by the Delegate */
        ChipLogError(AppServer, "HwSetState received invalid enum from caller");
        return Status::Failure;
    }

    return Status::Success;
}

/**
 * @brief    Called by EVSE Hardware to indicate a fault
 *
 * @param    FaultStateEnum - the fault condition detected
 */
Status EnergyEvseDelegate::HwSetFault(FaultStateEnum newFaultState)
{
    ChipLogProgress(AppServer, "EnergyEvseDelegate::Fault()");

    if (GetFaultState() == newFaultState)
    {
        ChipLogError(AppServer, "No change in fault state, ignoring call");
        return Status::Failure;
    }

    VerifyOrReturnValue(mInstance != nullptr, Status::Failure);

    /** Before we do anything we log the fault
     * any change in FaultState reports previous fault and new fault
     * and the state prior to the fault being raised */
    SendFaultEvent(newFaultState);

    /* Updated FaultState before we call into the handlers */
    TEMPORARY_RETURN_IGNORED mInstance->SetFaultState(newFaultState);

    if (newFaultState == FaultStateEnum::kNoError)
    {
        /* Fault has been cleared */
        HandleStateMachineEvent(EVSEStateMachineEvent::FaultCleared);
    }
    else
    {
        /* a new Fault has been raised */
        HandleStateMachineEvent(EVSEStateMachineEvent::FaultRaised);
    }

    return Status::Success;
}

/**
 * @brief    Called by EVSE Hardware to Send a RFID event
 *
 * @param    ByteSpan RFID tag value (max 10 octets)
 */
Status EnergyEvseDelegate::HwSetRFID(ByteSpan uid)
{
    Events::Rfid::Type event{ .uid = uid };
    EventNumber eventNumber;
    CHIP_ERROR error = LogEvent(event, mEndpointId, eventNumber);
    if (CHIP_NO_ERROR != error)
    {
        ChipLogError(Zcl, "[Notify] Unable to send notify event: %s [endpointId=%d]", error.AsString(), mEndpointId);
        return Status::Failure;
    }

    return Status::Success;
}

/**
 * @brief    Called by EVSE Hardware to share the VehicleID
 *
 * This routine will make a copy of the string so the callee doesn't
 * have to hold onto it forever.
 *
 * @param    CharSpan containing up to 32 chars.
 */
Status EnergyEvseDelegate::HwSetVehicleID(const CharSpan & newValue)
{
    VerifyOrReturnValue(mInstance != nullptr, Status::Failure);

    DataModel::Nullable<CharSpan> currentVehicleID = mInstance->GetVehicleID();

    if ((!currentVehicleID.IsNull() && newValue.data_equal(currentVehicleID.Value())) ||
        (currentVehicleID.IsNull() && newValue.empty()))
    {
        // No change in VehicleID, nothing to do
        return Status::Success;
    }

    if (newValue.size() > kMaxVehicleIDBufSize)
    {
        ChipLogError(AppServer, "HwSetVehicleID - input too long. Max size = %d", kMaxVehicleIDBufSize);
        return Status::Failure;
    }

    // If the input is empty, treat it as a request to clear the vehicle ID
    if (newValue.empty())
    {
        TEMPORARY_RETURN_IGNORED mInstance->SetVehicleID(DataModel::NullNullable);
        ChipLogDetail(AppServer, "VehicleID cleared");
        return Status::Success;
    }

    memcpy(mVehicleIDBuf, newValue.data(), newValue.size());
    DataModel::Nullable<CharSpan> vehicleID = MakeNullable(CharSpan(mVehicleIDBuf, newValue.size()));
    TEMPORARY_RETURN_IGNORED mInstance->SetVehicleID(vehicleID);

    ChipLogDetail(AppServer, "VehicleID updated %s", NullTerminated(vehicleID.Value()).c_str());

    return Status::Success;
}

/**
 * @brief Allows the caller to get a copy of the VehicleID into its own
 *        MutableCharSpan avoiding potential use-after-free if vehicleID
 *        was to change in the background
 */
CHIP_ERROR EnergyEvseDelegate::HwGetVehicleID(DataModel::Nullable<MutableCharSpan> & outValue)
{
    VerifyOrReturnError(mInstance != nullptr, CHIP_ERROR_INCORRECT_STATE);

    DataModel::Nullable<CharSpan> vehicleID = mInstance->GetVehicleID();

    if (vehicleID.IsNull())
    {
        outValue.SetNull();
        return CHIP_NO_ERROR;
    }

    if (outValue.IsNull())
    {
        // Defensive: outValue must be non-null to copy into it
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    return CopyCharSpanToMutableCharSpan(vehicleID.Value(), outValue.Value());
}

/**
 * @brief Called by EVSE Hardware to indicate that it has finished its diagnostics test
 */
Status EnergyEvseDelegate::HwDiagnosticsComplete()
{
    if (GetSupplyState() != SupplyStateEnum::kDisabledDiagnostics)
    {
        ChipLogError(AppServer, "Incorrect state to be completing diagnostics");
        return Status::Failure;
    }

    VerifyOrReturnValue(mInstance != nullptr, Status::Failure);

    /* Restore the SupplyState to Disabled (per spec) - client will need to
     * re-enable charging or discharging to get out of this state */
    TEMPORARY_RETURN_IGNORED mInstance->SetSupplyState(SupplyStateEnum::kDisabled);

    return Status::Success;
}
/* ---------------------------------------------------------------------------
 * Functions below are private helper functions internal to the delegate
 */

/**
 * @brief   Main EVSE state machine
 *
 * This routine handles state transition events to determine behaviour
 *
 *
 */
Status EnergyEvseDelegate::HandleStateMachineEvent(EVSEStateMachineEvent event)
{
    switch (event)
    {
    case EVSEStateMachineEvent::EVPluggedInEvent:
        ChipLogDetail(AppServer, "EVSE: EV PluggedIn event");
        return HandleEVPluggedInEvent();
        break;
    case EVSEStateMachineEvent::EVNotDetectedEvent:
        ChipLogDetail(AppServer, "EVSE: EV NotDetected event");
        return HandleEVNotDetectedEvent();
        break;
    case EVSEStateMachineEvent::EVNoDemandEvent:
        ChipLogDetail(AppServer, "EVSE: EV NoDemand event");
        return HandleEVNoDemandEvent();
        break;
    case EVSEStateMachineEvent::EVDemandEvent:
        ChipLogDetail(AppServer, "EVSE: EV Demand event");
        return HandleEVDemandEvent();
        break;
    case EVSEStateMachineEvent::ChargingEnabledEvent:
        ChipLogDetail(AppServer, "EVSE: ChargingEnabled event");
        return HandleChargingEnabledEvent();
        break;
    case EVSEStateMachineEvent::DischargingEnabledEvent:
        ChipLogDetail(AppServer, "EVSE: DischargingEnabled event");
        return HandleDischargingEnabledEvent();
        break;
    case EVSEStateMachineEvent::DisabledEvent:
        ChipLogDetail(AppServer, "EVSE: Disabled event");
        return HandleDisabledEvent();
        break;
    case EVSEStateMachineEvent::FaultRaised:
        ChipLogDetail(AppServer, "EVSE: FaultRaised event");
        return HandleFaultRaised();
        break;
    case EVSEStateMachineEvent::FaultCleared:
        ChipLogDetail(AppServer, "EVSE: FaultCleared event");
        return HandleFaultCleared();
        break;
    default:
        return Status::Failure;
    }
    return Status::Success;
}

Status EnergyEvseDelegate::HandleEVPluggedInEvent()
{
    StateEnum currentState = GetState();
    if (currentState == StateEnum::kNotPluggedIn)
    {
        VerifyOrReturnValue(mInstance != nullptr, Status::Failure);

        /* EV was not plugged in - start a new session */
        // TODO get energy meter readings - #35370
        mSession.StartSession(mInstance, 0, 0);
        SendEVConnectedEvent();

        /* Set the state to either PluggedInNoDemand or PluggedInDemand as indicated by mHwState */
        TEMPORARY_RETURN_IGNORED mInstance->SetState(mHwState);
    }
    // else we are already plugged in - ignore
    return Status::Success;
}

Status EnergyEvseDelegate::HandleEVNotDetectedEvent()
{
    VerifyOrReturnValue(mInstance != nullptr, Status::Failure);

    StateEnum currentState = GetState();
    if (currentState == StateEnum::kPluggedInCharging || currentState == StateEnum::kPluggedInDischarging)
    {
        /*
         * EV was transferring current - unusual to get to this case without
         * first having the state set to kPluggedInNoDemand or kPluggedInDemand
         */
        SendEnergyTransferStoppedEvent(EnergyTransferStoppedReasonEnum::kOther);
    }

    // TODO get energy meter readings - #35370
    mSession.StopSession(mInstance, 0, 0);
    SendEVNotDetectedEvent();
    TEMPORARY_RETURN_IGNORED mInstance->SetState(StateEnum::kNotPluggedIn);
    return Status::Success;
}

Status EnergyEvseDelegate::HandleEVNoDemandEvent()
{
    VerifyOrReturnValue(mInstance != nullptr, Status::Failure);

    StateEnum currentState = GetState();
    if (currentState == StateEnum::kPluggedInCharging || currentState == StateEnum::kPluggedInDischarging)
    {
        /*
         * EV was transferring current - EV decided to stop
         */
        mSession.RecalculateSessionDuration(mInstance);
        SendEnergyTransferStoppedEvent(EnergyTransferStoppedReasonEnum::kEVStopped);
    }
    /* We must still be plugged in to get here - so no need to check if we are plugged in! */
    TEMPORARY_RETURN_IGNORED mInstance->SetState(StateEnum::kPluggedInNoDemand);
    return Status::Success;
}
Status EnergyEvseDelegate::HandleEVDemandEvent()
{
    VerifyOrReturnValue(mInstance != nullptr, Status::Failure);

    /* Check to see if the supply is enabled for charging / discharging*/
    SupplyStateEnum currentSupplyState = GetSupplyState();
    switch (currentSupplyState)
    {
    case SupplyStateEnum::kChargingEnabled:
        ComputeMaxChargeCurrentLimit();
        TEMPORARY_RETURN_IGNORED mInstance->SetState(StateEnum::kPluggedInCharging);
        SendEnergyTransferStartedEvent();
        break;
    case SupplyStateEnum::kDischargingEnabled:
        ComputeMaxDischargeCurrentLimit();
        TEMPORARY_RETURN_IGNORED mInstance->SetState(StateEnum::kPluggedInDischarging);
        SendEnergyTransferStartedEvent();
        break;
    case SupplyStateEnum::kEnabled:
        /* We are enabled for both charging and discharging
        since the vehicle is asking for demand, we should start charging
        NOTE: for discharging the PowerAdjustment feature of DEM is used.
        This assumes we are not in TimeOfUse mode and charging should begin
        as soon as the vehicle asks for demand.
        */
        ComputeMaxChargeCurrentLimit();
        ComputeMaxDischargeCurrentLimit();
        TEMPORARY_RETURN_IGNORED mInstance->SetState(StateEnum::kPluggedInCharging);
        SendEnergyTransferStartedEvent();
        break;
    case SupplyStateEnum::kDisabled:
    case SupplyStateEnum::kDisabledError:
    case SupplyStateEnum::kDisabledDiagnostics:
        /* We must be plugged in, and the event is asking for demand
         * but we can't charge or discharge now - leave it as kPluggedInDemand */
        TEMPORARY_RETURN_IGNORED mInstance->SetState(StateEnum::kPluggedInDemand);
        break;
    case SupplyStateEnum::kUnknownEnumValue:
        ChipLogError(AppServer, "EVSE: HandleEVDemandEvent called in unexpected SupplyState");
        return Status::Failure;
    default:
        break;
    }
    return Status::Success;
}

Status EnergyEvseDelegate::CheckFaultOrDiagnostic()
{
    if (GetFaultState() != FaultStateEnum::kNoError)
    {
        ChipLogError(AppServer, "EVSE: Trying to handle command when fault is present");
        return Status::Failure;
    }

    if (GetSupplyState() == SupplyStateEnum::kDisabledDiagnostics)
    {
        ChipLogError(AppServer, "EVSE: Trying to handle command when in diagnostics mode");
        return Status::Failure;
    }
    return Status::Success;
}

Status EnergyEvseDelegate::HandleChargingEnabledEvent()
{
    /* Check there is no Fault or Diagnostics condition */
    Status status = CheckFaultOrDiagnostic();
    if (status != Status::Success)
    {
        return status;
    }

    VerifyOrReturnValue(mInstance != nullptr, Status::Failure);

    SupplyStateEnum currentSupplyState = GetSupplyState();
    switch (currentSupplyState)
    {
    case SupplyStateEnum::kDisabled:
        // it was kDisabled, then the state becomes kChargingEnabled
        TEMPORARY_RETURN_IGNORED mInstance->SetSupplyState(SupplyStateEnum::kChargingEnabled);
        break;
    case SupplyStateEnum::kChargingEnabled:
        // No change
        break;
    case SupplyStateEnum::kDischargingEnabled:
        // If the SupplyState was already kDischargingEnabled the state becomes kEnabled
        TEMPORARY_RETURN_IGNORED mInstance->SetSupplyState(SupplyStateEnum::kEnabled);
        break;
    case SupplyStateEnum::kDisabledError:
    case SupplyStateEnum::kDisabledDiagnostics:
        break;
    case SupplyStateEnum::kEnabled:
        // No change
        break;
    case SupplyStateEnum::kUnknownEnumValue:
        ChipLogError(AppServer, "EVSE: ChargingEnabledEvent called in unexpected SupplyState");
        return Status::Failure;
    }

    StateEnum currentState = GetState();
    switch (currentState)
    {
    case StateEnum::kNotPluggedIn:
    case StateEnum::kPluggedInNoDemand:
        break;
    case StateEnum::kPluggedInDemand:
        ComputeMaxChargeCurrentLimit();
        TEMPORARY_RETURN_IGNORED mInstance->SetState(StateEnum::kPluggedInCharging);
        SendEnergyTransferStartedEvent();
        break;
    case StateEnum::kPluggedInCharging:
    case StateEnum::kPluggedInDischarging:
        /* Switching from Discharging to Charging is controlled via PowerAdjust
           command from DEM - we don't do anything specific here
           and we do not send EnergyTransferStopped and StartedEvents when switching
           from Discharging to Charging and vice-versa. */
        break;
    default:
        break;
    }

    ScheduleCheckOnEnabledTimeout();

    return Status::Success;
}
Status EnergyEvseDelegate::HandleDischargingEnabledEvent()
{
    /* Check there is no Fault or Diagnostics condition */
    Status status = CheckFaultOrDiagnostic();
    if (status != Status::Success)
    {
        return status;
    }

    VerifyOrReturnValue(mInstance != nullptr, Status::Failure);

    SupplyStateEnum currentSupplyState = GetSupplyState();
    switch (currentSupplyState)
    {
    case SupplyStateEnum::kDisabled:
        // it was kDisabled, then the state becomes kDischargingEnabled
        TEMPORARY_RETURN_IGNORED mInstance->SetSupplyState(SupplyStateEnum::kDischargingEnabled);
        break;
    case SupplyStateEnum::kChargingEnabled:
        // If the SupplyState was already kChargingEnabled the state becomes kEnabled
        TEMPORARY_RETURN_IGNORED mInstance->SetSupplyState(SupplyStateEnum::kEnabled);
        break;
    case SupplyStateEnum::kDischargingEnabled:
        // No change
        break;
    case SupplyStateEnum::kDisabledError:
    case SupplyStateEnum::kDisabledDiagnostics:
        break;
    case SupplyStateEnum::kEnabled:
        // No change
        break;
    case SupplyStateEnum::kUnknownEnumValue:
        ChipLogError(AppServer, "EVSE: DischargingEnabledEvent called in unexpected SupplyState");
        return Status::Failure;
    }

    StateEnum currentState = GetState();
    switch (currentState)
    {
    case StateEnum::kNotPluggedIn:
    case StateEnum::kPluggedInNoDemand:
        break;
    case StateEnum::kPluggedInDemand:
    case StateEnum::kPluggedInCharging:
        /* Discharging is controlled from DEM PowerAdjust command - we don't change state here or send events here */
        ComputeMaxDischargeCurrentLimit();
        ComputeMaxDischargeCurrentLimit();
        break;
    case StateEnum::kPluggedInDischarging:
    default:
        break;
    }

    ScheduleCheckOnEnabledTimeout();

    return Status::Success;
}
Status EnergyEvseDelegate::HandleDisabledEvent()
{
    /* Check there is no Fault or Diagnostics condition */
    Status status = CheckFaultOrDiagnostic();
    if (status != Status::Success)
    {
        return status;
    }

    VerifyOrReturnValue(mInstance != nullptr, Status::Failure);

    /* update SupplyState to disabled */
    TEMPORARY_RETURN_IGNORED mInstance->SetSupplyState(SupplyStateEnum::kDisabled);

    StateEnum currentState = GetState();
    switch (currentState)
    {
    case StateEnum::kNotPluggedIn:
    case StateEnum::kPluggedInNoDemand:
    case StateEnum::kPluggedInDemand:
        break;
    case StateEnum::kPluggedInCharging:
    case StateEnum::kPluggedInDischarging:
        SendEnergyTransferStoppedEvent(EnergyTransferStoppedReasonEnum::kEVSEStopped);
        TEMPORARY_RETURN_IGNORED mInstance->SetState(mHwState);
        break;
    default:
        break;
    }

    return Status::Success;
}

/**
 * @brief This handles the new fault
 *
 * Note that if multiple faults happen and this is called repeatedly
 * We only save the previous State and SupplyState if its the first raising
 * of the fault, so we can restore the state back once all faults have cleared
)*/
Status EnergyEvseDelegate::HandleFaultRaised()
{
    VerifyOrReturnValue(mInstance != nullptr, Status::Failure);

    /* Save the current State and SupplyState so we can restore them if the fault clears */
    if (mStateBeforeFault == StateEnum::kUnknownEnumValue)
    {
        /* No existing fault - save this value to restore it later if it clears */
        mStateBeforeFault = GetState();
    }

    if (mSupplyStateBeforeFault == SupplyStateEnum::kUnknownEnumValue)
    {
        /* No existing fault */
        mSupplyStateBeforeFault = GetSupplyState();
    }

    /* Update State & SupplyState */
    TEMPORARY_RETURN_IGNORED mInstance->SetState(StateEnum::kFault);
    TEMPORARY_RETURN_IGNORED mInstance->SetSupplyState(SupplyStateEnum::kDisabledError);

    return Status::Success;
}
Status EnergyEvseDelegate::HandleFaultCleared()
{
    /* Check that something strange hasn't happened */
    if ((mStateBeforeFault == StateEnum::kUnknownEnumValue) || (mSupplyStateBeforeFault == SupplyStateEnum::kUnknownEnumValue))
    {
        ChipLogError(AppServer, "EVSE: Something wrong trying to clear fault");
        return Status::Failure;
    }

    VerifyOrReturnValue(mInstance != nullptr, Status::Failure);

    /* Restore the State and SupplyState back to old values once all the faults have cleared
     * Changing the State should notify the application, so it can continue charging etc
     */
    TEMPORARY_RETURN_IGNORED mInstance->SetState(mStateBeforeFault);
    TEMPORARY_RETURN_IGNORED mInstance->SetSupplyState(mSupplyStateBeforeFault);

    /* put back the sentinel to catch new faults if more are raised */
    mStateBeforeFault       = StateEnum::kUnknownEnumValue;
    mSupplyStateBeforeFault = SupplyStateEnum::kUnknownEnumValue;

    return Status::Success;
}

/**
 *  @brief   Called to compute the safe charging current limit
 *
 * mActualChargingCurrentLimit is the minimum of:
 *   - MaxHardwareChargeCurrentLimit (of the hardware)
 *   - CircuitCapacity (set by the electrician - less than the hardware)
 *   - CableAssemblyLimit (detected when the cable is inserted)
 *   - MaximumChargeCurrent (from charging command)
 *   - UserMaximumChargeCurrent (could dynamically change)
 *
 */
Status EnergyEvseDelegate::ComputeMaxChargeCurrentLimit()
{
    VerifyOrReturnValue(mInstance != nullptr, Status::Failure);

    int64_t oldValue            = mActualChargingCurrentLimit;
    mActualChargingCurrentLimit = mMaxHardwareChargeCurrentLimit;
    mActualChargingCurrentLimit = std::min(mActualChargingCurrentLimit, mInstance->GetCircuitCapacity());
    mActualChargingCurrentLimit = std::min(mActualChargingCurrentLimit, mCableAssemblyCurrentLimit);
    mActualChargingCurrentLimit = std::min(mActualChargingCurrentLimit, mMaximumChargingCurrentLimitFromCommand);
    mActualChargingCurrentLimit = std::min(mActualChargingCurrentLimit, mInstance->GetUserMaximumChargeCurrent());

    int64_t newMaximumChargeCurrent = mActualChargingCurrentLimit;

    if (oldValue != newMaximumChargeCurrent)
    {
        ChipLogDetail(AppServer, "MaximumChargeCurrent updated to %ld", static_cast<long>(newMaximumChargeCurrent));
        TEMPORARY_RETURN_IGNORED mInstance->SetMaximumChargeCurrent(newMaximumChargeCurrent);
        NotifyApplicationChargeCurrentLimitChange(newMaximumChargeCurrent);
    }
    return Status::Success;
}

/**
 *  @brief   Called to compute the safe discharging current limit
 *
 * mActualDischargingCurrentLimit is the minimum of:
 *   - MaxHardwareDischargeCurrentLimit (of the hardware)
 *   - CircuitCapacity (set by the electrician - less than the hardware)
 *   - CableAssemblyLimit (detected when the cable is inserted)
 *   - MaximumDischargeCurrent (from Enable Discharging command)
 */
Status EnergyEvseDelegate::ComputeMaxDischargeCurrentLimit()
{
    VerifyOrReturnValue(mInstance != nullptr, Status::Failure);

    int64_t oldValue               = mActualDischargingCurrentLimit;
    mActualDischargingCurrentLimit = mMaxHardwareDischargeCurrentLimit;
    mActualDischargingCurrentLimit = std::min(mActualDischargingCurrentLimit, mInstance->GetCircuitCapacity());
    mActualDischargingCurrentLimit = std::min(mActualDischargingCurrentLimit, mCableAssemblyCurrentLimit);
    mActualDischargingCurrentLimit = std::min(mActualDischargingCurrentLimit, mMaximumDischargingCurrentLimitFromCommand);

    int64_t newMaximumDischargeCurrent = mActualDischargingCurrentLimit;

    if (oldValue != newMaximumDischargeCurrent)
    {
        ChipLogDetail(AppServer, "MaximumDischargeCurrent updated to %ld", static_cast<long>(newMaximumDischargeCurrent));
        TEMPORARY_RETURN_IGNORED mInstance->SetMaximumDischargeCurrent(newMaximumDischargeCurrent);
        NotifyApplicationDischargeCurrentLimitChange(newMaximumDischargeCurrent);
    }
    return Status::Success;
}

Status EnergyEvseDelegate::NotifyApplicationChargeCurrentLimitChange(int64_t maximumChargeCurrent)
{
    EVSECbInfo cbInfo;

    cbInfo.type                                 = EVSECallbackType::ChargeCurrentChanged;
    cbInfo.ChargingCurrent.maximumChargeCurrent = maximumChargeCurrent;

    if (mCallbacks.handler != nullptr)
    {
        mCallbacks.handler(&cbInfo, mCallbacks.arg);
    }

    return Status::Success;
}

Status EnergyEvseDelegate::NotifyApplicationDischargeCurrentLimitChange(int64_t maximumDischargeCurrent)
{
    EVSECbInfo cbInfo;

    cbInfo.type                                       = EVSECallbackType::DischargeCurrentChanged;
    cbInfo.DischargingCurrent.maximumDischargeCurrent = maximumDischargeCurrent;

    if (mCallbacks.handler != nullptr)
    {
        mCallbacks.handler(&cbInfo, mCallbacks.arg);
    }

    return Status::Success;
}

Status EnergyEvseDelegate::NotifyApplicationStateChange()
{
    EVSECbInfo cbInfo;

    cbInfo.type                    = EVSECallbackType::StateChanged;
    cbInfo.StateChange.state       = GetState();
    cbInfo.StateChange.supplyState = GetSupplyState();

    if (mCallbacks.handler != nullptr)
    {
        mCallbacks.handler(&cbInfo, mCallbacks.arg);
    }

    return Status::Success;
}

Status EnergyEvseDelegate::NotifyApplicationChargingPreferencesChange()
{
    EVSECbInfo cbInfo;

    cbInfo.type = EVSECallbackType::ChargingPreferencesChanged;

    if (mCallbacks.handler != nullptr)
    {
        mCallbacks.handler(&cbInfo, mCallbacks.arg);
    }

    return Status::Success;
}

Status EnergyEvseDelegate::GetEVSEEnergyMeterValue(ChargingDischargingType meterType, int64_t & aMeterValue)
{
    EVSECbInfo cbInfo;

    cbInfo.type = EVSECallbackType::EnergyMeterReadingRequested;

    cbInfo.EnergyMeterReadingRequest.meterType           = meterType;
    cbInfo.EnergyMeterReadingRequest.energyMeterValuePtr = &aMeterValue;

    if (mCallbacks.handler != nullptr)
    {
        mCallbacks.handler(&cbInfo, mCallbacks.arg);
    }

    return Status::Success;
}

Status EnergyEvseDelegate::SendEVConnectedEvent()
{
    Events::EVConnected::Type event;
    EventNumber eventNumber;

    VerifyOrReturnError(mInstance != nullptr, Status::Failure, ChipLogError(AppServer, "Instance is Null"));
    DataModel::Nullable<uint32_t> sessionID = mInstance->GetSessionID();
    VerifyOrReturnError(!sessionID.IsNull(), Status::Failure, ChipLogError(AppServer, "SessionID is Null"));

    event.sessionID = sessionID.Value();

    CHIP_ERROR err = LogEvent(event, mEndpointId, eventNumber);
    if (CHIP_NO_ERROR != err)
    {
        ChipLogError(AppServer, "Unable to send notify event: %" CHIP_ERROR_FORMAT, err.Format());
        return Status::Failure;
    }
    return Status::Success;
}

Status EnergyEvseDelegate::SendEVNotDetectedEvent()
{
    Events::EVNotDetected::Type event;
    EventNumber eventNumber;

    VerifyOrReturnError(mInstance != nullptr, Status::Failure, ChipLogError(AppServer, "Instance is Null"));
    DataModel::Nullable<uint32_t> sessionID = mInstance->GetSessionID();
    VerifyOrReturnError(!sessionID.IsNull(), Status::Failure, ChipLogError(AppServer, "SessionID is Null"));

    event.sessionID               = sessionID.Value();
    event.state                   = GetState();
    event.sessionDuration         = mInstance->GetSessionDuration().Value();
    event.sessionEnergyCharged    = mInstance->GetSessionEnergyCharged().Value();
    event.sessionEnergyDischarged = MakeOptional(mInstance->GetSessionEnergyDischarged().Value());

    CHIP_ERROR err = LogEvent(event, mEndpointId, eventNumber);
    if (CHIP_NO_ERROR != err)
    {
        ChipLogError(AppServer, "Unable to send notify event: %" CHIP_ERROR_FORMAT, err.Format());
        return Status::Failure;
    }
    return Status::Success;
}

Status EnergyEvseDelegate::SendEnergyTransferStartedEvent()
{
    Events::EnergyTransferStarted::Type event;
    EventNumber eventNumber;

    VerifyOrReturnError(mInstance != nullptr, Status::Failure, ChipLogError(AppServer, "Instance is Null"));
    DataModel::Nullable<uint32_t> sessionID = mInstance->GetSessionID();
    VerifyOrReturnError(!sessionID.IsNull(), Status::Failure, ChipLogError(AppServer, "SessionID is Null"));

    event.sessionID = sessionID.Value();
    event.state     = GetState();

    /* Sample the energy meter for charging */
    GetEVSEEnergyMeterValue(ChargingDischargingType::kCharging, mImportedMeterValueAtEnergyTransferStart);
    event.maximumCurrent = GetMaximumChargeCurrent();

    /* For V2X we may switch between charging and discharging, but we don't
     * keep sending EnergyTransferStarted events */
    if (mInstance->HasFeature(Feature::kV2x))
    {
        /* Sample the energy meter for discharging */
        GetEVSEEnergyMeterValue(ChargingDischargingType::kDischarging, mExportedMeterValueAtEnergyTransferStart);

        event.maximumDischargeCurrent.SetValue(GetMaximumDischargeCurrent());
    }
    else
    {
        mExportedMeterValueAtEnergyTransferStart = 0;
        event.maximumDischargeCurrent.ClearValue();
    }

    CHIP_ERROR err = LogEvent(event, mEndpointId, eventNumber);
    if (CHIP_NO_ERROR != err)
    {
        ChipLogError(AppServer, "Unable to send notify event: %" CHIP_ERROR_FORMAT, err.Format());
        return Status::Failure;
    }

    return Status::Success;
}
Status EnergyEvseDelegate::SendEnergyTransferStoppedEvent(EnergyTransferStoppedReasonEnum reason)
{
    Events::EnergyTransferStopped::Type event;
    EventNumber eventNumber;

    VerifyOrReturnError(mInstance != nullptr, Status::Failure, ChipLogError(AppServer, "Instance is Null"));
    DataModel::Nullable<uint32_t> sessionID = mInstance->GetSessionID();
    VerifyOrReturnError(!sessionID.IsNull(), Status::Failure, ChipLogError(AppServer, "SessionID is Null"));

    event.sessionID       = sessionID.Value();
    event.state           = GetState();
    event.reason          = reason;
    int64_t meterValueNow = 0;

    GetEVSEEnergyMeterValue(ChargingDischargingType::kCharging, meterValueNow);
    event.energyTransferred = meterValueNow - mImportedMeterValueAtEnergyTransferStart;

    if (mInstance->HasFeature(Feature::kV2x))
    {
        GetEVSEEnergyMeterValue(ChargingDischargingType::kDischarging, meterValueNow);
        event.energyDischarged.SetValue(mExportedMeterValueAtEnergyTransferStart - meterValueNow);
    }
    else
    {
        event.energyDischarged.ClearValue();
    }

    CHIP_ERROR err = LogEvent(event, mEndpointId, eventNumber);
    if (CHIP_NO_ERROR != err)
    {
        ChipLogError(AppServer, "Unable to send notify event: %" CHIP_ERROR_FORMAT, err.Format());
        return Status::Failure;
    }
    return Status::Success;
}

Status EnergyEvseDelegate::SendFaultEvent(FaultStateEnum newFaultState)
{
    Events::Fault::Type event;
    EventNumber eventNumber;

    VerifyOrReturnError(mInstance != nullptr, Status::Failure, ChipLogError(AppServer, "Instance is Null"));
    event.sessionID               = mInstance->GetSessionID(); // Note here the event sessionID can be Null!
    event.state                   = GetState();                // This is the state prior to the fault being raised
    event.faultStatePreviousState = GetFaultState();
    event.faultStateCurrentState  = newFaultState;

    CHIP_ERROR err = LogEvent(event, mEndpointId, eventNumber);
    if (CHIP_NO_ERROR != err)
    {
        ChipLogError(AppServer, "Unable to send notify event: %" CHIP_ERROR_FORMAT, err.Format());
        return Status::Failure;
    }
    return Status::Success;
}

/**
 * Attribute change callbacks - called by cluster after attribute is updated
 */

void EnergyEvseDelegate::OnStateChanged(StateEnum newValue)
{
    ChipLogDetail(AppServer, "State updated to %d", static_cast<int>(newValue));
    NotifyApplicationStateChange();
}

void EnergyEvseDelegate::OnSupplyStateChanged(SupplyStateEnum newValue)
{
    ChipLogDetail(AppServer, "SupplyState updated to %d", static_cast<int>(newValue));
    NotifyApplicationStateChange();
}

void EnergyEvseDelegate::OnFaultStateChanged(FaultStateEnum newValue)
{
    ChipLogDetail(AppServer, "FaultState updated to %d", static_cast<int>(newValue));
}

void EnergyEvseDelegate::OnChargingEnabledUntilChanged(DataModel::Nullable<uint32_t> newValue)
{
    if (newValue.IsNull())
    {
        ChipLogDetail(AppServer, "ChargingEnabledUntil updated to Null");
    }
    else
    {
        ChipLogDetail(AppServer, "ChargingEnabledUntil updated to %lu", static_cast<unsigned long int>(newValue.Value()));
    }
    ConcreteAttributePath path = ConcreteAttributePath(mEndpointId, EnergyEvse::Id, ChargingEnabledUntil::Id);
    TEMPORARY_RETURN_IGNORED GetSafeAttributePersistenceProvider()->WriteScalarValue(path, newValue);
}

void EnergyEvseDelegate::OnDischargingEnabledUntilChanged(DataModel::Nullable<uint32_t> newValue)
{
    if (newValue.IsNull())
    {
        ChipLogDetail(AppServer, "DischargingEnabledUntil updated to Null");
    }
    else
    {
        ChipLogDetail(AppServer, "DischargingEnabledUntil updated to %lu", static_cast<unsigned long int>(newValue.Value()));
    }
    ConcreteAttributePath path = ConcreteAttributePath(mEndpointId, EnergyEvse::Id, DischargingEnabledUntil::Id);
    TEMPORARY_RETURN_IGNORED GetSafeAttributePersistenceProvider()->WriteScalarValue(path, newValue);
}

void EnergyEvseDelegate::OnCircuitCapacityChanged(int64_t newValue)
{
    ChipLogDetail(AppServer, "CircuitCapacity updated to %ld", static_cast<long>(newValue));
}

void EnergyEvseDelegate::OnMinimumChargeCurrentChanged(int64_t newValue)
{
    ChipLogDetail(AppServer, "MinimumChargeCurrent updated to %ld", static_cast<long>(newValue));
}

void EnergyEvseDelegate::OnMaximumChargeCurrentChanged(int64_t newValue)
{
    ChipLogDetail(AppServer, "MaximumChargeCurrent updated to %ld", static_cast<long>(newValue));
}

void EnergyEvseDelegate::OnMaximumDischargeCurrentChanged(int64_t newValue)
{
    ChipLogDetail(AppServer, "MaximumDischargeCurrent updated to %ld", static_cast<long>(newValue));
}

void EnergyEvseDelegate::OnUserMaximumChargeCurrentChanged(int64_t newValue)
{
    ChipLogDetail(AppServer, "UserMaximumChargeCurrent updated to %ld", static_cast<long>(newValue));
    ComputeMaxChargeCurrentLimit();
    ConcreteAttributePath path = ConcreteAttributePath(mEndpointId, EnergyEvse::Id, UserMaximumChargeCurrent::Id);
    TEMPORARY_RETURN_IGNORED GetSafeAttributePersistenceProvider()->WriteScalarValue(path, newValue);
}

void EnergyEvseDelegate::OnRandomizationDelayWindowChanged(uint32_t newValue)
{
    ChipLogDetail(AppServer, "RandomizationDelayWindow updated to %lu", static_cast<unsigned long int>(newValue));
    ConcreteAttributePath path = ConcreteAttributePath(mEndpointId, EnergyEvse::Id, RandomizationDelayWindow::Id);
    TEMPORARY_RETURN_IGNORED GetSafeAttributePersistenceProvider()->WriteScalarValue(path, newValue);
}

void EnergyEvseDelegate::OnNextChargeStartTimeChanged(DataModel::Nullable<uint32_t> newValue)
{
    if (newValue.IsNull())
    {
        ChipLogDetail(AppServer, "NextChargeStartTime updated to Null");
    }
    else
    {
        ChipLogDetail(AppServer, "NextChargeStartTime updated to %lu", static_cast<unsigned long int>(newValue.Value()));
    }
}

void EnergyEvseDelegate::OnNextChargeTargetTimeChanged(DataModel::Nullable<uint32_t> newValue)
{
    if (newValue.IsNull())
    {
        ChipLogDetail(AppServer, "NextChargeTargetTime updated to Null");
    }
    else
    {
        ChipLogDetail(AppServer, "NextChargeTargetTime updated to %lu", static_cast<unsigned long int>(newValue.Value()));
    }
}

void EnergyEvseDelegate::OnNextChargeRequiredEnergyChanged(DataModel::Nullable<int64_t> newValue)
{
    if (newValue.IsNull())
    {
        ChipLogDetail(AppServer, "NextChargeRequiredEnergy updated to Null");
    }
    else
    {
        ChipLogDetail(AppServer, "NextChargeRequiredEnergy updated to %ld", static_cast<long>(newValue.Value()));
    }
}

void EnergyEvseDelegate::OnNextChargeTargetSoCChanged(DataModel::Nullable<Percent> newValue)
{
    if (newValue.IsNull())
    {
        ChipLogDetail(AppServer, "NextChargeTargetSoC updated to Null");
    }
    else
    {
        ChipLogDetail(AppServer, "NextChargeTargetSoC updated to %d %%", newValue.Value());
    }
}

void EnergyEvseDelegate::OnApproximateEVEfficiencyChanged(DataModel::Nullable<uint16_t> newValue)
{
    if (newValue.IsNull())
    {
        ChipLogDetail(AppServer, "ApproximateEVEfficiency updated to Null");
    }
    else
    {
        ChipLogDetail(AppServer, "ApproximateEVEfficiency updated to %d", newValue.Value());
    }
    ConcreteAttributePath path = ConcreteAttributePath(mEndpointId, EnergyEvse::Id, ApproximateEVEfficiency::Id);
    TEMPORARY_RETURN_IGNORED GetSafeAttributePersistenceProvider()->WriteScalarValue(path, newValue);
}

void EnergyEvseDelegate::OnStateOfChargeChanged(DataModel::Nullable<Percent> newValue)
{
    if (newValue.IsNull())
    {
        ChipLogDetail(AppServer, "StateOfCharge updated to Null");
    }
    else
    {
        ChipLogDetail(AppServer, "StateOfCharge updated to %d", newValue.Value());
    }
}

void EnergyEvseDelegate::OnBatteryCapacityChanged(DataModel::Nullable<int64_t> newValue)
{
    if (newValue.IsNull())
    {
        ChipLogDetail(AppServer, "BatteryCapacity updated to Null");
    }
    else
    {
        ChipLogDetail(AppServer, "BatteryCapacity updated to %ld", static_cast<long>(newValue.Value()));
    }
}

void EnergyEvseDelegate::OnVehicleIDChanged(DataModel::Nullable<CharSpan> newValue)
{
    if (newValue.IsNull())
    {
        ChipLogDetail(AppServer, "VehicleID updated to Null");
    }
    else
    {
        ChipLogDetail(AppServer, "VehicleID updated to %.*s", static_cast<int>(newValue.Value().size()), newValue.Value().data());
    }
}

void EnergyEvseDelegate::OnSessionIDChanged(DataModel::Nullable<uint32_t> newValue)
{
    if (newValue.IsNull())
    {
        ChipLogDetail(AppServer, "SessionID updated to Null");
    }
    else
    {
        ChipLogDetail(AppServer, "SessionID updated to %lu", static_cast<unsigned long int>(newValue.Value()));
    }
    // Write value to persistent storage.
    ConcreteAttributePath path = ConcreteAttributePath(mEndpointId, EnergyEvse::Id, SessionID::Id);
    TEMPORARY_RETURN_IGNORED GetSafeAttributePersistenceProvider()->WriteScalarValue(path, newValue);
}

void EnergyEvseDelegate::OnSessionDurationChanged(DataModel::Nullable<uint32_t> newValue)
{
    if (newValue.IsNull())
    {
        ChipLogDetail(AppServer, "SessionDuration updated to Null");
    }
    else
    {
        ChipLogDetail(AppServer, "SessionDuration updated to %lu", static_cast<unsigned long int>(newValue.Value()));
    }
}

void EnergyEvseDelegate::OnSessionEnergyChargedChanged(DataModel::Nullable<int64_t> newValue)
{
    if (newValue.IsNull())
    {
        ChipLogDetail(AppServer, "SessionEnergyCharged updated to Null");
    }
    else
    {
        ChipLogDetail(AppServer, "SessionEnergyCharged updated to %ld", static_cast<long>(newValue.Value()));
    }
}

void EnergyEvseDelegate::OnSessionEnergyDischargedChanged(DataModel::Nullable<int64_t> newValue)
{
    if (newValue.IsNull())
    {
        ChipLogDetail(AppServer, "SessionEnergyDischarged updated to Null");
    }
    else
    {
        ChipLogDetail(AppServer, "SessionEnergyDischarged updated to %ld", static_cast<long>(newValue.Value()));
    }
}

/**
 * @brief   Helper function to get know if the EV is plugged in based on state
 *          (regardless of if it is actually transferring energy)
 */
bool EnergyEvseDelegate::IsEvsePluggedIn()
{
    StateEnum currentState = GetState();
    return (currentState == StateEnum::kPluggedInCharging || currentState == StateEnum::kPluggedInDemand ||
            currentState == StateEnum::kPluggedInDischarging || currentState == StateEnum::kPluggedInNoDemand);
}

/**
 * @brief This function samples the start-time, and energy meter to hold the session info
 *
 * @param endpointId            - The endpoint to report the update on
 * @param chargingMeterValue    - The current value of the energy meter (charging) in mWh
 * @param dischargingMeterValue - The current value of the energy meter (discharging) in mWh
 */
void EvseSession::StartSession(Instance * instance, int64_t chargingMeterValue, int64_t dischargingMeterValue)
{
    VerifyOrReturn(instance != nullptr);

    /* Get Timestamp */
    uint32_t matterEpochSeconds = 0;
    CHIP_ERROR err              = System::Clock::GetClock_MatterEpochS(matterEpochSeconds);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "EVSE: Unable to get current time when starting session - err:%" CHIP_ERROR_FORMAT, err.Format());
        return;
    }
    mStartTime = matterEpochSeconds;

    mSessionEnergyChargedAtStart    = chargingMeterValue;
    mSessionEnergyDischargedAtStart = dischargingMeterValue;

    // Compute next session ID
    DataModel::Nullable<uint32_t> currentSessionID = instance->GetSessionID();
    if (currentSessionID.IsNull())
    {
        TEMPORARY_RETURN_IGNORED instance->SetSessionID(MakeNullable(static_cast<uint32_t>(0)));
    }
    else
    {
        TEMPORARY_RETURN_IGNORED instance->SetSessionID(MakeNullable(currentSessionID.Value() + 1));
    }

    // Reset session counters
    TEMPORARY_RETURN_IGNORED instance->SetSessionDuration(MakeNullable(static_cast<uint32_t>(0)));
    TEMPORARY_RETURN_IGNORED instance->SetSessionEnergyCharged(MakeNullable(static_cast<int64_t>(0)));
    TEMPORARY_RETURN_IGNORED instance->SetSessionEnergyDischarged(MakeNullable(static_cast<int64_t>(0)));

    // TODO persist mStartTime
    // TODO persist mSessionEnergyChargedAtStart
    // TODO persist mSessionEnergyDischargedAtStart
}

/**
 * @brief This function updates the session information at the unplugged event
 *
 * @param endpointId            - The endpoint to report the update on
 * @param chargingMeterValue    - The current value of the energy meter (charging) in mWh
 * @param dischargingMeterValue - The current value of the energy meter (discharging) in mWh
 */
void EvseSession::StopSession(Instance * instance, int64_t chargingMeterValue, int64_t dischargingMeterValue)
{
    RecalculateSessionDuration(instance);
    UpdateEnergyCharged(instance, chargingMeterValue);
    UpdateEnergyDischarged(instance, dischargingMeterValue);
}

/*---------------------- EvseSession functions --------------------------*/

/**
 * @brief This function updates the session attrs to allow read attributes to return latest values
 *
 * @param endpointId            - The endpoint to report the update on
 */
void EvseSession::RecalculateSessionDuration(Instance * instance)
{
    VerifyOrReturn(instance != nullptr);

    uint32_t matterEpochSeconds = 0;
    CHIP_ERROR err              = System::Clock::GetClock_MatterEpochS(matterEpochSeconds);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "EVSE: Unable to get current time when updating session duration - err:%" CHIP_ERROR_FORMAT,
                     err.Format());
        return;
    }

    uint32_t duration = matterEpochSeconds - mStartTime;
    TEMPORARY_RETURN_IGNORED instance->SetSessionDuration(MakeNullable(duration));
}

/**
 * @brief This function updates the EnergyCharged meter value
 *
 * @param endpointId            - The endpoint to report the update on
 * @param chargingMeterValue    - The value of the energy meter (charging) in mWh
 */
void EvseSession::UpdateEnergyCharged(Instance * instance, int64_t chargingMeterValue)
{
    VerifyOrReturn(instance != nullptr);
    TEMPORARY_RETURN_IGNORED instance->SetSessionEnergyCharged(MakeNullable(chargingMeterValue - mSessionEnergyChargedAtStart));
}

/**
 * @brief This function updates the EnergyDischarged meter value
 *
 * @param endpointId            - The endpoint to report the update on
 * @param dischargingMeterValue - The value of the energy meter (discharging) in mWh
 */
void EvseSession::UpdateEnergyDischarged(Instance * instance, int64_t dischargingMeterValue)
{
    VerifyOrReturn(instance != nullptr);
    TEMPORARY_RETURN_IGNORED instance->SetSessionEnergyDischarged(
        MakeNullable(dischargingMeterValue - mSessionEnergyDischargedAtStart));
}

// ------------------------------------------------------------------
// Local getters for internal delegate use - delegates to cluster instance
// ------------------------------------------------------------------

StateEnum EnergyEvseDelegate::GetState() const
{
    VerifyOrDie(mInstance != nullptr);
    return mInstance->GetState();
}

SupplyStateEnum EnergyEvseDelegate::GetSupplyState() const
{
    VerifyOrDie(mInstance != nullptr);
    return mInstance->GetSupplyState();
}

FaultStateEnum EnergyEvseDelegate::GetFaultState() const
{
    VerifyOrDie(mInstance != nullptr);
    return mInstance->GetFaultState();
}

DataModel::Nullable<uint32_t> EnergyEvseDelegate::GetChargingEnabledUntil() const
{
    VerifyOrDie(mInstance != nullptr);
    return mInstance->GetChargingEnabledUntil();
}

DataModel::Nullable<uint32_t> EnergyEvseDelegate::GetDischargingEnabledUntil() const
{
    VerifyOrDie(mInstance != nullptr);
    return mInstance->GetDischargingEnabledUntil();
}

int64_t EnergyEvseDelegate::GetCircuitCapacity() const
{
    VerifyOrDie(mInstance != nullptr);
    return mInstance->GetCircuitCapacity();
}

int64_t EnergyEvseDelegate::GetMinimumChargeCurrent() const
{
    VerifyOrDie(mInstance != nullptr);
    return mInstance->GetMinimumChargeCurrent();
}

int64_t EnergyEvseDelegate::GetMaximumChargeCurrent() const
{
    VerifyOrDie(mInstance != nullptr);
    return mInstance->GetMaximumChargeCurrent();
}

int64_t EnergyEvseDelegate::GetMaximumDischargeCurrent() const
{
    VerifyOrDie(mInstance != nullptr);
    return mInstance->GetMaximumDischargeCurrent();
}

int64_t EnergyEvseDelegate::GetUserMaximumChargeCurrent() const
{
    VerifyOrDie(mInstance != nullptr);
    return mInstance->GetUserMaximumChargeCurrent();
}

uint32_t EnergyEvseDelegate::GetRandomizationDelayWindow() const
{
    VerifyOrDie(mInstance != nullptr);
    return mInstance->GetRandomizationDelayWindow();
}

DataModel::Nullable<uint32_t> EnergyEvseDelegate::GetNextChargeStartTime() const
{
    VerifyOrDie(mInstance != nullptr);
    return mInstance->GetNextChargeStartTime();
}

DataModel::Nullable<uint32_t> EnergyEvseDelegate::GetNextChargeTargetTime() const
{
    VerifyOrDie(mInstance != nullptr);
    return mInstance->GetNextChargeTargetTime();
}

DataModel::Nullable<int64_t> EnergyEvseDelegate::GetNextChargeRequiredEnergy() const
{
    VerifyOrDie(mInstance != nullptr);
    return mInstance->GetNextChargeRequiredEnergy();
}

DataModel::Nullable<Percent> EnergyEvseDelegate::GetNextChargeTargetSoC() const
{
    VerifyOrDie(mInstance != nullptr);
    return mInstance->GetNextChargeTargetSoC();
}

DataModel::Nullable<uint16_t> EnergyEvseDelegate::GetApproximateEVEfficiency() const
{
    VerifyOrDie(mInstance != nullptr);
    return mInstance->GetApproximateEVEfficiency();
}

DataModel::Nullable<Percent> EnergyEvseDelegate::GetStateOfCharge() const
{
    VerifyOrDie(mInstance != nullptr);
    return mInstance->GetStateOfCharge();
}

DataModel::Nullable<int64_t> EnergyEvseDelegate::GetBatteryCapacity() const
{
    VerifyOrDie(mInstance != nullptr);
    return mInstance->GetBatteryCapacity();
}

DataModel::Nullable<CharSpan> EnergyEvseDelegate::GetVehicleID() const
{
    VerifyOrDie(mInstance != nullptr);
    return mInstance->GetVehicleID();
}

DataModel::Nullable<uint32_t> EnergyEvseDelegate::GetSessionID() const
{
    VerifyOrDie(mInstance != nullptr);
    return mInstance->GetSessionID();
}

DataModel::Nullable<uint32_t> EnergyEvseDelegate::GetSessionDuration() const
{
    VerifyOrDie(mInstance != nullptr);
    return mInstance->GetSessionDuration();
}

DataModel::Nullable<int64_t> EnergyEvseDelegate::GetSessionEnergyCharged() const
{
    VerifyOrDie(mInstance != nullptr);
    return mInstance->GetSessionEnergyCharged();
}

DataModel::Nullable<int64_t> EnergyEvseDelegate::GetSessionEnergyDischarged() const
{
    VerifyOrDie(mInstance != nullptr);
    return mInstance->GetSessionEnergyDischarged();
}
