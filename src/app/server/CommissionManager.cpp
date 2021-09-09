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

#include <app/server/CommissionManager.h>
#include <app/server/Server.h>
#include <platform/CHIPDeviceLayer.h>

namespace {

void HandleCommissioningWindowTimeout(chip::System::Layer * aSystemLayer, void * aAppState)
{
    chip::CommissionManager * commissionMgr = static_cast<chip::CommissionManager *>(aAppState);
    commissionMgr->CloseCommissioningWindow();
}

} // namespace

namespace chip {

CHIP_ERROR CommissionManager::OpenBasicCommissioningWindow(ResetFabrics resetFabrics, uint16_t commissioningTimeoutSeconds,
                                                           CommissioningWindowAdvertisement advertisementMode)
{
    // TODO(cecille): If this is re-called when the window is already open, what should happen?
    RestoreDiscriminator();

    uint32_t pinCode;
    ReturnErrorOnFailure(DeviceLayer::ConfigurationMgr().GetSetupPinCode(pinCode));

    RendezvousParameters params;

    params.SetSetupPINCode(pinCode);
    params.SetAdvertisementDelegate(this);
#if CONFIG_NETWORK_LAYER_BLE
    SetBLE(advertisementMode == chip::CommissioningWindowAdvertisement::kBle);
    if (advertisementMode == chip::CommissioningWindowAdvertisement::kBle)
    {
        params.SetBleLayer(DeviceLayer::ConnectivityMgr().GetBleLayer()).SetPeerAddress(Transport::PeerAddress::BLE());
    }
#endif // CONFIG_NETWORK_LAYER_BLE

    if (resetFabrics == ResetFabrics::kYes)
    {
        mServer->GetFabricTable().DeleteAllFabrics();
        // Only resetting gNextAvailableFabricIndex at reboot otherwise previously paired device with fabricID 0
        // can continue sending messages to accessory as next available fabric will also be 0.
        // This logic is not up to spec, will be implemented up to spec once AddOptCert is implemented.
        mServer->GetFabricTable().Reset();
    }

    ReturnErrorOnFailure(mServer->GetRendezvousServer().WaitForPairing(
        std::move(params), kSpake2p_Iteration_Count,
        ByteSpan(reinterpret_cast<const uint8_t *>(kSpake2pKeyExchangeSalt), strlen(kSpake2pKeyExchangeSalt)), 0,
        &mServer->GetExchangManager(), &mServer->GetTransportManager(), &mServer->GetSecureSessionManager()));

    if (commissioningTimeoutSeconds != kNoCommissioningTimeout)
    {
        ReturnErrorOnFailure(
            DeviceLayer::SystemLayer.StartTimer(commissioningTimeoutSeconds * 1000, HandleCommissioningWindowTimeout, this));
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR CommissionManager::OpenEnhancedCommissioningWindow(uint16_t commissioningTimeoutSeconds, uint16_t discriminator,
                                                              PASEVerifier & verifier, uint32_t iterations, ByteSpan salt,
                                                              uint16_t passcodeID)
{
    RendezvousParameters params;

    ReturnErrorOnFailure(SetTemporaryDiscriminator(discriminator));

    SetBLE(false);
    params.SetAdvertisementDelegate(this);
#if CONFIG_NETWORK_LAYER_BLE
    SetBLE(true);
    params.SetBleLayer(DeviceLayer::ConnectivityMgr().GetBleLayer()).SetPeerAddress(Transport::PeerAddress::BLE());
#endif

    params.SetPASEVerifier(verifier).SetAdvertisementDelegate(this);

    ReturnErrorOnFailure(mServer->GetRendezvousServer().WaitForPairing(
        std::move(params), iterations, salt, passcodeID, &mServer->GetExchangManager(), &mServer->GetTransportManager(),
        &mServer->GetSecureSessionManager()));

    if (commissioningTimeoutSeconds != kNoCommissioningTimeout)
    {
        ReturnErrorOnFailure(
            DeviceLayer::SystemLayer.StartTimer(commissioningTimeoutSeconds * 1000, HandleCommissioningWindowTimeout, this));
    }

    return CHIP_NO_ERROR;
}

void CommissionManager::CloseCommissioningWindow()
{
    if (mCommissioningWindowOpen)
    {
        ChipLogProgress(AppServer, "Closing pairing window");
        mServer->GetRendezvousServer().Cleanup();
    }
}

CHIP_ERROR CommissionManager::StartAdvertisement()
{
    if (mIsBLE)
    {
        ReturnErrorOnFailure(chip::DeviceLayer::ConnectivityMgr().SetBLEAdvertisingEnabled(true));
    }
    if (mAppDelegate != nullptr)
    {
        mAppDelegate->OnPairingWindowOpened();
    }
    mCommissioningWindowOpen = true;
    return CHIP_NO_ERROR;
}

CHIP_ERROR CommissionManager::StopAdvertisement()
{
    RestoreDiscriminator();

    mCommissioningWindowOpen = false;

    if (mIsBLE)
    {
        ReturnErrorOnFailure(chip::DeviceLayer::ConnectivityMgr().SetBLEAdvertisingEnabled(false));
    }

    if (mAppDelegate != nullptr)
    {
        mAppDelegate->OnPairingWindowClosed();
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR CommissionManager::SetTemporaryDiscriminator(uint16_t discriminator)
{
    if (!mOriginalDiscriminatorCached)
    {
        // Cache the original discriminator
        ReturnErrorOnFailure(DeviceLayer::ConfigurationMgr().GetSetupDiscriminator(mOriginalDiscriminator));
        mOriginalDiscriminatorCached = true;
    }

    return DeviceLayer::ConfigurationMgr().StoreSetupDiscriminator(discriminator);
}

CHIP_ERROR CommissionManager::RestoreDiscriminator()
{
    if (mOriginalDiscriminatorCached)
    {
        // Restore the original discriminator
        ReturnErrorOnFailure(DeviceLayer::ConfigurationMgr().StoreSetupDiscriminator(mOriginalDiscriminator));
        mOriginalDiscriminatorCached = false;
    }

    return CHIP_NO_ERROR;
}

} // namespace chip
