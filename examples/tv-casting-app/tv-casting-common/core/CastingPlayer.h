/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#pragma once

#include "CommissionerDeclarationHandler.h"
#include "ConnectionCallbacks.h"
#include "Endpoint.h"
#include "IdentificationDeclarationOptions.h"
#include "Types.h"
#include "support/ChipDeviceEventHandler.h"
#include "support/EndpointListLoader.h"

#include "lib/support/logging/CHIPLogging.h"
#include <inet/IPAddress.h>
#include <inet/InetInterface.h>
#include <string.h>
#include <vector>

namespace matter {
namespace casting {
namespace core {

const int kPortMaxLength = 5; // port is uint16_t
// +1 for the : between the hostname and the port.
const int kIdMaxLength                        = chip::Dnssd::kHostNameMaxLength + kPortMaxLength + 1;
const uint64_t kCommissioningWindowTimeoutSec = 3 * 60; // 3 minutes

/**
 * @brief Describes an Endpoint that the client wants to connect to
 */
struct EndpointFilter
{
    // value of 0 means unspecified
    uint16_t vendorId  = 0;
    uint16_t productId = 0;
    std::vector<chip::app::Clusters::Descriptor::Structs::DeviceTypeStruct::DecodableType> requiredDeviceTypes;
};

class CastingPlayerAttributes
{
public:
    char id[kIdMaxLength + 1]                                              = {};
    char deviceName[chip::Dnssd::kMaxDeviceNameLen + 1]                    = {};
    char hostName[chip::Dnssd::kHostNameMaxLength + 1]                     = {};
    char instanceName[chip::Dnssd::Commission::kInstanceNameMaxLength + 1] = {};
    unsigned int numIPs; // Number of valid IP addresses
    chip::Inet::IPAddress ipAddresses[chip::Dnssd::CommonResolutionData::kMaxIPAddresses];
    chip::Inet::InterfaceId interfaceId;
    uint16_t port;
    uint16_t productId;
    uint16_t vendorId;
    uint32_t deviceType;
    bool supportsCommissionerGeneratedPasscode;

    chip::NodeId nodeId           = 0;
    chip::FabricIndex fabricIndex = 0;
};

class Endpoint;

/**
 * @brief Represents CastingPlayer ConnectionState.
 *
 */
enum ConnectionState
{
    CASTING_PLAYER_NOT_CONNECTED,
    CASTING_PLAYER_CONNECTING,
    CASTING_PLAYER_CONNECTED,
};

class ConnectionContext;
class CastingPlayer;

/**
 * @brief CastingPlayer represents a Matter commissioner that is able to play media to a physical
 * output or to a display screen which is part of the device.
 */
class CastingPlayer : public std::enable_shared_from_this<CastingPlayer>
{
public:
    CastingPlayer(CastingPlayerAttributes playerAttributes) { mAttributes = playerAttributes; }

    /**
     * @brief Get the CastingPlayer object targeted currently (may not be connected)
     */
    static CastingPlayer * GetTargetCastingPlayer() { return mTargetCastingPlayer; }

    /**
     * @brief Compares based on the Id
     */
    bool operator==(const CastingPlayer & other) const
    {
        int compareResult = strcmp(this->mAttributes.id, other.mAttributes.id);
        return (compareResult == 0) ? 1 : 0;
    }

    /**
     * @return true if this CastingPlayer is connected to the CastingApp
     */
    bool IsConnected() const { return mConnectionState == CASTING_PLAYER_CONNECTED; }

    /**
     * @brief Verifies that a connection exists with this CastingPlayer, or triggers a new session
     * request. If the CastingApp does not have the nodeId and fabricIndex of this CastingPlayer cached on disk,
     * this will execute the User Directed Commissioning (UDC) process.
     *
     * @param connectionCallbacks contains the ConnectCallback and CommissionerDeclarationCallback (Optional).
     * @param commissioningWindowTimeoutSec (Optional) time (in sec) to keep the commissioning window open, if commissioning is
     * required. Needs to be >= kCommissioningWindowTimeoutSec.
     * @param idOptions (Optional) Parameters in the IdentificationDeclaration message sent by the Commissionee to the Commissioner.
     * These parameters specify the information relating to the requested commissioning session.
     * Furthermore, attributes (such as VendorId) describe the TargetApp that the client wants to interact with after commissioning.
     * If this value is passed in, VerifyOrEstablishConnection() will force UDC, in case the desired
     * TargetApp is not found in the on-device CastingStore.
     */
    void VerifyOrEstablishConnection(ConnectionCallbacks connectionCallbacks,
                                     uint64_t commissioningWindowTimeoutSec     = kCommissioningWindowTimeoutSec,
                                     IdentificationDeclarationOptions idOptions = IdentificationDeclarationOptions());

    /**
     * @brief Continues the UDC process during the Commissioner-Generated passcode commissioning flow by sending a second
     * IdentificationDeclaration to Commissioner containing CommissionerPasscode and CommissionerPasscodeReady set to true. At this
     * point it is assumed that the following have occurred:
     * 1. Client has handled the Commissioner's CommissionerDecelration message with PasscodeDialogDisplayed and
     * CommissionerPasscode set to true.
     * 2. Client prompted user to input Passcode from Commissioner.
     * 3. Client has updated the commissioning session's PAKE verifier using the user input Passcode by updating the CastingApps
     * CommissionableDataProvider
     * (matter::casting::core::CastingApp::GetInstance()->UpdateCommissionableDataProvider(CommissionableDataProvider)).
     *
     * The same connectionCallbacks and commissioningWindowTimeoutSec parameters passed in to VerifyOrEstablishConnection() will be
     * used.
     */
    void ContinueConnecting();

