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
const uint16_t kCommissioningWindowTimeoutSec = 3 * 60; // 3 minutes

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
 * @brief CastingPlayer represents a Matter Commissioner that is able to play media to a physical
 * output or to a display screen which is part of the device.
 */
class CastingPlayer : public std::enable_shared_from_this<CastingPlayer>
{
public:
    CastingPlayer(CastingPlayerAttributes playerAttributes) { mAttributes = playerAttributes; }

    /**
     * @brief Get the CastingPlayer object targeted currently (may not be connected)
     */
    static CastingPlayer * GetTargetCastingPlayer()
    {
        std::shared_ptr<CastingPlayer> sharedPtr = mTargetCastingPlayer.lock();
        CastingPlayer * rawPtr                   = nullptr;
        if (sharedPtr)
        {
            rawPtr = sharedPtr.get();
            sharedPtr.reset();
        }
        else
        {
            ChipLogError(AppServer,
                         "CastingPlayer::GetTargetCastingPlayer() The shared pointer observed by mTargetCastingPlayer has expired "
                         "(become nullptr)");
        }
        return rawPtr;
    }

    /**
     * @brief Compares based on the Id
     */
    bool operator==(const CastingPlayer & other) const
    {
        int compareResult = strcmp(this->mAttributes.id, other.mAttributes.id);
        return (compareResult == 0) ? 1 : 0;
    }

    /**
     * @brief Define the copy constructor
     */
    CastingPlayer(const CastingPlayer & other);

    /**
     * @brief Define the assignment operator
     */
    CastingPlayer & operator=(const CastingPlayer & other);

    /**
     * @return true if this CastingPlayer is connected to the CastingApp
     */
    bool IsConnected() const { return mConnectionState == CASTING_PLAYER_CONNECTED; }

    /**
     * @brief Verifies that a connection exists with this CastingPlayer, or triggers a new commissioning session request. If the
     * CastingApp does not have the nodeId and fabricIndex of this CastingPlayer cached on disk, this will execute the User Directed
     * Commissioning (UDC) process by sending an IdentificationDeclaration message to the CastingPlayer/Commissioner. For certain
     * UDC features, where a Commissioner reply is expected, this API needs to be followed up with the ContinueConnecting() API
     * defiend below. See the Matter UDC specification or parameter class definitions for details on features not included in the
     * description below.
     *
     * @param connectionCallbacks contains the ConnectCallback (Required) and CommissionerDeclarationCallback (Optional) defiend in
     * ConnectCallbacks.h.
     *
     * For example: During CastingPlayer/Commissioner-Generated passcode commissioning, the Commissioner replies with a
     * CommissionerDeclaration message with PasscodeDialogDisplayed and CommissionerPasscode set to true. Given these Commissioner
     * state details, the client is expected to perform some actions, detailed in the ContinueConnecting() API below, and then call
     * the ContinueConnecting() API to complete the process.
     *
     * @param commissioningWindowTimeoutSec (Optional) time (in sec) to keep the commissioning window open, if commissioning is
     * required. Needs to be >= kCommissioningWindowTimeoutSec.
     *
     * @param idOptions (Optional) Parameters in the IdentificationDeclaration message sent by the Commissionee to the Commissioner.
     * These parameters specify the information relating to the requested commissioning session.
     *
     * For example: To invoke the CastingPlayer/Commissioner-Generated passcode commissioning flow, the client would call this API
     * with IdentificationDeclarationOptions containing CommissionerPasscode set to true. See IdentificationDeclarationOptions.h for
     * a complete list of optional parameters.
     *
     * Furthermore, attributes (such as VendorId) describe the TargetApp that the client wants to interact with after commissioning.
     * If this value is passed in, VerifyOrEstablishConnection() will force UDC, in case the desired
     * TargetApp is not found in the on-device CastingStore.
     */
    void VerifyOrEstablishConnection(ConnectionCallbacks connectionCallbacks,
                                     uint16_t commissioningWindowTimeoutSec     = kCommissioningWindowTimeoutSec,
                                     IdentificationDeclarationOptions idOptions = IdentificationDeclarationOptions());

    /**
     * @brief This is a continuation of the CastingPlayer/Commissioner-Generated passcode commissioning flow started via the
     * VerifyOrEstablishConnection() API above. It continues the UDC process by sending a second IdentificationDeclaration message
     * to Commissioner containing CommissionerPasscode and CommissionerPasscodeReady set to true. At this point it is assumed that
     * the following have occurred:
     *
     * 1. Client (Commissionee) has sent the first IdentificationDeclaration message, via VerifyOrEstablishConnection(), to the
     * Commissioner containing CommissionerPasscode set to true.
     * 2. Commissioner generated and displayed a passcode.
     * 3. The Commissioner replied with a CommissionerDecelration message with PasscodeDialogDisplayed and CommissionerPasscode set
     * to true.
     * 4. Client has handled the Commissioner's CommissionerDecelration message.
     * 5. Client prompted user to input Passcode from Commissioner.
     * 6. Client has updated the commissioning session's PAKE verifier using the user input Passcode by updating the CastingApp's
     * CommissionableDataProvider
     * (matter::casting::core::CastingApp::GetInstance()->UpdateCommissionableDataProvider(CommissionableDataProvider)).
     *
     * The same connectionCallbacks and commissioningWindowTimeoutSec parameters passed into VerifyOrEstablishConnection() will be
     * used.
     * @return CHIP_NO_ERROR if this function was called with the CastingPlayer in the correct state and an error otherwise.
     */
    CHIP_ERROR ContinueConnecting();

