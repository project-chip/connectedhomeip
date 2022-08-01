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
#include <platform/CommissionableDataProvider.h>
#include <platform/DeviceControlServer.h>

using namespace chip::app::Clusters;
using namespace chip::System::Clock;

using AdministratorCommissioning::CommissioningWindowStatus;

namespace {

// As per specifications (Section 13.3), Nodes SHALL exit commissioning mode after 20 failed commission attempts.
constexpr uint8_t kMaxFailedCommissioningAttempts = 20;

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
        ChipLogProgress(AppServer, "Commissioning completed successfully");
        DeviceLayer::SystemLayer().CancelTimer(HandleCommissioningWindowTimeout, this);
        mCommissioningTimeoutTimerArmed = false;
        Cleanup();
        mServer->GetSecureSessionManager().ExpireAllPASESessions();
        // That should have cleared out mPASESession.
#if CONFIG_NETWORK_LAYER_BLE
        mServer->GetBleLayerObject()->CloseAllBleConnections();
#endif
    }
    else if (event->Type == DeviceLayer::DeviceEventType::kFailSafeTimerExpired)
    {
        ChipLogError(AppServer, "Failsafe timer expired");
        if (mPASESession)
        {
            mPASESession->AsSecureSession()->MarkForEviction();
        }
        HandleFailedAttempt(CHIP_ERROR_TIMEOUT);
    }
    else if (event->Type == DeviceLayer::DeviceEventType::kOperationalNetworkEnabled)
    {
        app::DnssdServer::Instance().AdvertiseOperational();
        ChipLogProgress(AppServer, "Operational advertising enabled");
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
    mECMIterations    = 0;
    mECMSaltLength    = 0;
    mWindowStatus     = CommissioningWindowStatus::kWindowNotOpen;

    mOpenerFabricIndex.SetNull();
    mOpenerVendorId.SetNull();

    memset(&mECMPASEVerifier, 0, sizeof(mECMPASEVerifier));
    memset(mECMSalt, 0, sizeof(mECMSalt));

    DeviceLayer::SystemLayer().CancelTimer(HandleCommissioningWindowTimeout, this);
    mCommissioningTimeoutTimerArmed = false;
}

void CommissioningWindowManager::Cleanup()
{
    StopAdvertisement(/* aShuttingDown = */ false);
    ResetState();
}

void CommissioningWindowManager::OnSessionEstablishmentError(CHIP_ERROR err)
{
    DeviceLayer::SystemLayer().CancelTimer(HandleSessionEstablishmentTimeout, this);
    HandleFailedAttempt(err);
}

void CommissioningWindowManager::HandleFailedAttempt(CHIP_ERROR err)
{
    mFailedCommissioningAttempts++;
    ChipLogError(AppServer, "Commissioning failed (attempt %d): %" CHIP_ERROR_FORMAT, mFailedCommissioningAttempts, err.Format());
#if CONFIG_NETWORK_LAYER_BLE
    mServer->GetBleLayerObject()->CloseAllBleConnections();
#endif
    if (mFailedCommissioningAttempts < kMaxFailedCommissioningAttempts)
    {
        // If the number of commissioning attempts has not exceeded maximum
        // retries, let's start listening for commissioning connections again.
        err = AdvertiseAndListenForPASE();
    }

    if (err != CHIP_NO_ERROR)
    {
        // The commissioning attempts limit was exceeded, or listening for
        // commmissioning connections failed.
        Cleanup();

        if (mAppDelegate != nullptr)
        {
            mAppDelegate->OnCommissioningSessionStopped();
        }
    }
}

void CommissioningWindowManager::OnSessionEstablishmentStarted()
{
    // As per specifications, section 5.5: Commissioning Flows
    constexpr System::Clock::Timeout kPASESessionEstablishmentTimeout = System::Clock::Seconds16(60);
    DeviceLayer::SystemLayer().StartTimer(kPASESessionEstablishmentTimeout, HandleSessionEstablishmentTimeout, this);
}