    /**
     * @brief Sets the internal connection state of this CastingPlayer to "disconnected"
     */
    void Disconnect();

    /**
     * @brief Find an existing session for this CastingPlayer, or trigger a new session
     * request.
     *
     * The caller can optionally provide `onDeviceConnected` and `onDeviceConnectionFailure` callback
     * objects. If provided, these will be used to inform the caller about
     * successful or failed connection establishment.
     *
     * If the connection is already established, the `onDeviceConnected` callback
     * will be immediately called, before FindOrEstablishSession returns.
     *
     * The `onDeviceConnectionFailure` callback may be called before the FindOrEstablishSession
     * call returns, for error cases that are detected synchronously.
     */
    void FindOrEstablishSession(void * clientContext, chip::OnDeviceConnected onDeviceConnected,
                                chip::OnDeviceConnectionFailure onDeviceConnectionFailure);

    /**
     * @brief Register an endpoint on this CastingPlayer. If the provided endpoint was already registered, its information will be
     * updated in the registry.
     */
    void RegisterEndpoint(const memory::Strong<Endpoint> endpoint);

    const std::vector<memory::Strong<Endpoint>> GetEndpoints() const { return mEndpoints; }

    void LogDetail() const;

    const char * GetId() const { return mAttributes.id; }

    const char * GetDeviceName() const { return mAttributes.deviceName; }

    const char * GetHostName() const { return mAttributes.hostName; }

    const char * GetInstanceName() const { return mAttributes.instanceName; }

    uint GetNumIPs() const { return mAttributes.numIPs; }

    chip::Inet::IPAddress * GetIPAddresses() { return mAttributes.ipAddresses; }

    uint16_t GetPort() { return mAttributes.port; }

    uint16_t GetProductId() const { return mAttributes.productId; }

    uint16_t GetVendorId() const { return mAttributes.vendorId; }

    uint32_t GetDeviceType() const { return mAttributes.deviceType; }

    bool GetSupportsCommissionerGeneratedPasscode() const { return mAttributes.supportsCommissionerGeneratedPasscode; }

    chip::NodeId GetNodeId() const { return mAttributes.nodeId; }

    chip::FabricIndex GetFabricIndex() const { return mAttributes.fabricIndex; }

    void SetNodeId(chip::NodeId nodeId) { mAttributes.nodeId = nodeId; }

    void SetFabricIndex(chip::FabricIndex fabricIndex) { mAttributes.fabricIndex = fabricIndex; }

private:
    std::vector<memory::Strong<Endpoint>> mEndpoints;
    ConnectionState mConnectionState = CASTING_PLAYER_NOT_CONNECTED;
    CastingPlayerAttributes mAttributes;
    IdentificationDeclarationOptions mIdOptions;
    static CastingPlayer * mTargetCastingPlayer;
    uint64_t mCommissioningWindowTimeoutSec = kCommissioningWindowTimeoutSec;
    ConnectCallback mOnCompleted            = {};

    /**
     * @brief resets this CastingPlayer's state and calls mOnCompleted with the CHIP_ERROR. Also, after calling mOnCompleted, it
     * clears mOnCompleted by setting it to a nullptr.
     */
    void resetState(CHIP_ERROR err);

#if CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY_CLIENT
    /**
     * @brief Sends the user directed commissioning request to this CastingPlayer
     */
    CHIP_ERROR SendUserDirectedCommissioningRequest();

    /**
     * @brief Selects an IP Address to send the UDC request to.
     * Prioritizes IPV4 addresses over IPV6.
     *
     * @return chip::Inet::IPAddress*
     */
    chip::Inet::IPAddress * GetIpAddressForUDCRequest();
#endif // CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY_CLIENT

    /**
     * @brief Checks if the cachedCastingPlayer contains at least one Endpoint/TargetApp described in the desiredTargetApps list.
     * @return true - cachedCastingPlayer contains at least one endpoints with matching (non-default) values for vendorID and
     * productID as described in the desiredTargetApps list, false otherwise.
     */
    bool ContainsDesiredTargetApp(core::CastingPlayer * cachedCastingPlayer,
                                  std::vector<chip::Protocols::UserDirectedCommissioning::TargetAppInfo> desiredTargetApps);

    // ChipDeviceEventHandler handles chip::DeviceLayer::ChipDeviceEvent events and helps the CastingPlayer class commission with
    // and connect to a CastingPlayer
    friend class support::ChipDeviceEventHandler;

    friend class ConnectionContext;
    friend class support::EndpointListLoader;
};

class ConnectionContext
{
public:
    ConnectionContext(void * clientContext, core::CastingPlayer * targetCastingPlayer, chip::OnDeviceConnected onDeviceConnected,
                      chip::OnDeviceConnectionFailure onDeviceConnectionFailure);
    ~ConnectionContext();

    void * mClientContext                                                                    = nullptr;
    core::CastingPlayer * mTargetCastingPlayer                                               = nullptr;
    chip::OnDeviceConnected mOnDeviceConnectedFn                                             = nullptr;
    chip::OnDeviceConnectionFailure mOnDeviceConnectionFailureFn                             = nullptr;
    chip::Callback::Callback<chip::OnDeviceConnected> * mOnConnectedCallback                 = nullptr;
    chip::Callback::Callback<chip::OnDeviceConnectionFailure> * mOnConnectionFailureCallback = nullptr;
};

}; // namespace core
}; // namespace casting
}; // namespace matter
