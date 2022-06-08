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

/**
 *    @file
 *      Implementation of SetUp Code Pairer, a class that parses a given
 *      setup code and uses the extracted informations to discover and
 *      filter commissionables nodes, before initiating the pairing process.
 *
 */

#include <controller/SetUpCodePairer.h>

#include <controller/CHIPDeviceController.h>
#include <lib/dnssd/Resolver.h>
#include <lib/support/CodeUtils.h>
#include <system/SystemClock.h>

constexpr uint32_t kDeviceDiscoveredTimeout = CHIP_CONFIG_SETUP_CODE_PAIRER_DISCOVERY_TIMEOUT_SECS * chip::kMillisecondsPerSecond;

namespace chip {
namespace Controller {

CHIP_ERROR SetUpCodePairer::PairDevice(NodeId remoteId, const char * setUpCode, SetupCodePairerBehaviour commission)
{
    VerifyOrReturnError(mSystemLayer != nullptr, CHIP_ERROR_INCORRECT_STATE);

    SetupPayload payload;
    mConnectionType = commission;

    bool isQRCode = strncmp(setUpCode, kQRCodePrefix, strlen(kQRCodePrefix)) == 0;
    if (isQRCode)
    {
        ReturnErrorOnFailure(QRCodeSetupPayloadParser(setUpCode).populatePayload(payload));
        VerifyOrReturnError(payload.isValidQRCodePayload(), CHIP_ERROR_INVALID_ARGUMENT);
    }
    else
    {
        ReturnErrorOnFailure(ManualSetupPayloadParser(setUpCode).populatePayload(payload));
        VerifyOrReturnError(payload.isValidManualCode(), CHIP_ERROR_INVALID_ARGUMENT);
    }

    mRemoteId     = remoteId;
    mSetUpPINCode = payload.setUpPINCode;

    ResetDiscoveryState();

    ReturnErrorOnFailure(Connect(payload));

    return mSystemLayer->StartTimer(System::Clock::Milliseconds32(kDeviceDiscoveredTimeout), OnDeviceDiscoveredTimeoutCallback,
                                    this);
}

CHIP_ERROR SetUpCodePairer::Connect(SetupPayload & payload)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    bool isRunning = false;

    bool searchOverAll = payload.rendezvousInformation == RendezvousInformationFlag::kNone;
    if (searchOverAll || payload.rendezvousInformation == RendezvousInformationFlag::kBLE)
    {
        if (CHIP_NO_ERROR == (err = StartDiscoverOverBle(payload)))
        {
            isRunning = true;
        }
        VerifyOrReturnError(searchOverAll || CHIP_NO_ERROR == err, err);
    }

    if (searchOverAll || payload.rendezvousInformation == RendezvousInformationFlag::kSoftAP)
    {
        if (CHIP_NO_ERROR == (err = StartDiscoverOverSoftAP(payload)))
        {
            isRunning = true;
        }
        VerifyOrReturnError(searchOverAll || CHIP_NO_ERROR == err, err);
    }

    // We always want to search on network because any node that has already been commissioned will use on-network regardless of the
    // QR code flag.
    if (CHIP_NO_ERROR == (err = StartDiscoverOverIP(payload)))
    {
        isRunning = true;
    }
    VerifyOrReturnError(searchOverAll || CHIP_NO_ERROR == err, err);

