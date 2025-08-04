/*
 *
 *    Copyright (c) 2019 Google LLC.
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

#include "AppEvent.h"
#include <DEMManufacturerDelegate.h>
#include <DeviceEnergyManagementManager.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app/clusters/operational-state-server/operational-state-server.h>
#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace DeviceEnergyManagement {

class DishwasherManager : public DEMManufacturerDelegate
{
    using OperationalStateEnum = chip::app::Clusters::OperationalState::OperationalStateEnum;

public:
    static constexpr int64_t kApproximateEnergyUsedByESA = 1800'000; // mW

    DishwasherManager(DeviceEnergyManagementManager * aDEMInstance) : mDEMInstance(aDEMInstance) {}
    virtual ~DishwasherManager() {}

    CHIP_ERROR Init();
    void CycleOperationalState();
    void UpdateDishwasherLed();
    void UpdateOperationState(OperationalStateEnum state);
    OperationalStateEnum GetOperationalState();

    typedef void (*Callback_fn_initiated)(OperationalStateEnum action);
    typedef void (*Callback_fn_completed)();
    void SetCallbacks(Callback_fn_initiated aActionInitiated_CB, Callback_fn_completed aActionCompleted_CB);

    DeviceEnergyManagementDelegate * GetDEMDelegate()
    {
        if (mDEMInstance)
        {
            return mDEMInstance->GetDelegate();
        }
        return nullptr;
    }

    // The PowerAdjustEnd event needs to report the approximate energy used by the ESA during the session.
    int64_t GetApproxEnergyDuringSession() override;

    CHIP_ERROR HandleDeviceEnergyManagementPowerAdjustRequest(const int64_t powerMw, const uint32_t durationS,
                                                              AdjustmentCauseEnum cause) override;
    CHIP_ERROR HandleDeviceEnergyManagementPowerAdjustCompletion() override;

    CHIP_ERROR HandleDeviceEnergyManagementCancelPowerAdjustRequest(CauseEnum cause) override;
    CHIP_ERROR HandleDeviceEnergyManagementStartTimeAdjustRequest(const uint32_t requestedStartTimeUtc,
                                                                  AdjustmentCauseEnum cause) override;
    CHIP_ERROR HandleDeviceEnergyManagementPauseRequest(const uint32_t durationS, AdjustmentCauseEnum cause) override;
    CHIP_ERROR HandleDeviceEnergyManagementPauseCompletion() override;
    CHIP_ERROR HandleDeviceEnergyManagementCancelPauseRequest(CauseEnum cause) override;
    CHIP_ERROR HandleDeviceEnergyManagementCancelRequest() override;
    CHIP_ERROR
    HandleModifyForecastRequest(const uint32_t forecastID,
                                const DataModel::DecodableList<Structs::SlotAdjustmentStruct::DecodableType> & slotAdjustments,
                                AdjustmentCauseEnum cause) override;
    CHIP_ERROR RequestConstraintBasedForecast(
        const DataModel::DecodableList<DeviceEnergyManagement::Structs::ConstraintsStruct::DecodableType> & constraints,
        AdjustmentCauseEnum cause) override;

private:
    OperationalStateEnum mState;

    Callback_fn_initiated mActionInitiated_CB;
    Callback_fn_completed mActionCompleted_CB;

    // DEM Manufacturer Delegate
    DeviceEnergyManagementManager * mDEMInstance;
};

DishwasherManager * GetDishwasherManager();

} // namespace DeviceEnergyManagement
} // namespace Clusters
} // namespace app
} // namespace chip
