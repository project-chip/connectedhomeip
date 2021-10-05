/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
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

#include <app/InteractionModelDelegate.h>
#include <controller-clusters/zap-generated/CHIPClientCallbacks.h>
#include <controller/AbstractMdnsDiscoveryController.h>
#include <controller/CHIPDevice.h>
#include <controller/CHIPDeviceControllerSystemState.h>
#include <controller/DeviceControllerInteractionModelDelegate.h>
#include <controller/OperationalCredentialsDelegate.h>
#include <controller/SetUpCodePairer.h>
#include <credentials/DeviceAttestationVerifier.h>
#include <lib/core/CHIPConfig.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/CHIPPersistentStorageDelegate.h>
#include <lib/core/CHIPTLV.h>
#include <lib/support/DLLUtil.h>
#include <lib/support/SerializableIntegerSet.h>
#include <lib/support/Span.h>
#include <messaging/ExchangeMgr.h>
#include <messaging/ExchangeMgrDelegate.h>
#include <protocols/secure_channel/MessageCounterManager.h>
#include <protocols/secure_channel/RendezvousParameters.h>
#include <protocols/user_directed_commissioning/UserDirectedCommissioning.h>
#include <transport/FabricTable.h>
#include <transport/SessionManager.h>
#include <transport/TransportMgr.h>
#include <transport/raw/UDP.h>

#if CONFIG_DEVICE_LAYER
#include <platform/CHIPDeviceLayer.h>
#endif

#if CONFIG_NETWORK_LAYER_BLE
#include <ble/BleLayer.h>
#endif
#if CHIP_DEVICE_CONFIG_ENABLE_MDNS
#include <controller/DeviceAddressUpdateDelegate.h>
#include <controller/DeviceDiscoveryDelegate.h>
#include <lib/mdns/Resolver.h>
#endif

namespace chip {

namespace Controller {

using namespace chip::Protocols::UserDirectedCommissioning;

constexpr uint16_t kNumMaxActiveDevices = CHIP_CONFIG_CONTROLLER_MAX_ACTIVE_DEVICES;
constexpr uint16_t kNumMaxPairedDevices = 128;

// Raw functions for cluster callbacks
typedef void (*BasicSuccessCallback)(void * context, uint16_t val);
typedef void (*BasicFailureCallback)(void * context, uint8_t status);
void BasicSuccess(void * context, uint16_t val);
void BasicFailure(void * context, uint8_t status);

struct ControllerInitParams
{
    PersistentStorageDelegate * storageDelegate = nullptr;
    DeviceControllerSystemState * systemState   = nullptr;
#if CHIP_DEVICE_CONFIG_ENABLE_MDNS
    DeviceAddressUpdateDelegate * deviceAddressUpdateDelegate = nullptr;
    DeviceDiscoveryDelegate * deviceDiscoveryDelegate         = nullptr;
#endif
    OperationalCredentialsDelegate * operationalCredentialsDelegate = nullptr;

    /* The following keypair must correspond to the public key used for generating
       controllerNOC. It's used by controller to establish CASE sessions with devices */
    Crypto::P256Keypair * ephemeralKeypair = nullptr;

    /* The following certificates must be in x509 DER format */
    ByteSpan controllerNOC;
    ByteSpan controllerICAC;
    ByteSpan controllerRCAC;

    uint16_t controllerVendorId;

