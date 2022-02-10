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

#include <app/AttributeCache.h>
#include <app/CASEClientPool.h>
#include <app/CASESessionManager.h>
#include <app/OperationalDeviceProxy.h>
#include <app/OperationalDeviceProxyPool.h>
#include <controller/AbstractDnssdDiscoveryController.h>
#include <controller/AutoCommissioner.h>
#include <controller/CHIPCluster.h>
#include <controller/CHIPDeviceControllerSystemState.h>
#include <controller/CommissioneeDeviceProxy.h>
#include <controller/CommissioningDelegate.h>
#include <controller/OperationalCredentialsDelegate.h>
#include <controller/SetUpCodePairer.h>
#include <credentials/DeviceAttestationVerifier.h>
#include <credentials/FabricTable.h>
#include <lib/core/CHIPConfig.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/CHIPPersistentStorageDelegate.h>
#include <lib/core/CHIPTLV.h>
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
#include <transport/SessionManager.h>
#include <transport/TransportMgr.h>
#include <transport/raw/UDP.h>

#include <controller/CHIPDeviceControllerSystemState.h>

#if CONFIG_DEVICE_LAYER
#include <platform/CHIPDeviceLayer.h>
#endif

#if CONFIG_NETWORK_LAYER_BLE
#include <ble/BleLayer.h>
#endif
#if CHIP_DEVICE_CONFIG_ENABLE_DNSSD
#include <controller/DeviceAddressUpdateDelegate.h>
#include <controller/DeviceDiscoveryDelegate.h>
#include <lib/dnssd/Resolver.h>
#include <lib/dnssd/ResolverProxy.h>
#endif

namespace chip {

namespace Controller {

using namespace chip::Protocols::UserDirectedCommissioning;

constexpr uint16_t kNumMaxActiveDevices = CHIP_CONFIG_CONTROLLER_MAX_ACTIVE_DEVICES;
constexpr uint16_t kNumMaxPairedDevices = 128;

// Raw functions for cluster callbacks
void OnBasicFailure(void * context, CHIP_ERROR err);

struct ControllerInitParams
{
    PersistentStorageDelegate * storageDelegate = nullptr;
    DeviceControllerSystemState * systemState   = nullptr;
#if CHIP_DEVICE_CONFIG_ENABLE_DNSSD
    DeviceAddressUpdateDelegate * deviceAddressUpdateDelegate = nullptr;
    DeviceDiscoveryDelegate * deviceDiscoveryDelegate         = nullptr;
#endif
    OperationalCredentialsDelegate * operationalCredentialsDelegate = nullptr;

    /* The following keypair must correspond to the public key used for generating
       controllerNOC. It's used by controller to establish CASE sessions with devices */
    Crypto::P256Keypair * operationalKeypair = nullptr;

    /* The following certificates must be in x509 DER format */
    ByteSpan controllerNOC;
    ByteSpan controllerICAC;
    ByteSpan controllerRCAC;

    uint16_t controllerVendorId;
};

class DLL_EXPORT DevicePairingDelegate
{
public:
    virtual ~DevicePairingDelegate() {}

    enum Status : uint8_t
    {
        SecurePairingSuccess = 0,
        SecurePairingFailed,
    };

    /**
     * @brief
     *   Called when the pairing reaches a certain stage.
     *
     * @param status Current status of pairing
     */
    virtual void OnStatusUpdate(DevicePairingDelegate::Status status) {}

    /**
     * @brief
     *   Called when the pairing is complete (with success or error)
     *
     * @param error Error cause, if any
     */
    virtual void OnPairingComplete(CHIP_ERROR error) {}

    /**
     * @brief
     *   Called when the pairing is deleted (with success or error)
     *
     * @param error Error cause, if any
     */
    virtual void OnPairingDeleted(CHIP_ERROR error) {}

