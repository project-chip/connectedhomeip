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

#include <EVSECallbacks.h>
#include <EnergyEvseTargetsStore.h>
#include <EvseTargetsConfig.h>
#include <app/clusters/energy-evse-server/CodegenIntegration.h>
#include <app/clusters/energy-evse-server/EnergyEvseCluster.h>

#include <app/util/config.h>
#include <cstring>

using chip::Protocols::InteractionModel::Status;

namespace chip {
namespace app {
namespace Clusters {
namespace EnergyEvse {

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
 * Helper class to handle session timing and energy meter deltas.
 * Session attribute values are stored in the cluster (EnergyEvseCluster) and
 * updated through Instance setters. This class only tracks the internal
 * computation state (start time, energy meter baselines).
 */
class EvseSession
{
public:
    EvseSession() {}

    /**
     * @brief Start a new session: assigns a session ID, resets duration/energy counters.
     *
     * @param instance              - The cluster Instance to update attributes on
     * @param chargingMeterValue    - The current value of the energy meter (charging) in mWh
     * @param dischargingMeterValue - The current value of the energy meter (discharging) in mWh
     */
    void StartSession(Instance * instance, int64_t chargingMeterValue, int64_t dischargingMeterValue);

    /**
     * @brief Stop the current session: recalculates duration and energy values.
     *
     * @param instance              - The cluster Instance to update attributes on
     * @param chargingMeterValue    - The current value of the energy meter (charging) in mWh
     * @param dischargingMeterValue - The current value of the energy meter (discharging) in mWh
     */
    void StopSession(Instance * instance, int64_t chargingMeterValue, int64_t dischargingMeterValue);

    /**
     * @brief Recalculate session duration from start time to now.
     *
     * @param instance - The cluster Instance to update attributes on
     */
    void RecalculateSessionDuration(Instance * instance);

    /**
     * @brief Update the session's charged energy delta.
     *
     * @param instance           - The cluster Instance to update attributes on
     * @param chargingMeterValue - The current value of the energy meter (charging) in mWh
     */
    void UpdateEnergyCharged(Instance * instance, int64_t chargingMeterValue);

    /**
     * @brief Update the session's discharged energy delta.
     *
     * @param instance              - The cluster Instance to update attributes on
     * @param dischargingMeterValue - The current value of the energy meter (discharging) in mWh
     */
    void UpdateEnergyDischarged(Instance * instance, int64_t dischargingMeterValue);

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
    ~EnergyEvseDelegate() { CancelActiveTimers(); }

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
    void CancelActiveTimers();

    /**
     * @brief   Helper function to handle timer expiration when in enabled state
     * @param matterEpoch Current time in Matter epoch seconds
     */
    void HandleEnabledStateExpiration(uint32_t matterEpochSeconds);

    /**
     * @brief   Helper function to get know if the EV is plugged in based on state
     *          (regardless of if it is actually transferring energy)
     */
    bool IsEvsePluggedIn();

    // -----------------------------------------------------------------
    // Internal API to allow an EVSE to change its internal state etc
    Status HwSetMaxHardwareChargeCurrentLimit(int64_t currentmA);
    int64_t HwGetMaxHardwareChargeCurrentLimit() { return mMaxHardwareChargeCurrentLimit; }
    Status HwSetMaxHardwareDischargeCurrentLimit(int64_t currentmA);
    int64_t HwGetMaxHardwareDischargeCurrentLimit() { return mMaxHardwareDischargeCurrentLimit; }
    Status HwSetNominalMainsVoltage(int64_t voltage_mV);
    int64_t HwGetNominalMainsVoltage() { return mNominalMainsVoltage; }
    Status HwSetCircuitCapacity(int64_t currentmA);
    Status HwSetCableAssemblyLimit(int64_t currentmA);
    int64_t HwGetCableAssemblyLimit() { return mCableAssemblyCurrentLimit; }
    Status HwSetState(StateEnum state);
    StateEnum HwGetState() { return mHwState; };
    Status HwSetFault(FaultStateEnum fault);
    Status HwSetRFID(ByteSpan uid);
    Status HwSetVehicleID(const CharSpan & vehID);
    CHIP_ERROR HwGetVehicleID(DataModel::Nullable<MutableCharSpan> & outValue);
    Status HwDiagnosticsComplete();
    Status SendEVConnectedEvent();
    Status SendEVNotDetectedEvent();
    Status SendEnergyTransferStartedEvent();
    Status SendEnergyTransferStoppedEvent(EnergyTransferStoppedReasonEnum reason);
    Status SendFaultEvent(FaultStateEnum newFaultState);

