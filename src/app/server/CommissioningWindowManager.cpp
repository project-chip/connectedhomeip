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

#include <app/server/CommissioningWindowManager.h>
#include <app/server/Mdns.h>
#include <app/server/Server.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/SafeInt.h>
#include <platform/CHIPDeviceLayer.h>
#include <transport/SecureSessionMgr.h>

#if CHIP_ENABLE_OPENTHREAD
#include <platform/ThreadStackManager.h>
#endif
#include <lib/mdns/Advertiser.h>

namespace {

void HandleCommissioningWindowTimeout(chip::System::Layer * aSystemLayer, void * aAppState)
{
    chip::CommissioningWindowManager * commissionMgr = static_cast<chip::CommissioningWindowManager *>(aAppState);
    commissionMgr->CloseCommissioningWindow();
}

void OnPlatformEventWrapper(const chip::DeviceLayer::ChipDeviceEvent * event, intptr_t arg)
{
    chip::CommissioningWindowManager * commissionMgr = reinterpret_cast<chip::CommissioningWindowManager *>(arg);
    commissionMgr->OnPlatformEvent(event);
}
} // namespace

namespace chip {

void CommissioningWindowManager::OnPlatformEvent(const DeviceLayer::ChipDeviceEvent * event)
{
    if (event->Type == DeviceLayer::DeviceEventType::kCommissioningComplete)
    {
        if (event->CommissioningComplete.status == CHIP_NO_ERROR)
        {
            ChipLogProgress(AppServer, "Commissioning completed successfully");
        }
        else
        {
            ChipLogError(AppServer, "Commissioning errored out with error %" CHIP_ERROR_FORMAT,
                         event->CommissioningComplete.status.Format());
        }
        // reset all advertising
        app::MdnsServer::Instance().StartServer(Mdns::CommissioningMode::kDisabled);
    }
    else if (event->Type == DeviceLayer::DeviceEventType::kOperationalNetworkEnabled)
    {
        app::MdnsServer::Instance().AdvertiseOperational();
        ChipLogError(AppServer, "Operational advertising enabled");
    }
}

void CommissioningWindowManager::Cleanup()
{
    mServer->GetExchangManager().UnregisterUnsolicitedMessageHandlerForType(Protocols::SecureChannel::MsgType::PBKDFParamRequest);
    StopAdvertisement();

    // reset all advertising
    app::MdnsServer::Instance().StartServer(Mdns::CommissioningMode::kDisabled);
}

void CommissioningWindowManager::OnSessionEstablishmentError(CHIP_ERROR err)
{
    Cleanup();

    ChipLogError(AppServer, "Commissioning failed during session establishment: %s", ErrorStr(err));

    if (mAppDelegate != nullptr)
    {
        mAppDelegate->OnRendezvousStopped();
    }
}

void CommissioningWindowManager::OnSessionEstablished()
{
    CHIP_ERROR err = mServer->GetSecureSessionManager().NewPairing(
        Optional<Transport::PeerAddress>::Value(mPairingSession.GetPeerAddress()), mPairingSession.GetPeerNodeId(),
        &mPairingSession, SecureSession::SessionRole::kResponder, 0);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Commissioning failed while setting up secure channel: err %s", ErrorStr(err));
        OnSessionEstablishmentError(err);
        return;
    }

    ChipLogProgress(AppServer, "Commissioning completed session establishment step");
    if (mAppDelegate != nullptr)
    {
        mAppDelegate->OnRendezvousStarted();
    }

    DeviceLayer::PlatformMgr().AddEventHandler(OnPlatformEventWrapper, reinterpret_cast<intptr_t>(this));

    Cleanup();
    ChipLogProgress(AppServer, "Device completed Rendezvous process");
}

CHIP_ERROR CommissioningWindowManager::PrepareCommissioningWindow(uint16_t commissioningTimeoutSeconds,
                                                                  uint16_t & allocatedSessionID)
{
    ReturnErrorOnFailure(mIDAllocator->Allocate(allocatedSessionID));

    ReturnErrorOnFailure(mPairingSession.MessageDispatch().Init(&mServer->GetSecureSessionManager()));

    if (commissioningTimeoutSeconds != kNoCommissioningTimeout)
    {
        ReturnErrorOnFailure(
            DeviceLayer::SystemLayer().StartTimer(commissioningTimeoutSeconds * 1000, HandleCommissioningWindowTimeout, this));
    }

    ReturnErrorOnFailure(mServer->GetExchangManager().RegisterUnsolicitedMessageHandlerForType(
        Protocols::SecureChannel::MsgType::PBKDFParamRequest, &mPairingSession));

    return StartAdvertisement();
}

