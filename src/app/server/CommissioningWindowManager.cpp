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

#include <app/server/CommissioningWindowManager.h>
#include <app/server/Dnssd.h>
#include <app/server/Server.h>
#include <lib/dnssd/Advertiser.h>
#include <lib/support/CodeUtils.h>
#include <platform/CHIPDeviceLayer.h>

using namespace chip::app::Clusters;

namespace {

// As per specifications (Section 13.3), Nodes SHALL exit commissioning mode after 20 failed commission attempts.
constexpr uint8_t kMaxFailedCommissioningAttempts = 20;

void HandleCommissioningWindowTimeout(chip::System::Layer * aSystemLayer, void * aAppState)
{
    chip::CommissioningWindowManager * commissionMgr = static_cast<chip::CommissioningWindowManager *>(aAppState);
    commissionMgr->CloseCommissioningWindow();
}

void HandleSessionEstablishmentTimeout(chip::System::Layer * aSystemLayer, void * aAppState)
{
    chip::CommissioningWindowManager * commissionMgr = static_cast<chip::CommissioningWindowManager *>(aAppState);
    commissionMgr->OnSessionEstablishmentError(CHIP_ERROR_TIMEOUT);
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
            Cleanup();
        }
        else
        {
            ChipLogError(AppServer, "Commissioning failed with error %" CHIP_ERROR_FORMAT,
                         event->CommissioningComplete.status.Format());
            OnSessionEstablishmentError(event->CommissioningComplete.status);
        }
    }
    else if (event->Type == DeviceLayer::DeviceEventType::kOperationalNetworkEnabled)
    {
        app::DnssdServer::Instance().AdvertiseOperational();
        ChipLogError(AppServer, "Operational advertising enabled");
    }
}

void CommissioningWindowManager::Shutdown()
{
    StopAdvertisement(/* aShuttingDown = */ true);

    ResetState();
}

void CommissioningWindowManager::ResetState()
{
    mUseECM = false;

    mECMDiscriminator = 0;
    mECMPasscodeID    = 0;
    mECMIterations    = 0;
    mECMSaltLength    = 0;
    mWindowStatus     = app::Clusters::AdministratorCommissioning::CommissioningWindowStatus::kWindowNotOpen;

    memset(&mECMPASEVerifier, 0, sizeof(mECMPASEVerifier));
    memset(mECMSalt, 0, sizeof(mECMSalt));
}

void CommissioningWindowManager::Cleanup()
{
    StopAdvertisement(/* aShuttingDown = */ false);

    ResetState();
}

void CommissioningWindowManager::OnSessionEstablishmentError(CHIP_ERROR err)
{
    DeviceLayer::SystemLayer().CancelTimer(HandleSessionEstablishmentTimeout, this);
    mFailedCommissioningAttempts++;
    ChipLogError(AppServer, "Commissioning failed (attempt %d): %s", mFailedCommissioningAttempts, ErrorStr(err));

#if CONFIG_NETWORK_LAYER_BLE
    mServer->GetBleLayerObject()->CloseAllBleConnections();
#endif
    if (mFailedCommissioningAttempts < kMaxFailedCommissioningAttempts)
    {
        // If the number of commissioning attempts have not exceeded maximum retries, let's reopen
        // the pairing window.
        err = OpenCommissioningWindow();
    }

    // If the commissioning attempts limit exceeded, or reopening the commissioning window failed.
    if (err != CHIP_NO_ERROR)
    {
        Cleanup();

        if (mAppDelegate != nullptr)
        {
            mAppDelegate->OnRendezvousStopped();
        }
    }
}

void CommissioningWindowManager::OnSessionEstablishmentStarted()
{
    // As per specifications, section 5.5: Commissioning Flows
    constexpr System::Clock::Timeout kPASESessionEstablishmentTimeout = System::Clock::Seconds16(60);
    DeviceLayer::SystemLayer().StartTimer(kPASESessionEstablishmentTimeout, HandleSessionEstablishmentTimeout, this);
}

