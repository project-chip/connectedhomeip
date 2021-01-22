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

#include <controller/CHIPDevice.h>
#include <controller/CHIPPersistentStorageDelegate.h>
#include <core/CHIPCore.h>
#include <core/CHIPTLV.h>
#include <support/DLLUtil.h>
#include <support/SerializableIntegerSet.h>
#include <transport/RendezvousSession.h>
#include <transport/RendezvousSessionDelegate.h>
#include <transport/SecureSessionMgr.h>
#include <transport/TransportMgr.h>
#include <transport/raw/UDP.h>

namespace chip {

namespace Controller {

constexpr uint16_t kNumMaxActiveDevices = 64;
constexpr uint16_t kNumMaxPairedDevices = 128;

class DLL_EXPORT DevicePairingDelegate
{
public:
    virtual ~DevicePairingDelegate() {}

    /**
     * @brief
     *   Called when the pairing reaches a certain stage.
     *
     * @param status Current status of pairing
     */
    virtual void OnStatusUpdate(RendezvousSessionDelegate::Status status) {}

    /**
     * @brief
     *   Called when the network credentials are needed for the remote device
     *
     * @param callback Callback delegate that provisions the network credentials
     */
    virtual void OnNetworkCredentialsRequested(RendezvousDeviceCredentialsDelegate * callback) = 0;

    /**
     * @brief
     *   Called when the operational credentials are needed for the remote device
     *
     * @param csr Certificate signing request from the device
     * @param csr_length The length of CSR
     * @param callback Callback delegate that provisions the operational credentials
     */
    virtual void OnOperationalCredentialsRequested(const char * csr, size_t csr_length,
                                                   RendezvousDeviceCredentialsDelegate * callback) = 0;

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

/**
 * @brief
 *   Controller applications can use this class to communicate with already paired CHIP devices. The
 *   application is required to provide access to the persistent storage, where the paired device information
 *   is stored. This object of this class can be initialized with the data from the storage (List of devices,
 *   and device pairing information for individual devices). Alternatively, this class can retrieve the
 *   relevant information when the application tries to communicate with the device
 */
class DLL_EXPORT DeviceController : public SecureSessionMgrDelegate, public PersistentStorageResultDelegate
{
public:
    DeviceController();
    virtual ~DeviceController() {}

    /**
     * Init function to be used when there exists a device layer that takes care of initializing
     * System::Layer and InetLayer.
     */
    CHIP_ERROR Init(NodeId localDeviceId, PersistentStorageDelegate * storageDelegate = nullptr,
                    System::Layer * systemLayer = nullptr, Inet::InetLayer * inetLayer = nullptr);

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

    CHIP_ERROR SetUdpListenPort(uint16_t listenPort);

    virtual void ReleaseDevice(Device * device);

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
     *   This can be called in an event handler loop to tigger callbacks within the CHIP stack
     * @return CHIP_ERROR   The return status
     */
    CHIP_ERROR ServiceEventSignal();

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
    DeviceTransportMgr * mTransportMgr;
    SecureSessionMgr * mSessionManager;
    PersistentStorageDelegate * mStorageDelegate;
    Inet::InetLayer * mInetLayer;

    uint16_t mListenPort;
    uint16_t GetInactiveDeviceIndex();
    uint16_t FindDeviceIndex(SecureSessionHandle session);
    uint16_t FindDeviceIndex(NodeId id);
    void ReleaseDevice(uint16_t index);
    CHIP_ERROR SetPairedDeviceList(const char * pairedDeviceSerializedSet);

private:
    //////////// SecureSessionMgrDelegate Implementation ///////////////
    void OnMessageReceived(const PacketHeader & header, const PayloadHeader & payloadHeader, SecureSessionHandle session,
                           System::PacketBufferHandle msgBuf, SecureSessionMgr * mgr) override;

    void OnNewConnection(SecureSessionHandle session, SecureSessionMgr * mgr) override;
    void OnConnectionExpired(SecureSessionHandle session, SecureSessionMgr * mgr) override;

    //////////// PersistentStorageResultDelegate Implementation ///////////////
    void OnValue(const char * key, const char * value) override;
    void OnStatus(const char * key, Operation op, CHIP_ERROR err) override;

    void ReleaseAllDevices();

    System::Layer * mSystemLayer;
};

/**
 * @brief
 *   The commissioner applications can use this class to pair new/unpaired CHIP devices. The application is
 *   required to provide write access to the persistent storage, where the paired device information
 *   will be stored.
 */
class DLL_EXPORT DeviceCommissioner : public DeviceController, public RendezvousSessionDelegate
{
public:
    DeviceCommissioner();
    ~DeviceCommissioner() {}

    /**
     * Init function to be used when there exists a device layer that takes care of initializing
     * System::Layer and InetLayer.
     */
    CHIP_ERROR Init(NodeId localDeviceId, PersistentStorageDelegate * storageDelegate = nullptr,
                    DevicePairingDelegate * pairingDelegate = nullptr, System::Layer * systemLayer = nullptr,
                    Inet::InetLayer * inetLayer = nullptr);

    void SetDevicePairingDelegate(DevicePairingDelegate * pairingDelegate) { mPairingDelegate = pairingDelegate; }

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
     * @param[in] devicePort            [Optional] The CHIP Device's port, defaults to CHIP_PORT
     * @param[in] interfaceId           [Optional] The local inet interface to use to communicate with the device.
     *
     * @return CHIP_ERROR               The connection status
     */
    CHIP_ERROR PairDevice(NodeId remoteDeviceId, RendezvousParameters & params, uint16_t devicePort = CHIP_PORT,
                          Inet::InterfaceId interfaceId = INET_NULL_INTERFACEID);

    [[deprecated("Available until Rendezvous is implemented")]] CHIP_ERROR
    PairTestDeviceWithoutSecurity(NodeId remoteDeviceId, const Inet::IPAddress & deviceAddr, SerializedDevice & serialized,
                                  uint16_t devicePort = CHIP_PORT, Inet::InterfaceId interfaceId = INET_NULL_INTERFACEID);

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

    //////////// RendezvousSessionDelegate Implementation ///////////////
    void OnRendezvousError(CHIP_ERROR err) override;
    void OnRendezvousComplete() override;
    void OnRendezvousStatusUpdate(RendezvousSessionDelegate::Status status, CHIP_ERROR err) override;

    void RendezvousCleanup(CHIP_ERROR status);

    void ReleaseDevice(Device * device) override;

private:
    DevicePairingDelegate * mPairingDelegate;
    RendezvousSession * mRendezvousSession;

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

    void PersistDeviceList();
};

} // namespace Controller
} // namespace chip
