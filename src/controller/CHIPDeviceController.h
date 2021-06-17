/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
#include <controller/AbstractMdnsDiscoveryController.h>
#include <controller/CHIPDevice.h>
#include <controller/OperationalCredentialsDelegate.h>
#include <controller/data_model/gen/CHIPClientCallbacks.h>
#include <core/CHIPCore.h>
#include <core/CHIPPersistentStorageDelegate.h>
#include <core/CHIPTLV.h>
#include <credentials/CHIPOperationalCredentials.h>
#include <lib/support/Span.h>
#include <messaging/ExchangeMgr.h>
#include <messaging/ExchangeMgrDelegate.h>
#include <protocols/secure_channel/MessageCounterManager.h>
#include <protocols/secure_channel/RendezvousParameters.h>
#include <support/DLLUtil.h>
#include <support/SerializableIntegerSet.h>
#include <transport/AdminPairingTable.h>
#include <transport/SecureSessionMgr.h>
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
#include <mdns/Resolver.h>
#endif

namespace chip {

namespace Controller {

constexpr uint16_t kNumMaxActiveDevices = 64;
constexpr uint16_t kNumMaxPairedDevices = 128;

// Raw functions for cluster callbacks
typedef void (*BasicSuccessCallback)(void * context, uint16_t val);
typedef void (*BasicFailureCallback)(void * context, uint8_t status);
void BasicSuccess(void * context, uint16_t val);
void BasicFailure(void * context, uint8_t status);

struct ControllerInitParams
{
    PersistentStorageDelegate * storageDelegate = nullptr;
    System::Layer * systemLayer                 = nullptr;
    Inet::InetLayer * inetLayer                 = nullptr;

#if CONFIG_NETWORK_LAYER_BLE
    Ble::BleLayer * bleLayer = nullptr;
#endif
    app::InteractionModelDelegate * imDelegate = nullptr;
#if CHIP_DEVICE_CONFIG_ENABLE_MDNS
    DeviceAddressUpdateDelegate * mDeviceAddressUpdateDelegate = nullptr;
#endif
    OperationalCredentialsDelegate * operationalCredentialsDelegate = nullptr;
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
};

struct CommissionerInitParams : public ControllerInitParams
{
    DevicePairingDelegate * pairingDelegate = nullptr;
};

/**
 * @brief
 * Used for make current OnSuccessCallback & OnFailureCallback works when interaction model landed, it will be removed
 * after #6308 is landed.
 */
class DeviceControllerInteractionModelDelegate : public chip::app::InteractionModelDelegate
{
public:
    CHIP_ERROR CommandResponseStatus(const app::CommandSender * apCommandSender,
                                     const Protocols::SecureChannel::GeneralStatusCode aGeneralCode, const uint32_t aProtocolId,
                                     const uint16_t aProtocolCode, chip::EndpointId aEndpointId, const chip::ClusterId aClusterId,
                                     chip::CommandId aCommandId, uint8_t aCommandIndex) override;

    CHIP_ERROR CommandResponseProtocolError(const app::CommandSender * apCommandSender, uint8_t aCommandIndex) override;

    CHIP_ERROR CommandResponseError(const app::CommandSender * apCommandSender, CHIP_ERROR aError) override;

    CHIP_ERROR CommandResponseProcessed(const app::CommandSender * apCommandSender) override;
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

    CHIP_ERROR Init(NodeId localDeviceId, ControllerInitParams params);

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
     *   This function deserializes the provided deviceInfo object, and initializes and outputs the
     *   corresponding Device object. The lifetime of the output object is tied to that of the DeviceController
     *   object. The caller must not use the Device object If they free the DeviceController object, or
     *   after they call ReleaseDevice() on the returned device object.
     *
     * @param[in] deviceId   Node ID for the CHIP device
     * @param[in] deviceInfo Serialized device info for the device
     * @param[out] device    The output device object
     *
     * @return CHIP_ERROR CHIP_NO_ERROR on success, or corresponding error code.
     */
    CHIP_ERROR GetDevice(NodeId deviceId, const SerializedDevice & deviceInfo, Device ** device);

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
     * @brief
     *   This function update the device informations asynchronously using mdns.
     *   If new device informations has been found, it will be persisted.
     *
     * @param[in] device    The input device object to update
     * @param[in] fabricId  The fabricId used for mdns resolution
     *
     * @return CHIP_ERROR CHIP_NO_ERROR on success, or corresponding error code.
     */
    CHIP_ERROR UpdateDevice(Device * device, uint64_t fabricId);