void CommissioningWindowManager::OnSessionEstablished()
{
    DeviceLayer::SystemLayer().CancelTimer(HandleSessionEstablishmentTimeout, this);
    SessionHolder sessionHolder;
    CHIP_ERROR err = mServer->GetSecureSessionManager().NewPairing(
        sessionHolder, Optional<Transport::PeerAddress>::Value(mPairingSession.GetPeerAddress()), mPairingSession.GetPeerNodeId(),
        &mPairingSession, CryptoContext::SessionRole::kResponder, 0);
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

    StopAdvertisement(/* aShuttingDown = */ true);
    ChipLogProgress(AppServer, "Device completed Rendezvous process");
}

CHIP_ERROR CommissioningWindowManager::OpenCommissioningWindow()
{
    uint16_t keyID = 0;
    ReturnErrorOnFailure(mIDAllocator->Allocate(keyID));

    mPairingSession.Clear();

    if (mCommissioningTimeoutSeconds != kNoCommissioningTimeout)
    {
        ReturnErrorOnFailure(DeviceLayer::SystemLayer().StartTimer(System::Clock::Seconds16(mCommissioningTimeoutSeconds),
                                                                   HandleCommissioningWindowTimeout, this));
    }

    ReturnErrorOnFailure(mServer->GetExchangeManager().RegisterUnsolicitedMessageHandlerForType(
        Protocols::SecureChannel::MsgType::PBKDFParamRequest, &mPairingSession));

    if (mUseECM)
    {
        ReturnErrorOnFailure(SetTemporaryDiscriminator(mECMDiscriminator));
        ReturnErrorOnFailure(
            mPairingSession.WaitForPairing(mECMPASEVerifier, mECMIterations, ByteSpan(mECMSalt, mECMSaltLength), mECMPasscodeID,
                                           keyID, Optional<ReliableMessageProtocolConfig>::Value(GetLocalMRPConfig()), this));
    }
    else
    {
        uint32_t iterationCount                           = 0;
        uint8_t salt[Crypto::kSpake2pPBKDFMaximumSaltLen] = { 0 };
        size_t saltLen                                    = 0;
        PASEVerifierSerialized serializedVerifier         = { 0 };
        size_t serializedVerifierLen                      = 0;
        PASEVerifier verifier;

        ReturnErrorOnFailure(DeviceLayer::ConfigurationMgr().GetSpake2pIterationCount(iterationCount));
        ReturnErrorOnFailure(DeviceLayer::ConfigurationMgr().GetSpake2pSalt(salt, sizeof(salt), saltLen));
        ReturnErrorOnFailure(DeviceLayer::ConfigurationMgr().GetSpake2pVerifier(
            serializedVerifier, Crypto::kSpake2pSerializedVerifierSize, serializedVerifierLen));
        VerifyOrReturnError(Crypto::kSpake2pSerializedVerifierSize == serializedVerifierLen, CHIP_ERROR_INVALID_ARGUMENT);
        ReturnErrorOnFailure(verifier.Deserialize(ByteSpan(serializedVerifier)));

        ReturnErrorOnFailure(
            mPairingSession.WaitForPairing(verifier, iterationCount, ByteSpan(salt, saltLen), kDefaultCommissioningPasscodeId,
                                           keyID, Optional<ReliableMessageProtocolConfig>::Value(GetLocalMRPConfig()), this));
    }

    ReturnErrorOnFailure(StartAdvertisement());

    return CHIP_NO_ERROR;
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

    mFailedCommissioningAttempts = 0;
    mCommissioningTimeoutSeconds = commissioningTimeoutSeconds;

    mUseECM = false;

    CHIP_ERROR err = OpenCommissioningWindow();
    if (err != CHIP_NO_ERROR)
    {
        Cleanup();
    }

    return err;
}

