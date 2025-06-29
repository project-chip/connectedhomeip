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
#include <vector>

constexpr uint32_t kDeviceDiscoveredTimeout = CHIP_CONFIG_SETUP_CODE_PAIRER_DISCOVERY_TIMEOUT_SECS * chip::kMillisecondsPerSecond;

using namespace chip::Tracing;

namespace chip {
namespace Controller {

CHIP_ERROR SetUpCodePairer::PairDevice(NodeId remoteId, const char * setUpCode, SetupCodePairerBehaviour commission,
                                       DiscoveryType discoveryType, Optional<Dnssd::CommonResolutionData> resolutionData)
{
    VerifyOrReturnErrorWithMetric(kMetricSetupCodePairerPairDevice, mSystemLayer != nullptr, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnErrorWithMetric(kMetricSetupCodePairerPairDevice, remoteId != kUndefinedNodeId, CHIP_ERROR_INVALID_ARGUMENT);

    std::vector<SetupPayload> payloads;
    ReturnErrorOnFailure(SetupPayload::FromStringRepresentation(setUpCode, payloads));

    // If the caller has provided a specific single resolution data, and we were
    // only looking for one commissionee, and the caller says that the provided
    // data matches that one commissionee, just go ahead and use the provided data.
    //
    // If we were looking for more than one device (i.e. if either of the
    // payload arrays involved does not have length 1), we can't make use of the
    // incoming resolution data, since it does not contain the long
    // discriminator of the thing that was discovered, and therefore we can't
    // tell which setup passcode to use for it.
    if (resolutionData.HasValue() && payloads.size() == 1 && mSetupPayloads.size() == 1)
    {
        VerifyOrReturnErrorWithMetric(kMetricSetupCodePairerPairDevice, discoveryType != DiscoveryType::kAll,
                                      CHIP_ERROR_INVALID_ARGUMENT);
        if (mRemoteId == remoteId && mSetupPayloads[0].setUpPINCode == payloads[0].setUpPINCode && mConnectionType == commission &&
            mDiscoveryType == discoveryType)
        {
            // Not passing a discriminator is ok, since we have only one payload.
            NotifyCommissionableDeviceDiscovered(resolutionData.Value(), /* matchedLongDiscriminator = */ std::nullopt);
            return CHIP_NO_ERROR;
        }
    }

    ResetDiscoveryState();

    mConnectionType = commission;
    mDiscoveryType  = discoveryType;
    mRemoteId       = remoteId;
    mSetupPayloads  = std::move(payloads);

    if (resolutionData.HasValue() && mSetupPayloads.size() == 1)
    {
        // No need to pass in a discriminator if we have only one payload, which
        // is good because we don't have a full discriminator here anyway.
        NotifyCommissionableDeviceDiscovered(resolutionData.Value(), /* matchedLongDiscriminator = */ std::nullopt);
        return CHIP_NO_ERROR;
    }

    ReturnErrorOnFailureWithMetric(kMetricSetupCodePairerPairDevice, Connect());
    auto errorCode =
        mSystemLayer->StartTimer(System::Clock::Milliseconds32(kDeviceDiscoveredTimeout), OnDeviceDiscoveredTimeoutCallback, this);
    if (CHIP_NO_ERROR == errorCode)
    {
        MATTER_LOG_METRIC_BEGIN(kMetricSetupCodePairerPairDevice);
    }
    return errorCode;
}

CHIP_ERROR SetUpCodePairer::Connect()
{
    if (mDiscoveryType == DiscoveryType::kAll)
    {
        if (ShouldDiscoverUsing(RendezvousInformationFlag::kBLE))
        {
            CHIP_ERROR err = StartDiscoveryOverBLE();
            if ((CHIP_ERROR_NOT_IMPLEMENTED == err) || (CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE == err))
            {
                ChipLogProgress(Controller,
                                "Skipping commissionable node discovery over BLE since not supported by the controller!");
            }
            else if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Controller, "Failed to start commissionable node discovery over BLE: %" CHIP_ERROR_FORMAT,
                             err.Format());
            }
        }
        if (ShouldDiscoverUsing(RendezvousInformationFlag::kWiFiPAF))
        {
            CHIP_ERROR err = StartDiscoveryOverWiFiPAF();
            if ((CHIP_ERROR_NOT_IMPLEMENTED == err) || (CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE == err))
            {
                ChipLogProgress(Controller,
                                "Skipping commissionable node discovery over Wi-Fi PAF since not supported by the controller!");
            }
            else if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Controller, "Failed to start commissionable node discovery over Wi-Fi PAF: %" CHIP_ERROR_FORMAT,
                             err.Format());
            }
        }
    }

    // We always want to search on network because any node that has already been commissioned will use on-network regardless of the
    // QR code flag.
    CHIP_ERROR err = StartDiscoveryOverDNSSD();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "Failed to start commissionable node discovery over DNS-SD: %" CHIP_ERROR_FORMAT, err.Format());
    }
    return err;
}

