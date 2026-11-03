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

#include <app/clusters/energy-evse-server/Constants.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app/data-model/DecodableList.h>
#include <app/data-model/List.h>
#include <lib/core/CHIPError.h>
#include <lib/support/BitMask.h>

namespace chip {
namespace app {
namespace Clusters {
namespace EnergyEvse {

/**
 * @brief Abstraction for computing NextCharge* attributes from charging targets and EVSE state.
 *
 * Implementations supply targets, plug/supply state, SoC/battery reads, and cluster attribute writes.
 */
class EvseChargingScheduleContext
{
public:
    virtual ~EvseChargingScheduleContext() = default;

    virtual const DataModel::List<const Structs::ChargingTargetScheduleStruct::Type> & GetChargingTargets() const = 0;

    virtual bool IsEvsePluggedIn() const                                              = 0;
    virtual SupplyStateEnum GetSupplyState() const                                    = 0;
    virtual bool HasSoCReportingFeature() const                                       = 0;
    virtual DataModel::Nullable<Percent> GetStateOfCharge() const                     = 0;
    virtual DataModel::Nullable<int64_t> GetBatteryCapacity() const                   = 0;
    virtual int64_t HwGetNominalMainsVoltage() const                                    = 0;
    virtual int64_t GetMaximumChargeCurrent() const                                     = 0;

    virtual CHIP_ERROR SetNextChargeStartTime(DataModel::Nullable<uint32_t> value)     = 0;
    virtual CHIP_ERROR SetNextChargeTargetTime(DataModel::Nullable<uint32_t> value)    = 0;
    virtual CHIP_ERROR SetNextChargeRequiredEnergy(DataModel::Nullable<int64_t> value)   = 0;
    virtual CHIP_ERROR SetNextChargeTargetSoC(DataModel::Nullable<Percent> value)        = 0;
};

/**
 * @brief Recomputes NextChargeStartTime, NextChargeTargetTime, NextChargeRequiredEnergy, and NextChargeTargetSoC.
 */
CHIP_ERROR ComputeEvseChargingSchedule(EvseChargingScheduleContext & context);

} // namespace EnergyEvse
} // namespace Clusters
} // namespace app
} // namespace chip
