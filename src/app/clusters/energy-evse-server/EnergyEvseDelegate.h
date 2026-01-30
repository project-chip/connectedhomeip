/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <app-common/zap-generated/cluster-objects.h>
#include <app/data-model/Nullable.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <protocols/interaction_model/StatusCode.h>

namespace chip {
namespace app {
namespace Clusters {
namespace EnergyEvse {

/** @brief
 *    Defines methods for implementing application-specific logic for the EVSE Management Cluster.
 */
class Instance;

class Delegate
{
public:
    virtual ~Delegate() = default;

    void SetEndpointId(EndpointId aEndpoint) { mEndpointId = aEndpoint; }
    EndpointId GetEndpointId() { return mEndpointId; }
    void SetInstance(Instance * aInstance) { mInstance = aInstance; }
    Instance * GetInstance() { return mInstance; }

    /**
     * @brief Delegate should implement a handler to disable the EVSE.
     * It should report Status::Success if successful and may
     * return other Status codes if it fails
     */
    virtual Protocols::InteractionModel::Status Disable() = 0;

    /**
     * @brief Delegate should implement a handler to enable EVSE Charging.
     * It should report Status::Success if successful and may
     * return other Status codes if it fails
     */
    virtual Protocols::InteractionModel::Status EnableCharging(const DataModel::Nullable<uint32_t> & enableChargeTime,
                                                               const int64_t & minimumChargeCurrent,
                                                               const int64_t & maximumChargeCurrent) = 0;

    /**
     * @brief Delegate should implement a handler to enable EVSE Discharging.
     * It should report Status::Success if successful and may
     * return other Status codes if it fails
     */
    virtual Protocols::InteractionModel::Status EnableDischarging(const DataModel::Nullable<uint32_t> & enableDischargeTime,
                                                                  const int64_t & maximumDischargeCurrent) = 0;

    /**
     * @brief Delegate should implement a handler to enable EVSE Diagnostics.
     * It should report Status::Success if successful and may
     * return other Status codes if it fails
     */
    virtual Protocols::InteractionModel::Status StartDiagnostics() = 0;

    /**
     * @brief Delegate should implement a handler for the SetTargets command.
     * It should report Status::Success if successful and may
     * return other Status codes if it fails
     */
    virtual Protocols::InteractionModel::Status
    SetTargets(const DataModel::DecodableList<Structs::ChargingTargetScheduleStruct::DecodableType> & chargingTargetSchedules) = 0;

    /**
     * @brief Delegate should implement a handler for LoadTargets
     *
     * This needs to load any stored targets into memory
     */
    virtual Protocols::InteractionModel::Status LoadTargets() = 0;

    /**
     * @brief Delegate should implement a handler for GetTargets
     *
     * @param[out]  The full targets structure
     */
    virtual Protocols::InteractionModel::Status
    GetTargets(DataModel::List<const Structs::ChargingTargetScheduleStruct::Type> & chargingTargetSchedules) = 0;

    /**
     * @brief Delegate should implement a handler for ClearTargets command.
     * It should report Status::Success if successful and may
     * return other Status codes if it fails
     */
    virtual Protocols::InteractionModel::Status ClearTargets() = 0;

    // ------------------------------------------------------------------
    // Get attribute methods
    virtual StateEnum GetState()                                       = 0;
    virtual SupplyStateEnum GetSupplyState()                           = 0;
    virtual FaultStateEnum GetFaultState()                             = 0;
    virtual DataModel::Nullable<uint32_t> GetChargingEnabledUntil()    = 0;
    virtual DataModel::Nullable<uint32_t> GetDischargingEnabledUntil() = 0;
    virtual int64_t GetCircuitCapacity()                               = 0;
    virtual int64_t GetMinimumChargeCurrent()                          = 0;
    virtual int64_t GetMaximumChargeCurrent()                          = 0;
    virtual int64_t GetMaximumDischargeCurrent()                       = 0;
    virtual int64_t GetUserMaximumChargeCurrent()                      = 0;
    virtual uint32_t GetRandomizationDelayWindow()                     = 0;
    /* PREF attributes */
    virtual DataModel::Nullable<uint32_t> GetNextChargeStartTime()     = 0;
    virtual DataModel::Nullable<uint32_t> GetNextChargeTargetTime()    = 0;
    virtual DataModel::Nullable<int64_t> GetNextChargeRequiredEnergy() = 0;
    virtual DataModel::Nullable<Percent> GetNextChargeTargetSoC()      = 0;
    virtual DataModel::Nullable<uint16_t> GetApproximateEVEfficiency() = 0;

    /* SOC attributes */
    virtual DataModel::Nullable<Percent> GetStateOfCharge()   = 0;
    virtual DataModel::Nullable<int64_t> GetBatteryCapacity() = 0;

    /* PNC attributes*/
    virtual DataModel::Nullable<CharSpan> GetVehicleID() = 0;

    /* Session SESS attributes */
    virtual DataModel::Nullable<uint32_t> GetSessionID()              = 0;
    virtual DataModel::Nullable<uint32_t> GetSessionDuration()        = 0;
    virtual DataModel::Nullable<int64_t> GetSessionEnergyCharged()    = 0;
    virtual DataModel::Nullable<int64_t> GetSessionEnergyDischarged() = 0;

    // ------------------------------------------------------------------
    // Set attribute methods
    virtual CHIP_ERROR SetUserMaximumChargeCurrent(int64_t aNewValue)                      = 0;
    virtual CHIP_ERROR SetRandomizationDelayWindow(uint32_t aNewValue)                     = 0;
    virtual CHIP_ERROR SetApproximateEVEfficiency(DataModel::Nullable<uint16_t> aNewValue) = 0;

protected:
    EndpointId mEndpointId = 0;
    Instance * mInstance   = nullptr;
};

} // namespace EnergyEvse
} // namespace Clusters
} // namespace app
} // namespace chip
