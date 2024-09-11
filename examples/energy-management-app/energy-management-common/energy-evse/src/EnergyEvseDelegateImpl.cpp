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
#include <EnergyTimeUtils.h>
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app/EventLogging.h>
#include <app/SafeAttributePersistenceProvider.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::DataModel;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::EnergyEvse;
using namespace chip::app::Clusters::EnergyEvse::Attributes;

using chip::app::LogEvent;
using chip::Protocols::InteractionModel::Status;

EnergyEvseDelegate::~EnergyEvseDelegate()
{
    // TODO Fix this as part of issue #30993 refactoring
    if (!mVehicleID.IsNull())
    {
        ChipLogDetail(AppServer, "Freeing VehicleID");
        delete[] mVehicleID.Value().data();
    }
}

/**
 * @brief   Called when EVSE cluster receives Disable command
 */
Status EnergyEvseDelegate::Disable()
{
    ChipLogProgress(AppServer, "EnergyEvseDelegate::Disable()");

    DataModel::Nullable<uint32_t> disableTime(0);
    /* update ChargingEnabledUntil & DischargingEnabledUntil to show 0 */
    SetChargingEnabledUntil(disableTime);
    SetDischargingEnabledUntil(disableTime);

    /* update MinimumChargeCurrent & MaximumChargeCurrent to 0 */
    SetMinimumChargeCurrent(0);

    mMaximumChargingCurrentLimitFromCommand = 0;
    ComputeMaxChargeCurrentLimit();

    /* update MaximumDischargeCurrent to 0 */
    SetMaximumDischargeCurrent(0);

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

    if (maximumChargeCurrent < kMinimumChargeCurrent)
    {
        ChipLogError(AppServer, "Maximum Current outside limits");
        return Status::ConstraintError;
    }

    if (minimumChargeCurrent < kMinimumChargeCurrent)
    {
        ChipLogError(AppServer, "Maximum Current outside limits");
        return Status::ConstraintError;
    }

    if (minimumChargeCurrent > maximumChargeCurrent)
    {
        ChipLogError(AppServer, "Minium Current > Maximum Current!");
        return Status::ConstraintError;
    }

    if (chargingEnabledUntil.IsNull())
    {
        /* Charging enabled indefinitely */
        ChipLogError(AppServer, "Charging enabled indefinitely");
        SetChargingEnabledUntil(chargingEnabledUntil);
    }
    else
    {
        /* check chargingEnabledUntil is in the future */
        ChipLogError(AppServer, "Charging enabled until: %lu", static_cast<long unsigned int>(chargingEnabledUntil.Value()));
        SetChargingEnabledUntil(chargingEnabledUntil);
    }

    /* If it looks ok, store the min & max charging current */
    mMaximumChargingCurrentLimitFromCommand = maximumChargeCurrent;
    SetMinimumChargeCurrent(minimumChargeCurrent);
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

    // TODO save the maxDischarging Current
    // TODO Do something with timestamp

    return HandleStateMachineEvent(EVSEStateMachineEvent::DischargingEnabledEvent);
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

    uint32_t chipEpoch = 0;
    DataModel::Nullable<uint32_t> enabledUntilTime;

    if (mSupplyState == SupplyStateEnum::kChargingEnabled)
    {
        enabledUntilTime = GetChargingEnabledUntil();
    }
    else if (mSupplyState == SupplyStateEnum::kDischargingEnabled)
    {
        enabledUntilTime = GetDischargingEnabledUntil();
    }
    else
    {
        // In all other states the EVSE is disabled
        return Status::Success;
    }

    if (enabledUntilTime.IsNull())
    {
        /* This is enabled indefinitely so don't schedule a callback */
        return Status::Success;
    }

    CHIP_ERROR err = DeviceEnergyManagement::GetEpochTS(chipEpoch);
    if (err == CHIP_NO_ERROR)
    {
        /* time is sync'd */
        int32_t delta = static_cast<int32_t>(enabledUntilTime.Value() - chipEpoch);
        if (delta > 0)
        {
            /* The timer hasn't expired yet - set a timer to check in the future */
            ChipLogDetail(AppServer, "Setting EVSE Enable check timer for %ld seconds", static_cast<long int>(delta));
            DeviceLayer::SystemLayer().StartTimer(System::Clock::Seconds32(delta), EvseCheckTimerExpiry, this);
        }
        else
        {
            /* we have gone past the enabledUntilTime - so we need to disable */
            ChipLogDetail(AppServer, "EVSE enable time expired, disabling charging");
            Disable();
        }
    }
    else if (err == CHIP_ERROR_REAL_TIME_NOT_SYNCED)
    {
        /* Real time isn't sync'd -lets check again in 30 seconds - otherwise keep the charger enabled */
        DeviceLayer::SystemLayer().StartTimer(System::Clock::Seconds32(kPeriodicCheckIntervalRealTimeClockNotSynced_sec),
                                              EvseCheckTimerExpiry, this);
    }
    return Status::Success;
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

    if (mSupplyState != SupplyStateEnum::kDisabled)
    {
        ChipLogError(AppServer, "EVSE: cannot be put into diagnostics mode if it is not Disabled!");
        return Status::Failure;
    }

    // Update the SupplyState - this will automatically callback the Application StateChanged callback
    SetSupplyState(SupplyStateEnum::kDisabledDiagnostics);

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
 *           current limit supported by the hardware.
 *
 *           This is normally called at start-up.
 *
 * @param    currentmA - Maximum current limit supported by the hardware
 */
Status EnergyEvseDelegate::HwSetMaxHardwareCurrentLimit(int64_t currentmA)
{
    if (currentmA < kMinimumChargeCurrent)
    {
        return Status::ConstraintError;
    }

    /* there is no attribute to store this so store in private variable */
    mMaxHardwareCurrentLimit = currentmA;

    return ComputeMaxChargeCurrentLimit();
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
    if (currentmA < kMinimumChargeCurrent)
    {
        return Status::ConstraintError;
    }

    mCircuitCapacity = currentmA;
    MatterReportingAttributeChangeCallback(mEndpointId, EnergyEvse::Id, CircuitCapacity::Id);

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
    if (currentmA < kMinimumChargeCurrent)
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

    if (mFaultState == newFaultState)
    {
        ChipLogError(AppServer, "No change in fault state, ignoring call");
        return Status::Failure;
    }

    /** Before we do anything we log the fault
     * any change in FaultState reports previous fault and new fault
     * and the state prior to the fault being raised */
    SendFaultEvent(newFaultState);

    /* Updated FaultState before we call into the handlers */
    SetFaultState(newFaultState);

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
    // TODO this code to be refactored - See Issue #30993
    if (!mVehicleID.IsNull() && newValue.data_equal(mVehicleID.Value()))
    {
        return Status::Success;
    }

    /* create a copy of the string so the callee doesn't have to keep it */
    char * destinationBuffer = new char[kMaxVehicleIDBufSize];

    MutableCharSpan destinationString(destinationBuffer, kMaxVehicleIDBufSize);
    CHIP_ERROR err = CopyCharSpanToMutableCharSpan(newValue, destinationString);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "HwSetVehicleID - could not copy vehicleID");
        delete[] destinationBuffer;
        return Status::Failure;
    }

    if (!mVehicleID.IsNull())
    {
        delete[] mVehicleID.Value().data();
    }

    mVehicleID = MakeNullable(static_cast<CharSpan>(destinationString));

    ChipLogDetail(AppServer, "VehicleID updated %.*s", static_cast<int>(mVehicleID.Value().size()), mVehicleID.Value().data());
    MatterReportingAttributeChangeCallback(mEndpointId, EnergyEvse::Id, VehicleID::Id);

    return Status::Success;
}