CHIP_ERROR SetUpCodePairer::StartDiscoveryOverBLE()
{
#if CONFIG_NETWORK_LAYER_BLE
#if CHIP_DEVICE_CONFIG_ENABLE_BOTH_COMMISSIONER_AND_COMMISSIONEE
    VerifyOrReturnError(mCommissioner != nullptr, CHIP_ERROR_INCORRECT_STATE);
    mCommissioner->ConnectBleTransportToSelf();
#endif // CHIP_DEVICE_CONFIG_ENABLE_BOTH_COMMISSIONER_AND_COMMISSIONEE
    VerifyOrReturnError(mBleLayer != nullptr, CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

    ChipLogProgress(Controller, "Starting commissionable node discovery over BLE");

    // Handle possibly-sync callbacks.
    mWaitingForDiscovery[kBLETransport] = true;
    CHIP_ERROR err;
    // Not all BLE backends support the new NewBleConnectionByDiscriminators
    // API, so use the old one when we can (i.e. when we only have one setup
    // payload), to avoid breaking existing API consumers.
    if (mSetupPayloads.size() == 1)
    {
        err = mBleLayer->NewBleConnectionByDiscriminator(mSetupPayloads[0].discriminator, this, OnDiscoveredDeviceOverBleSuccess,
                                                         OnDiscoveredDeviceOverBleError);
    }
    else
    {
        std::vector<SetupDiscriminator> discriminators;
        discriminators.reserve(mSetupPayloads.size());
        for (auto & payload : mSetupPayloads)
        {
            discriminators.emplace_back(payload.discriminator);
        }
        err = mBleLayer->NewBleConnectionByDiscriminators(Span(discriminators.data(), discriminators.size()), this,
                                                          OnDiscoveredDeviceWithDiscriminatorOverBleSuccess,
                                                          OnDiscoveredDeviceOverBleError);
    }
    if (err != CHIP_NO_ERROR)
    {
        mWaitingForDiscovery[kBLETransport] = false;
    }
    return err;
#else
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
#endif // CONFIG_NETWORK_LAYER_BLE
}

CHIP_ERROR SetUpCodePairer::StopDiscoveryOverBLE()
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
    ChipLogProgress(Controller, "Stopping commissionable node discovery over BLE");
    return mBleLayer->CancelBleIncompleteConnection();
#else
    return CHIP_NO_ERROR;
#endif // CONFIG_NETWORK_LAYER_BLE
}

CHIP_ERROR SetUpCodePairer::StartDiscoveryOverDNSSD()
{
    ChipLogProgress(Controller, "Starting commissionable node discovery over DNS-SD");

    Dnssd::DiscoveryFilter filter(Dnssd::DiscoveryFilterType::kNone);
    if (mSetupPayloads.size() == 1)
    {
        auto & discriminator = mSetupPayloads[0].discriminator;
        if (discriminator.IsShortDiscriminator())
        {
            filter.type = Dnssd::DiscoveryFilterType::kShortDiscriminator;
            filter.code = discriminator.GetShortValue();
        }
        else
        {
            filter.type = Dnssd::DiscoveryFilterType::kLongDiscriminator;
            filter.code = discriminator.GetLongValue();
        }
    }

    // In theory we could try to filter on the vendor ID if it's the same across all the setup
    // payloads, but DNS-SD advertisements are not required to include the Vendor ID subtype, so in
    // practice that's not doable.

    // Handle possibly-sync callbacks.
    mWaitingForDiscovery[kIPTransport] = true;
    CHIP_ERROR err                     = mCommissioner->DiscoverCommissionableNodes(filter);
    if (err != CHIP_NO_ERROR)
    {
        mWaitingForDiscovery[kIPTransport] = false;
    }
    return err;
}

