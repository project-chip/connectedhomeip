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

#pragma once

#include "app/clusters/energy-evse-server/energy-evse-server.h"
#include <EVSECallbacks.h>
#include <EnergyEvseTargetsStore.h>

#include <app/util/config.h>
#include <cstring>

using chip::Protocols::InteractionModel::Status;

namespace chip {
namespace app {
namespace Clusters {
namespace EnergyEvse {

// A bitmap of all possible days (the union of the values in
// chip::app::Clusters::EnergyEvse::TargetDayOfWeekBitmap)
constexpr uint8_t kAllTargetDaysMask = 0x7f;

/* Local state machine Events to allow simpler handling of state transitions */
enum EVSEStateMachineEvent
{
    EVPluggedInEvent,        /* EV has been plugged in */
    EVNotDetectedEvent,      /* EV has been unplugged or detected as not connected */
    EVNoDemandEvent,         /* EV has stopped asking for demand */
    EVDemandEvent,           /* EV has asked for demand*/
    ChargingEnabledEvent,    /* Charging has been enabled */
    DischargingEnabledEvent, /* Discharging has been enabled */
    DisabledEvent,           /* EVSE has been disabled */
    FaultRaised,             /* Fault has been raised */
    FaultCleared,            /* Fault has been cleared */
};

/**
 * Helper class to handle all of the session related info
 */
class EvseSession
{
public:
    EvseSession() {}
    /**
     * @brief This function records the start time and provided energy meter values as part of the new session.
     *
     * @param endpointId            - The endpoint to report the update on
     * @param chargingMeterValue    - The current value of the energy meter (charging) in mWh
     * @param dischargingMeterValue - The current value of the energy meter (discharging) in mWh
     */
    void StartSession(EndpointId endpointId, int64_t chargingMeterValue, int64_t dischargingMeterValue);

    /**
     * @brief This function updates the session information at the unplugged event
     *
     * @param endpointId            - The endpoint to report the update on
     * @param chargingMeterValue    - The current value of the energy meter (charging) in mWh
     * @param dischargingMeterValue - The current value of the energy meter (discharging) in mWh
     */
    void StopSession(EndpointId endpointId, int64_t chargingMeterValue, int64_t dischargingMeterValue);

    /**
     * @brief This function updates the session Duration to allow read attributes to return latest values
     *
     * @param endpointId            - The endpoint to report the update on
     */
    void RecalculateSessionDuration(EndpointId endpointId);

    /**
     * @brief This function updates the EnergyCharged meter value
     *
     * @param endpointId            - The endpoint to report the update on
     * @param chargingMeterValue    - The value of the energy meter (charging) in mWh
     */
    void UpdateEnergyCharged(EndpointId endpointId, int64_t chargingMeterValue);

    /**
     * @brief This function updates the EnergyDischarged meter value
     *
     * @param endpointId            - The endpoint to report the update on
     * @param dischargingMeterValue - The value of the energy meter (discharging) in mWh
     */
    void UpdateEnergyDischarged(EndpointId endpointId, int64_t dischargingMeterValue);

    /* Public members - represent attributes in the cluster */
    DataModel::Nullable<uint32_t> mSessionID;
    DataModel::Nullable<uint32_t> mSessionDuration;
    DataModel::Nullable<int64_t> mSessionEnergyCharged;
    DataModel::Nullable<int64_t> mSessionEnergyDischarged;

private:
    uint32_t mStartTime                     = 0; // Epoch_s - 0 means it hasn't started yet
    int64_t mSessionEnergyChargedAtStart    = 0; // in mWh - 0 means it hasn't been set yet
    int64_t mSessionEnergyDischargedAtStart = 0; // in mWh - 0 means it hasn't been set yet
};

/**
 * The application delegate.
 */

class EnergyEvseDelegate : public EnergyEvse::Delegate
{
public:
    EnergyEvseDelegate(EvseTargetsDelegate & aDelegate) : EnergyEvse::Delegate() { mEvseTargetsDelegate = &aDelegate; }
    ~EnergyEvseDelegate();

    EvseTargetsDelegate * GetEvseTargetsDelegate() { return mEvseTargetsDelegate; }