/**
 * @brief Called by EVSE Hardware to indicate that it has finished its diagnostics test
 */
Status EnergyEvseDelegate::HwDiagnosticsComplete()
{
    if (mSupplyState != SupplyStateEnum::kDisabledDiagnostics)
    {
        ChipLogError(AppServer, "Incorrect state to be completing diagnostics");
        return Status::Failure;
    }

    /* Restore the SupplyState to Disabled (per spec) - client will need to
     * re-enable charging or discharging to get out of this state */
    SetSupplyState(SupplyStateEnum::kDisabled);

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
    /* check if we are already plugged in or not */
    if (mState == StateEnum::kNotPluggedIn)
    {
        /* EV was not plugged in - start a new session */
        // TODO get energy meter readings - #35370
        mSession.StartSession(mEndpointId, 0, 0);
        SendEVConnectedEvent();

        /* Set the state to either PluggedInNoDemand or PluggedInDemand as indicated by mHwState */
        SetState(mHwState);
    }
    // else we are already plugged in - ignore
    return Status::Success;
}

Status EnergyEvseDelegate::HandleEVNotDetectedEvent()
{
    if (mState == StateEnum::kPluggedInCharging || mState == StateEnum::kPluggedInDischarging)
    {
        /*
         * EV was transferring current - unusual to get to this case without
         * first having the state set to kPluggedInNoDemand or kPluggedInDemand
         */
        SendEnergyTransferStoppedEvent(EnergyTransferStoppedReasonEnum::kOther);
    }

    // TODO get energy meter readings - #35370
    mSession.StopSession(mEndpointId, 0, 0);
    SendEVNotDetectedEvent();
    SetState(StateEnum::kNotPluggedIn);
    return Status::Success;
}

