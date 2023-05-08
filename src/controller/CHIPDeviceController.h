/*
 *
 *    Copyright (c) 2020-2022 Project CHIP Authors
 *    Copyright (c) 2013-2017 Nest Labs, Inc.
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
 *      Declaration of CHIP Device Controller, a common class
 *      that implements connecting and messaging and will later
 *      be expanded to support discovery, pairing and
 *      provisioning of CHIP  devices.
 *
 */

#pragma once

#include <app/CASEClientPool.h>
#include <app/CASESessionManager.h>
#include <app/ClusterStateCache.h>
#include <app/OperationalSessionSetup.h>
#include <app/OperationalSessionSetupPool.h>
#include <controller/AbstractDnssdDiscoveryController.h>
#include <controller/AutoCommissioner.h>
#include <controller/CHIPCluster.h>
#include <controller/CHIPDeviceControllerSystemState.h>
#include <controller/CommissioneeDeviceProxy.h>
#include <controller/CommissioningDelegate.h>
#include <controller/DevicePairingDelegate.h>
#include <controller/OperationalCredentialsDelegate.h>
#include <controller/SetUpCodePairer.h>
#include <credentials/FabricTable.h>
#include <credentials/attestation_verifier/DeviceAttestationDelegate.h>
#include <credentials/attestation_verifier/DeviceAttestationVerifier.h>
#include <inet/InetInterface.h>
#include <lib/core/CHIPConfig.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/CHIPPersistentStorageDelegate.h>
#include <lib/core/DataModelTypes.h>
#include <lib/core/TLV.h>
#include <lib/support/DLLUtil.h>
#include <lib/support/Pool.h>
#include <lib/support/SafeInt.h>
#include <lib/support/SerializableIntegerSet.h>
#include <lib/support/Span.h>
#include <lib/support/ThreadOperationalDataset.h>
#include <messaging/ExchangeMgr.h>
#include <protocols/secure_channel/MessageCounterManager.h>
#include <protocols/secure_channel/RendezvousParameters.h>
#include <protocols/user_directed_commissioning/UserDirectedCommissioning.h>
#include <system/SystemClock.h>
#include <transport/SessionManager.h>
#include <transport/TransportMgr.h>
#include <transport/raw/UDP.h>

#if CONFIG_DEVICE_LAYER
#include <platform/CHIPDeviceLayer.h>
#endif

#if CONFIG_NETWORK_LAYER_BLE
#include <ble/BleLayer.h>
#endif
#include <controller/DeviceDiscoveryDelegate.h>

namespace chip {

namespace Controller {

using namespace chip::Protocols::UserDirectedCommissioning;

constexpr uint16_t kNumMaxActiveDevices = CHIP_CONFIG_CONTROLLER_MAX_ACTIVE_DEVICES;

// Raw functions for cluster callbacks
void OnBasicFailure(void * context, CHIP_ERROR err);

struct ControllerInitParams
{
    DeviceControllerSystemState * systemState                       = nullptr;
    DeviceDiscoveryDelegate * deviceDiscoveryDelegate               = nullptr;
    OperationalCredentialsDelegate * operationalCredentialsDelegate = nullptr;

    /* The following keypair must correspond to the public key used for generating
       controllerNOC. It's used by controller to establish CASE sessions with devices */
    Crypto::P256Keypair * operationalKeypair = nullptr;

    /**
     * Controls whether or not the operationalKeypair should be owned by the caller.
     * By default, this is false, but if the keypair cannot be serialized, then
     * setting this to true will allow the caller to manage this keypair's lifecycle.
     */
    bool hasExternallyOwnedOperationalKeypair = false;

    /* The following certificates must be in x509 DER format */
    ByteSpan controllerNOC;
    ByteSpan controllerICAC;
    ByteSpan controllerRCAC;

    /**
     * Controls whether we permit multiple DeviceController instances to exist
     * on the same logical fabric (identified by the tuple of the fabric's
     * root public key + fabric id).
     *
     * Each controller instance will be associated with its own FabricIndex.
     * This pivots the FabricTable to tracking identities instead of fabrics,
     * represented by FabricInfo instances that can have colliding logical fabrics.
     *
     */
    bool permitMultiControllerFabrics = false;

    //
    // Controls enabling server cluster interactions on a controller. This in turn
    // causes the following to get enabled:
    //
    //  - Advertisement of active controller operational identities.
    //
    bool enableServerInteractions = false;

    /**
     * Controls whether shutdown of the controller removes the corresponding
     * entry from the fabric table.  For now the removal is just from the
     * in-memory table, not from storage, which means that after controller
     * shutdown the storage and the in-memory fabric table will be out of sync.
     * This is acceptable for implementations that don't actually store any of
     * the fabric table information, but if someone wants a true removal at some
     * point another option will need to be added here.
     */
    bool removeFromFabricTableOnShutdown = true;