    /**
     * @brief   Called when EVSE cluster receives Disable command
     */
    Status Disable() override;

    /**
     * @brief   Called when EVSE cluster receives EnableCharging command
     *
     * @param chargingEnabledUntil
     * @param minimumChargeCurrent (in mA)
     * @param maximumChargeCurrent (in mA)
     */
    Status EnableCharging(const DataModel::Nullable<uint32_t> & chargingEnabledUntil, const int64_t & minimumChargeCurrent,
                          const int64_t & maximumChargeCurrent) override;

    /**
     * @brief   Called when EVSE cluster receives EnableDischarging command
     *
     * @param dischargingEnabledUntil
     * @param maximumChargeCurrent (in mA)
     */
    Status EnableDischarging(const DataModel::Nullable<uint32_t> & dischargingEnabledUntil,
                             const int64_t & maximumDischargeCurrent) override;

    /**
     * @brief    Called when EVSE cluster receives StartDiagnostics command
     */
    Status StartDiagnostics() override;

    /**
     * @brief    Called when EVSE cluster receives the SetTargets command
     */
    Status SetTargets(
        const DataModel::DecodableList<Structs::ChargingTargetScheduleStruct::DecodableType> & chargingTargetSchedules) override;

    /**
     * @brief Delegate should implement a handler for LoadTargets
     *
     * This needs to load any stored targets into memory and MUST be called before
     * GetTargets is called.
     */
    Status LoadTargets() override;

    /**
     * @brief    Called when EVSE cluster receives the GetTargets command
     *
     * NOTE: LoadTargets MUST be called GetTargets is called.
     */
    Protocols::InteractionModel::Status
    GetTargets(DataModel::List<const Structs::ChargingTargetScheduleStruct::Type> & chargingTargetSchedules) override;

    /**
     * @brief    Called when EVSE cluster receives ClearTargets command
     */
    Status ClearTargets() override;

    /* Helper functions for managing targets*/
    Status
    ValidateTargets(const DataModel::DecodableList<Structs::ChargingTargetScheduleStruct::DecodableType> & chargingTargetSchedules);

    /**
     * @brief    Called by EVSE Hardware to register a single callback handler
     */
    Status HwRegisterEvseCallbackHandler(EVSECallbackFunc handler, intptr_t arg);

    /**
     * @brief    Decides if a timer is needed based on EVSE state and sets a callback if needed
     *
     * In order to ensure the EVSE restarts charging (if enabled) after power loss
     * this should be called after the EVSE is initialised
     * (e.g. HwSetMaxHardwareCurrentLimit and HwSetCircuitCapacity have been called)
     * and the persisted attributes have been loaded, and time has been synchronised.
     *
     * If time isn't sync'd yet it will call itself back periodically (if required)
     * until time is sync'd.
     *
     * It is also called when a EnableCharging or EnableDischarging command
     * is recv'd to schedule when the EVSE should be automatically disabled based
     * on ChargingEnabledUntil / DischargingEnabledUntil expiring.
     */
    Status ScheduleCheckOnEnabledTimeout();

    /**
     * @brief   Helper function to get know if the EV is plugged in based on state
     *          (regardless of if it is actually transferring energy)
     */
    bool IsEvsePluggedIn();

    // -----------------------------------------------------------------
    // Internal API to allow an EVSE to change its internal state etc
    Status HwSetMaxHardwareCurrentLimit(int64_t currentmA);
    int64_t HwGetMaxHardwareCurrentLimit() { return mMaxHardwareCurrentLimit; }
    Status HwSetCircuitCapacity(int64_t currentmA);
    Status HwSetCableAssemblyLimit(int64_t currentmA);
    int64_t HwGetCableAssemblyLimit() { return mCableAssemblyCurrentLimit; }
    Status HwSetState(StateEnum state);
    StateEnum HwGetState() { return mHwState; };
    Status HwSetFault(FaultStateEnum fault);
    Status HwSetRFID(ByteSpan uid);
    Status HwSetVehicleID(const CharSpan & vehID);
    Status HwDiagnosticsComplete();
    Status SendEVConnectedEvent();
    Status SendEVNotDetectedEvent();
    Status SendEnergyTransferStartedEvent();
    Status SendEnergyTransferStoppedEvent(EnergyTransferStoppedReasonEnum reason);
    Status SendFaultEvent(FaultStateEnum newFaultState);