    FabricId fabricId = kUndefinedFabricId;
};

enum CommissioningStage : uint8_t
{
    kError,
    kSecurePairing,
    kArmFailsafe,
    // kConfigTime,  // NOT YET IMPLEMENTED
    // kConfigTimeZone,  // NOT YET IMPLEMENTED
    // kConfigDST,  // NOT YET IMPLEMENTED
    kConfigRegulatory,
    kDeviceAttestation,
    kCheckCertificates,
    kConfigACL,
    kNetworkSetup,
    kScanNetworks, // optional stage if network setup fails (not yet implemented)
    kNetworkEnable,
    kFindOperational,
    kSendComplete,
    kCleanup,
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
    DevicePairingDelegate * pairingDelegate = nullptr;
};

/**
 * @brief
 *   Controller applications can use this class to communicate with already paired CHIP devices. The
 *   application is required to provide access to the persistent storage, where the paired device information
 *   is stored. This object of this class can be initialized with the data from the storage (List of devices,
 *   and device pairing information for individual devices). Alternatively, this class can retrieve the
 *   relevant information when the application tries to communicate with the device
 */
class DLL_EXPORT DeviceController : public Messaging::ExchangeDelegate,
                                    public Messaging::ExchangeMgrDelegate,
#if CHIP_DEVICE_CONFIG_ENABLE_MDNS
                                    public AbstractMdnsDiscoveryController,
#endif
                                    public app::InteractionModelDelegate
{
public:
    DeviceController();
    virtual ~DeviceController() {}

    CHIP_ERROR Init(ControllerInitParams params);

    /**
     * @brief
     *  Tears down the entirety of the stack, including destructing key objects in the system.
     *  This expects to be called with external thread synchronization, and will not internally
     *  grab the CHIP stack lock.
     *
     *  This will also not stop the CHIP event queue / thread (if one exists).  Consumers are expected to
     *  ensure this happend before calling this method.
     */
    virtual CHIP_ERROR Shutdown();

    /**
     * @brief
     *   This function is similar to the other GetDevice object, except it reads the serialized object from
     *   the persistent storage.
     *
     * @param[in] deviceId   Node ID for the CHIP device
     * @param[out] device    The output device object
     *
     * @return CHIP_ERROR CHIP_NO_ERROR on success, or corresponding error code.
     */
    CHIP_ERROR GetDevice(NodeId deviceId, Device ** device);

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
    CHIP_ERROR GetConnectedDevice(NodeId deviceId, Callback::Callback<OnDeviceConnected> * onConnection,
                                  Callback::Callback<OnDeviceConnectionFailure> * onFailure);

    /**
     * @brief
     *   This function update the device informations asynchronously using mdns.
     *   If new device informations has been found, it will be persisted.
     *
     * @param[in] deviceId  Node ID for the CHIP device
     *
     * @return CHIP_ERROR CHIP_NO_ERROR on success, or corresponding error code.
     */
    CHIP_ERROR UpdateDevice(NodeId deviceId);

    void PersistDevice(Device * device);

    virtual void ReleaseDevice(Device * device);

#if CHIP_DEVICE_CONFIG_ENABLE_MDNS
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

    DeviceControllerInteractionModelDelegate * GetInteractionModelDelegate()
    {
        if (mSystemState != nullptr)
        {
            return mSystemState->IMDelegate();
        }
        return nullptr;
    }

protected:
    enum class State
    {
        NotInitialized,
        Initialized
    };

    State mState;

    /* A list of device objects that can be used for communicating with corresponding
       CHIP devices. The list does not contain all the paired devices, but only the ones
       which the controller application is currently accessing.
    */
    Device mActiveDevices[kNumMaxActiveDevices];

    SerializableU64Set<kNumMaxPairedDevices> mPairedDevices;
    bool mPairedDevicesInitialized;

    PeerId mLocalId    = PeerId();
    FabricId mFabricId = kUndefinedFabricId;

    PersistentStorageDelegate * mStorageDelegate = nullptr;
#if CHIP_DEVICE_CONFIG_ENABLE_MDNS
    DeviceAddressUpdateDelegate * mDeviceAddressUpdateDelegate = nullptr;
    // TODO(cecille): Make this configuarable.
    static constexpr int kMaxCommissionableNodes = 10;
    Mdns::DiscoveredNodeData mCommissionableNodes[kMaxCommissionableNodes];
#endif
    DeviceControllerSystemState * mSystemState = nullptr;

    uint16_t GetInactiveDeviceIndex();
    uint16_t FindDeviceIndex(SessionHandle session);
    uint16_t FindDeviceIndex(NodeId id);
    void ReleaseDevice(uint16_t index);
    void ReleaseDeviceById(NodeId remoteDeviceId);
    CHIP_ERROR InitializePairedDeviceList();
    CHIP_ERROR SetPairedDeviceList(ByteSpan pairedDeviceSerializedSet);
    ControllerDeviceInitParams GetControllerDeviceInitParams();

    void PersistNextKeyId();

    FabricIndex mFabricIndex = kMinValidFabricIndex;

    OperationalCredentialsDelegate * mOperationalCredentialsDelegate;

    SessionIDAllocator mIDAllocator;

    uint16_t mVendorId;

#if CHIP_DEVICE_CONFIG_ENABLE_MDNS
    //////////// ResolverDelegate Implementation ///////////////
    void OnNodeIdResolved(const chip::Mdns::ResolvedNodeData & nodeData) override;
    void OnNodeIdResolutionFailed(const chip::PeerId & peerId, CHIP_ERROR error) override;
    DiscoveredNodeList GetDiscoveredNodes() override { return DiscoveredNodeList(mCommissionableNodes); }
#endif // CHIP_DEVICE_CONFIG_ENABLE_MDNS

private:
    //////////// ExchangeDelegate Implementation ///////////////
    CHIP_ERROR OnMessageReceived(Messaging::ExchangeContext * ec, const PayloadHeader & payloadHeader,
                                 System::PacketBufferHandle && msgBuf) override;
    void OnResponseTimeout(Messaging::ExchangeContext * ec) override;

    //////////// ExchangeMgrDelegate Implementation ///////////////
    void OnNewConnection(SessionHandle session, Messaging::ExchangeManager * mgr) override;
    void OnConnectionExpired(SessionHandle session, Messaging::ExchangeManager * mgr) override;

    void ReleaseAllDevices();

    CHIP_ERROR ProcessControllerNOCChain(const ControllerInitParams & params);
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
                                      public Protocols::UserDirectedCommissioning::UserConfirmationProvider,
#endif
                                      public SessionEstablishmentDelegate

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
     * @param[in] params                The Rendezvous connection parameters
     */
    CHIP_ERROR PairDevice(NodeId remoteDeviceId, RendezvousParameters & params);

    [[deprecated("Available until Rendezvous is implemented")]] CHIP_ERROR
    PairTestDeviceWithoutSecurity(NodeId remoteDeviceId, const Transport::PeerAddress & peerAddress, SerializedDevice & serialized);

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

    /**
     *   This function call indicates that the device has been provisioned with operational
     *   credentials, and is reachable on operational network. At this point, the device is
     *   available for CASE session establishment.
     *
     *   The function updates the state of device proxy object such that all subsequent messages
     *   will use secure session established via CASE handshake.
     */
    CHIP_ERROR OperationalDiscoveryComplete(NodeId remoteDeviceId);

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
     *
     * @return CHIP_ERROR         CHIP_NO_ERROR on success, or corresponding error
     */
    CHIP_ERROR OpenCommissioningWindow(NodeId deviceId, uint16_t timeout, uint16_t iteration, uint16_t discriminator,
                                       uint8_t option);

    //////////// SessionEstablishmentDelegate Implementation ///////////////
    void OnSessionEstablishmentError(CHIP_ERROR error) override;
    void OnSessionEstablished() override;

    void RendezvousCleanup(CHIP_ERROR status);

    void ReleaseDevice(Device * device) override;

    void AdvanceCommissioningStage(CHIP_ERROR err);

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
#if CHIP_DEVICE_CONFIG_ENABLE_MDNS
    /**
     * @brief
     *   Discover all devices advertising as commissionable.
     *   Should be called on main loop thread.
     * * @param[in] filter  Browse filter - controller will look for only the specified subtype.
     * @return CHIP_ERROR   The return status
     */
    CHIP_ERROR DiscoverCommissionableNodes(Mdns::DiscoveryFilter filter);

    /**
     * @brief
     *   Returns information about discovered devices.
     *   Should be called on main loop thread.
     * @return const DiscoveredNodeData* info about the selected device. May be nullptr if no information has been returned yet.
     */
    const Mdns::DiscoveredNodeData * GetDiscoveredDevice(int idx);

    /**
     * @brief
     *   Returns the max number of commissionable nodes this commissioner can track mdns information for.
     * @return int  The max number of commissionable nodes supported
     */
    int GetMaxCommissionableNodesSupported() { return kMaxCommissionableNodes; }

    void OnNodeIdResolved(const chip::Mdns::ResolvedNodeData & nodeData) override;
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
     *   Called when a UDC message has been received and corresponding nodeData has been found.
     * It is expected that the implementer will prompt the user to confirm their intention to
     * commission the given node, and provide the setup code to allow commissioning to proceed.
     *
     * @param nodeData DNS-SD node information for the client requesting commissioning
     *
     */
    void OnUserDirectedCommissioningRequest(const Mdns::DiscoveredNodeData & nodeData) override;

    /**
     * @brief
     *   Overrides method from AbstractMdnsDiscoveryController
     *
     * @param nodeData DNS-SD node information
     *
     */
    void OnNodeDiscoveryComplete(const chip::Mdns::DiscoveredNodeData & nodeData) override;

    /**
     * @brief
     *   Return the UDC Server instance
     *
     */
    UserDirectedCommissioningServer * GetUserDirectedCommissioningServer() { return mUdcServer; }
#endif // CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY

    void RegisterPairingDelegate(DevicePairingDelegate * pairingDelegate) { mPairingDelegate = pairingDelegate; }

private:
    DevicePairingDelegate * mPairingDelegate;

    /* This field is an index in mActiveDevices list. The object at this index in the list
       contains the device object that's tracking the state of the device that's being paired.
       If no device is currently being paired, this value will be kNumMaxPairedDevices.  */
    uint16_t mDeviceBeingPaired;

    Credentials::CertificateType mCertificateTypeBeingRequested = Credentials::CertificateType::kUnknown;

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

#if CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY // make this commissioner discoverable
    UserDirectedCommissioningServer * mUdcServer = nullptr;
    // mUdcTransportMgr is for insecure communication (ex. user directed commissioning)
    DeviceTransportMgr * mUdcTransportMgr = nullptr;
    uint16_t mUdcListenPort               = CHIP_UDC_PORT;
#endif // CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY

    void PersistDeviceList();

    void FreeRendezvousSession();

    CHIP_ERROR LoadKeyId(PersistentStorageDelegate * delegate, uint16_t & out);

    void OnSessionEstablishmentTimeout();

    static void OnSessionEstablishmentTimeoutCallback(System::Layer * aLayer, void * aAppState);

    /* This function sends a Device Attestation Certificate chain request to the device.
       The function does not hold a reference to the device object.
     */
    CHIP_ERROR SendCertificateChainRequestCommand(Device * device, Credentials::CertificateType certificateType);
    /* This function sends an Attestation request to the device.
       The function does not hold a reference to the device object.
     */
    CHIP_ERROR SendAttestationRequestCommand(Device * device, const ByteSpan & attestationNonce);
    /* This function sends an OpCSR request to the device.
       The function does not hold a refernce to the device object.
     */
    CHIP_ERROR SendOperationalCertificateSigningRequestCommand(Device * device);
    /* This function sends the operational credentials to the device.
       The function does not hold a refernce to the device object.
     */
    CHIP_ERROR SendOperationalCertificate(Device * device, const ByteSpan & nocCertBuf, const ByteSpan & icaCertBuf);
    /* This function sends the trusted root certificate to the device.
       The function does not hold a refernce to the device object.
     */
    CHIP_ERROR SendTrustedRootCertificate(Device * device, const ByteSpan & rcac);

    /* This function is called by the commissioner code when the device completes
       the operational credential provisioning process.
       The function does not hold a refernce to the device object.
       */
    CHIP_ERROR OnOperationalCredentialsProvisioningCompletion(Device * device);

    /* Callback when the previously sent CSR request results in failure */
    static void OnCSRFailureResponse(void * context, uint8_t status);

    static void OnCertificateChainFailureResponse(void * context, uint8_t status);
    static void OnCertificateChainResponse(void * context, ByteSpan certificate);

    static void OnAttestationFailureResponse(void * context, uint8_t status);
    static void OnAttestationResponse(void * context, chip::ByteSpan attestationElements, chip::ByteSpan signature);

    /**
     * @brief
     *   This function is called by the IM layer when the commissioner receives the CSR from the device.
     *   (Reference: Specifications section 11.22.5.8. OpCSR Elements)
     *
     * @param[in] context               The context provided while registering the callback.
     * @param[in] NOCSRElements         CSR elements as per specifications section 11.22.5.6. NOCSR Elements.
     * @param[in] AttestationSignature  Cryptographic signature generated for the fields in the response message.
     */
    static void OnOperationalCertificateSigningRequest(void * context, ByteSpan NOCSRElements, ByteSpan AttestationSignature);

    /* Callback when adding operational certs to device results in failure */
    static void OnAddNOCFailureResponse(void * context, uint8_t status);
    /* Callback when the device confirms that it has added the operational certificates */
    static void OnOperationalCertificateAddResponse(void * context, uint8_t StatusCode, uint8_t FabricIndex, ByteSpan DebugText);

    /* Callback when the device confirms that it has added the root certificate */
    static void OnRootCertSuccessResponse(void * context);
    /* Callback called when adding root cert to device results in failure */
    static void OnRootCertFailureResponse(void * context, uint8_t status);

    static void OnDeviceConnectedFn(void * context, Device * device);
    static void OnDeviceConnectionFailureFn(void * context, NodeId deviceId, CHIP_ERROR error);

    static void OnDeviceNOCChainGeneration(void * context, CHIP_ERROR status, const ByteSpan & noc, const ByteSpan & icac,
                                           const ByteSpan & rcac);

    /**
     * @brief
     *   This function processes the CSR sent by the device.
     *   (Reference: Specifications section 11.22.5.8. OpCSR Elements)
     *
     * @param[in] NOCSRElements   CSR elements as per specifications section 11.22.5.6. NOCSR Elements.
     * @param[in] AttestationSignature       Cryptographic signature generated for all the above fields.
     */
    CHIP_ERROR ProcessOpCSR(const ByteSpan & NOCSRElements, const ByteSpan & AttestationSignature);

    /**
     * @brief
     *   This function processes the DAC or PAI certificate sent by the device.
     */
    CHIP_ERROR ProcessCertificateChain(const ByteSpan & certificate);

    /**
     * @brief
     *   This function validates the Attestation Information sent by the device.
     *
     * @param[in] attestationElements Attestation Elements TLV.
     * @param[in] signature           Attestation signature generated for all the above fields + Attestation Challenge.
     */
    CHIP_ERROR ValidateAttestationInfo(const ByteSpan & attestationElements, const ByteSpan & signature);

    void HandleAttestationResult(CHIP_ERROR err);

    // Cluster callbacks for advancing commissioning flows
    Callback::Callback<BasicSuccessCallback> mSuccess;
    Callback::Callback<BasicFailureCallback> mFailure;

    CommissioningStage GetNextCommissioningStage();
    static CHIP_ERROR ConvertFromNodeOperationalCertStatus(uint8_t err);

    Callback::Callback<OperationalCredentialsClusterCertificateChainResponseCallback> mCertificateChainResponseCallback;
    Callback::Callback<OperationalCredentialsClusterAttestationResponseCallback> mAttestationResponseCallback;
    Callback::Callback<OperationalCredentialsClusterOpCSRResponseCallback> mOpCSRResponseCallback;
    Callback::Callback<OperationalCredentialsClusterNOCResponseCallback> mNOCResponseCallback;
    Callback::Callback<DefaultSuccessCallback> mRootCertResponseCallback;
    Callback::Callback<DefaultFailureCallback> mOnCertificateChainFailureCallback;
    Callback::Callback<DefaultFailureCallback> mOnAttestationFailureCallback;
    Callback::Callback<DefaultFailureCallback> mOnCSRFailureCallback;
    Callback::Callback<DefaultFailureCallback> mOnCertFailureCallback;
    Callback::Callback<DefaultFailureCallback> mOnRootCertFailureCallback;

    Callback::Callback<OnDeviceConnected> mOnDeviceConnectedCallback;
    Callback::Callback<OnDeviceConnectionFailure> mOnDeviceConnectionFailureCallback;

    Callback::Callback<OnNOCChainGeneration> mDeviceNOCChainCallback;
    SetUpCodePairer mSetUpCodePairer;

    PASESession mPairingSession;
};

} // namespace Controller
} // namespace chip
