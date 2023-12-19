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

#pragma once

#include "app/clusters/energy-evse-server/energy-evse-server.h"
#include <EVSECallbacks.h>

#include <app/util/af.h>
#include <app/util/config.h>
#include <cstring>

using chip::Protocols::InteractionModel::Status;
namespace chip {
namespace app {
namespace Clusters {
namespace EnergyEvse {

/**
 * The application delegate.
 */

class EnergyEvseDelegate : public EnergyEvse::Delegate
{
public:
    ~EnergyEvseDelegate();

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
     * @brief    Called by EVSE Hardware to register a single callback handler
     */
    Status HwRegisterEvseCallbackHandler(EVSECallbackFunc handler, intptr_t arg);

    // -----------------------------------------------------------------
    // Internal API to allow an EVSE to change its internal state etc
    Status HwSetMaxHardwareCurrentLimit(int64_t currentmA);
    Status HwSetCircuitCapacity(int64_t currentmA);
    Status HwSetCableAssemblyLimit(int64_t currentmA);
    Status HwSetState(StateEnum state);
    Status HwSetFault(FaultStateEnum fault);
    Status HwSetRFID(ByteSpan uid);
    Status HwSetVehicleID(const CharSpan & vehID);

    // ------------------------------------------------------------------
    // Get attribute methods
    StateEnum GetState() override;
    CHIP_ERROR SetState(StateEnum);

    SupplyStateEnum GetSupplyState() override;
    CHIP_ERROR SetSupplyState(SupplyStateEnum);

    FaultStateEnum GetFaultState() override;
    CHIP_ERROR SetFaultState(FaultStateEnum);

    DataModel::Nullable<uint32_t> GetChargingEnabledUntil() override;
    CHIP_ERROR SetChargingEnabledUntil(uint32_t);

    DataModel::Nullable<uint32_t> GetDischargingEnabledUntil() override;
    CHIP_ERROR SetDischargingEnabledUntil(uint32_t);

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
    uint8_t GetNumberOfWeeklyTargets() override;
    uint8_t GetNumberOfDailyTargets() override;
    DataModel::Nullable<uint32_t> GetNextChargeStartTime() override;
    DataModel::Nullable<uint32_t> GetNextChargeTargetTime() override;
    DataModel::Nullable<int64_t> GetNextChargeRequiredEnergy() override;
    DataModel::Nullable<Percent> GetNextChargeTargetSoC() override;

    DataModel::Nullable<uint16_t> GetApproximateEVEfficiency() override;
    CHIP_ERROR SetApproximateEVEfficiency(uint16_t) override;

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
    static constexpr int DEFAULT_MIN_CHARGE_CURRENT          = 6000;                  /* 6A */
    static constexpr int DEFAULT_USER_MAXIMUM_CHARGE_CURRENT = kMaximumChargeCurrent; /* 80A */
    static constexpr int DEFAULT_RANDOMIZATION_DELAY_WINDOW  = 600;                   /* 600s */
    static constexpr int kMaxVehicleIDBufSize                = 32;

    /* private variables for controlling the hardware - these are not attributes */
    int64_t mMaxHardwareCurrentLimit                = 0; /* Hardware current limit in mA */
    int64_t mCableAssemblyCurrentLimit              = 0; /* Cable limit detected when cable is plugged in, in mA */
    int64_t mMaximumChargingCurrentLimitFromCommand = 0; /* Value of current maximum limit when charging enabled */
    int64_t mActualChargingCurrentLimit             = 0;
    StateEnum mHwState                              = StateEnum::kNotPluggedIn; /* Hardware state */

    /* Callback related */
    EVSECallbackWrapper mCallbacks = { .handler = nullptr, .arg = 0 }; /* Wrapper to allow callbacks to be registered */
    Status NotifyApplicationCurrentLimitChange(int64_t maximumChargeCurrent);
    Status NotifyApplicationStateChange();

    /**
     * @brief Helper function to work out the charge limit based on conditions and settings
     */
    Status ComputeMaxChargeCurrentLimit();

    /* Attributes */
    StateEnum mState             = StateEnum::kNotPluggedIn;
    SupplyStateEnum mSupplyState = SupplyStateEnum::kDisabled;
    FaultStateEnum mFaultState   = FaultStateEnum::kNoError;
    DataModel::Nullable<uint32_t> mChargingEnabledUntil;    // TODO Default to 0 to indicate disabled
    DataModel::Nullable<uint32_t> mDischargingEnabledUntil; // TODO Default to 0 to indicate disabled
    int64_t mCircuitCapacity           = 0;
    int64_t mMinimumChargeCurrent      = DEFAULT_MIN_CHARGE_CURRENT;
    int64_t mMaximumChargeCurrent      = 0;
    int64_t mMaximumDischargeCurrent   = 0;
    int64_t mUserMaximumChargeCurrent  = DEFAULT_USER_MAXIMUM_CHARGE_CURRENT; // TODO update spec
    uint32_t mRandomizationDelayWindow = DEFAULT_RANDOMIZATION_DELAY_WINDOW;
    /* PREF attributes */
    uint8_t mNumberOfWeeklyTargets = 0;
    uint8_t mNumberOfDailyTargets  = 1;
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

    /* Session SESS attributes */
    DataModel::Nullable<uint32_t> mSessionID;
    DataModel::Nullable<uint32_t> mSessionDuration;
    DataModel::Nullable<int64_t> mSessionEnergyCharged;
    DataModel::Nullable<int64_t> mSessionEnergyDischarged;
};

} // namespace EnergyEvse
} // namespace Clusters
} // namespace app
} // namespace chip