    chip::VendorId controllerVendorId;
};

struct CommissionerInitParams : public ControllerInitParams
{
    DevicePairingDelegate * pairingDelegate     = nullptr;
    CommissioningDelegate * defaultCommissioner = nullptr;
    // Device attestation verifier instance for the commissioning.
    // If null, the globally set attestation verifier (e.g. from GetDeviceAttestationVerifier()
    // singleton) will be used.
    Credentials::DeviceAttestationVerifier * deviceAttestationVerifier = nullptr;
};

/**
 * @brief
 *   Controller applications can use this class to communicate with already paired CHIP devices. The
 *   application is required to provide access to the persistent storage, where the paired device information
 *   is stored. This object of this class can be initialized with the data from the storage (List of devices,
 *   and device pairing information for individual devices). Alternatively, this class can retrieve the
 *   relevant information when the application tries to communicate with the device
 */
class DLL_EXPORT DeviceController : public AbstractDnssdDiscoveryController
{
public:
    DeviceController();
    ~DeviceController() override {}

    CHIP_ERROR Init(ControllerInitParams params);

    /**
     * @brief
     *  Tears down the entirety of the stack, including destructing key objects in the system.
     *  This expects to be called with external thread synchronization, and will not internally
     *  grab the CHIP stack lock.
     *
     *  This will also not stop the CHIP event queue / thread (if one exists).  Consumers are expected to
     *  ensure this happened before calling this method.
     */
    virtual void Shutdown();

    SessionManager * SessionMgr()
    {
        if (mSystemState)
        {
            return mSystemState->SessionMgr();
        }

        return nullptr;
    }

    CHIP_ERROR GetPeerAddressAndPort(NodeId peerId, Inet::IPAddress & addr, uint16_t & port);

    /**
     * @brief
     *   Looks up the PeerAddress for an established CASE session.
     *
     * @param[in] nodeId the NodeId of the target.
     * @param[out] addr the PeerAddress to be filled on success
     *
     * @return CHIP_ERROR CHIP_ERROR_NOT_CONNECTED if no CASE session exists for the device
     */
    CHIP_ERROR GetPeerAddress(NodeId nodeId, Transport::PeerAddress & addr);

    ScopedNodeId GetPeerScopedId(NodeId nodeId) { return ScopedNodeId(nodeId, GetFabricIndex()); }

    /**
     * This function finds the device corresponding to deviceId, and establishes
     * a CASE session with it.
     *
     * Once the CASE session is successfully established the `onConnectedDevice`
     * callback is called. This can happen before GetConnectedDevice returns if
     * there is an existing CASE session.
     *
     * If a CASE sessions fails to be established, the `onError` callback will
     * be called.  This can also happen before GetConnectedDevice returns.
     *
     * An error return from this function means that neither callback has been
     * called yet, and neither callback will be called in the future.
     */
    CHIP_ERROR GetConnectedDevice(NodeId peerNodeId, Callback::Callback<OnDeviceConnected> * onConnection,
                                  chip::Callback::Callback<OnDeviceConnectionFailure> * onFailure)
    {
        VerifyOrReturnError(mState == State::Initialized, CHIP_ERROR_INCORRECT_STATE);
        mSystemState->CASESessionMgr()->FindOrEstablishSession(ScopedNodeId(peerNodeId, GetFabricIndex()), onConnection, onFailure);
        return CHIP_NO_ERROR;
    }

    /**
     * @brief
     *   Compute a PASE verifier and passcode ID for the desired setup pincode.
     *
     *   This can be used to open a commissioning window on the device for
     *   additional administrator commissioning.
     *
     * @param[in] iterations      The number of iterations to use when generating the verifier
     * @param[in] setupPincode    The desired PIN code to use
     * @param[in] salt            The 16-byte salt for verifier computation
     * @param[out] outVerifier    The Spake2pVerifier to be populated on success
     *
     * @return CHIP_ERROR         CHIP_NO_ERROR on success, or corresponding error
     */
    CHIP_ERROR ComputePASEVerifier(uint32_t iterations, uint32_t setupPincode, const ByteSpan & salt,
                                   Spake2pVerifier & outVerifier);

    void RegisterDeviceDiscoveryDelegate(DeviceDiscoveryDelegate * delegate) { mDeviceDiscoveryDelegate = delegate; }

    /**
     * @brief Get the Compressed Fabric ID assigned to the device.
     */
    uint64_t GetCompressedFabricId() const
    {
        const auto * fabricInfo = GetFabricInfo();
        return (fabricInfo != nullptr) ? static_cast<uint64_t>(fabricInfo->GetCompressedFabricId()) : kUndefinedCompressedFabricId;
    }

    /**
     * @brief Get the Compressed Fabric Id as a big-endian 64 bit octet string.
     *
     * Output span is resized to 8 bytes on success if it was larger.
     *
     * @param outBytes span to contain the compressed fabric ID, must be at least 8 bytes long
     * @return CHIP_ERROR_BUFFER_TOO_SMALL if `outBytes` is too small, CHIP_ERROR_INVALID_FABRIC_INDEX
     *         if the controller is somehow not associated with a fabric (internal error!) or
     *         CHIP_NO_ERROR on success.
     */
    CHIP_ERROR GetCompressedFabricIdBytes(MutableByteSpan & outBytes) const;

    /**
     * @brief Get the raw Fabric ID assigned to the device.
     */
    uint64_t GetFabricId() const
    {
        const auto * fabricInfo = GetFabricInfo();
        return (fabricInfo != nullptr) ? static_cast<uint64_t>(fabricInfo->GetFabricId()) : kUndefinedFabricId;
    }