    // ------------------------------------------------------------------
    // Get attribute methods
    StateEnum GetState() override;
    CHIP_ERROR SetState(StateEnum);

    SupplyStateEnum GetSupplyState() override;
    CHIP_ERROR SetSupplyState(SupplyStateEnum);

    FaultStateEnum GetFaultState() override;
    CHIP_ERROR SetFaultState(FaultStateEnum);

    DataModel::Nullable<uint32_t> GetChargingEnabledUntil() override;
    CHIP_ERROR SetChargingEnabledUntil(DataModel::Nullable<uint32_t>);

    DataModel::Nullable<uint32_t> GetDischargingEnabledUntil() override;
    CHIP_ERROR SetDischargingEnabledUntil(DataModel::Nullable<uint32_t>);

    int64_t GetCircuitCapacity() override;
    CHIP_ERROR SetCircuitCapacity(int64_t);

    int64_t GetMinimumChargeCurrent() override;
    CHIP_ERROR SetMinimumChargeCurrent(int64_t);

    int64_t GetMaximumChargeCurrent() override;
    CHIP_ERROR SetMaximumChargeCurrent(int64_t);

    int64_t GetMaximumDischargeCurrent() override;
    CHIP_ERROR SetMaximumDischargeCurrent(int64_t);

    int64_t GetUserMaximumChargeCurrent() override;
    CHIP_ERROR SetUserMaximumChargeCurrent(int64_t) override;

    uint32_t GetRandomizationDelayWindow() override;
    CHIP_ERROR SetRandomizationDelayWindow(uint32_t) override;

    /* PREF attributes */
    DataModel::Nullable<uint32_t> GetNextChargeStartTime() override;
    CHIP_ERROR SetNextChargeStartTime(DataModel::Nullable<uint32_t> newNextChargeStartTimeUtc);

    DataModel::Nullable<uint32_t> GetNextChargeTargetTime() override;
    CHIP_ERROR SetNextChargeTargetTime(DataModel::Nullable<uint32_t> newNextChargeTargetTimeUtc);

    DataModel::Nullable<int64_t> GetNextChargeRequiredEnergy() override;
    CHIP_ERROR SetNextChargeRequiredEnergy(DataModel::Nullable<int64_t> newNextChargeRequiredEnergyMilliWattH);

    DataModel::Nullable<Percent> GetNextChargeTargetSoC() override;
    CHIP_ERROR SetNextChargeTargetSoC(DataModel::Nullable<Percent> newValue);

    DataModel::Nullable<uint16_t> GetApproximateEVEfficiency() override;
    CHIP_ERROR SetApproximateEVEfficiency(DataModel::Nullable<uint16_t>) override;

    /* SOC attributes */
    DataModel::Nullable<Percent> GetStateOfCharge() override;
    DataModel::Nullable<int64_t> GetBatteryCapacity() override;
    /* PNC attributes*/
    DataModel::Nullable<CharSpan> GetVehicleID() override;
    /* Session SESS attributes */
    DataModel::Nullable<uint32_t> GetSessionID() override;
    DataModel::Nullable<uint32_t> GetSessionDuration() override;
    DataModel::Nullable<int64_t> GetSessionEnergyCharged() override;
    DataModel::Nullable<int64_t> GetSessionEnergyDischarged() override;

private:
    /* Constants */
    static constexpr int kDefaultMinChargeCurrent_mA                      = 6000;  /* 6A */
    static constexpr int kDefaultUserMaximumChargeCurrent_mA              = 80000; /* 80A */
    static constexpr int kDefaultRandomizationDelayWindow_sec             = 600;   /* 600s */
    static constexpr int kMaxVehicleIDBufSize                             = 32;
    static constexpr int kPeriodicCheckIntervalRealTimeClockNotSynced_sec = 30;