    void PersistDevice(Device * device);

    CHIP_ERROR SetUdpListenPort(uint16_t listenPort);

    virtual void ReleaseDevice(Device * device);

#if CHIP_DEVICE_CONFIG_ENABLE_MDNS
    void RegisterDeviceAddressUpdateDelegate(DeviceAddressUpdateDelegate * delegate) { mDeviceAddressUpdateDelegate = delegate; }
#endif

    // ----- IO -----
    /**
     * @brief
     * Start the event loop task within the CHIP stack
     * @return CHIP_ERROR   The return status
     */
    CHIP_ERROR ServiceEvents();

    /**
     * @brief
     *   Allow the CHIP Stack to process any pending events
     *   This can be called in an event handler loop to trigger callbacks within the CHIP stack
     * @return CHIP_ERROR   The return status
     */
    CHIP_ERROR ServiceEventSignal();

    /**
     * @brief Get the Fabric ID assigned to the device.
     *
     * @param[out] fabricId   Fabric ID of the device.
     *
     * @return CHIP_ERROR CHIP_NO_ERROR on success, or corresponding error code.
     */
    CHIP_ERROR GetFabricId(uint64_t & fabricId);

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

    NodeId mLocalDeviceId;
    DeviceTransportMgr * mTransportMgr                             = nullptr;
    SecureSessionMgr * mSessionMgr                                 = nullptr;
    Messaging::ExchangeManager * mExchangeMgr                      = nullptr;
    secure_channel::MessageCounterManager * mMessageCounterManager = nullptr;
    PersistentStorageDelegate * mStorageDelegate                   = nullptr;
    DeviceControllerInteractionModelDelegate * mDefaultIMDelegate  = nullptr;
#if CHIP_DEVICE_CONFIG_ENABLE_MDNS
    DeviceAddressUpdateDelegate * mDeviceAddressUpdateDelegate = nullptr;
    // TODO(cecille): Make this configuarable.
    static constexpr int kMaxCommissionableNodes = 10;
    Mdns::DiscoveredNodeData mCommissionableNodes[kMaxCommissionableNodes];
#endif
    Inet::InetLayer * mInetLayer = nullptr;
#if CONFIG_NETWORK_LAYER_BLE
    Ble::BleLayer * mBleLayer = nullptr;
#endif
    System::Layer * mSystemLayer = nullptr;

    uint16_t mListenPort;
    uint16_t GetInactiveDeviceIndex();
    uint16_t FindDeviceIndex(SecureSessionHandle session);
    uint16_t FindDeviceIndex(NodeId id);
    void ReleaseDevice(uint16_t index);
    void ReleaseDeviceById(NodeId remoteDeviceId);
    CHIP_ERROR InitializePairedDeviceList();
    CHIP_ERROR SetPairedDeviceList(ByteSpan pairedDeviceSerializedSet);
    ControllerDeviceInitParams GetControllerDeviceInitParams();

    void PersistNextKeyId();

    Transport::AdminId mAdminId = 0;
    Transport::AdminPairingTable mAdmins;

    OperationalCredentialsDelegate * mOperationalCredentialsDelegate;

    Credentials::ChipCertificateSet mCertificates;
    Credentials::OperationalCredentialSet mCredentials;
    Credentials::CertificateKeyId mRootKeyId;