    /**
     * @brief Get the Node ID of this instance.
     */
    NodeId GetNodeId() const
    {
        const auto * fabricInfo = GetFabricInfo();
        return (fabricInfo != nullptr) ? static_cast<uint64_t>(fabricInfo->GetNodeId()) : kUndefinedNodeId;
    }

    /**
     * @brief Get the root public key for the fabric
     *
     * @param outRootPublicKey reference to public key object that gets updated on success.
     *
     * @return CHIP_NO_ERROR on success, CHIP_ERROR_INCORRECT_STATE if fabric table is unset, or another internal error
     *         on storage access failure.
     */
    CHIP_ERROR GetRootPublicKey(Crypto::P256PublicKey & outRootPublicKey) const;

    FabricIndex GetFabricIndex() const { return mFabricIndex; }

    const FabricTable * GetFabricTable() const
    {
        if (mSystemState == nullptr)
        {
            return nullptr;
        }
        return mSystemState->Fabrics();
    }

    OperationalCredentialsDelegate * GetOperationalCredentialsDelegate() { return mOperationalCredentialsDelegate; }

    /**
     * @brief
     *   Reconfigures a new set of operational credentials to be used with this
     *   controller given ControllerInitParams state.
     *
     * WARNING: This is a low-level method that should only be called directly
     *          if you know exactly how this will interact with controller state,
     *          since there are several integrations that do this call for you.
     *          It can be used for fine-grained dependency injection of a controller's
     *          NOC and operational keypair.
     */
    CHIP_ERROR InitControllerNOCChain(const ControllerInitParams & params);

protected:
    enum class State
    {
        NotInitialized,
        Initialized
    };

    // This is not public to avoid users of DeviceController relying on "innards" access to
    // the raw fabric table. Everything needed should be available with getters on DeviceController.
    const FabricInfo * GetFabricInfo() const
    {
        VerifyOrReturnError((mState == State::Initialized) && (mFabricIndex != kUndefinedFabricIndex), nullptr);
        VerifyOrReturnError(GetFabricTable() != nullptr, nullptr);

        return GetFabricTable()->FindFabricWithIndex(mFabricIndex);
    }

    State mState;

    FabricIndex mFabricIndex = kUndefinedFabricIndex;

    bool mRemoveFromFabricTableOnShutdown = true;

    // TODO(cecille): Make this configuarable.
    static constexpr int kMaxCommissionableNodes = 10;
    Dnssd::DiscoveredNodeData mCommissionableNodes[kMaxCommissionableNodes];
    DeviceControllerSystemState * mSystemState = nullptr;

    ControllerDeviceInitParams GetControllerDeviceInitParams();

    OperationalCredentialsDelegate * mOperationalCredentialsDelegate;

    chip::VendorId mVendorId;