Status EnergyEvseDelegate::HandleEVNoDemandEvent()
{
    if (mState == StateEnum::kPluggedInCharging || mState == StateEnum::kPluggedInDischarging)
    {
        /*
         * EV was transferring current - EV decided to stop
         */
        mSession.RecalculateSessionDuration(mEndpointId);
        SendEnergyTransferStoppedEvent(EnergyTransferStoppedReasonEnum::kEVStopped);
    }
    /* We must still be plugged in to get here - so no need to check if we are plugged in! */
    SetState(StateEnum::kPluggedInNoDemand);
    return Status::Success;
}
Status EnergyEvseDelegate::HandleEVDemandEvent()
{
    /* Check to see if the supply is enabled for charging / discharging*/
    switch (mSupplyState)
    {
    case SupplyStateEnum::kChargingEnabled:
        ComputeMaxChargeCurrentLimit();
        SetState(StateEnum::kPluggedInCharging);
        SendEnergyTransferStartedEvent();
        break;
    case SupplyStateEnum::kDischargingEnabled:
        // TODO ComputeMaxDischargeCurrentLimit() - Needs to be implemented
        SetState(StateEnum::kPluggedInDischarging);
        SendEnergyTransferStartedEvent();
        break;
    case SupplyStateEnum::kDisabled:
    case SupplyStateEnum::kDisabledError:
    case SupplyStateEnum::kDisabledDiagnostics:
        /* We must be plugged in, and the event is asking for demand
         * but we can't charge or discharge now - leave it as kPluggedInDemand */
        SetState(StateEnum::kPluggedInDemand);
        break;
    default:
        break;
    }
    return Status::Success;
}

