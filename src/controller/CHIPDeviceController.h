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
#include <transport/raw/UDP.h>

namespace chip {

namespace Controller {

constexpr uint16_t mNumMaxActiveDevices = 64;
constexpr uint16_t mNumMaxPairedDevices = 128;

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

    CHIP_ERROR SetPairedDeviceList(const char * pairedDeviceSerializedSet);

    virtual CHIP_ERROR Shutdown();

    CHIP_ERROR GetDevice(NodeId deviceId, const SerializedDevice & deviceInfo, Device ** device);

    CHIP_ERROR GetDevice(NodeId deviceId, Device ** device);

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
    enum
    {
        kState_NotInitialized = 0,
        kState_Initialized    = 1
    } mState;

    Device mActiveDevices[mNumMaxActiveDevices];

    SerializableU64Set<mNumMaxPairedDevices> mPairedDevices;
    bool mPairedDevicesInitialized;

    NodeId mLocalDeviceId;
    SecureSessionMgr<Transport::UDP> * mSessionManager;
    PersistentStorageDelegate * mStorageDelegate;

    uint16_t GetAvailableDevice();
    uint16_t FindDevice(NodeId id);
    void ReleaseDevice(uint16_t index);

private:
    //////////// SecureSessionMgrDelegate Implementation ///////////////
    void OnMessageReceived(const PacketHeader & header, const PayloadHeader & payloadHeader, Transport::PeerConnectionState * state,
                           System::PacketBuffer * msgBuf, SecureSessionMgrBase * mgr) override;

    void OnNewConnection(Transport::PeerConnectionState * state, SecureSessionMgrBase * mgr) override;

    //////////// PersistentStorageResultDelegate Implementation ///////////////
    void OnValue(const char * key, const char * value) override;
    void OnStatus(const char * key, Operation op, CHIP_ERROR err) override;

    System::Layer * mSystemLayer;
    Inet::InetLayer * mInetLayer;
};

class DLL_EXPORT DeviceCommissioner : public DeviceController, public RendezvousSessionDelegate
{
public:
    DeviceCommissioner();
    ~DeviceCommissioner();

    /**
     * Init function to be used when there exists a device layer that takes care of initializing
     * System::Layer and InetLayer.
     */
    CHIP_ERROR Init(NodeId localDeviceId, PersistentStorageDelegate * storageDelegate = nullptr,
                    DevicePairingDelegate * pairingDelegate = nullptr, System::Layer * systemLayer = nullptr,
                    Inet::InetLayer * inetLayer = nullptr);

    CHIP_ERROR SetDevicePairingDelegate(DevicePairingDelegate * pairingDelegate);

    CHIP_ERROR Shutdown() override;

    // ----- Connection Management -----
    /**
     * @brief
     *   Pair a CHIP device with the provided Rendezvous connection parameters.
     *   Use registered DevicePairingDelegate object to receive notifications on
     *   pairing status updates.
     *
     * @param[in] remoteDeviceId        The remote device Id.
     * @param[in] params                The Rendezvous connection parameters
     * @param[in] appReqState           Application specific context to be passed back when a message is received or on error
     * @param[in] devicePort            [Optional] The CHIP Device's port, defaults to CHIP_PORT
     * @param[in] interfaceId           [Optional] The interface indicator to use
     *
     * @return CHIP_ERROR               The connection status
     */
    CHIP_ERROR PairDevice(NodeId remoteDeviceId, RendezvousParameters & params, void * appReqState, uint16_t devicePort = CHIP_PORT,
                          Inet::InterfaceId interfaceId = INET_NULL_INTERFACEID);

    [[deprecated("Available until Rendezvous is implemented")]] CHIP_ERROR
    PairTestDeviceWithoutSecurity(NodeId remoteDeviceId, const Inet::IPAddress & deviceAddr, SerializedDevice & serialized,
                                  void * appReqState, uint16_t devicePort = CHIP_PORT,
                                  Inet::InterfaceId interfaceId = INET_NULL_INTERFACEID);

    CHIP_ERROR StopPairing(NodeId remoteDeviceId);

    /**
     * @brief
     *   Remove pairing for a paired device.
     *
     * @param[in] remoteDeviceId        The remote device Id.
     *
     * @return CHIP_ERROR               The connection status
     */
    CHIP_ERROR UnpairDevice(NodeId remoteDeviceId);

    //////////// RendezvousSessionDelegate Implementation ///////////////
    void OnRendezvousError(CHIP_ERROR err) override;
    void OnRendezvousComplete() override;
    void OnRendezvousStatusUpdate(RendezvousSessionDelegate::Status status, CHIP_ERROR err) override;

    void ReleaseDevice(Device * device) override;

private:
    // SecurePairingUsingTestSecret * mTestSecurePairingSecret;

    DevicePairingDelegate * mPairingDelegate;
    RendezvousSession * mRendezvousSession;

    uint16_t mDeviceBeingPaired;

    bool mPairedDevicesUpdated;

    void PersistDeviceList();
};

} // namespace Controller
} // namespace chip