    return isRunning ? CHIP_NO_ERROR : CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

CHIP_ERROR SetUpCodePairer::StartDiscoverOverBle(SetupPayload & payload)
{
#if CONFIG_NETWORK_LAYER_BLE
#if CHIP_DEVICE_CONFIG_ENABLE_BOTH_COMMISSIONER_AND_COMMISSIONEE
    VerifyOrReturnError(mCommissioner != nullptr, CHIP_ERROR_INCORRECT_STATE);
    mCommissioner->ConnectBleTransportToSelf();
#endif // CHIP_DEVICE_CONFIG_ENABLE_BOTH_COMMISSIONER_AND_COMMISSIONEE
    VerifyOrReturnError(mBleLayer != nullptr, CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

    ChipLogProgress(Controller, "Starting commissioning discovery over BLE");

    // Handle possibly-sync callbacks.
    mWaitingForDiscovery[kBLETransport] = true;
    CHIP_ERROR err = mBleLayer->NewBleConnectionByDiscriminator(payload.discriminator, this, OnDiscoveredDeviceOverBleSuccess,
                                                                OnDiscoveredDeviceOverBleError);
    if (err != CHIP_NO_ERROR)
    {
        mWaitingForDiscovery[kBLETransport] = false;
    }
    return err;
#else
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
#endif // CONFIG_NETWORK_LAYER_BLE
}

CHIP_ERROR SetUpCodePairer::StopConnectOverBle()
{
    mWaitingForDiscovery[kBLETransport] = false;
#if CONFIG_NETWORK_LAYER_BLE
    VerifyOrReturnError(mBleLayer != nullptr, CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    ChipLogDetail(Controller, "Stopping commissioning discovery over BLE");
    return mBleLayer->CancelBleIncompleteConnection();
#else
    return CHIP_NO_ERROR;
#endif // CONFIG_NETWORK_LAYER_BLE
}

CHIP_ERROR SetUpCodePairer::StartDiscoverOverIP(SetupPayload & payload)
{
    ChipLogProgress(Controller, "Starting commissioning discovery over DNS-SD");

    currentFilter.type = payload.isShortDiscriminator ? Dnssd::DiscoveryFilterType::kShortDiscriminator
                                                      : Dnssd::DiscoveryFilterType::kLongDiscriminator;
    currentFilter.code =
        payload.isShortDiscriminator ? static_cast<uint16_t>((payload.discriminator >> 8) & 0x0F) : payload.discriminator;
    // Handle possibly-sync callbacks.
    mWaitingForDiscovery[kIPTransport] = true;
    CHIP_ERROR err                     = mCommissioner->DiscoverCommissionableNodes(currentFilter);
    if (err != CHIP_NO_ERROR)
    {
        mWaitingForDiscovery[kIPTransport] = false;
    }
    return err;
}

CHIP_ERROR SetUpCodePairer::StopConnectOverIP()
{
    ChipLogDetail(Controller, "Stopping commissioning discovery over DNS-SD");

    mWaitingForDiscovery[kIPTransport] = false;
    currentFilter.type                 = Dnssd::DiscoveryFilterType::kNone;
    return CHIP_NO_ERROR;
}

CHIP_ERROR SetUpCodePairer::StartDiscoverOverSoftAP(SetupPayload & payload)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

CHIP_ERROR SetUpCodePairer::StopConnectOverSoftAP()
{
    mWaitingForDiscovery[kSoftAPTransport] = false;
    return CHIP_NO_ERROR;
}

bool SetUpCodePairer::ConnectToDiscoveredDevice()
{
    mSystemLayer->CancelTimer(OnDeviceDiscoveredTimeoutCallback, this);

    if (mWaitingForPASE)
    {
        // Nothing to do.  Just wait until we either succeed or fail at that
        // PASE session establishment.
        return false;
    }

    for (auto & storedParams : mDiscoveredParameters)
    {
        if (!storedParams.HasPeerAddress())
        {
            continue;
        }

        // Clear out those params, since we will kick off a connection to them
        // now.
        RendezvousParameters params(storedParams);
        storedParams = RendezvousParameters();

        params.SetSetupPINCode(mSetUpPINCode);

#if CHIP_PROGRESS_LOGGING
        char buf[Transport::PeerAddress::kMaxToStringSize];
        params.GetPeerAddress().ToString(buf);
        ChipLogProgress(Controller, "Attempting PASE connection to %s", buf);
#endif // CHIP_PROGRESS_LOGGING

        // Handle possibly-sync call backs from attempts to establish PASE.
        ExpectPASEEstablishment();

        CHIP_ERROR err;
        if (mConnectionType == SetupCodePairerBehaviour::kCommission)
        {
            err = mCommissioner->PairDevice(mRemoteId, params);
        }
        else
        {
            err = mCommissioner->EstablishPASEConnection(mRemoteId, params);
        }

        LogErrorOnFailure(err);
        if (err == CHIP_NO_ERROR)
        {
            return true;
        }

        // Failed to start establishing PASE.
        PASEEstablishmentComplete();
    }

    return false;
}

#if CONFIG_NETWORK_LAYER_BLE
void SetUpCodePairer::OnDiscoveredDeviceOverBle(BLE_CONNECTION_OBJECT connObj)
{
    ChipLogProgress(Controller, "Discovered device to be commissioned over BLE");

    mWaitingForDiscovery[kBLETransport] = false;

    // Probably safe to stop connections over other transports at this point?
    LogErrorOnFailure(StopConnectOverIP());
    LogErrorOnFailure(StopConnectOverSoftAP());

    Transport::PeerAddress peerAddress   = Transport::PeerAddress::BLE();
    mDiscoveredParameters[kBLETransport] = RendezvousParameters().SetPeerAddress(peerAddress).SetConnectionObject(connObj);
    ConnectToDiscoveredDevice();
}

void SetUpCodePairer::OnDiscoveredDeviceOverBleSuccess(void * appState, BLE_CONNECTION_OBJECT connObj)
{
    (static_cast<SetUpCodePairer *>(appState))->OnDiscoveredDeviceOverBle(connObj);
}

void SetUpCodePairer::OnDiscoveredDeviceOverBleError(void * appState, CHIP_ERROR err)
{
    static_cast<SetUpCodePairer *>(appState)->OnBLEDiscoveryError(err);
}

void SetUpCodePairer::OnBLEDiscoveryError(CHIP_ERROR err)
{
    ChipLogError(Controller, "Commissioning discovery over BLE failed: %" CHIP_ERROR_FORMAT, err.Format());
    mWaitingForDiscovery[kBLETransport] = false;
    LogErrorOnFailure(err);
}
#endif // CONFIG_NETWORK_LAYER_BLE

bool SetUpCodePairer::NodeMatchesCurrentFilter(const Dnssd::DiscoveredNodeData & nodeData) const
{
    if (nodeData.commissionData.commissioningMode == 0)
    {
        return false;
    }

    switch (currentFilter.type)
    {
    case Dnssd::DiscoveryFilterType::kShortDiscriminator:
        return ((nodeData.commissionData.longDiscriminator >> 8) & 0x0F) == currentFilter.code;
    case Dnssd::DiscoveryFilterType::kLongDiscriminator:
        return nodeData.commissionData.longDiscriminator == currentFilter.code;
    default:
        return false;
    }
    return false;
}

void SetUpCodePairer::NotifyCommissionableDeviceDiscovered(const Dnssd::DiscoveredNodeData & nodeData)
{
    if (!NodeMatchesCurrentFilter(nodeData))
    {
        return;
    }

    ChipLogProgress(Controller, "Discovered device to be commissioned over DNS-SD");

    // Don't stop trying to connect over BLE, because we may be dealing with
    // stale DNS-SD records.
    LogErrorOnFailure(StopConnectOverIP());
    LogErrorOnFailure(StopConnectOverSoftAP());

    Inet::InterfaceId interfaceId =
        nodeData.resolutionData.ipAddress[0].IsIPv6LinkLocal() ? nodeData.resolutionData.interfaceId : Inet::InterfaceId::Null();
    Transport::PeerAddress peerAddress =
        Transport::PeerAddress::UDP(nodeData.resolutionData.ipAddress[0], nodeData.resolutionData.port, interfaceId);
    mDiscoveredParameters[kIPTransport] = RendezvousParameters().SetPeerAddress(peerAddress);
    ConnectToDiscoveredDevice();
}

bool SetUpCodePairer::TryNextRendezvousParameters()
{
    if (ConnectToDiscoveredDevice())
    {
        ChipLogProgress(Controller, "Trying connection to commissionee over different transport");
        return true;
    }

    for (const auto & waiting : mWaitingForDiscovery)
    {
        if (waiting)
        {
            ChipLogProgress(Controller, "Waiting to discover commissionees that match our filters");
            return true;
        }
    }

    return false;
}

void SetUpCodePairer::ResetDiscoveryState()
{
    for (auto & waiting : mWaitingForDiscovery)
    {
        waiting = false;
    }

    for (auto & params : mDiscoveredParameters)
    {
        params = RendezvousParameters();
    }
}

void SetUpCodePairer::ExpectPASEEstablishment()
{
    mWaitingForPASE = true;
    auto * delegate = mCommissioner->GetPairingDelegate();
    if (this == delegate)
    {
        // This should really not happen, but if it does, do nothing, to avoid
        // delegate loops.
        return;
    }

    mPairingDelegate = delegate;
    mCommissioner->RegisterPairingDelegate(this);
}

void SetUpCodePairer::PASEEstablishmentComplete()
{
    mWaitingForPASE = false;
    mCommissioner->RegisterPairingDelegate(mPairingDelegate);
    mPairingDelegate = nullptr;
}

void SetUpCodePairer::OnStatusUpdate(DevicePairingDelegate::Status status)
{
    if (mPairingDelegate)
    {
        mPairingDelegate->OnStatusUpdate(status);
    }
}

void SetUpCodePairer::OnPairingComplete(CHIP_ERROR error)
{
    // Save the pairing delegate so we can notify it.  We want to notify it
    // _after_ we restore the state on the commissioner, in case the delegate
    // ends up immediately calling back into the commissioner again when
    // notified.
    auto * pairingDelegate = mPairingDelegate;
    PASEEstablishmentComplete();

    if (CHIP_NO_ERROR == error)
    {
        // StopConnectOverBle calls CancelBleIncompleteConnection, which will
        // cancel connections that are in fact completed. In particular, if we
        // just established PASE over BLE calling StopConnectOverBle here
        // unconditionally would cancel the BLE connection underlying the PASE
        // session.  So make sure to only call StopConnectOverBle if we're still
        // waiting to hear back on the BLE discovery bits.
        if (mWaitingForDiscovery[kBLETransport])
        {
            StopConnectOverBle();
        }
        StopConnectOverIP();
        StopConnectOverSoftAP();
        ResetDiscoveryState();
        pairingDelegate->OnPairingComplete(error);
        return;
    }

    // We failed to establish PASE.  Try the next thing we have discovered, if
    // any.
    if (TryNextRendezvousParameters())
    {
        // Keep waiting until that finishes.  Don't call OnPairingComplete yet.
        return;
    }

    pairingDelegate->OnPairingComplete(error);
}

void SetUpCodePairer::OnPairingDeleted(CHIP_ERROR error)
{
    if (mPairingDelegate)
    {
        mPairingDelegate->OnPairingDeleted(error);
    }
}

void SetUpCodePairer::OnCommissioningComplete(NodeId deviceId, CHIP_ERROR error)
{
    // Not really expecting this, but handle it anyway.
    if (mPairingDelegate)
    {
        mPairingDelegate->OnCommissioningComplete(deviceId, error);
    }
}

void SetUpCodePairer::OnDeviceDiscoveredTimeoutCallback(System::Layer * layer, void * context)
{
    auto * pairer = static_cast<SetUpCodePairer *>(context);
    LogErrorOnFailure(pairer->StopConnectOverBle());
    LogErrorOnFailure(pairer->StopConnectOverIP());
    LogErrorOnFailure(pairer->StopConnectOverSoftAP());
    pairer->mCommissioner->OnSessionEstablishmentError(CHIP_ERROR_TIMEOUT);
}

} // namespace Controller
} // namespace chip