    /* private variables for controlling the hardware - these are not attributes */
    int64_t mMaxHardwareCurrentLimit                = 0; /* Hardware current limit in mA */
    int64_t mCableAssemblyCurrentLimit              = 0; /* Cable limit detected when cable is plugged in, in mA */
    int64_t mMaximumChargingCurrentLimitFromCommand = 0; /* Value of current maximum limit when charging enabled */
    int64_t mActualChargingCurrentLimit             = 0;
    StateEnum mHwState                              = StateEnum::kNotPluggedIn; /* Hardware state */

    /* Variables to hold State and SupplyState in case a fault is raised */
    StateEnum mStateBeforeFault             = StateEnum::kUnknownEnumValue;
    SupplyStateEnum mSupplyStateBeforeFault = SupplyStateEnum::kUnknownEnumValue;

    /* Callback related */
    EVSECallbackWrapper mCallbacks = { .handler = nullptr, .arg = 0 }; /* Wrapper to allow callbacks to be registered */
    Status NotifyApplicationCurrentLimitChange(int64_t maximumChargeCurrent);
    Status NotifyApplicationStateChange();
    Status NotifyApplicationChargingPreferencesChange();
    Status GetEVSEEnergyMeterValue(ChargingDischargingType meterType, int64_t & aMeterValue);

    /* Local State machine handling */
    Status CheckFaultOrDiagnostic();
    Status HandleStateMachineEvent(EVSEStateMachineEvent event);
    Status HandleEVPluggedInEvent();
    Status HandleEVNotDetectedEvent();
    Status HandleEVNoDemandEvent();
    Status HandleEVDemandEvent();
    Status HandleChargingEnabledEvent();
    Status HandleDischargingEnabledEvent();
    Status HandleDisabledEvent();
    Status HandleFaultRaised();
    Status HandleFaultCleared();

    /**
     * @brief Helper function to work out the charge limit based on conditions and settings
     */
    Status ComputeMaxChargeCurrentLimit();

    /**
     * @brief This checks if the charging or discharging needs to be disabled
     *
     * @params pointer to SystemLayer
     * @params pointer to EnergyEvseDelegate
     */
    static void EvseCheckTimerExpiry(System::Layer * systemLayer, void * delegate);

    /* Attributes */
    StateEnum mState             = StateEnum::kNotPluggedIn;
    SupplyStateEnum mSupplyState = SupplyStateEnum::kDisabled;
    FaultStateEnum mFaultState   = FaultStateEnum::kNoError;
    DataModel::Nullable<uint32_t> mChargingEnabledUntil;    // TODO Default to 0 to indicate disabled
    DataModel::Nullable<uint32_t> mDischargingEnabledUntil; // TODO Default to 0 to indicate disabled
    int64_t mCircuitCapacity           = 0;
    int64_t mMinimumChargeCurrent      = kDefaultMinChargeCurrent_mA;
    int64_t mMaximumChargeCurrent      = 0;
    int64_t mMaximumDischargeCurrent   = 0;
    int64_t mUserMaximumChargeCurrent  = kDefaultUserMaximumChargeCurrent_mA; // TODO update spec
    uint32_t mRandomizationDelayWindow = kDefaultRandomizationDelayWindow_sec;
    /* PREF attributes */
    DataModel::Nullable<uint32_t> mNextChargeStartTime;
    DataModel::Nullable<uint32_t> mNextChargeTargetTime;
    DataModel::Nullable<int64_t> mNextChargeRequiredEnergy;
    DataModel::Nullable<Percent> mNextChargeTargetSoC;
    DataModel::Nullable<uint16_t> mApproximateEVEfficiency;

    /* SOC attributes */
    DataModel::Nullable<Percent> mStateOfCharge;
    DataModel::Nullable<int64_t> mBatteryCapacity;

    /* PNC attributes*/
    DataModel::Nullable<CharSpan> mVehicleID;

    /* Session Object */
    EvseSession mSession = EvseSession();

    /* Helper variable to hold meter val since last EnergyTransferStarted event */
    int64_t mMeterValueAtEnergyTransferStart;

    /* Targets Delegate */
    EvseTargetsDelegate * mEvseTargetsDelegate = nullptr;
};

} // namespace EnergyEvse
} // namespace Clusters
} // namespace app
} // namespace chip