    /**
     * @brief This cancels the CastingPlayer/Commissioner-Generated passcode commissioning flow started via the
     * VerifyOrEstablishConnection() API above. It constructs and sends an IdentificationDeclaration message to the
     * CastingPlayer/Commissioner containing CancelPasscode set to true. It is used to indicate that the Client/Commissionee user
     * has cancelled the commissioning process. This indicates that the CastingPlayer/Commissioner can dismiss any dialogs
     * corresponding to commissioning, such as a Passcode input dialog or a Passcode display dialog. Note: StopConnecting() does not
     * call the ConnectCallback() callback passed to the VerifyOrEstablishConnection() API above since no connection is established.
     * @return CHIP_NO_ERROR if this function was called with the CastingPlayer in the correct state and CHIP_ERROR_INCORRECT_STATE.
     * otherwise. StopConnecting() can only be called by the client during the CastingPlayer/Commissioner-Generated passcode
     * commissioning flow. Calling StopConnecting() during the Client/Commissionee-Generated commissioning flow will return a
     * CHIP_ERROR_INCORRECT_STATE error.
     *
     * @note This method will free the calling object as a side effect.
     */
    CHIP_ERROR StopConnecting();

    /**
     * @brief Sets the internal connection state of this CastingPlayer to "disconnected"
     *
     * @note This method will free the calling object as a side effect.
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

    /**
     * @brief Return the current state of the CastingPlayer
     */
    ConnectionState GetConnectionState() const
    {
        ChipLogError(AppServer, "CastingPlayer::GetConnectionState() state: %d", mConnectionState);
        return mConnectionState;
    }

private:
    std::vector<memory::Strong<Endpoint>> mEndpoints;
    ConnectionState mConnectionState = CASTING_PLAYER_NOT_CONNECTED;
    CastingPlayerAttributes mAttributes;
    IdentificationDeclarationOptions mIdOptions;
    // This is a std::weak_ptr. A std::weak_ptr is a non-owning reference to an object managed by one
    // or more std::shared_ptr instances. When the last std::shared_ptr instance that owns the managed
    // object is destroyed or reset, the object itself is automatically destroyed, and all
    // std::weak_ptr instances that reference that object become expired.
    static memory::Weak<CastingPlayer> mTargetCastingPlayer;
    uint16_t mCommissioningWindowTimeoutSec = kCommissioningWindowTimeoutSec;
    ConnectCallback mOnCompleted            = {};
    bool mClientProvidedCommissionerDeclarationCallback;

    /**
     * @brief This internal version of the StopConnecting API cancels the Client/Commissionee-Generated passcode or the
     * CastingPlayer/Commissioner-Generated passcode commissioning flow started via the VerifyOrEstablishConnection() API above.
     * Furthermore, StopConnecting operates in two ways as governed by the shouldSendIdentificationDeclarationMessage flag:
     * 1. If shouldSendIdentificationDeclarationMessage is true. StopConnecting constructs and sends an IdentificationDeclaration
     * message to the CastingPlayer/Commissioner containing CancelPasscode set to true. The CancelPasscode flag set to true conveys
     * that the Client/Commissionee user has cancelled the commissioning session. This indicates that the CastingPlayer/Commissioner
     * can dismiss any dialogs corresponding to commissioning, such as a Passcode input dialog or a Passcode display dialog. In this
     * case, since StopConnecting was called by the Client/Commissionee, StopConnecting() does not call the ConnectCallback()
     * callback passed to the VerifyOrEstablishConnection().
     * 2. If shouldSendIdentificationDeclarationMessage is false. StopConnecting does not send an IdentificationDeclaration message
     * to the CastingPlayer/Commissioner since the CastingPlayer/Commissioner notified the Client/Commissionee that the connection
     * is aborted. If the (Optional) ConnectionCallbacks mCommissionerDeclarationCallback is not set, it calls ConnectionCallbacks
     * mOnConnectionComplete callback with CHIP_ERROR_CONNECTION_ABORTED.
     * @param shouldSendIdentificationDeclarationMessage if true, send the IdentificationDeclaration message to the CastingPlayer
     * with CancelPasscode set to true. If false, only call the ConnectionCallbacks mCommissionerDeclarationCallback callback passed
     * to the VerifyOrEstablishConnection() API above, without sending the IdentificationDeclaration message.
     * @return CHIP_NO_ERROR if this function was called with the CastingPlayer in the correct state and an error otherwise.
     */
    CHIP_ERROR StopConnecting(bool shouldSendIdentificationDeclarationMessage);

    /**
     * @brief resets this CastingPlayer's state and calls mOnCompleted with the CHIP_ERROR. Also, after calling mOnCompleted, it
     * clears mOnCompleted by setting it to a nullptr.
     *
     * @note This method will free the calling object as a side effect.
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

    friend class CommissionerDeclarationHandler;
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
