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
#include <messaging/ExchangeMgr.h>
#include <support/DLLUtil.h>
#include <support/SerializableIntegerSet.h>
#include <transport/SecureSessionMgr.h>
#include <transport/TransportMgr.h>
#include <transport/raw/UDP.h>

namespace chip {

namespace Controller {

constexpr uint16_t kNumMaxActiveDevices = 64;
constexpr uint16_t kNumMaxPairedDevices = 128;

/**
 * @brief
 *   Controller applications can use this class to communicate with already paired CHIP devices. The
 *   application is required to provide access to the persistent storage, where the paired device information
 *   is stored. This object of this class can be initialized with the data from the storage (List of devices,
 *   and device pairing information for individual devices). Alternatively, this class can retrieve the
 *   relevant information when the application tries to communicate with the device
 */
class DLL_EXPORT DeviceController : public SecureSessionMgrDelegate, public PersistentStorageResultDelegate, public TransportMgrDelegate
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

    NodeId GetLocalNodeId() { return mLocalDeviceId; }

    /**
     * @brief
     *   This function creates a new device in NotConnected state. The caller must not use the Device object If they
     *   free the DeviceController object, or after they call ReleaseDevice() on the returned device object.
     *
     * @param[in] deviceId   Node ID for the CHIP device
     * @param[out] device    The output device object
     *
     * @return CHIP_ERROR CHIP_NO_ERROR on success, or corresponding error code.
     */
    CHIP_ERROR NewDevice(Device ** out_device, NodeId deviceId);

    /**
     * @brief
     *   This function reads the serialized object from the persistent storage and outputs the corresponding Device
     *   object. The lifetime of the output object is tied to that of the DeviceController object. The caller must not
     *   use the Device object If they free the DeviceController object, or after they call ReleaseDevice() on the
     *   returned device object.
     *
     * @param[in] deviceId   Node ID for the CHIP device
     * @param[out] device    The output device object
     *
     * @return CHIP_ERROR CHIP_NO_ERROR on success, or corresponding error code.
     */
    CHIP_ERROR GetDevice(NodeId deviceId, Device ** device);

    /**
     * @brief
     *   This function serialize the given device, and writes the serialized object to the persistent storage.
     *
     * @param[in]  device    The output device object
     *
     * @return CHIP_ERROR CHIP_NO_ERROR on success, or corresponding error code.
     */
    CHIP_ERROR PersistDevice(Device * device);

    /**
     * @brief
     *   Remove pairing for a paired device from the persistent storage.
     *
     * @param[in] remoteDeviceId        The remote device Id.
     *
     * @return CHIP_ERROR               CHIP_NO_ERROR on success, or corresponding error
     */
    CHIP_ERROR UnpairDevice(NodeId remoteDeviceId);

    CHIP_ERROR SetUdpListenPort(uint16_t listenPort);

    virtual void ReleaseDevice(Device * device);

    uint16_t GetNextKeyId() { return mNextKeyId++; }

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
    Messaging::ExchangeManager * mExchangeManager;
    PersistentStorageDelegate * mStorageDelegate;
    Inet::InetLayer * mInetLayer;

    uint16_t mListenPort;
    uint16_t GetInactiveDeviceIndex();
    uint16_t FindDeviceIndex(SecureSessionHandle session);
    uint16_t FindDeviceIndex(NodeId id);
    void ReleaseDevice(uint16_t index);
    CHIP_ERROR SetPairedDeviceList(const char * pairedDeviceSerializedSet);
    void PersistDeviceList();

private:
    //////////// TransportMgrDelegate implementation ///////////////
    void OnMessageReceived(const PacketHeader & header, const Transport::PeerAddress & source, System::PacketBufferHandle msgBuf) override;

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
    uint16_t mNextKeyId                         = 0;
};

} // namespace Controller
} // namespace chip