void CommissioningWindowManager::OnSessionEstablished(const SessionHandle & session)
{
    DeviceLayer::SystemLayer().CancelTimer(HandleSessionEstablishmentTimeout, this);

    ChipLogProgress(AppServer, "Commissioning completed session establishment step");
    if (mAppDelegate != nullptr)
    {
        mAppDelegate->OnCommissioningSessionStarted();
    }

    DeviceLayer::PlatformMgr().AddEventHandler(OnPlatformEventWrapper, reinterpret_cast<intptr_t>(this));

    StopAdvertisement(/* aShuttingDown = */ false);

    auto & failSafeContext = Server::GetInstance().GetFailSafeContext();
    // This should never be armed because we don't allow CASE sessions to arm the failsafe when the commissioning window is open and
    // we check that the failsafe is not armed before opening the commissioning window. None the less, it is good to double-check.
    CHIP_ERROR err = CHIP_NO_ERROR;
    if (failSafeContext.IsFailSafeArmed())
    {
        ChipLogError(AppServer, "Error - arm failsafe is already armed on PASE session establishment completion");
    }
    else
    {
        err = failSafeContext.ArmFailSafe(kUndefinedFabricId, System::Clock::Seconds16(60));
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(AppServer, "Error arming failsafe on PASE session establishment completion");
            // Don't allow a PASE session to hang around without a fail-safe.
            session->AsSecureSession()->MarkForEviction();
            HandleFailedAttempt(err);
        }
    }

    ChipLogProgress(AppServer, "Device completed Rendezvous process");

    if (err == CHIP_NO_ERROR)
    {
        // When the now-armed fail-safe is disarmed or expires it will handle
        // clearing out mPASESession.
        mPASESession.Grab(session);
    }
}

CHIP_ERROR CommissioningWindowManager::OpenCommissioningWindow(Seconds16 commissioningTimeout)
{
    VerifyOrReturnError(commissioningTimeout <= MaxCommissioningTimeout() && commissioningTimeout >= MinCommissioningTimeout(),
                        CHIP_ERROR_INVALID_ARGUMENT);
    auto & failSafeContext = Server::GetInstance().GetFailSafeContext();
    VerifyOrReturnError(failSafeContext.IsFailSafeFullyDisarmed(), CHIP_ERROR_INCORRECT_STATE);

    ReturnErrorOnFailure(Dnssd::ServiceAdvertiser::Instance().UpdateCommissionableInstanceName());

    ReturnErrorOnFailure(DeviceLayer::SystemLayer().StartTimer(commissioningTimeout, HandleCommissioningWindowTimeout, this));

    mCommissioningTimeoutTimerArmed = true;

    return AdvertiseAndListenForPASE();
}

CHIP_ERROR CommissioningWindowManager::AdvertiseAndListenForPASE()
{
    VerifyOrReturnError(mCommissioningTimeoutTimerArmed, CHIP_ERROR_INCORRECT_STATE);

    mPairingSession.Clear();

    ReturnErrorOnFailure(mServer->GetExchangeManager().RegisterUnsolicitedMessageHandlerForType(
        Protocols::SecureChannel::MsgType::PBKDFParamRequest, &mPairingSession));
    mListeningForPASE = true;

    if (mUseECM)
    {
        ReturnErrorOnFailure(SetTemporaryDiscriminator(mECMDiscriminator));
        ReturnErrorOnFailure(mPairingSession.WaitForPairing(mServer->GetSecureSessionManager(), mECMPASEVerifier, mECMIterations,
                                                            ByteSpan(mECMSalt, mECMSaltLength), GetLocalMRPConfig(), this));
    }
    else
    {
        uint32_t iterationCount                      = 0;
        uint8_t salt[kSpake2p_Max_PBKDF_Salt_Length] = { 0 };
        Spake2pVerifierSerialized serializedVerifier = { 0 };
        size_t serializedVerifierLen                 = 0;
        Spake2pVerifier verifier;
        MutableByteSpan saltSpan{ salt };
        MutableByteSpan verifierSpan{ serializedVerifier };

        auto * commissionableDataProvider = DeviceLayer::GetCommissionableDataProvider();
        ReturnErrorOnFailure(commissionableDataProvider->GetSpake2pIterationCount(iterationCount));
        ReturnErrorOnFailure(commissionableDataProvider->GetSpake2pSalt(saltSpan));
        ReturnErrorOnFailure(commissionableDataProvider->GetSpake2pVerifier(verifierSpan, serializedVerifierLen));
        VerifyOrReturnError(Crypto::kSpake2p_VerifierSerialized_Length == serializedVerifierLen, CHIP_ERROR_INVALID_ARGUMENT);
        VerifyOrReturnError(verifierSpan.size() == serializedVerifierLen, CHIP_ERROR_INTERNAL);

        ReturnErrorOnFailure(verifier.Deserialize(ByteSpan(serializedVerifier)));

        ReturnErrorOnFailure(mPairingSession.WaitForPairing(mServer->GetSecureSessionManager(), verifier, iterationCount, saltSpan,
                                                            GetLocalMRPConfig(), this));
    }

    ReturnErrorOnFailure(StartAdvertisement());

    return CHIP_NO_ERROR;
}

