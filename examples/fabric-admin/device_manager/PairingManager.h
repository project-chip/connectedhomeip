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

#include <controller/CHIPDeviceController.h>
#include <controller/CommissioningWindowOpener.h>
#include <crypto/CHIPCryptoPAL.h>

class CommissioningWindowDelegate
{
public:
    virtual void OnCommissioningWindowOpened(chip::NodeId deviceId, CHIP_ERROR err, chip::SetupPayload payload) = 0;
    virtual ~CommissioningWindowDelegate()                                                                      = default;
};

class PairingManager
{
public:
    static PairingManager & Instance()
    {
        static PairingManager instance;
        return instance;
    }

    CHIP_ERROR Init(chip::Controller::DeviceCommissioner * commissioner);

    CHIP_ERROR OpenCommissioningWindow(chip::NodeId nodeId, chip::EndpointId endpointId, uint16_t commissioningTimeout,
                                       uint32_t iterations, uint16_t discriminator, const chip::ByteSpan & salt,
                                       const chip::ByteSpan & verifier);

    void RegisterOpenCommissioningWindowDelegate(CommissioningWindowDelegate * delegate)
    {
        ChipLogProgress(NotSpecified, "yujuan: PairingManager::RegisterOpenCommissioningWindowDelegate");
        mCommissioningWindowDelegate = delegate;
    }

    void UnregisterOpenCommissioningWindowDelegate() { mCommissioningWindowDelegate = nullptr; }

private:
    PairingManager();
    PairingManager(const PairingManager &)             = delete;
    PairingManager & operator=(const PairingManager &) = delete;

    chip::Controller::DeviceCommissioner * mCommissioner = nullptr;

    /////////// Open Commissioning Window Command Interface /////////
    struct CommissioningWindowParams
    {
        chip::NodeId nodeId;
        chip::EndpointId endpointId;
        uint16_t commissioningWindowTimeout;
        uint32_t iteration;
        uint16_t discriminator;
        chip::Optional<uint32_t> setupPIN;
        uint8_t verifierBuffer[chip::Crypto::kSpake2p_VerifierSerialized_Length];
        chip::ByteSpan verifier;
        uint8_t saltBuffer[chip::Crypto::kSpake2p_Max_PBKDF_Salt_Length];
        chip::ByteSpan salt;
    };

    CommissioningWindowDelegate * mCommissioningWindowDelegate = nullptr;

    static chip::Platform::UniquePtr<chip::Controller::CommissioningWindowOpener> mWindowOpener;
    static void OnOpenCommissioningWindow(intptr_t context);
    static void OnOpenCommissioningWindowResponse(void * context, chip::NodeId deviceId, CHIP_ERROR status,
                                                  chip::SetupPayload payload);
    static void OnOpenCommissioningWindowVerifierResponse(void * context, chip::NodeId deviceId, CHIP_ERROR status);

    chip::Callback::Callback<chip::Controller::OnOpenCommissioningWindow> mOnOpenCommissioningWindowCallback;
    chip::Callback::Callback<chip::Controller::OnOpenCommissioningWindowWithVerifier> mOnOpenCommissioningWindowVerifierCallback;
};
