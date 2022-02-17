/*
 *
 *    Copyright (c) 2021-2022 Project CHIP Authors
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
#include <app/server/AppDelegate.h>
#include <protocols/secure_channel/RendezvousParameters.h>
#include <protocols/secure_channel/SessionIDAllocator.h>

namespace chip {

constexpr uint16_t kNoCommissioningTimeout = UINT16_MAX;

enum class CommissioningWindowAdvertisement
{
    kAllSupported,
    kDnssdOnly,
};

class Server;

class CommissioningWindowManager : public SessionEstablishmentDelegate
{
public:
    CommissioningWindowManager(Server * server) : mAppDelegate(nullptr), mServer(server) {}

    void SetAppDelegate(AppDelegate * delegate) { mAppDelegate = delegate; }

    void SetBLE(bool ble) { mIsBLE = ble; }

    void SetSessionIDAllocator(SessionIDAllocator * idAllocator) { mIDAllocator = idAllocator; }

    /**
     * Open the pairing window using default configured parameters.
     */
    CHIP_ERROR
    OpenBasicCommissioningWindow(
        uint16_t commissioningTimeoutSeconds               = kNoCommissioningTimeout,
        CommissioningWindowAdvertisement advertisementMode = chip::CommissioningWindowAdvertisement::kAllSupported);

    CHIP_ERROR OpenEnhancedCommissioningWindow(uint16_t commissioningTimeoutSeconds, uint16_t discriminator,
                                               PASEVerifier & verifier, uint32_t iterations, chip::ByteSpan salt,
                                               PasscodeId passcodeID);

    void CloseCommissioningWindow();

    app::Clusters::AdministratorCommissioning::CommissioningWindowStatus CommissioningWindowStatus() const { return mWindowStatus; }

    //////////// SessionEstablishmentDelegate Implementation ///////////////
    void OnSessionEstablishmentError(CHIP_ERROR error) override;
    void OnSessionEstablishmentStarted() override;
    void OnSessionEstablished() override;

    void Shutdown();
    void Cleanup();

    void OnPlatformEvent(const DeviceLayer::ChipDeviceEvent * event);

private:
    CHIP_ERROR SetTemporaryDiscriminator(uint16_t discriminator);

    CHIP_ERROR RestoreDiscriminator();

    CHIP_ERROR StartAdvertisement();

    CHIP_ERROR StopAdvertisement(bool aShuttingDown);

    CHIP_ERROR OpenCommissioningWindow();

    // Helper for Shutdown and Cleanup.  Does not do anything with
    // advertisements, because Shutdown and Cleanup want to handle those
    // differently.
    void ResetState();

    AppDelegate * mAppDelegate = nullptr;
    Server * mServer           = nullptr;

    app::Clusters::AdministratorCommissioning::CommissioningWindowStatus mWindowStatus =
        app::Clusters::AdministratorCommissioning::CommissioningWindowStatus::kWindowNotOpen;

    bool mIsBLE = true;

    bool mOriginalDiscriminatorCached = false;
    uint16_t mOriginalDiscriminator   = 0;

    SessionIDAllocator * mIDAllocator = nullptr;
    PASESession mPairingSession;

    uint16_t mCommissioningTimeoutSeconds = 0;

    uint8_t mFailedCommissioningAttempts = 0;

    bool mUseECM = false;
    PASEVerifier mECMPASEVerifier;
    uint16_t mECMDiscriminator = 0;
    PasscodeId mECMPasscodeID  = kDefaultCommissioningPasscodeId;
    uint32_t mECMIterations    = 0;
    uint32_t mECMSaltLength    = 0;
    uint8_t mECMSalt[Crypto::kSpake2pPBKDFMaximumSaltLen];
};

} // namespace chip