Status EnergyEvseDelegate::CheckFaultOrDiagnostic()
{
    if (mFaultState != FaultStateEnum::kNoError)
    {
        ChipLogError(AppServer, "EVSE: Trying to handle command when fault is present");
        return Status::Failure;
    }

    if (mSupplyState == SupplyStateEnum::kDisabledDiagnostics)
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

    /* update SupplyState to say that charging is now enabled */
    SetSupplyState(SupplyStateEnum::kChargingEnabled);

    switch (mState)
    {
    case StateEnum::kNotPluggedIn:
    case StateEnum::kPluggedInNoDemand:
        break;
    case StateEnum::kPluggedInDemand:
        ComputeMaxChargeCurrentLimit();
        SetState(StateEnum::kPluggedInCharging);
        SendEnergyTransferStartedEvent();
        break;
    case StateEnum::kPluggedInCharging:
        break;
    case StateEnum::kPluggedInDischarging:
        /* Switched from discharging to charging */
        SendEnergyTransferStoppedEvent(EnergyTransferStoppedReasonEnum::kEVSEStopped);

        ComputeMaxChargeCurrentLimit();
        SetState(StateEnum::kPluggedInCharging);
        SendEnergyTransferStartedEvent();
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
    /* update SupplyState to say that charging is now enabled */
    SetSupplyState(SupplyStateEnum::kDischargingEnabled);

    switch (mState)
    {
    case StateEnum::kNotPluggedIn:
    case StateEnum::kPluggedInNoDemand:
        break;
    case StateEnum::kPluggedInDemand:
        // TODO call ComputeMaxDischargeCurrentLimit()
        SetState(StateEnum::kPluggedInDischarging);
        SendEnergyTransferStartedEvent();
        break;
    case StateEnum::kPluggedInCharging:
        /* Switched from charging to discharging */
        SendEnergyTransferStoppedEvent(EnergyTransferStoppedReasonEnum::kEVSEStopped);

        // TODO call ComputeMaxDischargeCurrentLimit()
        SetState(StateEnum::kPluggedInDischarging);
        SendEnergyTransferStartedEvent();
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

    /* update SupplyState to say that charging is now enabled */
    SetSupplyState(SupplyStateEnum::kDisabled);

    switch (mState)
    {
    case StateEnum::kNotPluggedIn:
    case StateEnum::kPluggedInNoDemand:
    case StateEnum::kPluggedInDemand:
        break;
    case StateEnum::kPluggedInCharging:
    case StateEnum::kPluggedInDischarging:
        SendEnergyTransferStoppedEvent(EnergyTransferStoppedReasonEnum::kEVSEStopped);
        SetState(mHwState);
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
    /* Save the current State and SupplyState so we can restore them if the fault clears */
    if (mStateBeforeFault == StateEnum::kUnknownEnumValue)
    {
        /* No existing fault - save this value to restore it later if it clears */
        mStateBeforeFault = mState;
    }

    if (mSupplyStateBeforeFault == SupplyStateEnum::kUnknownEnumValue)
    {
        /* No existing fault */
        mSupplyStateBeforeFault = mSupplyState;
    }

    /* Update State & SupplyState */
    SetState(StateEnum::kFault);
    SetSupplyState(SupplyStateEnum::kDisabledError);

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

    /* Restore the State and SupplyState back to old values once all the faults have cleared
     * Changing the State should notify the application, so it can continue charging etc
     */
    SetState(mStateBeforeFault);
    SetSupplyState(mSupplyStateBeforeFault);

    /* put back the sentinel to catch new faults if more are raised */
    mStateBeforeFault       = StateEnum::kUnknownEnumValue;
    mSupplyStateBeforeFault = SupplyStateEnum::kUnknownEnumValue;

    return Status::Success;
}

/**
 *  @brief   Called to compute the safe charging current limit
 *
 * mActualChargingCurrentLimit is the minimum of:
 *   - MaxHardwareCurrentLimit (of the hardware)
 *   - CircuitCapacity (set by the electrician - less than the hardware)
 *   - CableAssemblyLimit (detected when the cable is inserted)
 *   - MaximumChargeCurrent (from charging command)
 *   - UserMaximumChargeCurrent (could dynamically change)
 *
 */
Status EnergyEvseDelegate::ComputeMaxChargeCurrentLimit()
{
    int64_t oldValue;

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
        NotifyApplicationCurrentLimitChange(mMaximumChargeCurrent);
    }
    return Status::Success;
}

Status EnergyEvseDelegate::NotifyApplicationCurrentLimitChange(int64_t maximumChargeCurrent)
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

Status EnergyEvseDelegate::NotifyApplicationStateChange()
{
    EVSECbInfo cbInfo;

    cbInfo.type                    = EVSECallbackType::StateChanged;
    cbInfo.StateChange.state       = mState;
    cbInfo.StateChange.supplyState = mSupplyState;

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

    if (mSession.mSessionID.IsNull())
    {
        ChipLogError(AppServer, "SessionID is Null");
        return Status::Failure;
    }

    event.sessionID = mSession.mSessionID.Value();

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

    if (mSession.mSessionID.IsNull())
    {
        ChipLogError(AppServer, "SessionID is Null");
        return Status::Failure;
    }

    event.sessionID               = mSession.mSessionID.Value();
    event.state                   = mState;
    event.sessionDuration         = mSession.mSessionDuration.Value();
    event.sessionEnergyCharged    = mSession.mSessionEnergyCharged.Value();
    event.sessionEnergyDischarged = MakeOptional(mSession.mSessionEnergyDischarged.Value());

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

    if (mSession.mSessionID.IsNull())
    {
        ChipLogError(AppServer, "SessionID is Null");
        return Status::Failure;
    }

    event.sessionID = mSession.mSessionID.Value();
    event.state     = mState;
    /**
     * A positive value indicates the EV has been enabled for charging and the value is
     * taken directly from the MaximumChargeCurrent attribute.
     * A negative value indicates that the EV has been enabled for discharging and the value can be taken
     * from the MaximumDischargeCurrent attribute with its sign inverted.
     */

    if (mState == StateEnum::kPluggedInCharging)
    {
        /* Sample the energy meter for charging */
        GetEVSEEnergyMeterValue(ChargingDischargingType::kCharging, mMeterValueAtEnergyTransferStart);
        event.maximumCurrent = mMaximumChargeCurrent;
    }
    else if (mState == StateEnum::kPluggedInDischarging)
    {
        /* Sample the energy meter for discharging */
        GetEVSEEnergyMeterValue(ChargingDischargingType::kDischarging, mMeterValueAtEnergyTransferStart);

        /* discharging should have a negative current  */
        event.maximumCurrent = -mMaximumDischargeCurrent;
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

    if (mSession.mSessionID.IsNull())
    {
        ChipLogError(AppServer, "SessionID is Null");
        return Status::Failure;
    }

    event.sessionID       = mSession.mSessionID.Value();
    event.state           = mState;
    event.reason          = reason;
    int64_t meterValueNow = 0;

    if (mState == StateEnum::kPluggedInCharging)
    {
        GetEVSEEnergyMeterValue(ChargingDischargingType::kCharging, meterValueNow);
        event.energyTransferred = meterValueNow - mMeterValueAtEnergyTransferStart;
    }
    else if (mState == StateEnum::kPluggedInDischarging)
    {
        GetEVSEEnergyMeterValue(ChargingDischargingType::kDischarging, meterValueNow);

        /* discharging should have a negative value */
        event.energyTransferred = mMeterValueAtEnergyTransferStart - meterValueNow;
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

    event.sessionID               = mSession.mSessionID; // Note here the event sessionID can be Null!
    event.state                   = mState;              // This is the state prior to the fault being raised
    event.faultStatePreviousState = mFaultState;
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
        ChipLogDetail(AppServer, "State updated to %d", static_cast<int>(mState));
        MatterReportingAttributeChangeCallback(mEndpointId, EnergyEvse::Id, State::Id);
        NotifyApplicationStateChange();
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
        ChipLogDetail(AppServer, "SupplyState updated to %d", static_cast<int>(mSupplyState));
        MatterReportingAttributeChangeCallback(mEndpointId, EnergyEvse::Id, SupplyState::Id);
        NotifyApplicationStateChange();
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
        ChipLogDetail(AppServer, "FaultState updated to %d", static_cast<int>(mFaultState));
        MatterReportingAttributeChangeCallback(mEndpointId, EnergyEvse::Id, FaultState::Id);
    }
    return CHIP_NO_ERROR;
}

/* ChargingEnabledUntil */
DataModel::Nullable<uint32_t> EnergyEvseDelegate::GetChargingEnabledUntil()
{
    return mChargingEnabledUntil;
}

CHIP_ERROR EnergyEvseDelegate::SetChargingEnabledUntil(DataModel::Nullable<uint32_t> newValue)
{
    DataModel::Nullable<uint32_t> oldValue = mChargingEnabledUntil;

    mChargingEnabledUntil = newValue;
    if (oldValue != newValue)
    {
        if (newValue.IsNull())
        {
            ChipLogDetail(AppServer, "ChargingEnabledUntil updated to Null");
        }
        else
        {
            ChipLogDetail(AppServer, "ChargingEnabledUntil updated to %lu",
                          static_cast<unsigned long int>(mChargingEnabledUntil.Value()));
        }

        // Write new value to persistent storage.
        ConcreteAttributePath path = ConcreteAttributePath(mEndpointId, EnergyEvse::Id, ChargingEnabledUntil::Id);
        GetSafeAttributePersistenceProvider()->WriteScalarValue(path, mChargingEnabledUntil);

        MatterReportingAttributeChangeCallback(mEndpointId, EnergyEvse::Id, ChargingEnabledUntil::Id);
    }

    return CHIP_NO_ERROR;
}

/* DischargingEnabledUntil */
DataModel::Nullable<uint32_t> EnergyEvseDelegate::GetDischargingEnabledUntil()
{
    return mDischargingEnabledUntil;
}

CHIP_ERROR EnergyEvseDelegate::SetDischargingEnabledUntil(DataModel::Nullable<uint32_t> newValue)
{
    DataModel::Nullable<uint32_t> oldValue = mDischargingEnabledUntil;

    mDischargingEnabledUntil = newValue;
    if (oldValue != newValue)
    {
        if (newValue.IsNull())
        {
            ChipLogDetail(AppServer, "DischargingEnabledUntil updated to Null");
        }
        else
        {
            ChipLogDetail(AppServer, "DischargingEnabledUntil updated to %lu",
                          static_cast<unsigned long int>(mDischargingEnabledUntil.Value()));
        }
        // Write new value to persistent storage.
        ConcreteAttributePath path = ConcreteAttributePath(mEndpointId, EnergyEvse::Id, DischargingEnabledUntil::Id);
        GetSafeAttributePersistenceProvider()->WriteScalarValue(path, mDischargingEnabledUntil);

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

    if (newValue < 0)
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

    if (newValue < 0)
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

/* MaximumDischargeCurrent */
int64_t EnergyEvseDelegate::GetMaximumDischargeCurrent()
{
    return mMaximumDischargeCurrent;
}

CHIP_ERROR EnergyEvseDelegate::SetMaximumDischargeCurrent(int64_t newValue)
{
    int64_t oldValue = mMaximumDischargeCurrent;

    if (newValue < 0)
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
    if (newValue < 0)
    {
        return CHIP_IM_GLOBAL_STATUS(ConstraintError);
    }

    int64_t oldValue          = mUserMaximumChargeCurrent;
    mUserMaximumChargeCurrent = newValue;
    if (oldValue != newValue)
    {
        ChipLogDetail(AppServer, "UserMaximumChargeCurrent updated to %ld", static_cast<long>(mUserMaximumChargeCurrent));

        ComputeMaxChargeCurrentLimit();

        // Write new value to persistent storage.
        ConcreteAttributePath path = ConcreteAttributePath(mEndpointId, EnergyEvse::Id, UserMaximumChargeCurrent::Id);
        GetSafeAttributePersistenceProvider()->WriteScalarValue(path, mUserMaximumChargeCurrent);

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

        // Write new value to persistent storage.
        ConcreteAttributePath path = ConcreteAttributePath(mEndpointId, EnergyEvse::Id, RandomizationDelayWindow::Id);
        GetSafeAttributePersistenceProvider()->WriteScalarValue(path, mRandomizationDelayWindow);

        MatterReportingAttributeChangeCallback(mEndpointId, EnergyEvse::Id, RandomizationDelayWindow::Id);
    }
    return CHIP_NO_ERROR;
}

/* PREF attributes */
DataModel::Nullable<uint32_t> EnergyEvseDelegate::GetNextChargeStartTime()
{
    return mNextChargeStartTime;
}
CHIP_ERROR EnergyEvseDelegate::SetNextChargeStartTime(DataModel::Nullable<uint32_t> newNextChargeStartTimeUtc)
{
    if (newNextChargeStartTimeUtc == mNextChargeStartTime)
    {
        return CHIP_NO_ERROR;
    }

    mNextChargeStartTime = newNextChargeStartTimeUtc;
    if (mNextChargeStartTime.IsNull())
    {
        ChipLogDetail(AppServer, "NextChargeStartTime updated to Null");
    }
    else
    {
        ChipLogDetail(AppServer, "NextChargeStartTime updated to %lu",
                      static_cast<unsigned long int>(mNextChargeStartTime.Value()));
    }

    MatterReportingAttributeChangeCallback(mEndpointId, EnergyEvse::Id, NextChargeStartTime::Id);

    return CHIP_NO_ERROR;
}

DataModel::Nullable<uint32_t> EnergyEvseDelegate::GetNextChargeTargetTime()
{
    return mNextChargeTargetTime;
}
CHIP_ERROR EnergyEvseDelegate::SetNextChargeTargetTime(DataModel::Nullable<uint32_t> newNextChargeTargetTimeUtc)
{
    if (newNextChargeTargetTimeUtc == mNextChargeTargetTime)
    {
        return CHIP_NO_ERROR;
    }

    mNextChargeTargetTime = newNextChargeTargetTimeUtc;
    if (mNextChargeTargetTime.IsNull())
    {
        ChipLogDetail(AppServer, "NextChargeTargetTime updated to Null");
    }
    else
    {
        ChipLogDetail(AppServer, "NextChargeTargetTime updated to %lu",
                      static_cast<unsigned long int>(mNextChargeTargetTime.Value()));
    }

    MatterReportingAttributeChangeCallback(mEndpointId, EnergyEvse::Id, NextChargeTargetTime::Id);

    return CHIP_NO_ERROR;
}

DataModel::Nullable<int64_t> EnergyEvseDelegate::GetNextChargeRequiredEnergy()
{
    return mNextChargeRequiredEnergy;
}
CHIP_ERROR EnergyEvseDelegate::SetNextChargeRequiredEnergy(DataModel::Nullable<int64_t> newNextChargeRequiredEnergyMilliWattH)
{
    if (mNextChargeRequiredEnergy == newNextChargeRequiredEnergyMilliWattH)
    {
        return CHIP_NO_ERROR;
    }

    mNextChargeRequiredEnergy = newNextChargeRequiredEnergyMilliWattH;
    if (mNextChargeRequiredEnergy.IsNull())
    {
        ChipLogDetail(AppServer, "NextChargeRequiredEnergy updated to Null");
    }
    else
    {
        ChipLogDetail(AppServer, "NextChargeRequiredEnergy updated to %ld", static_cast<long>(mNextChargeRequiredEnergy.Value()));
    }

    MatterReportingAttributeChangeCallback(mEndpointId, EnergyEvse::Id, NextChargeRequiredEnergy::Id);

    return CHIP_NO_ERROR;
}

DataModel::Nullable<Percent> EnergyEvseDelegate::GetNextChargeTargetSoC()
{
    return mNextChargeTargetSoC;
}
CHIP_ERROR EnergyEvseDelegate::SetNextChargeTargetSoC(DataModel::Nullable<Percent> newValue)
{
    DataModel::Nullable<Percent> oldValue = mNextChargeTargetSoC;

    mNextChargeTargetSoC = newValue;
    if (oldValue != newValue)
    {
        if (newValue.IsNull())
        {
            ChipLogDetail(AppServer, "NextChargeTargetSoC updated to Null");
        }
        else
        {
            ChipLogDetail(AppServer, "NextChargeTargetSoC updated to %d %%", mNextChargeTargetSoC.Value());
        }
        MatterReportingAttributeChangeCallback(mEndpointId, EnergyEvse::Id, NextChargeTargetSoC::Id);
    }
    return CHIP_NO_ERROR;
}

/* ApproximateEVEfficiency */
DataModel::Nullable<uint16_t> EnergyEvseDelegate::GetApproximateEVEfficiency()
{
    return mApproximateEVEfficiency;
}

CHIP_ERROR EnergyEvseDelegate::SetApproximateEVEfficiency(DataModel::Nullable<uint16_t> newValue)
{
    DataModel::Nullable<uint16_t> oldValue = mApproximateEVEfficiency;

    mApproximateEVEfficiency = newValue;
    if (oldValue != newValue)
    {
        if (newValue.IsNull())
        {
            ChipLogDetail(AppServer, "ApproximateEVEfficiency updated to Null");
        }
        else
        {
            ChipLogDetail(AppServer, "ApproximateEVEfficiency updated to %d", mApproximateEVEfficiency.Value());
        }
        // Write new value to persistent storage.
        ConcreteAttributePath path = ConcreteAttributePath(mEndpointId, EnergyEvse::Id, ApproximateEVEfficiency::Id);
        GetSafeAttributePersistenceProvider()->WriteScalarValue(path, mApproximateEVEfficiency);

        MatterReportingAttributeChangeCallback(mEndpointId, EnergyEvse::Id, ApproximateEVEfficiency::Id);
    }

    return CHIP_NO_ERROR;
}

/* SOC attributes */
DataModel::Nullable<Percent> EnergyEvseDelegate::GetStateOfCharge()
{
    return mStateOfCharge;
}
DataModel::Nullable<int64_t> EnergyEvseDelegate::GetBatteryCapacity()
{
    return mBatteryCapacity;
}

/* PNC attributes*/
DataModel::Nullable<CharSpan> EnergyEvseDelegate::GetVehicleID()
{
    return mVehicleID;
}

/* Session SESS attributes */
DataModel::Nullable<uint32_t> EnergyEvseDelegate::GetSessionID()
{
    return mSession.mSessionID;
}
DataModel::Nullable<uint32_t> EnergyEvseDelegate::GetSessionDuration()
{
    return mSession.mSessionDuration;
}
DataModel::Nullable<int64_t> EnergyEvseDelegate::GetSessionEnergyCharged()
{
    return mSession.mSessionEnergyCharged;
}
DataModel::Nullable<int64_t> EnergyEvseDelegate::GetSessionEnergyDischarged()
{
    return mSession.mSessionEnergyDischarged;
}

/**
 * @brief   Helper function to get know if the EV is plugged in based on state
 *          (regardless of if it is actually transferring energy)
 */
bool EnergyEvseDelegate::IsEvsePluggedIn()
{
    return (mState == StateEnum::kPluggedInCharging || mState == StateEnum::kPluggedInDemand ||
            mState == StateEnum::kPluggedInDischarging || mState == StateEnum::kPluggedInNoDemand);
}

/**
 * @brief This function samples the start-time, and energy meter to hold the session info
 *
 * @param endpointId            - The endpoint to report the update on
 * @param chargingMeterValue    - The current value of the energy meter (charging) in mWh
 * @param dischargingMeterValue - The current value of the energy meter (discharging) in mWh
 */
void EvseSession::StartSession(EndpointId endpointId, int64_t chargingMeterValue, int64_t dischargingMeterValue)
{
    /* Get Timestamp */
    uint32_t chipEpoch = 0;
    CHIP_ERROR err     = DeviceEnergyManagement::GetEpochTS(chipEpoch);
    if (err != CHIP_NO_ERROR)
    {
        /* Note that the error will be also be logged inside GetErrorTS() -
         * adding context here to help debugging */
        ChipLogError(AppServer, "EVSE: Unable to get current time when starting session - err:%" CHIP_ERROR_FORMAT, err.Format());
        return;
    }
    mStartTime = chipEpoch;

    mSessionEnergyChargedAtStart    = chargingMeterValue;
    mSessionEnergyDischargedAtStart = dischargingMeterValue;

    if (mSessionID.IsNull())
    {
        mSessionID = MakeNullable(static_cast<uint32_t>(0));
    }
    else
    {
        uint32_t sessionID = mSessionID.Value() + 1;
        mSessionID         = MakeNullable(sessionID);
    }

    /* Reset other session values */
    mSessionDuration         = MakeNullable(static_cast<uint32_t>(0));
    mSessionEnergyCharged    = MakeNullable(static_cast<int64_t>(0));
    mSessionEnergyDischarged = MakeNullable(static_cast<int64_t>(0));

    MatterReportingAttributeChangeCallback(endpointId, EnergyEvse::Id, SessionID::Id);
    MatterReportingAttributeChangeCallback(endpointId, EnergyEvse::Id, SessionDuration::Id);
    MatterReportingAttributeChangeCallback(endpointId, EnergyEvse::Id, SessionEnergyCharged::Id);
    MatterReportingAttributeChangeCallback(endpointId, EnergyEvse::Id, SessionEnergyDischarged::Id);

    // Write values to persistent storage.
    ConcreteAttributePath path = ConcreteAttributePath(endpointId, EnergyEvse::Id, SessionID::Id);
    GetSafeAttributePersistenceProvider()->WriteScalarValue(path, mSessionID);

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
void EvseSession::StopSession(EndpointId endpointId, int64_t chargingMeterValue, int64_t dischargingMeterValue)
{
    RecalculateSessionDuration(endpointId);
    UpdateEnergyCharged(endpointId, chargingMeterValue);
    UpdateEnergyDischarged(endpointId, dischargingMeterValue);
}

/*---------------------- EvseSession functions --------------------------*/

/**
 * @brief This function updates the session attrs to allow read attributes to return latest values
 *
 * @param endpointId            - The endpoint to report the update on
 */
void EvseSession::RecalculateSessionDuration(EndpointId endpointId)
{
    /* Get Timestamp */
    uint32_t chipEpoch = 0;
    CHIP_ERROR err     = DeviceEnergyManagement::GetEpochTS(chipEpoch);
    if (err != CHIP_NO_ERROR)
    {
        /* Note that the error will be also be logged inside GetErrorTS() -
         * adding context here to help debugging */
        ChipLogError(AppServer, "EVSE: Unable to get current time when updating session duration - err:%" CHIP_ERROR_FORMAT,
                     err.Format());
        return;
    }

    uint32_t duration = chipEpoch - mStartTime;
    mSessionDuration  = MakeNullable(duration);
    MatterReportingAttributeChangeCallback(endpointId, EnergyEvse::Id, SessionDuration::Id);
}

/**
 * @brief This function updates the EnergyCharged meter value
 *
 * @param endpointId            - The endpoint to report the update on
 * @param chargingMeterValue    - The value of the energy meter (charging) in mWh
 */
void EvseSession::UpdateEnergyCharged(EndpointId endpointId, int64_t chargingMeterValue)
{
    mSessionEnergyCharged = MakeNullable(chargingMeterValue - mSessionEnergyChargedAtStart);
    MatterReportingAttributeChangeCallback(endpointId, EnergyEvse::Id, SessionEnergyCharged::Id);
}

/**
 * @brief This function updates the EnergyDischarged meter value
 *
 * @param endpointId            - The endpoint to report the update on
 * @param dischargingMeterValue - The value of the energy meter (discharging) in mWh
 */
void EvseSession::UpdateEnergyDischarged(EndpointId endpointId, int64_t dischargingMeterValue)
{
    mSessionEnergyDischarged = MakeNullable(dischargingMeterValue - mSessionEnergyDischargedAtStart);
    MatterReportingAttributeChangeCallback(endpointId, EnergyEvse::Id, SessionEnergyDischarged::Id);
}
