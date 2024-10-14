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
#include <memory>
#include <system/SystemClock.h>
#include <tracing/metric_event.h>

constexpr uint32_t kDeviceDiscoveredTimeout = CHIP_CONFIG_SETUP_CODE_PAIRER_DISCOVERY_TIMEOUT_SECS * chip::kMillisecondsPerSecond;

using namespace chip::Tracing;

namespace chip {
namespace Controller {

namespace {

CHIP_ERROR GetPayload(const char * setUpCode, SetupPayload & payload)
{
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

    return CHIP_NO_ERROR;
}
} // namespace

SetUpCodePairer::~SetUpCodePairer()
{
#if CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF
    DeviceLayer::ConnectivityMgr().WiFiPAFCancelConnect();
#endif
}

CHIP_ERROR SetUpCodePairer::PairDevice(NodeId remoteId, const char * setUpCode, SetupCodePairerBehaviour commission,
                                       DiscoveryType discoveryType, Optional<Dnssd::CommonResolutionData> resolutionData)
{
    VerifyOrReturnErrorWithMetric(kMetricSetupCodePairerPairDevice, mSystemLayer != nullptr, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnErrorWithMetric(kMetricSetupCodePairerPairDevice, remoteId != kUndefinedNodeId, CHIP_ERROR_INVALID_ARGUMENT);

    SetupPayload payload;
    ReturnErrorOnFailure(GetPayload(setUpCode, payload));

    if (resolutionData.HasValue())
    {
        VerifyOrReturnErrorWithMetric(kMetricSetupCodePairerPairDevice, discoveryType != DiscoveryType::kAll,
                                      CHIP_ERROR_INVALID_ARGUMENT);
        if (mRemoteId == remoteId && mSetUpPINCode == payload.setUpPINCode && mConnectionType == commission &&
            mDiscoveryType == discoveryType)
        {
            NotifyCommissionableDeviceDiscovered(resolutionData.Value());
            return CHIP_NO_ERROR;
        }
    }

    mConnectionType = commission;
    mDiscoveryType  = discoveryType;
    mRemoteId       = remoteId;
    mSetUpPINCode   = payload.setUpPINCode;

    ResetDiscoveryState();

    if (resolutionData.HasValue())
    {
        NotifyCommissionableDeviceDiscovered(resolutionData.Value());
        return CHIP_NO_ERROR;
    }

    ReturnErrorOnFailureWithMetric(kMetricSetupCodePairerPairDevice, Connect(payload));
    auto errorCode =
        mSystemLayer->StartTimer(System::Clock::Milliseconds32(kDeviceDiscoveredTimeout), OnDeviceDiscoveredTimeoutCallback, this);
    if (CHIP_NO_ERROR == errorCode)
    {
        MATTER_LOG_METRIC_BEGIN(kMetricSetupCodePairerPairDevice);
    }
    return errorCode;
}

CHIP_ERROR SetUpCodePairer::Connect(SetupPayload & payload)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    bool isRunning = false;

    bool searchOverAll = !payload.rendezvousInformation.HasValue();

    if (mDiscoveryType == DiscoveryType::kAll)
    {
        if (searchOverAll || payload.rendezvousInformation.Value().Has(RendezvousInformationFlag::kBLE))
        {
            if (CHIP_NO_ERROR == (err = StartDiscoverOverBle(payload)))
            {
                isRunning = true;
            }
            VerifyOrReturnError(searchOverAll || CHIP_NO_ERROR == err || CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE == err, err);
        }

        if (searchOverAll || payload.rendezvousInformation.Value().Has(RendezvousInformationFlag::kSoftAP))
        {
            if (CHIP_NO_ERROR == (err = StartDiscoverOverSoftAP(payload)))
            {
                isRunning = true;
            }
            VerifyOrReturnError(searchOverAll || CHIP_NO_ERROR == err || CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE == err, err);
        }
        if (searchOverAll || payload.rendezvousInformation.Value().Has(RendezvousInformationFlag::kWiFiPAF))
        {
            ChipLogProgress(Controller, "WiFi-PAF: has RendezvousInformationFlag::kWiFiPAF");
            if (CHIP_NO_ERROR == (err = StartDiscoverOverWiFiPAF(payload)))
            {
                isRunning = true;
            }
            VerifyOrReturnError(searchOverAll || CHIP_NO_ERROR == err || CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE == err, err);
        }
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
    // Make sure to not call CancelBleIncompleteConnection unless we are in fact
    // waiting on BLE discovery.  It will cancel connections that are in fact
    // completed. In particular, if we just established PASE over BLE calling
    // CancelBleIncompleteConnection here unconditionally would cancel the BLE
    // connection underlying the PASE session.  So make sure to only call
    // CancelBleIncompleteConnection if we're still waiting to hear back on the
    // BLE discovery bits.
    if (!mWaitingForDiscovery[kBLETransport])
    {
        return CHIP_NO_ERROR;
    }

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

    auto & discriminator = payload.discriminator;
    if (discriminator.IsShortDiscriminator())
    {
        mCurrentFilter.type = Dnssd::DiscoveryFilterType::kShortDiscriminator;
        mCurrentFilter.code = discriminator.GetShortValue();
    }
    else
    {
        mCurrentFilter.type = Dnssd::DiscoveryFilterType::kLongDiscriminator;
        mCurrentFilter.code = discriminator.GetLongValue();
    }
    mPayloadVendorID  = payload.vendorID;
    mPayloadProductID = payload.productID;

    // Handle possibly-sync callbacks.
    mWaitingForDiscovery[kIPTransport] = true;
    CHIP_ERROR err                     = mCommissioner->DiscoverCommissionableNodes(mCurrentFilter);
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
    mCurrentFilter.type                = Dnssd::DiscoveryFilterType::kNone;
    mPayloadVendorID                   = kNotAvailable;
    mPayloadProductID                  = kNotAvailable;

    mCommissioner->StopCommissionableDiscovery();
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

CHIP_ERROR SetUpCodePairer::StartDiscoverOverWiFiPAF(SetupPayload & payload)
{
#if CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF
    ChipLogProgress(Controller, "Starting commissioning discovery over WiFiPAF");
    VerifyOrReturnError(mCommissioner != nullptr, CHIP_ERROR_INCORRECT_STATE);
    mWaitingForDiscovery[kWiFiPAFTransport] = true;
    CHIP_ERROR err = DeviceLayer::ConnectivityMgr().WiFiPAFConnect(payload.discriminator, (void *) this, OnWiFiPAFSubscribeComplete,
                                                                   OnWiFiPAFSubscribeError);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "Commissioning discovery over WiFiPAF failed, err = %" CHIP_ERROR_FORMAT, err.Format());
        mWaitingForDiscovery[kWiFiPAFTransport] = false;
    }
    return err;
#else
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
#endif // CONFIG_NETWORK_LAYER_BLE
}

CHIP_ERROR SetUpCodePairer::StopConnectOverWiFiPAF()
{
    mWaitingForDiscovery[kWiFiPAFTransport] = false;
    return CHIP_NO_ERROR;
}

bool SetUpCodePairer::ConnectToDiscoveredDevice()
{
    if (mWaitingForPASE)
    {
        // Nothing to do.  Just wait until we either succeed or fail at that
        // PASE session establishment.
        return false;
    }

    while (!mDiscoveredParameters.empty())
    {
        // Grab the first element from the queue and try connecting to it.
        // Remove it from the queue before we try to connect, in case the
        // connection attempt fails and calls right back into us to try the next
        // thing.
        SetUpCodePairerParameters params(mDiscoveredParameters.front());
        mDiscoveredParameters.pop_front();

        params.SetSetupPINCode(mSetUpPINCode);

#if CHIP_PROGRESS_LOGGING
        char buf[Transport::PeerAddress::kMaxToStringSize];
        params.GetPeerAddress().ToString(buf);
        ChipLogProgress(Controller, "Attempting PASE connection to %s", buf);
#endif // CHIP_PROGRESS_LOGGING

        // Handle possibly-sync call backs from attempts to establish PASE.
        ExpectPASEEstablishment();

        if (params.GetPeerAddress().GetTransportType() == Transport::Type::kUdp)
        {
            mCurrentPASEParameters.SetValue(params);
        }

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

        // Failed to start establishing PASE.  Move on to the next item.
        PASEEstablishmentComplete();
    }

    return false;
}

#if CONFIG_NETWORK_LAYER_BLE
void SetUpCodePairer::OnDiscoveredDeviceOverBle(BLE_CONNECTION_OBJECT connObj)
{
    ChipLogProgress(Controller, "Discovered device to be commissioned over BLE");

    mWaitingForDiscovery[kBLETransport] = false;

    // In order to not wait for all the possible addresses discovered over mdns to
    // be tried before trying to connect over BLE, the discovered connection object is
    // inserted at the beginning of the list.
    //
    // It makes it the 'next' thing to try to connect to if there are already some
    // discovered parameters in the list.
    mDiscoveredParameters.emplace_front(connObj);
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

#if CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF
void SetUpCodePairer::OnDiscoveredDeviceOverWifiPAF()
{
    ChipLogProgress(Controller, "Discovered device to be commissioned over WiFiPAF, RemoteId: %lu", mRemoteId);

    mWaitingForDiscovery[kWiFiPAFTransport] = false;
    auto param                              = SetUpCodePairerParameters();
    param.SetPeerAddress(Transport::PeerAddress(Transport::Type::kWiFiPAF, mRemoteId));
    mDiscoveredParameters.emplace_back(param);
    ConnectToDiscoveredDevice();
}

void SetUpCodePairer::OnWifiPAFDiscoveryError(CHIP_ERROR err)
{
    ChipLogError(Controller, "Commissioning discovery over WiFiPAF failed: %" CHIP_ERROR_FORMAT, err.Format());
    mWaitingForDiscovery[kWiFiPAFTransport] = false;
}

void SetUpCodePairer::OnWiFiPAFSubscribeComplete(void * appState)
{
    auto self = (SetUpCodePairer *) appState;
    self->OnDiscoveredDeviceOverWifiPAF();
}

void SetUpCodePairer::OnWiFiPAFSubscribeError(void * appState, CHIP_ERROR err)
{
    auto self = (SetUpCodePairer *) appState;
    self->OnWifiPAFDiscoveryError(err);
}
#endif

bool SetUpCodePairer::IdIsPresent(uint16_t vendorOrProductID)
{
    return vendorOrProductID != kNotAvailable;
}

bool SetUpCodePairer::NodeMatchesCurrentFilter(const Dnssd::DiscoveredNodeData & discNodeData) const
{
    if (!discNodeData.Is<Dnssd::CommissionNodeData>())
    {
        return false;
    }

    const Dnssd::CommissionNodeData & nodeData = discNodeData.Get<Dnssd::CommissionNodeData>();
    if (nodeData.commissioningMode == 0)
    {
        ChipLogProgress(Controller, "Discovered device does not have an open commissioning window.");
        return false;
    }

    // The advertisement may not include a vendor id.
    if (IdIsPresent(mPayloadVendorID) && IdIsPresent(nodeData.vendorId) && mPayloadVendorID != nodeData.vendorId)
    {
        ChipLogProgress(Controller, "Discovered device does not match our vendor id.");
        return false;
    }

    // The advertisement may not include a product id.
    if (IdIsPresent(mPayloadProductID) && IdIsPresent(nodeData.productId) && mPayloadProductID != nodeData.productId)
    {
        ChipLogProgress(Controller, "Discovered device does not match our product id.");
        return false;
    }

    bool discriminatorMatches = false;
    switch (mCurrentFilter.type)
    {
    case Dnssd::DiscoveryFilterType::kShortDiscriminator:
        discriminatorMatches = (((nodeData.longDiscriminator >> 8) & 0x0F) == mCurrentFilter.code);
        break;
    case Dnssd::DiscoveryFilterType::kLongDiscriminator:
        discriminatorMatches = (nodeData.longDiscriminator == mCurrentFilter.code);
        break;
    case Dnssd::DiscoveryFilterType::kNone:
        ChipLogDetail(Controller, "Filter type none; all matches will fail");
        return false;
    default:
        ChipLogError(Controller, "Unknown filter type; all matches will fail");
        return false;
    }
    if (!discriminatorMatches)
    {
        ChipLogProgress(Controller, "Discovered device does not match our discriminator.");
    }
    return discriminatorMatches;
}

void SetUpCodePairer::NotifyCommissionableDeviceDiscovered(const Dnssd::DiscoveredNodeData & nodeData)
{
    if (!NodeMatchesCurrentFilter(nodeData))
    {
        return;
    }

    ChipLogProgress(Controller, "Discovered device to be commissioned over DNS-SD");

    NotifyCommissionableDeviceDiscovered(nodeData.Get<Dnssd::CommissionNodeData>());
}

void SetUpCodePairer::NotifyCommissionableDeviceDiscovered(const Dnssd::CommonResolutionData & resolutionData)
{
    if (mDiscoveryType == DiscoveryType::kDiscoveryNetworkOnlyWithoutPASEAutoRetry)
    {
        // If the discovery type does not want the PASE auto retry mechanism, we will just store
        // a single IP. So the discovery process is stopped as it won't be of any help anymore.
        StopConnectOverIP();
        mDiscoveredParameters.emplace_back(resolutionData, 0);
    }
    else
    {
        for (size_t i = 0; i < resolutionData.numIPs; i++)
        {
            mDiscoveredParameters.emplace_back(resolutionData, i);
        }
    }

    ConnectToDiscoveredDevice();
}

bool SetUpCodePairer::StopPairing(NodeId remoteId)
{
    VerifyOrReturnValue(mRemoteId != kUndefinedNodeId, false);
    VerifyOrReturnValue(remoteId == kUndefinedNodeId || remoteId == mRemoteId, false);

    if (mWaitingForPASE)
    {
        PASEEstablishmentComplete();
    }

    ResetDiscoveryState();
    mRemoteId = kUndefinedNodeId;
    return true;
}

bool SetUpCodePairer::TryNextRendezvousParameters()
{
    if (ConnectToDiscoveredDevice())
    {
        ChipLogProgress(Controller, "Trying connection to commissionee over different transport");
        return true;
    }

    if (DiscoveryInProgress())
    {
        ChipLogProgress(Controller, "Waiting to discover commissionees that match our filters");
        return true;
    }

    return false;
}

bool SetUpCodePairer::DiscoveryInProgress() const
{
    for (const auto & waiting : mWaitingForDiscovery)
    {
        if (waiting)
        {
            return true;
        }
    }

    return false;
}

void SetUpCodePairer::ResetDiscoveryState()
{
    StopConnectOverBle();
    StopConnectOverIP();
    StopConnectOverSoftAP();
    StopConnectOverWiFiPAF();

    // Just in case any of those failed to reset the waiting state properly.
    for (auto & waiting : mWaitingForDiscovery)
    {
        waiting = false;
    }

    mDiscoveredParameters.clear();
    mCurrentPASEParameters.ClearValue();
    mLastPASEError = CHIP_NO_ERROR;

    mSystemLayer->CancelTimer(OnDeviceDiscoveredTimeoutCallback, this);
}

void SetUpCodePairer::ExpectPASEEstablishment()
{
    VerifyOrDie(!mWaitingForPASE);
    mWaitingForPASE = true;
    auto * delegate = mCommissioner->GetPairingDelegate();
    VerifyOrDie(delegate != this);
    mPairingDelegate = delegate;
    mCommissioner->RegisterPairingDelegate(this);
}

void SetUpCodePairer::PASEEstablishmentComplete()
{
    VerifyOrDie(mWaitingForPASE);
    mWaitingForPASE = false;
    mCommissioner->RegisterPairingDelegate(mPairingDelegate);
    mPairingDelegate = nullptr;
}

void SetUpCodePairer::OnStatusUpdate(DevicePairingDelegate::Status status)
{
    if (status == DevicePairingDelegate::Status::SecurePairingFailed)
    {
        // If we're still waiting on discovery, don't propagate this failure
        // (which is due to PASE failure with something we discovered, but the
        // "something" may not have been the right thing) for now.  Wait until
        // discovery completes.  Then we will either succeed and notify
        // accordingly or time out and land in OnStatusUpdate again, but at that
        // point we will not be waiting on discovery anymore.
        if (!mDiscoveredParameters.empty())
        {
            ChipLogProgress(Controller, "Ignoring SecurePairingFailed status for now; we have more discovered devices to try");
            return;
        }

        if (DiscoveryInProgress())
        {
            ChipLogProgress(Controller,
                            "Ignoring SecurePairingFailed status for now; we are waiting to see if we discover more devices");
            return;
        }
    }

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
        ChipLogProgress(Controller, "PASE session established with commissionee. Stopping discovery.");
        ResetDiscoveryState();
        mRemoteId = kUndefinedNodeId;
        MATTER_LOG_METRIC_END(kMetricSetupCodePairerPairDevice, error);
        if (pairingDelegate != nullptr)
        {
            pairingDelegate->OnPairingComplete(error);
        }
        return;
    }

    // It may happen that there is a stale DNS entry. If so, ReconfirmRecord will flush
    // the record from the daemon cache once it determines that it is invalid.
    // It may not help for this particular resolve, but may help subsequent resolves.
    if (CHIP_ERROR_TIMEOUT == error && mCurrentPASEParameters.HasValue())
    {
        const auto & params = mCurrentPASEParameters.Value();
        const auto & peer   = params.GetPeerAddress();
        const auto & ip     = peer.GetIPAddress();
        auto err            = Dnssd::Resolver::Instance().ReconfirmRecord(params.mHostName, ip, params.mInterfaceId);
        if (CHIP_NO_ERROR != err && CHIP_ERROR_NOT_IMPLEMENTED != err)
        {
            ChipLogError(Controller, "Error when verifying the validity of an address: %" CHIP_ERROR_FORMAT, err.Format());
        }
    }
    mCurrentPASEParameters.ClearValue();

    // We failed to establish PASE.  Try the next thing we have discovered, if
    // any.
    if (TryNextRendezvousParameters())
    {
        // Keep waiting until that finishes.  Don't call OnPairingComplete yet.
        mLastPASEError = error;
        return;
    }

    MATTER_LOG_METRIC_END(kMetricSetupCodePairerPairDevice, error);
    if (pairingDelegate != nullptr)
    {
        pairingDelegate->OnPairingComplete(error);
    }
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
    ChipLogError(Controller, "Discovery timed out");
    auto * pairer = static_cast<SetUpCodePairer *>(context);
    LogErrorOnFailure(pairer->StopConnectOverBle());
    LogErrorOnFailure(pairer->StopConnectOverIP());
    LogErrorOnFailure(pairer->StopConnectOverSoftAP());
    if (!pairer->mWaitingForPASE && pairer->mDiscoveredParameters.empty())
    {
        // We're not waiting on any more PASE attempts, and we're not going to
        // discover anything at this point, so we should just notify our
        // listener.
        CHIP_ERROR err = pairer->mLastPASEError;
        if (err == CHIP_NO_ERROR)
        {
            err = CHIP_ERROR_TIMEOUT;
        }
        MATTER_LOG_METRIC_END(kMetricSetupCodePairerPairDevice, err);
        pairer->mCommissioner->OnSessionEstablishmentError(err);
    }
}

SetUpCodePairerParameters::SetUpCodePairerParameters(const Dnssd::CommonResolutionData & data, size_t index)
{
    mInterfaceId = data.interfaceId;
    Platform::CopyString(mHostName, data.hostName);

    auto & ip = data.ipAddress[index];
    SetPeerAddress(Transport::PeerAddress::UDP(ip, data.port, ip.IsIPv6LinkLocal() ? data.interfaceId : Inet::InterfaceId::Null()));

    if (data.mrpRetryIntervalIdle.has_value())
    {
        SetIdleInterval(*data.mrpRetryIntervalIdle);
    }

    if (data.mrpRetryIntervalActive.has_value())
    {
        SetActiveInterval(*data.mrpRetryIntervalActive);
    }
}

#if CONFIG_NETWORK_LAYER_BLE
SetUpCodePairerParameters::SetUpCodePairerParameters(BLE_CONNECTION_OBJECT connObj, bool connected)
{
    Transport::PeerAddress peerAddress = Transport::PeerAddress::BLE();
    SetPeerAddress(peerAddress);
    if (connected)
    {
        SetConnectionObject(connObj);
    }
    else
    {
        SetDiscoveredObject(connObj);
    }
}
#endif // CONFIG_NETWORK_LAYER_BLE

} // namespace Controller
} // namespace chip
