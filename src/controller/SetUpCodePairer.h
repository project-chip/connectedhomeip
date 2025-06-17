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
#include <ble/Ble.h>
#endif // CONFIG_NETWORK_BLE

#include <controller/DeviceDiscoveryDelegate.h>

#include <deque>
#include <optional>
#include <vector>

namespace chip {
namespace Controller {

class DeviceCommissioner;

/**
 * A class that represents a discovered device.  This includes both the inputs to discovery (via the
 * RendezvousParameters super-class), and the outputs from discovery (the PeerAddress in
 * RendezvousParameters but also some of our members like mHostName, mInterfaceId,
 * mLongDiscriminator).
 */
class SetUpCodePairerParameters : public RendezvousParameters
{
public:
    SetUpCodePairerParameters() = default;
    SetUpCodePairerParameters(const Dnssd::CommonResolutionData & data, std::optional<uint16_t> longDiscriminator, size_t index);
#if CONFIG_NETWORK_LAYER_BLE
    SetUpCodePairerParameters(BLE_CONNECTION_OBJECT connObj, std::optional<uint16_t> longDiscriminator, bool connected = true);
#endif // CONFIG_NETWORK_LAYER_BLE
    char mHostName[Dnssd::kHostNameMaxLength + 1] = {};
    Inet::InterfaceId mInterfaceId;

    // The long discriminator of the device that was actually discovered, if this is known.  This
    // differs from the mSetupDiscriminator member of RendezvousParameters in that the latter may be
    // a short discriminator from a numeric setup code (which may match multiple devices), while
    // this member, if set, is always a long discriminator that was actually advertised by the
    // device represented by our PeerAddress.
    std::optional<uint16_t> mLongDiscriminator = std::nullopt;
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
    SetUpCodePairer(DeviceCommissioner * commissioner) : mCommissioner(commissioner) {}
    virtual ~SetUpCodePairer() {}

    CHIP_ERROR PairDevice(chip::NodeId remoteId, const char * setUpCode,
                          SetupCodePairerBehaviour connectionType              = SetupCodePairerBehaviour::kCommission,
                          DiscoveryType discoveryType                          = DiscoveryType::kAll,
                          Optional<Dnssd::CommonResolutionData> resolutionData = NullOptional);

    // Called by the DeviceCommissioner to notify that we have discovered a new device.
    void NotifyCommissionableDeviceDiscovered(const chip::Dnssd::DiscoveredNodeData & nodeData);

    void SetSystemLayer(System::Layer * systemLayer) { mSystemLayer = systemLayer; };

#if CONFIG_NETWORK_LAYER_BLE
    void SetBleLayer(Ble::BleLayer * bleLayer) { mBleLayer = bleLayer; };
#endif // CONFIG_NETWORK_LAYER_BLE

    // Stop ongoing discovery / pairing of the specified node, or of
    // whichever node we're pairing if kUndefinedNodeId is passed.
    bool StopPairing(NodeId remoteId = kUndefinedNodeId);

private:
    // DevicePairingDelegate implementation.
    void OnStatusUpdate(DevicePairingDelegate::Status status) override;
    void OnPairingComplete(CHIP_ERROR error) override;
    void OnPairingDeleted(CHIP_ERROR error) override;
    void OnCommissioningComplete(NodeId deviceId, CHIP_ERROR error) override;

    CHIP_ERROR Connect();
    CHIP_ERROR StartDiscoveryOverBLE();
    CHIP_ERROR StopDiscoveryOverBLE();
    CHIP_ERROR StartDiscoveryOverDNSSD();
    CHIP_ERROR StopDiscoveryOverDNSSD();
    CHIP_ERROR StartDiscoveryOverWiFiPAF();
    CHIP_ERROR StopDiscoveryOverWiFiPAF();

    // Returns whether we have kicked off a new connection attempt.
    bool ConnectToDiscoveredDevice();

    // Stop attempts to discover more things to connect to, but keep trying to
    // connect to the ones we have already discovered.
    void StopAllDiscoveryAttempts();

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
        kWiFiPAFTransport,
        kTransportTypeCount,
    };

    void NotifyCommissionableDeviceDiscovered(const chip::Dnssd::CommonResolutionData & resolutionData,
                                              std::optional<uint16_t> matchedLongDiscriminator);

    static void OnDeviceDiscoveredTimeoutCallback(System::Layer * layer, void * context);

#if CONFIG_NETWORK_LAYER_BLE
    Ble::BleLayer * mBleLayer = nullptr;
    void OnDiscoveredDeviceOverBle(BLE_CONNECTION_OBJECT connObj, std::optional<uint16_t> matchedLongDiscriminator);
    void OnBLEDiscoveryError(CHIP_ERROR err);
    /////////// BLEConnectionDelegate Callbacks /////////
    static void OnDiscoveredDeviceOverBleSuccess(void * appState, BLE_CONNECTION_OBJECT connObj);
    static void OnDiscoveredDeviceWithDiscriminatorOverBleSuccess(void * appState, uint16_t matchedLongDiscriminator,
                                                                  BLE_CONNECTION_OBJECT connObj);
    static void OnDiscoveredDeviceOverBleError(void * appState, CHIP_ERROR err);
#endif // CONFIG_NETWORK_LAYER_BLE
#if CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF
    void OnDiscoveredDeviceOverWifiPAF();
    void OnWifiPAFDiscoveryError(CHIP_ERROR err);
    static void OnWiFiPAFSubscribeComplete(void * appState);
    static void OnWiFiPAFSubscribeError(void * appState, CHIP_ERROR err);
#endif

    bool NodeMatchesCurrentFilter(const Dnssd::DiscoveredNodeData & nodeData) const;
    static bool IdIsPresent(uint16_t vendorOrProductID);

    bool ShouldDiscoverUsing(RendezvousInformationFlag commissioningChannel) const;

    // kNotAvailable represents unavailable vendor/product ID values in setup payloads.
    static constexpr uint16_t kNotAvailable = 0;

    DeviceCommissioner * mCommissioner       = nullptr;
    System::Layer * mSystemLayer             = nullptr;
    chip::NodeId mRemoteId                   = kUndefinedNodeId;
    SetupCodePairerBehaviour mConnectionType = SetupCodePairerBehaviour::kCommission;
    DiscoveryType mDiscoveryType             = DiscoveryType::kAll;
    std::vector<SetupPayload> mSetupPayloads;

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