    uint16_t mNextKeyId = 0;

#if CHIP_DEVICE_CONFIG_ENABLE_MDNS
    //////////// ResolverDelegate Implementation ///////////////
    void OnNodeIdResolved(const chip::Mdns::ResolvedNodeData & nodeData) override;
    void OnNodeIdResolutionFailed(const chip::PeerId & peerId, CHIP_ERROR error) override;
    Mdns::DiscoveredNodeData * GetDiscoveredNodes() override { return mCommissionableNodes; }
#endif // CHIP_DEVICE_CONFIG_ENABLE_MDNS

private:
    //////////// ExchangeDelegate Implementation ///////////////
    void OnMessageReceived(Messaging::ExchangeContext * ec, const PacketHeader & packetHeader, const PayloadHeader & payloadHeader,
                           System::PacketBufferHandle && msgBuf) override;
    void OnResponseTimeout(Messaging::ExchangeContext * ec) override;

    //////////// ExchangeMgrDelegate Implementation ///////////////
    void OnNewConnection(SecureSessionHandle session, Messaging::ExchangeManager * mgr) override;
    void OnConnectionExpired(SecureSessionHandle session, Messaging::ExchangeManager * mgr) override;

    void ReleaseAllDevices();

    CHIP_ERROR LoadLocalCredentials(Transport::AdminPairingInfo * admin);
};

/**
 * @brief
 *   The commissioner applications doesn't advertise itself as an available device for rendezvous
 *   process. This delegate class provides no-op functions for the advertisement delegate.
 */
class DeviceCommissionerRendezvousAdvertisementDelegate : public RendezvousAdvertisementDelegate
{
public:
    /**
     * @brief
     *   Starts advertisement of the device for rendezvous availability.
     */
    CHIP_ERROR StartAdvertisement() const override { return CHIP_NO_ERROR; }

    /**
     * @brief
     *   Stops advertisement of the device for rendezvous availability.
     */
    CHIP_ERROR StopAdvertisement() const override { return CHIP_NO_ERROR; }
};

/**
 * @brief
 *   The commissioner applications can use this class to pair new/unpaired CHIP devices. The application is
 *   required to provide write access to the persistent storage, where the paired device information
 *   will be stored.
 */
class DLL_EXPORT DeviceCommissioner : public DeviceController, public SessionEstablishmentDelegate

{
public:
    DeviceCommissioner();
    ~DeviceCommissioner() {}

    /**
     * Commissioner-specific initialization, includes parameters such as the pairing delegate.
     */
    CHIP_ERROR Init(NodeId localDeviceId, CommissionerInitParams params);

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
     *   Discover devices advertising as commissionable that match the long discriminator.
     * @return CHIP_ERROR   The return status
     */
    CHIP_ERROR DiscoverCommissioningLongDiscriminator(uint16_t long_discriminator);

    /**
     * @brief
     *   Discover all devices advertising as commissionable.
     *   Should be called on main loop thread.
     * @return CHIP_ERROR   The return status
     */
    CHIP_ERROR DiscoverAllCommissioning();

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

    void RegisterPairingDelegate(DevicePairingDelegate * pairingDelegate) { mPairingDelegate = pairingDelegate; }

private:
    DevicePairingDelegate * mPairingDelegate;

    /* This field is an index in mActiveDevices list. The object at this index in the list
       contains the device object that's tracking the state of the device that's being paired.
       If no device is currently being paired, this value will be kNumMaxPairedDevices.  */
    uint16_t mDeviceBeingPaired;

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

    DeviceCommissionerRendezvousAdvertisementDelegate mRendezvousAdvDelegate;

    void PersistDeviceList();

    void FreeRendezvousSession();

    CHIP_ERROR LoadKeyId(PersistentStorageDelegate * delegate, uint16_t & out);

    void OnSessionEstablishmentTimeout();

    static void OnSessionEstablishmentTimeoutCallback(System::Layer * aLayer, void * aAppState, System::Error aError);