CHIP_ERROR SetUpCodePairer::StopDiscoveryOverDNSSD()
{
    ChipLogProgress(Controller, "Stopping commissionable node discovery over DNS-SD");

    mWaitingForDiscovery[kIPTransport] = false;

    mCommissioner->StopCommissionableDiscovery();
    return CHIP_NO_ERROR;
}

CHIP_ERROR SetUpCodePairer::StartDiscoveryOverWiFiPAF()
{
#if CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF
    if (mSetupPayloads.size() != 1)
    {
        ChipLogError(Controller, "Wi-Fi PAF commissioning does not support concatenated QR codes yet.");
        return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
    }

    auto & payload = mSetupPayloads[0];

    ChipLogProgress(Controller, "Starting commissionable node discovery over Wi-Fi PAF");
    VerifyOrReturnError(mCommissioner != nullptr, CHIP_ERROR_INCORRECT_STATE);

    const SetupDiscriminator connDiscriminator(payload.discriminator);
    VerifyOrReturnValue(!connDiscriminator.IsShortDiscriminator(), CHIP_ERROR_INVALID_ARGUMENT,
                        ChipLogError(Controller, "Error, Long discriminator is required"));
    uint16_t discriminator              = connDiscriminator.GetLongValue();
    WiFiPAF::WiFiPAFSession sessionInfo = { .role          = WiFiPAF::WiFiPafRole::kWiFiPafRole_Subscriber,
                                            .nodeId        = mRemoteId,
                                            .discriminator = discriminator };
    ReturnErrorOnFailure(
        DeviceLayer::ConnectivityMgr().GetWiFiPAF()->AddPafSession(WiFiPAF::PafInfoAccess::kAccNodeInfo, sessionInfo));

    mWaitingForDiscovery[kWiFiPAFTransport] = true;
    CHIP_ERROR err = DeviceLayer::ConnectivityMgr().WiFiPAFSubscribe(discriminator, (void *) this, OnWiFiPAFSubscribeComplete,
                                                                     OnWiFiPAFSubscribeError);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "Commissionable node discovery over Wi-Fi PAF failed, err = %" CHIP_ERROR_FORMAT, err.Format());
        mWaitingForDiscovery[kWiFiPAFTransport] = false;
    }
    return err;
#else
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF
}