CHIP_ERROR CommissioningWindowManager::OpenEnhancedCommissioningWindow(uint16_t commissioningTimeoutSeconds, uint16_t discriminator,
                                                                       PASEVerifier & verifier, uint32_t iterations, ByteSpan salt,
                                                                       PasscodeId passcodeID)
{
    // Once a device is operational, it shall be commissioned into subsequent fabrics using
    // the operational network only.
    SetBLE(false);

    VerifyOrReturnError(salt.size() <= sizeof(mECMSalt), CHIP_ERROR_INVALID_ARGUMENT);

    memcpy(mECMSalt, salt.data(), salt.size());
    mECMSaltLength = static_cast<uint32_t>(salt.size());

    mFailedCommissioningAttempts = 0;
    mCommissioningTimeoutSeconds = commissioningTimeoutSeconds;

    mECMDiscriminator = discriminator;
    mECMPasscodeID    = passcodeID;
    mECMIterations    = iterations;

    memcpy(&mECMPASEVerifier, &verifier, sizeof(PASEVerifier));

    mUseECM = true;

    CHIP_ERROR err = OpenCommissioningWindow();
    if (err != CHIP_NO_ERROR)
    {
        Cleanup();
    }
    return err;
}

void CommissioningWindowManager::CloseCommissioningWindow()
{
    if (mWindowStatus != AdministratorCommissioning::CommissioningWindowStatus::kWindowNotOpen)
    {
        ChipLogProgress(AppServer, "Closing pairing window");
        Cleanup();
    }
}

CHIP_ERROR CommissioningWindowManager::StartAdvertisement()
{
#if CHIP_ENABLE_ADDITIONAL_DATA_ADVERTISING
    // notify device layer that advertisement is beginning (to do work such as increment rotating id)
    DeviceLayer::ConfigurationMgr().NotifyOfAdvertisementStart();
#endif

#if CHIP_DEVICE_CONFIG_ENABLE_SED
    if (!mIsBLE && mWindowStatus == AdministratorCommissioning::CommissioningWindowStatus::kWindowNotOpen)
    {
        DeviceLayer::ConnectivityMgr().RequestSEDFastPollingMode(true);
    }
#endif

    if (mIsBLE)
    {
        ReturnErrorOnFailure(chip::DeviceLayer::ConnectivityMgr().SetBLEAdvertisingEnabled(true));
    }

    if (mAppDelegate != nullptr)
    {
        mAppDelegate->OnPairingWindowOpened();
    }

    Dnssd::CommissioningMode commissioningMode;
    if (mUseECM)
    {
        mWindowStatus     = AdministratorCommissioning::CommissioningWindowStatus::kEnhancedWindowOpen;
        commissioningMode = Dnssd::CommissioningMode::kEnabledEnhanced;
    }
    else
    {
        mWindowStatus     = AdministratorCommissioning::CommissioningWindowStatus::kBasicWindowOpen;
        commissioningMode = Dnssd::CommissioningMode::kEnabledBasic;
    }

    // reset all advertising, indicating we are in commissioningMode
    app::DnssdServer::Instance().StartServer(commissioningMode);

    return CHIP_NO_ERROR;
}

CHIP_ERROR CommissioningWindowManager::StopAdvertisement(bool aShuttingDown)
{
    RestoreDiscriminator();

    mServer->GetExchangeManager().UnregisterUnsolicitedMessageHandlerForType(Protocols::SecureChannel::MsgType::PBKDFParamRequest);
    mPairingSession.Clear();

#if CHIP_DEVICE_CONFIG_ENABLE_SED
    if (!mIsBLE && mWindowStatus != AdministratorCommissioning::CommissioningWindowStatus::kWindowNotOpen)
    {
        DeviceLayer::ConnectivityMgr().RequestSEDFastPollingMode(false);
    }
#endif

    // If aShuttingDown, don't try to change our DNS-SD advertisements.
    if (!aShuttingDown)
    {
        // Stop advertising commissioning mode, since we're not accepting PASE
        // connections right now.  If we start accepting them again (via
        // OpenCommissioningWindow) that will call StartAdvertisement as needed.
        app::DnssdServer::Instance().StartServer(Dnssd::CommissioningMode::kDisabled);
    }

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
