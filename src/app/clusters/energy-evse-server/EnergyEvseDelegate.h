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

class Delegate
{
public:
    virtual ~Delegate() = default;

    void SetEndpointId(EndpointId aEndpoint) { mEndpointId = aEndpoint; }
    EndpointId GetEndpointId() { return mEndpointId; }

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
    // Attribute change callbacks - called by cluster after attribute is updated
    // These allow the delegate to react to attribute changes (e.g., persist values, update app state)
    virtual void OnStateChanged(StateEnum newValue)                                       = 0;
    virtual void OnSupplyStateChanged(SupplyStateEnum newValue)                           = 0;
    virtual void OnFaultStateChanged(FaultStateEnum newValue)                             = 0;
    virtual void OnChargingEnabledUntilChanged(DataModel::Nullable<uint32_t> newValue)    = 0;
    virtual void OnDischargingEnabledUntilChanged(DataModel::Nullable<uint32_t> newValue) = 0;
    virtual void OnCircuitCapacityChanged(int64_t newValue)                               = 0;
    virtual void OnMinimumChargeCurrentChanged(int64_t newValue)                          = 0;
    virtual void OnMaximumChargeCurrentChanged(int64_t newValue)                          = 0;
    virtual void OnMaximumDischargeCurrentChanged(int64_t newValue)                       = 0;
    virtual void OnUserMaximumChargeCurrentChanged(int64_t newValue)                      = 0;
    virtual void OnRandomizationDelayWindowChanged(uint32_t newValue)                     = 0;
    virtual void OnNextChargeStartTimeChanged(DataModel::Nullable<uint32_t> newValue)     = 0;
    virtual void OnNextChargeTargetTimeChanged(DataModel::Nullable<uint32_t> newValue)    = 0;
    virtual void OnNextChargeRequiredEnergyChanged(DataModel::Nullable<int64_t> newValue) = 0;
    virtual void OnNextChargeTargetSoCChanged(DataModel::Nullable<Percent> newValue)      = 0;
    virtual void OnApproximateEVEfficiencyChanged(DataModel::Nullable<uint16_t> newValue) = 0;
    virtual void OnStateOfChargeChanged(DataModel::Nullable<Percent> newValue)            = 0;
    virtual void OnBatteryCapacityChanged(DataModel::Nullable<int64_t> newValue)          = 0;
    virtual void OnVehicleIDChanged(DataModel::Nullable<CharSpan> newValue)               = 0;
    virtual void OnSessionIDChanged(DataModel::Nullable<uint32_t> newValue)               = 0;
    virtual void OnSessionDurationChanged(DataModel::Nullable<uint32_t> newValue)         = 0;
    virtual void OnSessionEnergyChargedChanged(DataModel::Nullable<int64_t> newValue)     = 0;
    virtual void OnSessionEnergyDischargedChanged(DataModel::Nullable<int64_t> newValue)  = 0;

protected:
    EndpointId mEndpointId = 0;
};

} // namespace EnergyEvse
} // namespace Clusters
} // namespace app
} // namespace chip
