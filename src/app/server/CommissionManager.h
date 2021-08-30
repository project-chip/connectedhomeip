/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <app/server/AppDelegate.h>
#include <protocols/secure_channel/RendezvousParameters.h>

namespace chip {

enum class ResetFabrics
{
    kYes,
    kNo,
};

constexpr uint16_t kNoCommissioningTimeout = UINT16_MAX;

enum class CommissioningWindowAdvertisement
{
    kBle,
    kMdns,
};

class Server;

class CommissionManager : public RendezvousAdvertisementDelegate
{
public:
    CommissionManager(Server * server) : mAppDelegate(nullptr), mServer(server) {}

    void SetAppDelegate(AppDelegate * delegate) { mAppDelegate = delegate; }

    void SetBLE(bool ble) { mIsBLE = ble; }

    /**
     * Open the pairing window using default configured parameters.
     */
    CHIP_ERROR
    OpenBasicCommissioningWindow(ResetFabrics resetFabrics, uint16_t commissioningTimeoutSeconds = kNoCommissioningTimeout,
                                 CommissioningWindowAdvertisement advertisementMode = chip::CommissioningWindowAdvertisement::kBle);

    CHIP_ERROR OpenEnhancedCommissioningWindow(uint16_t commissioningTimeoutSeconds, uint16_t discriminator,
                                               PASEVerifier & verifier, uint32_t iterations, chip::ByteSpan salt,
                                               uint16_t passcodeID);

    void CloseCommissioningWindow();

    bool IsPairingWindowOpen() { return mCommissioningWindowOpen; }

    CHIP_ERROR StartAdvertisement() override;

    CHIP_ERROR StopAdvertisement() override;

private:
    CHIP_ERROR SetTemporaryDiscriminator(uint16_t discriminator);

    CHIP_ERROR RestoreDiscriminator();

    AppDelegate * mAppDelegate = nullptr;
    Server * mServer           = nullptr;

    bool mCommissioningWindowOpen = false;
    bool mIsBLE                   = true;

    bool mOriginalDiscriminatorCached = false;
    uint16_t mOriginalDiscriminator   = 0;
};

} // namespace chip
