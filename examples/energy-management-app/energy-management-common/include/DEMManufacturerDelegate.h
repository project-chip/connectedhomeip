/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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
#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace DeviceEnergyManagement {

/**
 * Class to abstract manufacturer specific functionality
 */
class DEMManufacturerDelegate
{
public:
    DEMManufacturerDelegate() {}

    virtual ~DEMManufacturerDelegate() {}

    // The PowerAdjustEnd event needs to report the approximate energy used by the ESA during the session.
    virtual int64_t GetApproxEnergyDuringSession() = 0;

    virtual CHIP_ERROR HandleDeviceEnergyManagementPowerAdjustRequest(const int64_t powerMw, const uint32_t durationS,
                                                                      AdjustmentCauseEnum cause)
    {
        return CHIP_NO_ERROR;
    }

    virtual CHIP_ERROR HandleDeviceEnergyManagementPowerAdjustCompletion() { return CHIP_NO_ERROR; }

    virtual CHIP_ERROR HandleDeviceEnergyManagementCancelPowerAdjustRequest(CauseEnum cause) { return CHIP_NO_ERROR; }

    virtual CHIP_ERROR HandleDeviceEnergyManagementStartTimeAdjustRequest(const uint32_t requestedStartTimeUtc,
                                                                          AdjustmentCauseEnum cause)
    {
        return CHIP_NO_ERROR;
    }

    virtual CHIP_ERROR HandleDeviceEnergyManagementPauseRequest(const uint32_t durationS, AdjustmentCauseEnum cause)
    {
        return CHIP_NO_ERROR;
    }

    virtual CHIP_ERROR HandleDeviceEnergyManagementPauseCompletion() { return CHIP_NO_ERROR; }

    virtual CHIP_ERROR HandleDeviceEnergyManagementCancelPauseRequest(CauseEnum cause) { return CHIP_NO_ERROR; }

    virtual CHIP_ERROR HandleDeviceEnergyManagementCancelRequest() { return CHIP_NO_ERROR; }

    virtual CHIP_ERROR
    HandleModifyForecastRequest(const uint32_t forecastID,
                                const DataModel::DecodableList<Structs::SlotAdjustmentStruct::DecodableType> & slotAdjustments,
                                AdjustmentCauseEnum cause)
    {
        return CHIP_NO_ERROR;
    }

    virtual CHIP_ERROR RequestConstraintBasedForecast(
        const DataModel::DecodableList<DeviceEnergyManagement::Structs::ConstraintsStruct::DecodableType> & constraints,
        AdjustmentCauseEnum cause)
    {
        return CHIP_NO_ERROR;
    }
};

} // namespace DeviceEnergyManagement
} // namespace Clusters
} // namespace app
} // namespace chip
