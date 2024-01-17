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

#include "app/clusters/device-energy-management-server/device-energy-management-server.h"

#include <app/util/af.h>
#include <app/util/config.h>
#include <cstring>

using chip::Protocols::InteractionModel::Status;
namespace chip {
namespace app {
namespace Clusters {
namespace DeviceEnergyManagement {

/**
 * The application delegate.
 */
class DeviceEnergyManagementDelegate : public DeviceEnergyManagement::Delegate
{
public:
    virtual Status PowerAdjustRequest(const int64_t power, const uint32_t duration, AdjustmentCauseEnum cause) override;
    virtual Status CancelPowerAdjustRequest() override;
    virtual Status StartTimeAdjustRequest(const uint32_t requestedStartTime, AdjustmentCauseEnum cause) override;
    virtual Status PauseRequest(const uint32_t duration, AdjustmentCauseEnum cause) override;
    virtual Status ResumeRequest() override;
    virtual Status
    ModifyForecastRequest(const uint32_t forecastId,
                          const DataModel::DecodableList<Structs::SlotAdjustmentStruct::DecodableType> & slotAdjustments,
                          AdjustmentCauseEnum cause) override;
    virtual Status
    RequestConstraintBasedForecast(const DataModel::DecodableList<Structs::ConstraintsStruct::DecodableType> & constraints,
                                   AdjustmentCauseEnum cause) override;
    virtual Status CancelRequest() override;

    // ------------------------------------------------------------------
    // Get attribute methods
    virtual ESATypeEnum GetESAType() override;
    virtual bool GetESACanGenerate() override;
    virtual ESAStateEnum GetESAState() override;
    virtual int64_t GetAbsMinPower() override;
    virtual int64_t GetAbsMaxPower() override;
    virtual Attributes::PowerAdjustmentCapability::TypeInfo::Type GetPowerAdjustmentCapability() override;
    virtual DataModel::Nullable<Structs::ForecastStruct::Type> GetForecast() override;
    virtual OptOutStateEnum GetOptOutState() override;

    // ------------------------------------------------------------------
    // Set attribute methods
    virtual CHIP_ERROR SetESAType(ESATypeEnum) override;
    virtual CHIP_ERROR SetESACanGenerate(bool) override;
    virtual CHIP_ERROR SetESAState(ESAStateEnum) override;
    virtual CHIP_ERROR SetAbsMinPower(int64_t) override;
    virtual CHIP_ERROR SetAbsMaxPower(int64_t) override;
    virtual CHIP_ERROR SetPowerAdjustmentCapability(Attributes::PowerAdjustmentCapability::TypeInfo::Type) override;
    virtual CHIP_ERROR SetForecast(DataModel::Nullable<Structs::ForecastStruct::Type>) override;

private:
    ESATypeEnum mEsaType;
    bool mEsaCanGenerate;
    ESAStateEnum mEsaState;
    int64_t mAbsMinPower;
    int64_t mAbsMaxPower;
    Attributes::PowerAdjustmentCapability::TypeInfo::Type mPowerAdjustmentCapability;
    DataModel::Nullable<Structs::ForecastStruct::Type> mForecast;
    // Default to NoOptOut
    OptOutStateEnum mOptOutState = OptOutStateEnum::kNoOptOut;
};

} // namespace DeviceEnergyManagement
} // namespace Clusters
} // namespace app
} // namespace chip
