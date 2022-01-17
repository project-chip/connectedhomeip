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
    CHIP_ERROR SetCommissioningParameters(const CommissioningParameters & params);
    void StartCommissioning(CommissioneeDeviceProxy * proxy);

    void CommissioningStepFinished(CHIP_ERROR err, CommissioningDelegate::CommissioningReport report) override;

private:
    CommissioningStage GetNextCommissioningStage(CommissioningStage currentStage, CHIP_ERROR lastErr);
    Optional<System::Clock::Timeout> GetCommandTimeout(CommissioningStage stage);

    DeviceCommissioner * mCommissioner;
    CommissioneeDeviceProxy * mCommissioneeDeviceProxy = nullptr;
    OperationalDeviceProxy * mOperationalDeviceProxy   = nullptr;
    CommissioningParameters mParams                    = CommissioningParameters();
    // Memory space for the commisisoning parameters that come in as ByteSpans - the caller is not guaranteed to retain this memory
    // TODO(cecille): Include memory from CommissioneeDeviceProxy once BLE is moved over
    uint8_t mSsid[CommissioningParameters::kMaxSsidLen];
    uint8_t mCredentials[CommissioningParameters::kMaxCredentialsLen];
    uint8_t mThreadOperationalDataset[CommissioningParameters::kMaxThreadDatasetLen];
};

} // namespace Controller
} // namespace chip
