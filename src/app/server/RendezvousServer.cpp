/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include <app/server/RendezvousServer.h>

#include <app/server/Mdns.h>
#include <lib/core/CHIPError.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/SafeInt.h>
#include <transport/SecureSessionMgr.h>

#if CHIP_ENABLE_OPENTHREAD
#include <platform/ThreadStackManager.h>
#endif
#include <lib/mdns/Advertiser.h>

using namespace ::chip::Inet;
using namespace ::chip::Transport;
using namespace ::chip::DeviceLayer;

namespace chip {

namespace {
void OnPlatformEventWrapper(const DeviceLayer::ChipDeviceEvent * event, intptr_t arg)
{
    RendezvousServer * server = reinterpret_cast<RendezvousServer *>(arg);
    server->OnPlatformEvent(event);
}
} // namespace

void RendezvousServer::OnPlatformEvent(const DeviceLayer::ChipDeviceEvent * event)
{
    if (event->Type == DeviceLayer::DeviceEventType::kCommissioningComplete)
    {
        if (event->CommissioningComplete.status == CHIP_NO_ERROR)
        {
            ChipLogProgress(Discovery, "Commissioning completed successfully");
        }
        else
        {
            ChipLogError(Discovery, "Commissioning errored out with error %" CHIP_ERROR_FORMAT,
                         event->CommissioningComplete.status.Format());
        }
        // reset all advertising
        app::Mdns::StartServer(app::Mdns::CommissioningMode::kDisabled);
    }
    else if (event->Type == DeviceLayer::DeviceEventType::kOperationalNetworkEnabled)
    {
        app::Mdns::AdvertiseOperational();
        ChipLogError(Discovery, "Operational advertising enabled");
    }
}

CHIP_ERROR RendezvousServer::WaitForPairing(const RendezvousParameters & params, uint32_t pbkdf2IterCount, const ByteSpan & salt,
                                            uint16_t passcodeID, Messaging::ExchangeManager * exchangeManager,
                                            TransportMgrBase * transportMgr, SecureSessionMgr * sessionMgr)
{
    VerifyOrReturnError(transportMgr != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(exchangeManager != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(sessionMgr != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(params.HasSetupPINCode() || params.HasPASEVerifier(), CHIP_ERROR_INVALID_ARGUMENT);

#if CONFIG_NETWORK_LAYER_BLE
    VerifyOrReturnError(params.HasAdvertisementDelegate(), CHIP_ERROR_INVALID_ARGUMENT);
#endif

    mAdvDelegate = params.GetAdvertisementDelegate();

    if (params.GetPeerAddress().GetTransportType() == Transport::Type::kBle)
#if !CONFIG_NETWORK_LAYER_BLE
    {
        return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
    }
#endif

    ChipLogProgress(Discovery, "WaitForPairing(): HasAdvertisementDelegate %d", HasAdvertisementDelegate());
    if (HasAdvertisementDelegate())
    {
        ReturnErrorOnFailure(GetAdvertisementDelegate()->StartAdvertisement());
    }

    // reset all advertising, indicating we are in commissioningMode
    // and we were put into this state via a command for additional commissioning
    // NOTE: when device has never been commissioned, Rendezvous will ensure AP is false
    app::Mdns::StartServer(params.HasPASEVerifier() ? app::Mdns::CommissioningMode::kEnabledBasic
                                                    : app::Mdns::CommissioningMode::kEnabledEnhanced);

    mSessionMgr      = sessionMgr;
    mExchangeManager = exchangeManager;

    ReturnErrorOnFailure(mExchangeManager->RegisterUnsolicitedMessageHandlerForType(
        Protocols::SecureChannel::MsgType::PBKDFParamRequest, &mPairingSession));

    uint16_t keyID = 0;
    ReturnErrorOnFailure(mIDAllocator->Allocate(keyID));

    if (params.HasPASEVerifier())
    {
        ReturnErrorOnFailure(
            mPairingSession.WaitForPairing(params.GetPASEVerifier(), pbkdf2IterCount, salt, passcodeID, keyID, this));
    }
    else
    {
        ReturnErrorOnFailure(mPairingSession.WaitForPairing(params.GetSetupPINCode(), pbkdf2IterCount, salt, keyID, this));
    }

    ReturnErrorOnFailure(mPairingSession.MessageDispatch().Init(transportMgr));
    mPairingSession.MessageDispatch().SetPeerAddress(params.GetPeerAddress());

    return CHIP_NO_ERROR;
}

void RendezvousServer::Cleanup()
{
    mExchangeManager->UnregisterUnsolicitedMessageHandlerForType(Protocols::SecureChannel::MsgType::PBKDFParamRequest);

    if (HasAdvertisementDelegate())
    {
        GetAdvertisementDelegate()->StopAdvertisement();
    }

    // reset all advertising
    app::Mdns::StartServer(app::Mdns::CommissioningMode::kDisabled);
}

void RendezvousServer::OnSessionEstablishmentError(CHIP_ERROR err)
{
    Cleanup();

    ChipLogProgress(AppServer, "OnSessionEstablishmentError: %s", ErrorStr(err));
    ChipLogProgress(AppServer, "Failed in SPAKE2+ handshake");

    if (mDelegate != nullptr)
    {
        mDelegate->OnRendezvousStopped();
    }
}

void RendezvousServer::OnSessionEstablished()
{
    CHIP_ERROR err =
        mSessionMgr->NewPairing(Optional<Transport::PeerAddress>::Value(mPairingSession.GetPeerAddress()),
                                mPairingSession.GetPeerNodeId(), &mPairingSession, SecureSession::SessionRole::kResponder, 0);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Ble, "Failed in setting up secure channel: err %s", ErrorStr(err));
        OnSessionEstablishmentError(err);
        return;
    }

    ChipLogProgress(AppServer, "Device completed SPAKE2+ handshake");
    if (mDelegate != nullptr)
    {
        mDelegate->OnRendezvousStarted();
    }

    DeviceLayer::PlatformMgr().AddEventHandler(OnPlatformEventWrapper, reinterpret_cast<intptr_t>(this));

    if (mPairingSession.GetPeerAddress().GetTransportType() == Transport::Type::kBle)
    {
        Cleanup();
    }
    else
    {
        Cleanup();
    }

    ChipLogProgress(AppServer, "Device completed Rendezvous process");
}
} // namespace chip