    /* This function sends an OpCSR request to the device.
       The function does not hold a refernce to the device object.
     */
    CHIP_ERROR SendOperationalCertificateSigningRequestCommand(Device * device);
    /* This function sends the operational credentials to the device.
       The function does not hold a refernce to the device object.
     */
    CHIP_ERROR SendOperationalCertificate(Device * device, const ByteSpan & opCertBuf);
    /* This function sends the trusted root certificate to the device.
       The function does not hold a refernce to the device object.
     */
    CHIP_ERROR SendTrustedRootCertificate(Device * device);

    /* This function is called by the commissioner code when the device completes
       the operational credential provisioning process.
       The function does not hold a refernce to the device object.
       */
    CHIP_ERROR OnOperationalCredentialsProvisioningCompletion(Device * device);

    /* Callback when the previously sent CSR request results in failure */
    static void OnCSRFailureResponse(void * context, uint8_t status);

    /**
     * @brief
     *   This function is called by the IM layer when the commissioner receives the CSR from the device.
     *   (Reference: Specifications section 11.22.5.8. OpCSR Elements)
     *
     * @param[in] context         The context provided while registering the callback.
     * @param[in] CSR             The Certificate Signing Request.
     * @param[in] CSRNonce        The Nonce sent by us when we requested the CSR.
     * @param[in] VendorReserved1 vendor-specific information that may aid in device commissioning.
     * @param[in] VendorReserved2 vendor-specific information that may aid in device commissioning.
     * @param[in] VendorReserved3 vendor-specific information that may aid in device commissioning.
     * @param[in] Signature       Cryptographic signature generated for the fields in the response message.
     */
    static void OnOperationalCertificateSigningRequest(void * context, ByteSpan CSR, ByteSpan CSRNonce, ByteSpan VendorReserved1,
                                                       ByteSpan VendorReserved2, ByteSpan VendorReserved3, ByteSpan Signature);

    /* Callback when adding operational certs to device results in failure */
    static void OnAddOpCertFailureResponse(void * context, uint8_t status);
    /* Callback when the device confirms that it has added the operational certificates */
    static void OnOperationalCertificateAddResponse(void * context, uint8_t StatusCode, uint64_t FabricIndex, uint8_t * DebugText);

    /* Callback when the device confirms that it has added the root certificate */
    static void OnRootCertSuccessResponse(void * context);
    /* Callback called when adding root cert to device results in failure */
    static void OnRootCertFailureResponse(void * context, uint8_t status);

    /**
     * @brief
     *   This function processes the CSR sent by the device.
     *   (Reference: Specifications section 11.22.5.8. OpCSR Elements)
     *
     * @param[in] CSR             The Certificate Signing Request.
     * @param[in] CSRNonce        The Nonce sent by us when we requested the CSR.
     * @param[in] VendorReserved1 vendor-specific information that may aid in device commissioning.
     * @param[in] VendorReserved2 vendor-specific information that may aid in device commissioning.
     * @param[in] VendorReserved3 vendor-specific information that may aid in device commissioning.
     * @param[in] Signature       Cryptographic signature generated for all the above fields.
     */
    CHIP_ERROR ProcessOpCSR(const ByteSpan & CSR, const ByteSpan & CSRNonce, const ByteSpan & VendorReserved1,
                            const ByteSpan & VendorReserved2, const ByteSpan & VendorReserved3, const ByteSpan & Signature);

    // Cluster callbacks for advancing commissioning flows
    Callback::Callback<BasicSuccessCallback> mSuccess;
    Callback::Callback<BasicFailureCallback> mFailure;

    CommissioningStage GetNextCommissioningStage();

    Callback::Callback<OperationalCredentialsClusterOpCSRResponseCallback> mOpCSRResponseCallback;
    Callback::Callback<OperationalCredentialsClusterOpCertResponseCallback> mOpCertResponseCallback;
    Callback::Callback<DefaultSuccessCallback> mRootCertResponseCallback;
    Callback::Callback<DefaultFailureCallback> mOnCSRFailureCallback;
    Callback::Callback<DefaultFailureCallback> mOnCertFailureCallback;
    Callback::Callback<DefaultFailureCallback> mOnRootCertFailureCallback;

    PASESession mPairingSession;
};

} // namespace Controller
} // namespace chip