CHIP_ERROR CommissioningWindowManager::OpenBasicCommissioningWindow(Seconds16 commissioningTimeout,
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

    mUseECM = false;

    CHIP_ERROR err = OpenCommissioningWindow(commissioningTimeout);
    if (err != CHIP_NO_ERROR)
    {
        Cleanup();
    }

    return err;
}

CHIP_ERROR
CommissioningWindowManager::OpenBasicCommissioningWindowForAdministratorCommissioningCluster(
    System::Clock::Seconds16 commissioningTimeout, FabricIndex fabricIndex, VendorId vendorId)
{
    ReturnErrorOnFailure(OpenBasicCommissioningWindow(commissioningTimeout, CommissioningWindowAdvertisement::kDnssdOnly));

    mOpenerFabricIndex.SetNonNull(fabricIndex);
    mOpenerVendorId.SetNonNull(vendorId);

    return CHIP_NO_ERROR;
}

CHIP_ERROR CommissioningWindowManager::OpenEnhancedCommissioningWindow(Seconds16 commissioningTimeout, uint16_t discriminator,
                                                                       Spake2pVerifier & verifier, uint32_t iterations,
                                                                       ByteSpan salt, FabricIndex fabricIndex, VendorId vendorId)
{
    // Once a device is operational, it shall be commissioned into subsequent fabrics using
    // the operational network only.
    SetBLE(false);

    VerifyOrReturnError(salt.size() <= sizeof(mECMSalt), CHIP_ERROR_INVALID_ARGUMENT);

    memcpy(mECMSalt, salt.data(), salt.size());
    mECMSaltLength = static_cast<uint32_t>(salt.size());

    mFailedCommissioningAttempts = 0;

    mECMDiscriminator = discriminator;
    mECMIterations    = iterations;

    memcpy(&mECMPASEVerifier, &verifier, sizeof(Spake2pVerifier));

    mUseECM = true;

    CHIP_ERROR err = OpenCommissioningWindow(commissioningTimeout);
    if (err != CHIP_NO_ERROR)
    {
        Cleanup();
    }
    else
    {
        mOpenerFabricIndex.SetNonNull(fabricIndex);
        mOpenerVendorId.SetNonNull(vendorId);
    }

    return err;
}

void CommissioningWindowManager::CloseCommissioningWindow()
{
    if (IsCommissioningWindowOpen())
    {
#if CONFIG_NETWORK_LAYER_BLE
        if (mListeningForPASE)
        {
            // We never established PASE, so never armed a fail-safe and hence
            // can't rely on it expiring to close our BLE connection.  Do that
            // manually here.
            mServer->GetBleLayerObject()->CloseAllBleConnections();
        }
#endif
        ChipLogProgress(AppServer, "Closing pairing window");
        Cleanup();
    }
}

CommissioningWindowStatus CommissioningWindowManager::CommissioningWindowStatusForCluster() const
{
    if (mOpenerVendorId.IsNull())
    {
        // Not opened via the cluster.
        return CommissioningWindowStatus::kWindowNotOpen;
    }

    return mWindowStatus;
}

bool CommissioningWindowManager::IsCommissioningWindowOpen() const
{
    return mWindowStatus != CommissioningWindowStatus::kWindowNotOpen;
}

void CommissioningWindowManager::OnFabricRemoved(FabricIndex removedIndex)
{
    if (!mOpenerFabricIndex.IsNull() && mOpenerFabricIndex.Value() == removedIndex)
    {
        // Per spec, we should clear out the stale fabric index.
        mOpenerFabricIndex.SetNull();
    }
}

Dnssd::CommissioningMode CommissioningWindowManager::GetCommissioningMode() const
{
    if (!mListeningForPASE)
    {
        // We should not be advertising ourselves as in commissioning mode.
        // We need to check this before mWindowStatus, because we might have an
        // open window even while we are not listening for PASE.
        return Dnssd::CommissioningMode::kDisabled;
    }

    switch (mWindowStatus)
    {
    case CommissioningWindowStatus::kEnhancedWindowOpen:
        return Dnssd::CommissioningMode::kEnabledEnhanced;
    case CommissioningWindowStatus::kBasicWindowOpen:
        return Dnssd::CommissioningMode::kEnabledBasic;
    default:
        return Dnssd::CommissioningMode::kDisabled;
    }
}

