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
 *      Declaration of SetUp Code Pairer, a class that parses a given
 *      setup code and uses the extracted informations to discover and
 *      filter commissionables nodes, before initiating the pairing process.
 *
 */

#pragma once

#include <controller/DevicePairingDelegate.h>
#include <lib/core/CHIPError.h>
#include <lib/core/NodeId.h>
#include <lib/support/DLLUtil.h>
#include <platform/CHIPDeviceConfig.h>
#include <protocols/secure_channel/RendezvousParameters.h>
#include <setup_payload/ManualSetupPayloadParser.h>
#include <setup_payload/QRCodeSetupPayloadParser.h>

#if CONFIG_NETWORK_LAYER_BLE
#include <ble/BleLayer.h>
#endif // CONFIG_NETWORK_BLE

#include <controller/DeviceDiscoveryDelegate.h>

#include <deque>

namespace chip {
namespace Controller {

class DeviceCommissioner;

class SetUpCodePairerParameters : public RendezvousParameters
{
public:
    SetUpCodePairerParameters(const Dnssd::CommonResolutionData & data, size_t index);
#if CONFIG_NETWORK_LAYER_BLE
    SetUpCodePairerParameters(BLE_CONNECTION_OBJECT connObj, bool connected = true);
#endif // CONFIG_NETWORK_LAYER_BLE
    char mHostName[Dnssd::kHostNameMaxLength + 1] = {};
    Inet::InterfaceId mInterfaceId;
};

enum class SetupCodePairerBehaviour : uint8_t
{
    kCommission,
    kPaseOnly,
};

enum class DiscoveryType : uint8_t
{
    kDiscoveryNetworkOnly,
    kDiscoveryNetworkOnlyWithoutPASEAutoRetry,
    kAll,
};

class DLL_EXPORT SetUpCodePairer : public DevicePairingDelegate
{
public:
    SetUpCodePairer(DeviceCommissioner * commissioner) : mCommissioner(commissioner) { ResetDiscoveryState(); }
    virtual ~SetUpCodePairer() {}

    CHIP_ERROR PairDevice(chip::NodeId remoteId, const char * setUpCode,
                          SetupCodePairerBehaviour connectionType = SetupCodePairerBehaviour::kCommission,
                          DiscoveryType discoveryType             = DiscoveryType::kAll);

    // Called by the DeviceCommissioner to notify that we have discovered a new device.
    void NotifyCommissionableDeviceDiscovered(const chip::Dnssd::DiscoveredNodeData & nodeData);

    void SetSystemLayer(System::Layer * systemLayer) { mSystemLayer = systemLayer; };

#if CONFIG_NETWORK_LAYER_BLE
    void SetBleLayer(Ble::BleLayer * bleLayer) { mBleLayer = bleLayer; };
#endif // CONFIG_NETWORK_LAYER_BLE

    // Called to notify us that the DeviceCommissioner is shutting down and we
    // should not try to do any more new work.
    void CommissionerShuttingDown();

private:
    // DevicePairingDelegate implementation.
    void OnStatusUpdate(DevicePairingDelegate::Status status) override;
    void OnPairingComplete(CHIP_ERROR error) override;
    void OnPairingDeleted(CHIP_ERROR error) override;
    void OnCommissioningComplete(NodeId deviceId, CHIP_ERROR error) override;

    CHIP_ERROR Connect(SetupPayload & paload);
    CHIP_ERROR StartDiscoverOverBle(SetupPayload & payload);
    CHIP_ERROR StopConnectOverBle();
    CHIP_ERROR StartDiscoverOverIP(SetupPayload & payload);
    CHIP_ERROR StopConnectOverIP();
    CHIP_ERROR StartDiscoverOverSoftAP(SetupPayload & payload);
    CHIP_ERROR StopConnectOverSoftAP();

    // Returns whether we have kicked off a new connection attempt.
    bool ConnectToDiscoveredDevice();

    // Reset our mWaitingForDiscovery/mDiscoveredParameters state to indicate no
    // pending work.
    void ResetDiscoveryState();

