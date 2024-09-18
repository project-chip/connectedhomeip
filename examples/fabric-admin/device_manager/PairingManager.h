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

/**
 * The PairingManager class is responsible for managing the commissioning and pairing process
 * of Matter devices. PairingManager is designed to be used as a singleton, meaning that there
 * should only be one instance of it running at any given time.
 *
 * Usage:
 *
 * 1. The class should be initialized when the system starts up, typically by invoking the static
 *    instance method to get the singleton.
 * 2. To open a commissioning window, the appropriate method should be called on the PairingManager instance.
 * 3. The PairingManager will handle the lifecycle of the CommissioningWindowOpener and ensure that
 *    resources are cleaned up appropriately when pairing is complete or the process is aborted.
 *
 * Example:
 *
 * @code
 * PairingManager& manager = PairingManager::Instance();
 * manager.OpenCommissioningWindow();
 * @endcode
 */
class PairingManager
{
public:
    static PairingManager & Instance()
    {
        static PairingManager instance;
        return instance;
    }

    void Init(chip::Controller::DeviceCommissioner * commissioner);

    /**
     * Opens a commissioning window on the specified node and endpoint.
     * Only one commissioning window can be active at a time. If a commissioning
     * window is already open, this function will return an error.
     *
     * @param nodeId The target node ID for commissioning.
     * @param endpointId The target endpoint ID for commissioning.
     * @param commissioningTimeoutSec Timeout for the commissioning window in seconds.
     * @param iterations Iterations for PBKDF calculations.
     * @param discriminator Discriminator for commissioning.
     * @param salt Optional salt for verifier-based commissioning.
     * @param verifier Optional verifier for enhanced commissioning security.
     *
     * @return CHIP_ERROR_INCORRECT_STATE if a commissioning window is already open.
     */
    CHIP_ERROR OpenCommissioningWindow(chip::NodeId nodeId, chip::EndpointId endpointId, uint16_t commissioningTimeoutSec,
                                       uint32_t iterations, uint16_t discriminator, const chip::ByteSpan & salt,
                                       const chip::ByteSpan & verifier);

    void SetOpenCommissioningWindowDelegate(CommissioningWindowDelegate * delegate) { mCommissioningWindowDelegate = delegate; }

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

    /**
     * Holds the unique_ptr to the current CommissioningWindowOpener.
     * Only one commissioning window opener can be active at a time.
     * The pointer is reset when the commissioning window is closed or when an error occurs.
     */
    chip::Platform::UniquePtr<chip::Controller::CommissioningWindowOpener> mWindowOpener;

    static void OnOpenCommissioningWindow(intptr_t context);
    static void OnOpenCommissioningWindowResponse(void * context, chip::NodeId deviceId, CHIP_ERROR status,
                                                  chip::SetupPayload payload);
    static void OnOpenCommissioningWindowVerifierResponse(void * context, chip::NodeId deviceId, CHIP_ERROR status);

    chip::Callback::Callback<chip::Controller::OnOpenCommissioningWindow> mOnOpenCommissioningWindowCallback;
    chip::Callback::Callback<chip::Controller::OnOpenCommissioningWindowWithVerifier> mOnOpenCommissioningWindowVerifierCallback;
};