CHIP_ERROR SetUpCodePairer::StopDiscoveryOverWiFiPAF()
{
    mWaitingForDiscovery[kWiFiPAFTransport] = false;
#if CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF
    DeviceLayer::ConnectivityMgr().WiFiPAFCancelIncompleteSubscribe();
#endif
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

        if (params.mLongDiscriminator)
        {
            auto longDiscriminator = *params.mLongDiscriminator;
            // Look for a matching setup passcode.
            bool found = false;
            for (auto & payload : mSetupPayloads)
            {
                if (payload.discriminator.MatchesLongDiscriminator(longDiscriminator))
                {
                    params.SetSetupPINCode(payload.setUpPINCode);
                    found = true;
                    break;
                }
            }
            if (!found)
            {
                ChipLogError(Controller, "SetUpCodePairer: Discovered discriminator %u does not match any of our setup payloads",
                             longDiscriminator);
                // Move on to the the next discovered params; nothing we can do here.
                continue;
            }
        }
        else
        {
            // No discriminator known for this discovered device.  This can work if we have only one
            // setup payload, but otherwise we have no idea what setup passcode to use for it.
            if (mSetupPayloads.size() == 1)
            {
                params.SetSetupPINCode(mSetupPayloads[0].setUpPINCode);
            }
            else
            {
                ChipLogError(Controller,
                             "SetUpCodePairer: Unable to handle discovered parameters with no discriminator, because it has %u "
                             "possible payloads",
                             static_cast<unsigned>(mSetupPayloads.size()));
                continue;
            }
        }

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
void SetUpCodePairer::OnDiscoveredDeviceOverBle(BLE_CONNECTION_OBJECT connObj, std::optional<uint16_t> matchedLongDiscriminator)
{
    ChipLogProgress(Controller, "Discovered device to be commissioned over BLE");

    mWaitingForDiscovery[kBLETransport] = false;

    // In order to not wait for all the possible addresses discovered over mdns to
    // be tried before trying to connect over BLE, the discovered connection object is
    // inserted at the beginning of the list.
    //
    // It makes it the 'next' thing to try to connect to if there are already some
    // discovered parameters in the list.
    //
    // TODO: Consider implementing the SHOULD the spec has about commissioning things
    // in QR code order by waiting for a second or something before actually starting
    // the first PASE session when we have multiple setup payloads, and sorting the
    // results in setup payload order.  If we do this, we might want to restrict it to
    // cases when the different payloads have different vendor/product IDs, since if
    // they are all the same product presumably ordering really does not matter.
    mDiscoveredParameters.emplace_front(connObj, matchedLongDiscriminator);
    ConnectToDiscoveredDevice();
}

void SetUpCodePairer::OnDiscoveredDeviceOverBleSuccess(void * appState, BLE_CONNECTION_OBJECT connObj)
{
    (static_cast<SetUpCodePairer *>(appState))->OnDiscoveredDeviceOverBle(connObj, std::nullopt);
}

void SetUpCodePairer::OnDiscoveredDeviceWithDiscriminatorOverBleSuccess(void * appState, uint16_t matchedLongDiscriminator,
                                                                        BLE_CONNECTION_OBJECT connObj)
{
    (static_cast<SetUpCodePairer *>(appState))->OnDiscoveredDeviceOverBle(connObj, std::make_optional(matchedLongDiscriminator));
}

void SetUpCodePairer::OnDiscoveredDeviceOverBleError(void * appState, CHIP_ERROR err)
{
    static_cast<SetUpCodePairer *>(appState)->OnBLEDiscoveryError(err);
}

void SetUpCodePairer::OnBLEDiscoveryError(CHIP_ERROR err)
{
    ChipLogError(Controller, "Commissionable node discovery over BLE failed: %" CHIP_ERROR_FORMAT, err.Format());
    mWaitingForDiscovery[kBLETransport] = false;
    LogErrorOnFailure(err);
}
#endif // CONFIG_NETWORK_LAYER_BLE

#if CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF
void SetUpCodePairer::OnDiscoveredDeviceOverWifiPAF()
{
    ChipLogProgress(Controller, "Discovered device to be commissioned over Wi-Fi PAF, RemoteId: %lu", mRemoteId);

    mWaitingForDiscovery[kWiFiPAFTransport] = false;
    auto param                              = SetUpCodePairerParameters();
    param.SetPeerAddress(Transport::PeerAddress(Transport::Type::kWiFiPAF, mRemoteId));
    // TODO: This needs to support concatenated QR codes and set the relevant
    // long discriminator on param.
    //
    // See https://github.com/project-chip/connectedhomeip/issues/39134
    mDiscoveredParameters.emplace_back(param);
    ConnectToDiscoveredDevice();
}

void SetUpCodePairer::OnWifiPAFDiscoveryError(CHIP_ERROR err)
{
    ChipLogError(Controller, "Commissionable node discovery over Wi-Fi PAF failed: %" CHIP_ERROR_FORMAT, err.Format());
    mWaitingForDiscovery[kWiFiPAFTransport] = false;
}

void SetUpCodePairer::OnWiFiPAFSubscribeComplete(void * appState)
{
    auto self = reinterpret_cast<SetUpCodePairer *>(appState);
    self->OnDiscoveredDeviceOverWifiPAF();
}

void SetUpCodePairer::OnWiFiPAFSubscribeError(void * appState, CHIP_ERROR err)
{
    auto self = reinterpret_cast<SetUpCodePairer *>(appState);
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

    // Check whether this matches one of our setup payloads.
    for (auto & payload : mSetupPayloads)
    {
        // The advertisement may not include a vendor id, and the payload may not have one either.
        if (IdIsPresent(payload.vendorID) && IdIsPresent(nodeData.vendorId) && payload.vendorID != nodeData.vendorId)
        {
            ChipLogProgress(Controller, "Discovered device vendor ID (%u) does not match our vendor ID (%u).", nodeData.vendorId,
                            payload.vendorID);
            continue;
        }

        // The advertisement may not include a product id, and the payload may not have one either.
        if (IdIsPresent(payload.productID) && IdIsPresent(nodeData.productId) && payload.productID != nodeData.productId)
        {
            ChipLogProgress(Controller, "Discovered device product ID (%u) does not match our product ID (%u).", nodeData.productId,
                            payload.productID);
            continue;
        }

        if (!payload.discriminator.MatchesLongDiscriminator(nodeData.longDiscriminator))
        {
            ChipLogProgress(Controller, "Discovered device discriminator (%u) does not match our discriminator.",
                            nodeData.longDiscriminator);
            continue;
        }

        ChipLogProgress(Controller, "Discovered device with discriminator %u matches one of our setup payloads",
                        nodeData.longDiscriminator);
        return true;
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

    auto & commissionableNodeData = nodeData.Get<Dnssd::CommissionNodeData>();

    NotifyCommissionableDeviceDiscovered(commissionableNodeData, std::make_optional(commissionableNodeData.longDiscriminator));
}

void SetUpCodePairer::NotifyCommissionableDeviceDiscovered(const Dnssd::CommonResolutionData & resolutionData,
                                                           std::optional<uint16_t> matchedLongDiscriminator)
{
    if (mDiscoveryType == DiscoveryType::kDiscoveryNetworkOnlyWithoutPASEAutoRetry)
    {
        // If the discovery type does not want the PASE auto retry mechanism, we will just store
        // a single IP. So the discovery process is stopped as it won't be of any help anymore.
        StopDiscoveryOverDNSSD();
        mDiscoveredParameters.emplace_back(resolutionData, matchedLongDiscriminator, 0);
    }
    else
    {
        for (size_t i = 0; i < resolutionData.numIPs; i++)
        {
            mDiscoveredParameters.emplace_back(resolutionData, matchedLongDiscriminator, i);
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

void SetUpCodePairer::StopAllDiscoveryAttempts()
{
    LogErrorOnFailure(StopDiscoveryOverBLE());
    LogErrorOnFailure(StopDiscoveryOverDNSSD());
    LogErrorOnFailure(StopDiscoveryOverWiFiPAF());

    // Just in case any of those failed to reset the waiting state properly.
    for (auto & waiting : mWaitingForDiscovery)
    {
        waiting = false;
    }
}

void SetUpCodePairer::ResetDiscoveryState()
{
    StopAllDiscoveryAttempts();

    mDiscoveredParameters.clear();
    mCurrentPASEParameters.ClearValue();
    mLastPASEError = CHIP_NO_ERROR;

    mSetupPayloads.clear();

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
    pairer->StopAllDiscoveryAttempts();
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

bool SetUpCodePairer::ShouldDiscoverUsing(RendezvousInformationFlag commissioningChannel) const
{
    for (auto & payload : mSetupPayloads)
    {
        auto & rendezvousInformation = payload.rendezvousInformation;
        if (!rendezvousInformation.HasValue())
        {
            // No idea which commissioning channels this device supports, so we
            // should be trying using all of them.
            return true;
        }

        if (rendezvousInformation.Value().Has(commissioningChannel))
        {
            return true;
        }
    }

    // None of the payloads claimed support for this commissioning channel.
    return false;
}

SetUpCodePairerParameters::SetUpCodePairerParameters(const Dnssd::CommonResolutionData & data,
                                                     std::optional<uint16_t> longDiscriminator, size_t index) :
    mLongDiscriminator(longDiscriminator)
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
SetUpCodePairerParameters::SetUpCodePairerParameters(BLE_CONNECTION_OBJECT connObj, std::optional<uint16_t> longDiscriminator,
                                                     bool connected) :
    mLongDiscriminator(longDiscriminator)
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
