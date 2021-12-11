/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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
#include <controller/CommissioneeDeviceProxy.h>
#include <controller/CommissioningDelegate.h>
#include <protocols/secure_channel/RendezvousParameters.h>

namespace chip {
namespace Controller {

class DeviceCommissioner;

class AutoCommissioner : public CommissioningDelegate
{
public:
    AutoCommissioner(DeviceCommissioner * commissioner) : mCommissioner(commissioner) {}
    void SetCommissioningParameters(CommissioningParameters & params) { mParams = params; }
    void StartCommissioning(CommissioneeDeviceProxy * proxy);

    void CommissioningStepFinished(CHIP_ERROR err, CommissioningDelegate::CommissioningReport report) override;

private:
    CommissioningStage GetNextCommissioningStage(CommissioningStage currentStage);
    DeviceCommissioner * mCommissioner;
    CommissioneeDeviceProxy * mCommissioneeDeviceProxy = nullptr;
    OperationalDeviceProxy * mOperationalDeviceProxy   = nullptr;
    CommissioningParameters mParams                    = CommissioningParameters();
};

} // namespace Controller
} // namespace chip