    // ------------------------------------------------------------------
    // Attribute methods - called by cluster to propagate attribute changes to the delegate
    void OnStateChanged(StateEnum newValue) override;
    void OnSupplyStateChanged(SupplyStateEnum newValue) override;
    void OnFaultStateChanged(FaultStateEnum newValue) override;
    void OnChargingEnabledUntilChanged(DataModel::Nullable<uint32_t> newValue) override;
    void OnDischargingEnabledUntilChanged(DataModel::Nullable<uint32_t> newValue) override;
    void OnCircuitCapacityChanged(int64_t newValue) override;
    void OnMinimumChargeCurrentChanged(int64_t newValue) override;
    void OnMaximumChargeCurrentChanged(int64_t newValue) override;
    void OnMaximumDischargeCurrentChanged(int64_t newValue) override;
    void OnUserMaximumChargeCurrentChanged(int64_t newValue) override;
    void OnRandomizationDelayWindowChanged(uint32_t newValue) override;
    void OnNextChargeStartTimeChanged(DataModel::Nullable<uint32_t> newValue) override;
    void OnNextChargeTargetTimeChanged(DataModel::Nullable<uint32_t> newValue) override;
    void OnNextChargeRequiredEnergyChanged(DataModel::Nullable<int64_t> newValue) override;
    void OnNextChargeTargetSoCChanged(DataModel::Nullable<Percent> newValue) override;
    void OnApproximateEVEfficiencyChanged(DataModel::Nullable<uint16_t> newValue) override;
    void OnStateOfChargeChanged(DataModel::Nullable<Percent> newValue) override;
    void OnBatteryCapacityChanged(DataModel::Nullable<int64_t> newValue) override;
    void OnVehicleIDChanged(DataModel::Nullable<CharSpan> newValue) override;
    void OnSessionIDChanged(DataModel::Nullable<uint32_t> newValue) override;
    void OnSessionDurationChanged(DataModel::Nullable<uint32_t> newValue) override;
    void OnSessionEnergyChargedChanged(DataModel::Nullable<int64_t> newValue) override;
    void OnSessionEnergyDischargedChanged(DataModel::Nullable<int64_t> newValue) override;

    // ------------------------------------------------------------------
    // Instance management for codegen integration
    void SetInstance(Instance * aInstance) { mInstance = aInstance; }
    Instance * GetInstance() { return mInstance; }

