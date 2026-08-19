/*
 *
 *    Copyright (c) 2020-2025 Project CHIP Authors
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

#include <app/DeviceProxy.h>
#include <clusters/IcdManagement/Commands.h>
#include <controller/CHIPDeviceController.h>

namespace chip {

namespace Testing {

class DeviceCommissionerTestAccess
{
public:
    DeviceCommissionerTestAccess() = delete;
    DeviceCommissionerTestAccess(Controller::DeviceCommissioner * commissioner) : mCommissioner(commissioner) {}

    CHIP_ERROR ParseICDInfo(Controller::ReadCommissioningInfo & info) { return mCommissioner->ParseICDInfo(info); }

    void SetAttributeCache(Platform::UniquePtr<app::ClusterStateCache> cache) { mCommissioner->mAttributeCache = std::move(cache); }

    void SetCommissioningStage(Controller::CommissioningStage stage) { mCommissioner->mCommissioningStage = stage; }

    void SetDeviceBeingCommissioned(DeviceProxy * device) { mCommissioner->mDeviceBeingCommissioned = device; }

    static void OnICDManagementRegisterClientResponse(
        Controller::DeviceCommissioner * commissioner,
        const app::Clusters::IcdManagement::Commands::RegisterClientResponse::DecodableType & data)
    {
        Controller::DeviceCommissioner::OnICDManagementRegisterClientResponse(commissioner, data);
    }

    static void
    OnICDManagementStayActiveResponse(Controller::DeviceCommissioner * commissioner,
                                      const app::Clusters::IcdManagement::Commands::StayActiveResponse::DecodableType & data)
    {
        Controller::DeviceCommissioner::OnICDManagementStayActiveResponse(commissioner, data);
    }

private:
    Controller::DeviceCommissioner * mCommissioner = nullptr;
};

} // namespace Testing
} // namespace chip