    // Get ready to start PASE establishment via mCommissioner.  Sets up
    // whatever state is needed for that.
    void ExpectPASEEstablishment();

    // PASE establishment by mCommissioner has completed: we either have a PASE
    // session now or we failed to set one up, but we are done waiting on
    // mCommissioner.
    void PASEEstablishmentComplete();

    // Called when PASE establishment fails.
    //
    // May start a new PASE establishment.
    //
    // Will return whether we might in fact have more rendezvous parameters to
    // try (e.g. because we started a new PASE establishment or are waiting on
    // more device discovery).
    //
    // The commissioner can use the return value to decide whether pairing has
    // actually failed or not.
    bool TryNextRendezvousParameters();

    // True if we are still waiting on discovery to possibly produce new
    // RendezvousParameters in the future.
    bool DiscoveryInProgress() const;

    // Not an enum class because we use this for indexing into arrays.
    enum TransportTypes
    {
        kBLETransport = 0,
        kIPTransport,
        kSoftAPTransport,
        kTransportTypeCount,
    };

    static void OnDeviceDiscoveredTimeoutCallback(System::Layer * layer, void * context);

#if CONFIG_NETWORK_LAYER_BLE
    Ble::BleLayer * mBleLayer = nullptr;
    void OnDiscoveredDeviceOverBle(BLE_CONNECTION_OBJECT connObj);
    void OnBLEDiscoveryError(CHIP_ERROR err);
    /////////// BLEConnectionDelegate Callbacks /////////
    static void OnDiscoveredDeviceOverBleSuccess(void * appState, BLE_CONNECTION_OBJECT connObj);
    static void OnDiscoveredDeviceOverBleError(void * appState, CHIP_ERROR err);
#endif // CONFIG_NETWORK_LAYER_BLE

    bool NodeMatchesCurrentFilter(const Dnssd::DiscoveredNodeData & nodeData) const;
    static bool IdIsPresent(uint16_t vendorOrProductID);

    Dnssd::DiscoveryFilter mCurrentFilter;
    // The vendor id and product id from the SetupPayload.  They may be 0, which
    // indicates "not available" (e.g. because the SetupPayload came from a
    // short manual code).  In that case we should not filter on those values.
    static constexpr uint16_t kNotAvailable = 0;
    uint16_t mPayloadVendorID               = kNotAvailable;
    uint16_t mPayloadProductID              = kNotAvailable;

    DeviceCommissioner * mCommissioner = nullptr;
    System::Layer * mSystemLayer       = nullptr;
    chip::NodeId mRemoteId;
    uint32_t mSetUpPINCode                   = 0;
    SetupCodePairerBehaviour mConnectionType = SetupCodePairerBehaviour::kCommission;
    DiscoveryType mDiscoveryType             = DiscoveryType::kAll;

    // While we are trying to pair, we intercept the DevicePairingDelegate
    // notifications from mCommissioner.  We want to make sure we send them on
    // to the original pairing delegate, if any.
    DevicePairingDelegate * mPairingDelegate = nullptr;

    // Boolean will be set to true if we currently have an async discovery
    // process happening via the relevant transport.
    bool mWaitingForDiscovery[kTransportTypeCount] = { false };

    // Double ended-queue of things we have discovered but not tried connecting to yet.  The
    // general discovery/pairing process will terminate once this queue is empty
    // and all the booleans in mWaitingForDiscovery are false.
    std::deque<SetUpCodePairerParameters> mDiscoveredParameters;

    // Current thing we are trying to connect to over UDP. If a PASE connection fails with
    // a CHIP_ERROR_TIMEOUT, the discovered parameters will be used to ask the
    // mdns daemon to invalidate the
    Optional<SetUpCodePairerParameters> mCurrentPASEParameters;

    // mWaitingForPASE is true if we have called either
    // EstablishPASEConnection or PairDevice on mCommissioner and are now just
    // waiting to see whether that works.
    bool mWaitingForPASE = false;

    // mLastPASEError is the error from the last OnPairingComplete call we got.
    CHIP_ERROR mLastPASEError = CHIP_NO_ERROR;
};

} // namespace Controller
} // namespace chip