    // ------------------------------------------------------------------
    // Local getters for internal delegate use - delegates to cluster instance
    StateEnum GetState() const;
    SupplyStateEnum GetSupplyState() const;
    FaultStateEnum GetFaultState() const;
    DataModel::Nullable<uint32_t> GetChargingEnabledUntil() const;
    DataModel::Nullable<uint32_t> GetDischargingEnabledUntil() const;
    int64_t GetCircuitCapacity() const;
    int64_t GetMinimumChargeCurrent() const;
    int64_t GetMaximumChargeCurrent() const;
    int64_t GetMaximumDischargeCurrent() const;
    int64_t GetUserMaximumChargeCurrent() const;
    uint32_t GetRandomizationDelayWindow() const;
    DataModel::Nullable<uint32_t> GetNextChargeStartTime() const;
    DataModel::Nullable<uint32_t> GetNextChargeTargetTime() const;
    DataModel::Nullable<int64_t> GetNextChargeRequiredEnergy() const;
    DataModel::Nullable<Percent> GetNextChargeTargetSoC() const;
    DataModel::Nullable<uint16_t> GetApproximateEVEfficiency() const;
    DataModel::Nullable<Percent> GetStateOfCharge() const;
    DataModel::Nullable<int64_t> GetBatteryCapacity() const;
    DataModel::Nullable<CharSpan> GetVehicleID() const;
    DataModel::Nullable<uint32_t> GetSessionID() const;
    DataModel::Nullable<uint32_t> GetSessionDuration() const;
    DataModel::Nullable<int64_t> GetSessionEnergyCharged() const;
    DataModel::Nullable<int64_t> GetSessionEnergyDischarged() const;

private:
    /* Constants */
    static constexpr int kDefaultMinChargeCurrent_mA                      = 6000;  /* 6A */
    static constexpr int kDefaultUserMaximumChargeCurrent_mA              = 80000; /* 80A */
    static constexpr int kDefaultRandomizationDelayWindow_sec             = 600;   /* 600s */
    static constexpr int kMaxVehicleIDBufSize                             = 32;
    static constexpr int kPeriodicCheckIntervalRealTimeClockNotSynced_sec = 30;

    /* private variables for controlling the hardware - these are not attributes */
    int64_t mMaxHardwareChargeCurrentLimit             = 0; /* Hardware current limit in mA for charging */
    int64_t mMaxHardwareDischargeCurrentLimit          = 0; /* Hardware current limit in mA for discharging */
    int64_t mCableAssemblyCurrentLimit                 = 0; /* Cable limit detected when cable is plugged in, in mA */
    int64_t mMaximumChargingCurrentLimitFromCommand    = 0; /* Value of current maximum limit when charging enabled */
    int64_t mActualChargingCurrentLimit                = 0;
    int64_t mMaximumDischargingCurrentLimitFromCommand = 0; /* Value of current maximum limit when discharging enabled */
    int64_t mActualDischargingCurrentLimit             = 0;
    int64_t mNominalMainsVoltage                       = 230000; /* Assume a sensible default mains voltage (mV) */

    StateEnum mHwState = StateEnum::kNotPluggedIn; /* Hardware state */

    /* Variables to hold State and SupplyState in case a fault is raised */
    StateEnum mStateBeforeFault             = StateEnum::kUnknownEnumValue;
    SupplyStateEnum mSupplyStateBeforeFault = SupplyStateEnum::kUnknownEnumValue;

    /* Callback related */
    EVSECallbackWrapper mCallbacks = { .handler = nullptr, .arg = 0 }; /* Wrapper to allow callbacks to be registered */
    Status NotifyApplicationChargeCurrentLimitChange(int64_t maximumChargeCurrent);
    Status NotifyApplicationDischargeCurrentLimitChange(int64_t maximumDischargeCurrent);
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
     * @brief Helper functions to work out the charge & discharge limits based on conditions and settings
     */
    Status ComputeMaxChargeCurrentLimit();
    Status ComputeMaxDischargeCurrentLimit();

    /**
     * @brief This checks if the charging or discharging needs to be disabled
     *
     * @params pointer to SystemLayer
     * @params pointer to EnergyEvseDelegate
     */
    static void EvseCheckTimerExpiry(System::Layer * systemLayer, void * delegate);

    /* Instance pointer for accessing cluster */
    Instance * mInstance = nullptr;

    /* Session Object - delegate owns session state management */
    EvseSession mSession = EvseSession();

    /* Helper variables to hold meter val since last EnergyTransferStarted event */
    int64_t mImportedMeterValueAtEnergyTransferStart;
    int64_t mExportedMeterValueAtEnergyTransferStart;

    /* VehicleID buffer for delegate use */
    char mVehicleIDBuf[kMaxVehicleIDBufSize];

    /* Targets Delegate */
    EvseTargetsDelegate * mEvseTargetsDelegate = nullptr;
};

} // namespace EnergyEvse
} // namespace Clusters
} // namespace app
} // namespace chip