    /**
     *   Called when the commissioning process is complete (with success or error)
     */
    virtual void OnCommissioningComplete(NodeId deviceId, CHIP_ERROR error) {}
};

struct CommissionerInitParams : public ControllerInitParams
{
    DevicePairingDelegate * pairingDelegate     = nullptr;
    CommissioningDelegate * defaultCommissioner = nullptr;
};

typedef void (*OnOpenCommissioningWindow)(void * context, NodeId deviceId, CHIP_ERROR status, SetupPayload payload);

/**
 * @brief
 *   Controller applications can use this class to communicate with already paired CHIP devices. The
 *   application is required to provide access to the persistent storage, where the paired device information
 *   is stored. This object of this class can be initialized with the data from the storage (List of devices,
 *   and device pairing information for individual devices). Alternatively, this class can retrieve the
 *   relevant information when the application tries to communicate with the device
 */
class DLL_EXPORT DeviceController : public SessionRecoveryDelegate
#if CHIP_DEVICE_CONFIG_ENABLE_DNSSD
    ,
                                    public AbstractDnssdDiscoveryController
#endif
{
public:
    DeviceController();
    virtual ~DeviceController() {}

    enum class CommissioningWindowOption
    {
        kOriginalSetupCode = 0,
        kTokenWithRandomPIN,
        kTokenWithProvidedPIN,
    };

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
    virtual CHIP_ERROR Shutdown();

    CHIP_ERROR GetPeerAddressAndPort(PeerId peerId, Inet::IPAddress & addr, uint16_t & port);

    /**
     *   This function returns true if the device corresponding to `deviceId` has previously been commissioned
     *   on the fabric.
     */
    bool DoesDevicePairingExist(const PeerId & deviceId);

    /**
     *   This function finds the device corresponding to deviceId, and establishes a secure connection with it.
     *   Once the connection is successfully establishes (or if it's already connected), it calls `onConnectedDevice`
     *   callback. If it fails to establish the connection, it calls `onError` callback.
     */
    virtual CHIP_ERROR GetConnectedDevice(NodeId deviceId, Callback::Callback<OnDeviceConnected> * onConnection,
                                          chip::Callback::Callback<OnDeviceConnectionFailure> * onFailure)
    {
        VerifyOrReturnError(mState == State::Initialized && mFabricInfo != nullptr, CHIP_ERROR_INCORRECT_STATE);
        return mCASESessionManager->FindOrEstablishSession(mFabricInfo->GetPeerIdForNode(deviceId), onConnection, onFailure);
    }

    /**
     * @brief
     *   This function update the device informations asynchronously using dnssd.
     *
     * @param[in] deviceId  Node ID for the CHIP device
     *
     * @return CHIP_ERROR CHIP_NO_ERROR on success, or corresponding error code.
     */
    CHIP_ERROR UpdateDevice(NodeId deviceId)
    {
        VerifyOrReturnError(mState == State::Initialized && mFabricInfo != nullptr, CHIP_ERROR_INCORRECT_STATE);
        return mCASESessionManager->ResolveDeviceAddress(mFabricInfo, deviceId);
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
     * @param[out] outVerifier    The PASEVerifier to be populated on success
     * @param[out] outPasscodeId  The passcode ID to be populated on success
     *
     * @return CHIP_ERROR         CHIP_NO_ERROR on success, or corresponding error
     */
    CHIP_ERROR ComputePASEVerifier(uint32_t iterations, uint32_t setupPincode, const ByteSpan & salt, PASEVerifier & outVerifier,
                                   PasscodeId & outPasscodeId);

    /**
     * @brief
     *   Trigger a paired device to re-enter the commissioning mode. The device will exit the commissioning mode
     *   after a successful commissioning, or after the given `timeout` time.
     *
     * @param[in] deviceId        The device Id.
     * @param[in] timeout         The commissioning mode should terminate after this much time.
     * @param[in] iteration       The PAKE iteration count associated with the PAKE Passcode ID and ephemeral
     *                            PAKE passcode verifier to be used for this commissioning.
     * @param[in] discriminator   The long discriminator for the DNS-SD advertisement.
     * @param[in] option          The commissioning window can be opened using the original setup code, or an
     *                            onboarding token can be generated using a random setup PIN code (or with
     *                            the PIN code provied in the setupPayload).
     * @param[in,out] payload     The generated setup payload.
     *                            - The payload is generated only if the user didn't ask for using the original setup code.
     *                            - If the user asked to use the provided setup PIN, the PIN must be provided as part of
     *                              this payload
     *
     * @return CHIP_ERROR         CHIP_NO_ERROR on success, or corresponding error
     */
    CHIP_ERROR OpenCommissioningWindow(NodeId deviceId, uint16_t timeout, uint32_t iteration, uint16_t discriminator,
                                       uint8_t option, SetupPayload & payload)
    {
        mSuggestedSetUpPINCode = payload.setUpPINCode;
        ReturnErrorOnFailure(OpenCommissioningWindowWithCallback(deviceId, timeout, iteration, discriminator, option, nullptr));
        payload = mSetupPayload;
        return CHIP_NO_ERROR;
    }

    /**
     * @brief
     *   Trigger a paired device to re-enter the commissioning mode. The device will exit the commissioning mode
     *   after a successful commissioning, or after the given `timeout` time.
     *
     * @param[in] deviceId        The device Id.
     * @param[in] timeout         The commissioning mode should terminate after this much time.
     * @param[in] iteration       The PAKE iteration count associated with the PAKE Passcode ID and ephemeral
     *                            PAKE passcode verifier to be used for this commissioning.
     * @param[in] discriminator   The long discriminator for the DNS-SD advertisement.
     * @param[in] option          The commissioning window can be opened using the original setup code, or an
     *                            onboarding token can be generated using a random setup PIN code (or with
     *                            the PIN code provied in the setupPayload).
     * @param[in] callback        The function to be called on success or failure of opening of commissioning window.
     *
     * @param[in] readVIDPIDAttributes Should the API internally read VID and PID from the device while opening the
     *                                 commissioning window. VID and PID is only needed for enchanced commissioning mode.
     *                                 If this argument is `true`, and enhanced commissioning mode is used, the API will
     *                                 read VID and PID from the device.
     *
     * @return CHIP_ERROR         CHIP_NO_ERROR on success, or corresponding error
     */
    CHIP_ERROR OpenCommissioningWindowWithCallback(NodeId deviceId, uint16_t timeout, uint32_t iteration, uint16_t discriminator,
                                                   uint8_t option, Callback::Callback<OnOpenCommissioningWindow> * callback,
                                                   bool readVIDPIDAttributes = false);

#if CHIP_DEVICE_CONFIG_ENABLE_DNSSD
    void RegisterDeviceAddressUpdateDelegate(DeviceAddressUpdateDelegate * delegate) { mDeviceAddressUpdateDelegate = delegate; }
    void RegisterDeviceDiscoveryDelegate(DeviceDiscoveryDelegate * delegate) { mDeviceDiscoveryDelegate = delegate; }
#endif

    /**
     * @brief Get the Compressed Fabric ID assigned to the device.
     */
    uint64_t GetCompressedFabricId() const { return mLocalId.GetCompressedFabricId(); }

    /**
     * @brief Get the raw Fabric ID assigned to the device.
     */
    uint64_t GetFabricId() const { return mFabricId; }

    /**
     * @brief Get the Node ID of this instance.
     */
    NodeId GetNodeId() const { return mLocalId.GetNodeId(); }

    void ReleaseOperationalDevice(NodeId remoteDeviceId);

    OperationalCredentialsDelegate * GetOperationalCredentialsDelegate() { return mOperationalCredentialsDelegate; }

protected:
    enum class State
    {
        NotInitialized,
        Initialized
    };

    State mState;

    CASESessionManager * mCASESessionManager = nullptr;

    Dnssd::DnssdCache<CHIP_CONFIG_MDNS_CACHE_SIZE> mDNSCache;
    CASEClientPool<CHIP_CONFIG_CONTROLLER_MAX_ACTIVE_CASE_CLIENTS> mCASEClientPool;
    OperationalDeviceProxyPool<CHIP_CONFIG_CONTROLLER_MAX_ACTIVE_DEVICES> mDevicePool;

    SerializableU64Set<kNumMaxPairedDevices> mPairedDevices;
    bool mPairedDevicesInitialized;

    PeerId mLocalId          = PeerId();
    FabricId mFabricId       = kUndefinedFabricId;
    FabricInfo * mFabricInfo = nullptr;

    PersistentStorageDelegate * mStorageDelegate = nullptr;
#if CHIP_DEVICE_CONFIG_ENABLE_DNSSD
    DeviceAddressUpdateDelegate * mDeviceAddressUpdateDelegate = nullptr;
    // TODO(cecille): Make this configuarable.
    static constexpr int kMaxCommissionableNodes = 10;
    Dnssd::DiscoveredNodeData mCommissionableNodes[kMaxCommissionableNodes];
#endif
    DeviceControllerSystemState * mSystemState = nullptr;

    CHIP_ERROR InitializePairedDeviceList();
    CHIP_ERROR SetPairedDeviceList(ByteSpan pairedDeviceSerializedSet);
    ControllerDeviceInitParams GetControllerDeviceInitParams();

    void PersistNextKeyId();

    OperationalCredentialsDelegate * mOperationalCredentialsDelegate;

    SessionIDAllocator mIDAllocator;

    uint16_t mVendorId;

    //////////// SessionRecoveryDelegate Implementation ///////////////
    void OnFirstMessageDeliveryFailed(const SessionHandle & session) override;

#if CHIP_DEVICE_CONFIG_ENABLE_DNSSD
    //////////// ResolverDelegate Implementation ///////////////
    void OnNodeIdResolved(const chip::Dnssd::ResolvedNodeData & nodeData) override;
    void OnNodeIdResolutionFailed(const chip::PeerId & peerId, CHIP_ERROR error) override;
    DiscoveredNodeList GetDiscoveredNodes() override { return DiscoveredNodeList(mCommissionableNodes); }
#endif // CHIP_DEVICE_CONFIG_ENABLE_DNSSD

private:
    void ReleaseOperationalDevice(OperationalDeviceProxy * device);

    static void OnPIDReadResponse(void * context, uint16_t value);
    static void OnVIDReadResponse(void * context, VendorId value);
    static void OnVIDPIDReadFailureResponse(void * context, CHIP_ERROR error);

    CHIP_ERROR OpenCommissioningWindowInternal();

    PeerId GetPeerIdWithCommissioningWindowOpen()
    {
        return mFabricInfo ? mFabricInfo->GetPeerIdForNode(mDeviceWithCommissioningWindowOpen) : PeerId();
    }

    // TODO - Support opening commissioning window simultaneously on multiple devices
    Callback::Callback<OnOpenCommissioningWindow> * mCommissioningWindowCallback = nullptr;
    SetupPayload mSetupPayload;
    NodeId mDeviceWithCommissioningWindowOpen;
    uint32_t mSuggestedSetUpPINCode = 0;

    uint16_t mCommissioningWindowTimeout;
    uint32_t mCommissioningWindowIteration;

    CommissioningWindowOption mCommissioningWindowOption;

    static void OnOpenPairingWindowSuccessResponse(void * context, const chip::app::DataModel::NullObjectType &);
    static void OnOpenPairingWindowFailureResponse(void * context, CHIP_ERROR error);

    CHIP_ERROR ProcessControllerNOCChain(const ControllerInitParams & params);
    PasscodeId mPAKEVerifierID = 1;
};

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
                                      public app::AttributeCache::Callback
{
public:
    DeviceCommissioner();
    ~DeviceCommissioner() {}

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
    CHIP_ERROR Shutdown() override;

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
     */
    CHIP_ERROR PairDevice(NodeId remoteDeviceId, const char * setUpCode);

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

    CHIP_ERROR GetDeviceBeingCommissioned(NodeId deviceId, CommissioneeDeviceProxy ** device);

    CHIP_ERROR GetConnectedDevice(NodeId deviceId, chip::Callback::Callback<OnDeviceConnected> * onConnection,
                                  chip::Callback::Callback<OnDeviceConnectionFailure> * onFailure) override;

    /**
     * @brief
     *   This function returns the attestation challenge for the secure session of the device being commissioned.
     *
     * @param[out] attestationChallenge The output for the attestationChallenge
     *
     * @return CHIP_ERROR               CHIP_NO_ERROR on success, or CHIP_ERROR_INVALID_ARGUMENT if no secure session is active
     */
    CHIP_ERROR GetAttestationChallenge(ByteSpan & attestationChallenge);

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
    void OnSessionEstablished() override;

    void RendezvousCleanup(CHIP_ERROR status);

    void PerformCommissioningStep(DeviceProxy * device, CommissioningStage step, CommissioningParameters & params,
                                  CommissioningDelegate * delegate, EndpointId endpoint, Optional<System::Clock::Timeout> timeout);

    /**
     * @brief
     *   This function validates the Attestation Information sent by the device.
     *
     * @param[in] attestationElements Attestation Elements TLV.
     * @param[in] signature           Attestation signature generated for all the above fields + Attestation Challenge.
     * @param[in] attestationNonce    Attestation nonce
     * @param[in] pai                 PAI certificate
     * @param[in] dac                 DAC certificates
     * @param[in] remoteVendorId      vendor ID read from the device Basic Information cluster
     * @param[in] remoteProductId     product ID read from the device Basic Information cluster
     * @param[in] proxy               device proxy that is being attested.
     */
    CHIP_ERROR ValidateAttestationInfo(const ByteSpan & attestationElements, const ByteSpan & signature,
                                       const ByteSpan & attestationNonce, const ByteSpan & pai, const ByteSpan & dac,
                                       VendorId remoteVendorId, uint16_t remoteProductId, DeviceProxy * proxy);

    /**
     * @brief
     * Sends CommissioningStepComplete report to the commissioning delegate. Function will fill in current step.
     * @params[in] err      error from the current step
     * @params[in] report   report to send. Current step will be filled in automatically
     */
    void
    CommissioningStageComplete(CHIP_ERROR err,
                               CommissioningDelegate::CommissioningReport report = CommissioningDelegate::CommissioningReport());

#if CONFIG_NETWORK_LAYER_BLE
    /**
     * @brief
     *   Once we have finished all commissioning work, the Controller should close the BLE
     *   connection to the device and establish CASE session / another PASE session to the device
     *   if needed.
     * @return CHIP_ERROR   The return status
     */
    CHIP_ERROR CloseBleConnection();
#endif
#if CHIP_DEVICE_CONFIG_ENABLE_DNSSD
    /**
     * @brief
     *   Discover all devices advertising as commissionable.
     *   Should be called on main loop thread.
     * * @param[in] filter  Browse filter - controller will look for only the specified subtype.
     * @return CHIP_ERROR   The return status
     */
    CHIP_ERROR DiscoverCommissionableNodes(Dnssd::DiscoveryFilter filter);

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

    void OnNodeIdResolved(const chip::Dnssd::ResolvedNodeData & nodeData) override;
    void OnNodeIdResolutionFailed(const chip::PeerId & peerId, CHIP_ERROR error) override;
#endif
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

#if CHIP_DEVICE_CONFIG_ENABLE_DNSSD
    /**
     * @brief
     *   Overrides method from AbstractDnssdDiscoveryController
     *
     * @param nodeData DNS-SD node information
     *
     */
    void OnNodeDiscoveryComplete(const chip::Dnssd::DiscoveredNodeData & nodeData) override;
#endif

    void RegisterPairingDelegate(DevicePairingDelegate * pairingDelegate) { mPairingDelegate = pairingDelegate; }

    // AttributeCache::Callback impl
    void OnDone() override;

private:
    DevicePairingDelegate * mPairingDelegate;

    CommissioneeDeviceProxy * mDeviceBeingCommissioned = nullptr;

    /* TODO: BLE rendezvous and IP rendezvous should share the same procedure, so this is just a
       workaround-like flag and should be removed in the future.
       When using IP rendezvous, we need to disable network provisioning. In the future, network
       provisioning will no longer be a part of rendezvous procedure. */
    bool mIsIPRendezvous;

    /* This field is true when device pairing information changes, e.g. a new device is paired, or
       the pairing for a device is removed. The DeviceCommissioner uses this to decide when to
       persist the device list */
    bool mPairedDevicesUpdated;

    CommissioningStage mCommissioningStage = CommissioningStage::kSecurePairing;
    bool mRunCommissioningAfterConnection  = false;

    ObjectPool<CommissioneeDeviceProxy, kNumMaxActiveDevices> mCommissioneeDevicePool;

#if CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY // make this commissioner discoverable
    UserDirectedCommissioningServer * mUdcServer = nullptr;
    // mUdcTransportMgr is for insecure communication (ex. user directed commissioning)
    DeviceIPTransportMgr * mUdcTransportMgr = nullptr;
    uint16_t mUdcListenPort                 = CHIP_UDC_PORT;
#endif // CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY

    void SetupCluster(ClusterBase & base, DeviceProxy * proxy, EndpointId endpoint, Optional<System::Clock::Timeout> timeout);

    void FreeRendezvousSession();

    CHIP_ERROR LoadKeyId(PersistentStorageDelegate * delegate, uint16_t & out);

    void OnSessionEstablishmentTimeout();

    static void OnSessionEstablishmentTimeoutCallback(System::Layer * aLayer, void * aAppState);

    /* This function sends a Device Attestation Certificate chain request to the device.
       The function does not hold a reference to the device object.
     */
    CHIP_ERROR SendCertificateChainRequestCommand(DeviceProxy * device, Credentials::CertificateType certificateType);
    /* This function sends an Attestation request to the device.
       The function does not hold a reference to the device object.
     */
    CHIP_ERROR SendAttestationRequestCommand(DeviceProxy * device, const ByteSpan & attestationNonce);
    /* This function sends an CSR request to the device.
       The function does not hold a reference to the device object.
     */
    CHIP_ERROR SendOperationalCertificateSigningRequestCommand(DeviceProxy * device, const ByteSpan & csrNonce);
    /* This function sends the operational credentials to the device.
       The function does not hold a reference to the device object.
     */
    CHIP_ERROR SendOperationalCertificate(DeviceProxy * device, const ByteSpan & nocCertBuf, const ByteSpan & icaCertBuf,
                                          AesCcm128KeySpan ipk, NodeId adminSubject);
    /* This function sends the trusted root certificate to the device.
       The function does not hold a reference to the device object.
     */
    CHIP_ERROR SendTrustedRootCertificate(DeviceProxy * device, const ByteSpan & rcac);

    /* This function is called by the commissioner code when the device completes
       the operational credential provisioning process.
       The function does not hold a reference to the device object.
       */
    CHIP_ERROR OnOperationalCredentialsProvisioningCompletion(CommissioneeDeviceProxy * device);

    /* Callback when the previously sent CSR request results in failure */
    static void OnCSRFailureResponse(void * context, CHIP_ERROR error);

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

    static void OnDeviceConnectedFn(void * context, OperationalDeviceProxy * device);
    static void OnDeviceConnectionFailureFn(void * context, PeerId peerId, CHIP_ERROR error);

    static void OnDeviceAttestationInformationVerification(void * context, Credentials::AttestationVerificationResult result);

    static void OnDeviceNOCChainGeneration(void * context, CHIP_ERROR status, const ByteSpan & noc, const ByteSpan & icac,
                                           const ByteSpan & rcac, Optional<AesCcm128KeySpan> ipk, Optional<NodeId> adminSubject);
    static void OnArmFailSafe(void * context,
                              const chip::app::Clusters::GeneralCommissioning::Commands::ArmFailSafeResponse::DecodableType & data);
    static void OnSetRegulatoryConfigResponse(
        void * context,
        const chip::app::Clusters::GeneralCommissioning::Commands::SetRegulatoryConfigResponse::DecodableType & data);
    static void
    OnNetworkConfigResponse(void * context,
                            const chip::app::Clusters::NetworkCommissioning::Commands::NetworkConfigResponse::DecodableType & data);
    static void OnConnectNetworkResponse(
        void * context, const chip::app::Clusters::NetworkCommissioning::Commands::ConnectNetworkResponse::DecodableType & data);
    static void OnCommissioningCompleteResponse(
        void * context,
        const chip::app::Clusters::GeneralCommissioning::Commands::CommissioningCompleteResponse::DecodableType & data);

    /**
     * @brief
     *   This function processes the CSR sent by the device.
     *   (Reference: Specifications section 11.18.5.6. NOCSR Elements)
     *
     * @param[in] proxy           device proxy
     * @param[in] NOCSRElements   CSR elements as per specifications section 11.22.5.6. NOCSR Elements.
     * @param[in] AttestationSignature       Cryptographic signature generated for all the above fields.
     * @param[in] dac               device attestation certificate
     * @param[in] csrNonce          certificate signing request nonce
     */
    CHIP_ERROR ProcessCSR(DeviceProxy * proxy, const ByteSpan & NOCSRElements, const ByteSpan & AttestationSignature, ByteSpan dac,
                          ByteSpan csrNonce);

    /**
     * @brief
     *   This function processes the DAC or PAI certificate sent by the device.
     */
    CHIP_ERROR ProcessCertificateChain(const ByteSpan & certificate);

    void HandleAttestationResult(CHIP_ERROR err);

    CommissioneeDeviceProxy * FindCommissioneeDevice(const SessionHandle & session);
    CommissioneeDeviceProxy * FindCommissioneeDevice(NodeId id);
    void ReleaseCommissioneeDevice(CommissioneeDeviceProxy * device);

    template <typename ClusterObjectT, typename RequestObjectT>
    CHIP_ERROR SendCommand(DeviceProxy * device, const RequestObjectT & request,
                           CommandResponseSuccessCallback<typename RequestObjectT::ResponseType> successCb,
                           CommandResponseFailureCallback failureCb)
    {
        return SendCommand<ClusterObjectT>(device, request, successCb, failureCb, 0, NullOptional);
    }

    template <typename ClusterObjectT, typename RequestObjectT>
    CHIP_ERROR SendCommand(DeviceProxy * device, const RequestObjectT & request,
                           CommandResponseSuccessCallback<typename RequestObjectT::ResponseType> successCb,
                           CommandResponseFailureCallback failureCb, EndpointId endpoint, Optional<System::Clock::Timeout> timeout)
    {
        ClusterObjectT cluster;
        cluster.Associate(device, endpoint);
        cluster.SetCommandTimeout(timeout);

        return cluster.InvokeCommand(request, this, successCb, failureCb);
    }

    static CHIP_ERROR ConvertFromOperationalCertStatus(chip::app::Clusters::OperationalCredentials::OperationalCertStatus err);

    chip::Callback::Callback<OnDeviceConnected> mOnDeviceConnectedCallback;
    chip::Callback::Callback<OnDeviceConnectionFailure> mOnDeviceConnectionFailureCallback;

    chip::Callback::Callback<Credentials::OnAttestationInformationVerification> mDeviceAttestationInformationVerificationCallback;

    chip::Callback::Callback<OnNOCChainGeneration> mDeviceNOCChainCallback;
    SetUpCodePairer mSetUpCodePairer;
    AutoCommissioner mAutoCommissioner;
    CommissioningDelegate * mDefaultCommissioner =
        nullptr; // Commissioning delegate to call when PairDevice / Commission functions are used
    CommissioningDelegate * mCommissioningDelegate =
        nullptr; // Commissioning delegate that issued the PerformCommissioningStep command

    Platform::UniquePtr<app::AttributeCache> mAttributeCache;
    Platform::UniquePtr<app::ReadClient> mReadClient;
};

} // namespace Controller
} // namespace chip