    DiscoveredNodeList GetDiscoveredNodes() override { return DiscoveredNodeList(mCommissionableNodes); }
};

#if CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY
using UdcTransportMgr = TransportMgr<Transport::UDP /* IPv6 */
#if INET_CONFIG_ENABLE_IPV4
                                     ,
                                     Transport::UDP /* IPv4 */
#endif
                                     >;
#endif

/**
 * @brief Callback prototype for ExtendArmFailSafe command.
 */
typedef void (*OnExtendFailsafeSuccess)(
    void * context, const app::Clusters::GeneralCommissioning::Commands::ArmFailSafeResponse::DecodableType & data);
typedef void (*OnExtendFailsafeFailure)(void * context, CHIP_ERROR error);

/**
 * @brief
 *   The commissioner applications can use this class to pair new/unpaired CHIP devices. The application is
 *   required to provide write access to the persistent storage, where the paired device information
 *   will be stored.
 */
class DLL_EXPORT DeviceCommissioner : public DeviceController,
#if CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY // make this commissioner discoverable
                                      public Protocols::UserDirectedCommissioning::InstanceNameResolver,
#endif
                                      public SessionEstablishmentDelegate,
                                      public app::ClusterStateCache::Callback
{
public:
    DeviceCommissioner();
    ~DeviceCommissioner() override {}

#if CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY // make this commissioner discoverable
    /**
     * Set port for User Directed Commissioning
     */
    CHIP_ERROR SetUdcListenPort(uint16_t listenPort);
#endif // CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY

    /**
     * Commissioner-specific initialization, includes parameters such as the pairing delegate.
     */
    CHIP_ERROR Init(CommissionerInitParams params);

    /**
     * @brief
     *  Tears down the entirety of the stack, including destructing key objects in the system.
     *  This is not a thread-safe API, and should be called with external synchronization.
     *
     *  Please see implementation for more details.
     */
    void Shutdown() override;

    // ----- Connection Management -----
    /**
     * @brief
     *   Pair a CHIP device with the provided code. The code can be either a QRCode
     *   or a Manual Setup Code.
     *   Use registered DevicePairingDelegate object to receive notifications on
     *   pairing status updates.
     *
     *   Note: Pairing process requires that the caller has registered PersistentStorageDelegate
     *         in the Init() call.
     *
     * @param[in] remoteDeviceId        The remote device Id.
     * @param[in] setUpCode             The setup code for connecting to the device
     * @param[in] discoveryType         The network discovery type, defaults to DiscoveryType::kAll.
     */
    CHIP_ERROR PairDevice(NodeId remoteDeviceId, const char * setUpCode, DiscoveryType discoveryType = DiscoveryType::kAll);
    CHIP_ERROR PairDevice(NodeId remoteDeviceId, const char * setUpCode, const CommissioningParameters & CommissioningParameters,
                          DiscoveryType discoveryType = DiscoveryType::kAll);

    /**
     * @brief
     *   Pair a CHIP device with the provided Rendezvous connection parameters.
     *   Use registered DevicePairingDelegate object to receive notifications on
     *   pairing status updates.
     *
     *   Note: Pairing process requires that the caller has registered PersistentStorageDelegate
     *         in the Init() call.
     *
     * @param[in] remoteDeviceId        The remote device Id.
     * @param[in] rendezvousParams      The Rendezvous connection parameters
     */
    CHIP_ERROR PairDevice(NodeId remoteDeviceId, RendezvousParameters & rendezvousParams);
    /**
     * @overload
     * @param[in] remoteDeviceId        The remote device Id.
     * @param[in] rendezvousParams      The Rendezvous connection parameters
     * @param[in] commissioningParams    The commissioning parameters (uses default if not supplied)
     */
    CHIP_ERROR PairDevice(NodeId remoteDeviceId, RendezvousParameters & rendezvousParams,
                          CommissioningParameters & commissioningParams);

    /**
     * @brief
     *   Start establishing a PASE connection with a node for the purposes of commissioning.
     *   Commissioners that wish to use the auto-commissioning functions should use the
     *   supplied "PairDevice" functions above to automatically establish a connection then
     *   perform commissioning. This function is intended to be use by commissioners that
     *   are not using the supplied auto-commissioner.
     *
     *   This function is non-blocking. PASE is established once the DevicePairingDelegate
     *   receives the OnPairingComplete call.
     *
     *   PASE connections can only be established with nodes that have their commissioning
     *   window open. The PASE connection will fail if this window is not open and the
     *   OnPairingComplete will be called with an error.
     *
     * @param[in] remoteDeviceId        The remote device Id.
     * @param[in] params                The Rendezvous connection parameters
     */
    CHIP_ERROR EstablishPASEConnection(NodeId remoteDeviceId, RendezvousParameters & params);

    /**
     * @brief
     *   Start establishing a PASE connection with a node for the purposes of commissioning.
     *   Commissioners that wish to use the auto-commissioning functions should use the
     *   supplied "PairDevice" functions above to automatically establish a connection then
     *   perform commissioning. This function is intended to be used by commissioners that
     *   are not using the supplied auto-commissioner.
     *
     *   This function is non-blocking. PASE is established once the DevicePairingDelegate
     *   receives the OnPairingComplete call.
     *
     *   PASE connections can only be established with nodes that have their commissioning
     *   window open. The PASE connection will fail if this window is not open and in that case
     *   OnPairingComplete will be called with an error.
     *
     * @param[in] remoteDeviceId        The remote device Id.
     * @param[in] setUpCode             The setup code for connecting to the device
     * @param[in] discoveryType         The network discovery type, defaults to DiscoveryType::kAll.
     */
    CHIP_ERROR EstablishPASEConnection(NodeId remoteDeviceId, const char * setUpCode,
                                       DiscoveryType discoveryType = DiscoveryType::kAll);

    /**
     * @brief
     *   Start the auto-commissioning process on a node after establishing a PASE connection.
     *   This function is intended to be used in conjunction with the EstablishPASEConnection
     *   function. It can be called either before or after the DevicePairingDelegate receives
     *   the OnPairingComplete call. Commissioners that want to perform simple auto-commissioning
     *   should use the supplied "PairDevice" functions above, which will establish the PASE
     *   connection and commission automatically.
     *
     * @param[in] remoteDeviceId        The remote device Id.
     * @param[in] params                The commissioning parameters
     */
    CHIP_ERROR Commission(NodeId remoteDeviceId, CommissioningParameters & params);
    CHIP_ERROR Commission(NodeId remoteDeviceId);

    /**
     * @brief
     *   This function instructs the commissioner to proceed to the next stage of commissioning after
     *   attestation is reported to an installed attestation delegate.
     *
     * @param[in] device                The device being commissioned.
     * @param[in] attestationResult     The attestation result to use instead of whatever the device
     *                                  attestation verifier came up with. May be a success or an error result.
     */
    CHIP_ERROR
    ContinueCommissioningAfterDeviceAttestation(DeviceProxy * device, Credentials::AttestationVerificationResult attestationResult);

    CHIP_ERROR GetDeviceBeingCommissioned(NodeId deviceId, CommissioneeDeviceProxy ** device);

    /**
     * @brief
     *   This function stops a pairing process that's in progress. It does not delete the pairing of a previously
     *   paired device.
     *
     * @param[in] remoteDeviceId        The remote device Id.
     *
     * @return CHIP_ERROR               CHIP_NO_ERROR on success, or corresponding error
     */
    CHIP_ERROR StopPairing(NodeId remoteDeviceId);

    /**
     * @brief
     *   Remove pairing for a paired device. If the device is currently being paired, it'll stop the pairing process.
     *
     * @param[in] remoteDeviceId        The remote device Id.
     *
     * @return CHIP_ERROR               CHIP_NO_ERROR on success, or corresponding error
     */
    CHIP_ERROR UnpairDevice(NodeId remoteDeviceId);

    //////////// SessionEstablishmentDelegate Implementation ///////////////
    void OnSessionEstablishmentError(CHIP_ERROR error) override;
    void OnSessionEstablished(const SessionHandle & session) override;

    void RendezvousCleanup(CHIP_ERROR status);

    void PerformCommissioningStep(DeviceProxy * device, CommissioningStage step, CommissioningParameters & params,
                                  CommissioningDelegate * delegate, EndpointId endpoint, Optional<System::Clock::Timeout> timeout);

    /**
     * @brief
     *   This function validates the Attestation Information sent by the device.
     *
     * @param[in] info Structure contatining all the required information for validating the device attestation.
     */
    CHIP_ERROR ValidateAttestationInfo(const Credentials::DeviceAttestationVerifier::AttestationInfo & info);

    /**
     * @brief
     * Sends CommissioningStepComplete report to the commissioning delegate. Function will fill in current step.
     * @params[in] err      error from the current step
     * @params[in] report   report to send. Current step will be filled in automatically
     */
    void
    CommissioningStageComplete(CHIP_ERROR err,
                               CommissioningDelegate::CommissioningReport report = CommissioningDelegate::CommissioningReport());

    /**
     * @brief
     *   This function is called by the DevicePairingDelegate to indicate that network credentials have been set
     * on the CommissioningParameters of the CommissioningDelegate using CommissioningDelegate.SetCommissioningParameters().
     * As a result, commissioning can advance to the next stage.
     *
     * The DevicePairingDelegate may call this method from the OnScanNetworksSuccess and OnScanNetworksFailure callbacks,
     * or it may call this method after obtaining network credentials using asyncronous methods (prompting user, cloud API call,
     * etc).
     *
     * If an error happens in the subsequent network commissioning step (either NetworkConfig or ConnectNetwork commands)
     * then the DevicePairingDelegate will receive the error in completionStatus.networkCommissioningStatus and the
     * commissioning stage will return to kNeedsNetworkCreds so that the DevicePairingDelegate can re-attempt with new
     * network information. The DevicePairingDelegate can exit the commissioning process by calling StopPairing.
     *
     * @return CHIP_ERROR   The return status. Returns CHIP_ERROR_INCORRECT_STATE if not in the correct state (kNeedsNetworkCreds).
     */
    CHIP_ERROR NetworkCredentialsReady();

    /**
     * @brief
     *  This function returns the current CommissioningStage for this commissioner.
     */
    CommissioningStage GetCommissioningStage() { return mCommissioningStage; }

#if CONFIG_NETWORK_LAYER_BLE
#if CHIP_DEVICE_CONFIG_ENABLE_BOTH_COMMISSIONER_AND_COMMISSIONEE
    /**
     * @brief
     *   Prior to commissioning, the Controller should make sure the BleLayer transport
     *   is set to the Commissioner transport and not the Server transport.
     */
    void ConnectBleTransportToSelf();
#endif // CHIP_DEVICE_CONFIG_ENABLE_BOTH_COMMISSIONER_AND_COMMISSIONEE

    /**
     * @brief
     *   Once we have finished all commissioning work, the Controller should close the BLE
     *   connection to the device and establish CASE session / another PASE session to the device
     *   if needed.
     */
    void CloseBleConnection();
#endif
    /**
     * @brief
     *   Discover all devices advertising as commissionable.
     *   Should be called on main loop thread.
     * * @param[in] filter  Browse filter - controller will look for only the specified subtype.
     * @return CHIP_ERROR   The return status
     */
    CHIP_ERROR DiscoverCommissionableNodes(Dnssd::DiscoveryFilter filter);

    /**
     * Stop commissionable discovery triggered by a previous
     * DiscoverCommissionableNodes call.
     */
    CHIP_ERROR StopCommissionableDiscovery();

    /**
     * @brief
     *   Returns information about discovered devices.
     *   Should be called on main loop thread.
     * @return const DiscoveredNodeData* info about the selected device. May be nullptr if no information has been returned yet.
     */
    const Dnssd::DiscoveredNodeData * GetDiscoveredDevice(int idx);

    /**
     * @brief
     *   Returns the max number of commissionable nodes this commissioner can track mdns information for.
     * @return int  The max number of commissionable nodes supported
     */
    int GetMaxCommissionableNodesSupported() { return kMaxCommissionableNodes; }

#if CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY // make this commissioner discoverable
    /**
     * @brief
     *   Called when a UDC message is received specifying the given instanceName
     * This method indicates that UDC Server needs the Commissionable Node corresponding to
     * the given instance name to be found. UDC Server will wait for OnCommissionableNodeFound.
     *
     * @param instanceName DNS-SD instance name for the client requesting commissioning
     *
     */
    void FindCommissionableNode(char * instanceName) override;

    /**
     * @brief
     *   Return the UDC Server instance
     *
     */
    UserDirectedCommissioningServer * GetUserDirectedCommissioningServer() { return mUdcServer; }
#endif // CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY

    /**
     * @brief
     *   Overrides method from AbstractDnssdDiscoveryController
     *
     * @param nodeData DNS-SD node information
     *
     */
    void OnNodeDiscovered(const chip::Dnssd::DiscoveredNodeData & nodeData) override;

    void RegisterPairingDelegate(DevicePairingDelegate * pairingDelegate) { mPairingDelegate = pairingDelegate; }
    DevicePairingDelegate * GetPairingDelegate() const { return mPairingDelegate; }

    // ClusterStateCache::Callback impl
    void OnDone(app::ReadClient *) override;

    // Issue an NOC chain using the associated OperationalCredentialsDelegate. The NOC chain will
    // be provided in X509 DER format.
    // NOTE: This is only valid assuming that `mOperationalCredentialsDelegate` is what is desired
    // to issue the NOC chain.
    CHIP_ERROR IssueNOCChain(const ByteSpan & NOCSRElements, NodeId nodeId,
                             chip::Callback::Callback<OnNOCChainGeneration> * callback);

    void SetDeviceAttestationVerifier(Credentials::DeviceAttestationVerifier * deviceAttestationVerifier)
    {
        mDeviceAttestationVerifier = deviceAttestationVerifier;
    }

    Optional<CommissioningParameters> GetCommissioningParameters()
    {
        return mDefaultCommissioner == nullptr ? NullOptional : MakeOptional(mDefaultCommissioner->GetCommissioningParameters());
    }

    // Reset the arm failsafe timer during commissioning.  If this returns
    // false, that means that the timer was already set for a longer time period
    // than the new time we are trying to set.  In this case, neither
    // onSuccess nor onFailure will be called.
    bool ExtendArmFailSafe(DeviceProxy * proxy, CommissioningStage step, uint16_t armFailSafeTimeout,
                           Optional<System::Clock::Timeout> commandTimeout, OnExtendFailsafeSuccess onSuccess,
                           OnExtendFailsafeFailure onFailure);

private:
    DevicePairingDelegate * mPairingDelegate;

    DeviceProxy * mDeviceBeingCommissioned               = nullptr;
    CommissioneeDeviceProxy * mDeviceInPASEEstablishment = nullptr;

    CommissioningStage mCommissioningStage = CommissioningStage::kSecurePairing;
    bool mRunCommissioningAfterConnection  = false;

    ObjectPool<CommissioneeDeviceProxy, kNumMaxActiveDevices> mCommissioneeDevicePool;

#if CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY // make this commissioner discoverable
    UserDirectedCommissioningServer * mUdcServer = nullptr;
    // mUdcTransportMgr is for insecure communication (ex. user directed commissioning)
    UdcTransportMgr * mUdcTransportMgr = nullptr;
    uint16_t mUdcListenPort            = CHIP_UDC_PORT;
#endif // CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY

#if CONFIG_NETWORK_LAYER_BLE
    static void OnDiscoveredDeviceOverBleSuccess(void * appState, BLE_CONNECTION_OBJECT connObj);
    static void OnDiscoveredDeviceOverBleError(void * appState, CHIP_ERROR err);
    RendezvousParameters mRendezvousParametersForDeviceDiscoveredOverBle;
#endif

    CHIP_ERROR LoadKeyId(PersistentStorageDelegate * delegate, uint16_t & out);

    /* This function sends a Device Attestation Certificate chain request to the device.
       The function does not hold a reference to the device object.
     */
    CHIP_ERROR SendCertificateChainRequestCommand(DeviceProxy * device, Credentials::CertificateType certificateType,
                                                  Optional<System::Clock::Timeout> timeout);
    /* This function sends an Attestation request to the device.
       The function does not hold a reference to the device object.
     */
    CHIP_ERROR SendAttestationRequestCommand(DeviceProxy * device, const ByteSpan & attestationNonce,
                                             Optional<System::Clock::Timeout> timeout);
    /* This function sends an CSR request to the device.
       The function does not hold a reference to the device object.
     */
    CHIP_ERROR SendOperationalCertificateSigningRequestCommand(DeviceProxy * device, const ByteSpan & csrNonce,
                                                               Optional<System::Clock::Timeout> timeout);
    /* This function sends the operational credentials to the device.
       The function does not hold a reference to the device object.
     */
    CHIP_ERROR SendOperationalCertificate(DeviceProxy * device, const ByteSpan & nocCertBuf, const Optional<ByteSpan> & icaCertBuf,
                                          IdentityProtectionKeySpan ipk, NodeId adminSubject,
                                          Optional<System::Clock::Timeout> timeout);
    /* This function sends the trusted root certificate to the device.
       The function does not hold a reference to the device object.
     */
    CHIP_ERROR SendTrustedRootCertificate(DeviceProxy * device, const ByteSpan & rcac, Optional<System::Clock::Timeout> timeout);

    /* This function is called by the commissioner code when the device completes
       the operational credential provisioning process.
       The function does not hold a reference to the device object.
       */
    CHIP_ERROR OnOperationalCredentialsProvisioningCompletion(DeviceProxy * device);

    /* Callback when the previously sent CSR request results in failure */
    static void OnCSRFailureResponse(void * context, CHIP_ERROR error);

    void ExtendArmFailSafeForDeviceAttestation(const Credentials::DeviceAttestationVerifier::AttestationInfo & info,
                                               Credentials::AttestationVerificationResult result);
    static void OnCertificateChainFailureResponse(void * context, CHIP_ERROR error);
    static void OnCertificateChainResponse(
        void * context, const app::Clusters::OperationalCredentials::Commands::CertificateChainResponse::DecodableType & response);

    static void OnAttestationFailureResponse(void * context, CHIP_ERROR error);
    static void
    OnAttestationResponse(void * context,
                          const app::Clusters::OperationalCredentials::Commands::AttestationResponse::DecodableType & data);

    /**
     * @brief
     *   This function is called by the IM layer when the commissioner receives the CSR from the device.
     *   (Reference: Specifications section 11.18.5.6. NOCSR Elements)
     *
     * @param[in] context               The context provided while registering the callback.
     * @param[in] data                  The response struct containing the following fields:
     *                                    NOCSRElements: CSR elements as per specifications section 11.22.5.6. NOCSR Elements.
     *                                    AttestationSignature: Cryptographic signature generated for the fields in the response
     * message.
     */
    static void OnOperationalCertificateSigningRequest(
        void * context, const app::Clusters::OperationalCredentials::Commands::CSRResponse::DecodableType & data);

    /* Callback when adding operational certs to device results in failure */
    static void OnAddNOCFailureResponse(void * context, CHIP_ERROR errro);
    /* Callback when the device confirms that it has added the operational certificates */
    static void
    OnOperationalCertificateAddResponse(void * context,
                                        const app::Clusters::OperationalCredentials::Commands::NOCResponse::DecodableType & data);

    /* Callback when the device confirms that it has added the root certificate */
    static void OnRootCertSuccessResponse(void * context, const chip::app::DataModel::NullObjectType &);
    /* Callback called when adding root cert to device results in failure */
    static void OnRootCertFailureResponse(void * context, CHIP_ERROR error);

    static void OnDeviceConnectedFn(void * context, Messaging::ExchangeManager & exchangeMgr, const SessionHandle & sessionHandle);
    static void OnDeviceConnectionFailureFn(void * context, const ScopedNodeId & peerId, CHIP_ERROR error);
#if CHIP_DEVICE_CONFIG_ENABLE_AUTOMATIC_CASE_RETRIES
    static void OnDeviceConnectionRetryFn(void * context, const ScopedNodeId & peerId, CHIP_ERROR error,
                                          System::Clock::Seconds16 retryTimeout);
#endif // CHIP_DEVICE_CONFIG_ENABLE_AUTOMATIC_CASE_RETRIES

    static void OnDeviceAttestationInformationVerification(void * context,
                                                           const Credentials::DeviceAttestationVerifier::AttestationInfo & info,
                                                           Credentials::AttestationVerificationResult result);

    static void OnDeviceNOCChainGeneration(void * context, CHIP_ERROR status, const ByteSpan & noc, const ByteSpan & icac,
                                           const ByteSpan & rcac, Optional<IdentityProtectionKeySpan> ipk,
                                           Optional<NodeId> adminSubject);
    static void OnArmFailSafe(void * context,
                              const chip::app::Clusters::GeneralCommissioning::Commands::ArmFailSafeResponse::DecodableType & data);
    static void OnSetRegulatoryConfigResponse(
        void * context,
        const chip::app::Clusters::GeneralCommissioning::Commands::SetRegulatoryConfigResponse::DecodableType & data);
    static void
    OnScanNetworksResponse(void * context,
                           const app::Clusters::NetworkCommissioning::Commands::ScanNetworksResponse::DecodableType & data);
    static void OnScanNetworksFailure(void * context, CHIP_ERROR err);
    static void
    OnNetworkConfigResponse(void * context,
                            const app::Clusters::NetworkCommissioning::Commands::NetworkConfigResponse::DecodableType & data);
    static void OnConnectNetworkResponse(
        void * context, const chip::app::Clusters::NetworkCommissioning::Commands::ConnectNetworkResponse::DecodableType & data);
    static void OnCommissioningCompleteResponse(
        void * context,
        const chip::app::Clusters::GeneralCommissioning::Commands::CommissioningCompleteResponse::DecodableType & data);
    static void OnDisarmFailsafe(void * context,
                                 const app::Clusters::GeneralCommissioning::Commands::ArmFailSafeResponse::DecodableType & data);
    static void OnDisarmFailsafeFailure(void * context, CHIP_ERROR error);
    void DisarmDone();
    static void OnArmFailSafeExtendedForDeviceAttestation(
        void * context, const chip::app::Clusters::GeneralCommissioning::Commands::ArmFailSafeResponse::DecodableType & data);
    static void OnFailedToExtendedArmFailSafeDeviceAttestation(void * context, CHIP_ERROR error);

    /**
     * @brief
     *   This function processes the CSR sent by the device.
     *   (Reference: Specifications section 11.18.5.6. NOCSR Elements)
     *
     * @param[in] proxy           device proxy
     * @param[in] NOCSRElements   CSR elements as per specifications section 11.22.5.6. NOCSR Elements.
     * @param[in] AttestationSignature       Cryptographic signature generated for all the above fields.
     * @param[in] dac               device attestation certificate
     * @param[in] pai               Product Attestation Intermediate certificate
     * @param[in] csrNonce          certificate signing request nonce
     */
    CHIP_ERROR ProcessCSR(DeviceProxy * proxy, const ByteSpan & NOCSRElements, const ByteSpan & AttestationSignature,
                          const ByteSpan & dac, const ByteSpan & pai, const ByteSpan & csrNonce);

    /**
     * @brief
     *   This function validates the CSR information from the device.
     *   (Reference: Specifications section 11.18.5.6. NOCSR Elements)
     *
     * @param[in] proxy           device proxy
     * @param[in] NOCSRElements   CSR elements as per specifications section 11.22.5.6. NOCSR Elements.
     * @param[in] AttestationSignature       Cryptographic signature generated for all the above fields.
     * @param[in] dac               device attestation certificate
     * @param[in] csrNonce          certificate signing request nonce
     */
    CHIP_ERROR ValidateCSR(DeviceProxy * proxy, const ByteSpan & NOCSRElements, const ByteSpan & AttestationSignature,
                           const ByteSpan & dac, const ByteSpan & csrNonce);

    /**
     * @brief
     *   This function processes the DAC or PAI certificate sent by the device.
     */
    CHIP_ERROR ProcessCertificateChain(const ByteSpan & certificate);

    void HandleAttestationResult(CHIP_ERROR err);

    CommissioneeDeviceProxy * FindCommissioneeDevice(NodeId id);
    CommissioneeDeviceProxy * FindCommissioneeDevice(const Transport::PeerAddress & peerAddress);
    void ReleaseCommissioneeDevice(CommissioneeDeviceProxy * device);

    template <typename RequestObjectT>
    CHIP_ERROR SendCommand(DeviceProxy * device, const RequestObjectT & request,
                           CommandResponseSuccessCallback<typename RequestObjectT::ResponseType> successCb,
                           CommandResponseFailureCallback failureCb, Optional<System::Clock::Timeout> timeout)
    {
        return SendCommand(device, request, successCb, failureCb, 0, timeout);
    }

    template <typename RequestObjectT>
    CHIP_ERROR SendCommand(DeviceProxy * device, const RequestObjectT & request,
                           CommandResponseSuccessCallback<typename RequestObjectT::ResponseType> successCb,
                           CommandResponseFailureCallback failureCb, EndpointId endpoint, Optional<System::Clock::Timeout> timeout)
    {
        ClusterBase cluster(*device->GetExchangeManager(), device->GetSecureSession().Value(), endpoint);
        cluster.SetCommandTimeout(timeout);

        return cluster.InvokeCommand(request, this, successCb, failureCb);
    }

    static CHIP_ERROR
    ConvertFromOperationalCertStatus(chip::app::Clusters::OperationalCredentials::NodeOperationalCertStatusEnum err);

    // Sends commissioning complete callbacks to the delegate depending on the status. Sends
    // OnCommissioningComplete and either OnCommissioningSuccess or OnCommissioningFailure depending on the given completion status.
    void SendCommissioningCompleteCallbacks(NodeId nodeId, const CompletionStatus & completionStatus);

    // Cleans up and resets failsafe as appropriate depending on the error and the failed stage.
    // For success, sends completion report with the CommissioningDelegate and sends callbacks to the PairingDelegate
    // For failures after AddNOC succeeds, sends completion report with the CommissioningDelegate and sends callbacks to the
    // PairingDelegate. In this case, it does not disarm the failsafe or close the pase connection. For failures up through AddNOC,
    // sends a command to immediately expire the failsafe, then sends completion report with the CommissioningDelegate and callbacks
    // to the PairingDelegate upon arm failsafe command completion.
    void CleanupCommissioning(DeviceProxy * proxy, NodeId nodeId, const CompletionStatus & completionStatus);

    // Extend the fail-safe before trying to do network-enable (since after that
    // point, for non-concurrent-commissioning devices, we may not have a way to
    // extend it).
    void ExtendFailsafeBeforeNetworkEnable(DeviceProxy * device, CommissioningParameters & params, CommissioningStage step);

    chip::Callback::Callback<OnDeviceConnected> mOnDeviceConnectedCallback;
    chip::Callback::Callback<OnDeviceConnectionFailure> mOnDeviceConnectionFailureCallback;
#if CHIP_DEVICE_CONFIG_ENABLE_AUTOMATIC_CASE_RETRIES
    chip::Callback::Callback<OnDeviceConnectionRetry> mOnDeviceConnectionRetryCallback;
#endif // CHIP_DEVICE_CONFIG_ENABLE_AUTOMATIC_CASE_RETRIES

    chip::Callback::Callback<Credentials::DeviceAttestationVerifier::OnAttestationInformationVerification>
        mDeviceAttestationInformationVerificationCallback;

    chip::Callback::Callback<OnNOCChainGeneration> mDeviceNOCChainCallback;
    SetUpCodePairer mSetUpCodePairer;
    AutoCommissioner mAutoCommissioner;
    CommissioningDelegate * mDefaultCommissioner =
        nullptr; // Commissioning delegate to call when PairDevice / Commission functions are used
    CommissioningDelegate * mCommissioningDelegate =
        nullptr; // Commissioning delegate that issued the PerformCommissioningStep command
    CompletionStatus commissioningCompletionStatus;

    Platform::UniquePtr<app::ClusterStateCache> mAttributeCache;
    Platform::UniquePtr<app::ReadClient> mReadClient;
    Credentials::AttestationVerificationResult mAttestationResult;
    Platform::UniquePtr<Credentials::DeviceAttestationVerifier::AttestationDeviceInfo> mAttestationDeviceInfo;
    Credentials::DeviceAttestationVerifier * mDeviceAttestationVerifier = nullptr;
};

} // namespace Controller
} // namespace chip