CHIP_ERROR CommissioningWindowManager::OpenBasicCommissioningWindow(uint16_t commissioningTimeoutSeconds,
                                                                    CommissioningWindowAdvertisement advertisementMode)
{
    RestoreDiscriminator();

#if CONFIG_NETWORK_LAYER_BLE
    // Enable BLE advertisements if commissioning window is to be opened on all supported
    // transports, and BLE is supported on the current device.
    SetBLE(advertisementMode == chip::CommissioningWindowAdvertisement::kAllSupported);
#else
    SetBLE(false);
#endif // CONFIG_NETWORK_LAYER_BLE

    uint16_t keyID = 0;
    CHIP_ERROR err = CHIP_NO_ERROR;
    uint32_t pinCode;

    SuccessOrExit(err = DeviceLayer::ConfigurationMgr().GetSetupPinCode(pinCode));
    SuccessOrExit(err = PrepareCommissioningWindow(commissioningTimeoutSeconds, keyID));
    SuccessOrExit(err = mPairingSession.WaitForPairing(
                      pinCode, kSpake2p_Iteration_Count,
                      ByteSpan(reinterpret_cast<const uint8_t *>(kSpake2pKeyExchangeSalt), strlen(kSpake2pKeyExchangeSalt)), keyID,
                      this));

    // reset all advertising, indicating we are in commissioningMode
    // and we were put into this state via a command for additional commissioning
    app::MdnsServer::Instance().StartServer(Mdns::CommissioningMode::kEnabledBasic);

exit:
    if (err != CHIP_NO_ERROR)
    {
        Cleanup();
    }
    return err;
}

CHIP_ERROR CommissioningWindowManager::OpenEnhancedCommissioningWindow(uint16_t commissioningTimeoutSeconds, uint16_t discriminator,
                                                                       PASEVerifier & verifier, uint32_t iterations, ByteSpan salt,
                                                                       uint16_t passcodeID)
{
#if CONFIG_NETWORK_LAYER_BLE
    // TODO: Don't use BLE for commissioning additional fabrics on a device
    SetBLE(true);
#else
    SetBLE(false);
#endif

    uint16_t keyID = 0;
    CHIP_ERROR err = CHIP_NO_ERROR;

    SuccessOrExit(err = SetTemporaryDiscriminator(discriminator));
    SuccessOrExit(err = PrepareCommissioningWindow(commissioningTimeoutSeconds, keyID));
    SuccessOrExit(err = mPairingSession.WaitForPairing(verifier, iterations, salt, passcodeID, keyID, this));

    // reset all advertising, indicating we are in commissioningMode
    // and we were put into this state via a command for additional commissioning
    app::MdnsServer::Instance().StartServer(Mdns::CommissioningMode::kEnabledEnhanced);

exit:
    if (err != CHIP_NO_ERROR)
    {
        Cleanup();
    }
    return err;
}

void CommissioningWindowManager::CloseCommissioningWindow()
{
    if (mCommissioningWindowOpen)
    {
        ChipLogProgress(AppServer, "Closing pairing window");
        Cleanup();
    }
}

CHIP_ERROR CommissioningWindowManager::StartAdvertisement()
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

CHIP_ERROR CommissioningWindowManager::StopAdvertisement()
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

CHIP_ERROR CommissioningWindowManager::SetTemporaryDiscriminator(uint16_t discriminator)
{
    if (!mOriginalDiscriminatorCached)
    {
        // Cache the original discriminator
        ReturnErrorOnFailure(DeviceLayer::ConfigurationMgr().GetSetupDiscriminator(mOriginalDiscriminator));
        mOriginalDiscriminatorCached = true;
    }

    return DeviceLayer::ConfigurationMgr().StoreSetupDiscriminator(discriminator);
}

CHIP_ERROR CommissioningWindowManager::RestoreDiscriminator()
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