CHIP_ERROR CommissioningWindowManager::StartAdvertisement()
{
#if CHIP_ENABLE_ADDITIONAL_DATA_ADVERTISING
    // notify device layer that advertisement is beginning (to do work such as increment rotating id)
    DeviceLayer::ConfigurationMgr().NotifyOfAdvertisementStart();
#endif

#if CHIP_DEVICE_CONFIG_ENABLE_SED
    if (!mIsBLE && !IsCommissioningWindowOpen())
    {
        DeviceLayer::ConnectivityMgr().RequestSEDActiveMode(true);
    }
#endif

#if CONFIG_NETWORK_LAYER_BLE
    if (mIsBLE)
    {
        CHIP_ERROR err = chip::DeviceLayer::ConnectivityMgr().SetBLEAdvertisingEnabled(true);
        // BLE advertising may just not be supported.  That should not prevent
        // us from opening a commissioning window and advertising over IP.
        if (err == CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE)
        {
            ChipLogProgress(AppServer, "BLE networking available but BLE advertising is not supported");
            err = CHIP_NO_ERROR;
        }
        ReturnErrorOnFailure(err);
    }
#endif // CONFIG_NETWORK_LAYER_BLE

    if (mUseECM)
    {
        mWindowStatus = CommissioningWindowStatus::kEnhancedWindowOpen;
    }
    else
    {
        mWindowStatus = CommissioningWindowStatus::kBasicWindowOpen;
    }

    if (mAppDelegate != nullptr)
    {
        mAppDelegate->OnCommissioningWindowOpened();
    }

    // reset all advertising, switching to our new commissioning mode.
    app::DnssdServer::Instance().StartServer();

    return CHIP_NO_ERROR;
}

CHIP_ERROR CommissioningWindowManager::StopAdvertisement(bool aShuttingDown)
{
    RestoreDiscriminator();

    mServer->GetExchangeManager().UnregisterUnsolicitedMessageHandlerForType(Protocols::SecureChannel::MsgType::PBKDFParamRequest);
    mListeningForPASE = false;
    mPairingSession.Clear();

#if CHIP_DEVICE_CONFIG_ENABLE_SED
    if (!mIsBLE && IsCommissioningWindowOpen())
    {
        DeviceLayer::ConnectivityMgr().RequestSEDActiveMode(false);
    }
#endif

    // If aShuttingDown, don't try to change our DNS-SD advertisements.
    if (!aShuttingDown)
    {
        // Stop advertising commissioning mode, since we're not accepting PASE
        // connections right now.  If we start accepting them again (via
        // AdvertiseAndListenForPASE) that will call StartAdvertisement as needed.
        app::DnssdServer::Instance().StartServer();
    }

#if CONFIG_NETWORK_LAYER_BLE
    if (mIsBLE)
    {
        // Ignore errors from SetBLEAdvertisingEnabled (which could be due to
        // BLE advertising not being supported at all).  Our commissioning
        // window is now closed and we need to notify our delegate of that.
        (void) chip::DeviceLayer::ConnectivityMgr().SetBLEAdvertisingEnabled(false);
    }
#endif // CONFIG_NETWORK_LAYER_BLE

    if (mAppDelegate != nullptr)
    {
        mAppDelegate->OnCommissioningWindowClosed();
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR CommissioningWindowManager::SetTemporaryDiscriminator(uint16_t discriminator)
{
    return app::DnssdServer::Instance().SetEphemeralDiscriminator(MakeOptional(discriminator));
}

CHIP_ERROR CommissioningWindowManager::RestoreDiscriminator()
{
    return app::DnssdServer::Instance().SetEphemeralDiscriminator(NullOptional);
}

void CommissioningWindowManager::HandleCommissioningWindowTimeout(chip::System::Layer * aSystemLayer, void * aAppState)
{
    auto * commissionMgr                           = static_cast<CommissioningWindowManager *>(aAppState);
    commissionMgr->mCommissioningTimeoutTimerArmed = false;
    commissionMgr->CloseCommissioningWindow();
}

void CommissioningWindowManager::OnSessionReleased()
{
    // The PASE session has died, probably due to CloseSession.  Immediately
    // expire the fail-safe, if it's still armed (which it might not be if the
    // PASE session is being released due to the fail-safe expiring or being
    // disarmed).
    //
    // Expiring the fail-safe will make us start listening for new PASE sessions
    // as needed.
    //
    // Note that at this point the fail-safe _must_ be associated with our PASE
    // session, since we arm it when the PASE session is set up, and anything
    // that disarms the fail-safe would also tear down the PASE session.
    ExpireFailSafeIfArmed();
}

void CommissioningWindowManager::ExpireFailSafeIfArmed()
{
    auto & failSafeContext = Server::GetInstance().GetFailSafeContext();
    if (failSafeContext.IsFailSafeArmed())
    {
        failSafeContext.ForceFailSafeTimerExpiry();
    }
}

} // namespace chip
