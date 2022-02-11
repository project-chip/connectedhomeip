/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
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
 *  @file
 *    This file contains definitions for DeviceProxy for a device that's undergoing
 *    commissioning process. The objects of this will be used by Controller applications
 *    to interact with the device. The class provides mechanism to construct, send and receive
 *    messages to and from the corresponding CHIP devices.
 */

#pragma once

#include <app/CommandSender.h>
#include <app/DeviceProxy.h>
#include <app/util/attribute-filter.h>
#include <app/util/basic-types.h>
#include <controller/CHIPDeviceControllerSystemState.h>
#include <controller/OperationalCredentialsDelegate.h>
#include <lib/core/CHIPCallback.h>
#include <lib/core/CHIPCore.h>
#include <lib/support/DLLUtil.h>
#include <messaging/ExchangeContext.h>
#include <messaging/ExchangeMgr.h>
#include <messaging/Flags.h>
#include <protocols/secure_channel/PASESession.h>
#include <protocols/secure_channel/SessionIDAllocator.h>
#include <transport/SessionHolder.h>
#include <transport/SessionManager.h>
#include <transport/TransportMgr.h>
#include <transport/raw/MessageHeader.h>
#include <transport/raw/UDP.h>

#if CONFIG_NETWORK_LAYER_BLE
#include <ble/BleLayer.h>
#include <transport/raw/BLE.h>
#endif

namespace chip {

constexpr size_t kAttestationNonceLength = 32;

using DeviceIPTransportMgr = TransportMgr<Transport::UDP /* IPv6 */
#if INET_CONFIG_ENABLE_IPV4
                                          ,
                                          Transport::UDP /* IPv4 */
#endif
                                          >;

struct ControllerDeviceInitParams
{
    DeviceTransportMgr * transportMgr                             = nullptr;
    SessionManager * sessionManager                               = nullptr;
    Messaging::ExchangeManager * exchangeMgr                      = nullptr;
    Inet::EndPointManager<Inet::UDPEndPoint> * udpEndPointManager = nullptr;
    PersistentStorageDelegate * storageDelegate                   = nullptr;
    SessionIDAllocator * idAllocator                              = nullptr;
#if CONFIG_NETWORK_LAYER_BLE
    Ble::BleLayer * bleLayer = nullptr;
#endif
    FabricTable * fabricsTable = nullptr;
};

class CommissioneeDeviceProxy : public DeviceProxy, public SessionReleaseDelegate
{
public:
    ~CommissioneeDeviceProxy();
    CommissioneeDeviceProxy() : mSecureSession(*this) {}
    CommissioneeDeviceProxy(const CommissioneeDeviceProxy &) = delete;

    /**
     * @brief
     *   Send the command in internal command sender.
     */
    CHIP_ERROR SendCommands(app::CommandSender * commandObj, Optional<System::Clock::Timeout> timeout) override;

    /**
     * @brief Get the IP address and port assigned to the device.
     *
     * @param[out] addr   IP address of the device.
     * @param[out] port   Port number of the device.
     *
     * @return true, if the IP address and port were filled in the out parameters, false otherwise
     */
    bool GetAddress(Inet::IPAddress & addr, uint16_t & port) const override;

    /**
     * @brief
     *   Initialize the device object with secure session manager and inet layer object
     *   references. This variant of function is typically used when the device object
     *   is created from a serialized device information. The other parameters (address, port,
     *   interface etc) are part of the serialized device, so those are not required to be
     *   initialized.
     *
     *   Note: The lifetime of session manager and inet layer objects must be longer than
     *   that of this device object. If these objects are freed, while the device object is
     *   still using them, it can lead to unknown behavior and crashes.
     *
     * @param[in] params       Wrapper object for transport manager etc.
     * @param[in] fabric        Local administrator that's initializing this device object
     */
    void Init(ControllerDeviceInitParams params, FabricIndex fabric)
    {
        mSessionManager     = params.sessionManager;
        mExchangeMgr        = params.exchangeMgr;
        mUDPEndPointManager = params.udpEndPointManager;
        mFabricIndex        = fabric;
        mIDAllocator        = params.idAllocator;
#if CONFIG_NETWORK_LAYER_BLE
        mBleLayer = params.bleLayer;
#endif
    }

    /**
     * @brief
     *   Initialize a new device object with secure session manager, inet layer object,
     *   and other device specific parameters. This variant of function is typically used when
     *   a new device is paired, and the corresponding device object needs to updated with
     *   all device specifc parameters (address, port, interface etc).
     *
     *   This is not done as part of constructor so that the controller can have a list of
     *   uninitialized/unpaired device objects. The object is initialized only when the device
     *   is actually paired.
     *
     * @param[in] params       Wrapper object for transport manager etc.
     * @param[in] deviceId     Node ID of the device
     * @param[in] peerAddress  The location of the peer. MUST be of type Transport::Type::kUdp
     * @param[in] fabric        Local administrator that's initializing this device object
     */
    void Init(ControllerDeviceInitParams params, NodeId deviceId, const Transport::PeerAddress & peerAddress, FabricIndex fabric)
    {
        Init(params, fabric);
        mPeerId = PeerId().SetNodeId(deviceId);
        mState  = ConnectionState::Connecting;

        mDeviceAddress = peerAddress;
    }

    /**
     * @brief
     *   Called when the associated session is released
     *
     *   The receiver should release all resources associated with the connection.
     */
    void OnSessionReleased() override;

    /**
     *  In case there exists an open session to the device, mark it as expired.
     */
    CHIP_ERROR CloseSession();

    CHIP_ERROR Disconnect() override { return CloseSession(); }

    /**
     * @brief
     *   Update data of the device.
     *
     *   This function will set new IP address, port and MRP retransmission intervals of the device.
     *   Since the device settings might have been moved from RAM to the persistent storage, the function
     *   will load the device settings first, before making the changes.
     *
     * @param[in] addr   Address of the device to be set.
     * @param[in] config MRP parameters
     *
     * @return CHIP_NO_ERROR if the data has been updated, an error code otherwise.
     */
    CHIP_ERROR UpdateDeviceData(const Transport::PeerAddress & addr, const ReliableMessageProtocolConfig & config);
    /**
     * @brief
     *   Return whether the current device object is actively associated with a paired CHIP
     *   device. An active object can be used to communicate with the corresponding device.
     */
    bool IsActive() const override { return mActive; }

    void SetActive(bool active) { mActive = active; }

    /**
     * @brief
     * Called to indicate this proxy has been paired successfully.
     *
     * This causes the secure session parameters to be loaded and stores the session details in the session manager.
     */
    CHIP_ERROR SetConnected();

    bool IsSecureConnected() const override { return IsActive() && mState == ConnectionState::SecureConnected; }

    bool IsSessionSetupInProgress() const { return IsActive() && mState == ConnectionState::Connecting; }

    void Reset();

    NodeId GetDeviceId() const override { return mPeerId.GetNodeId(); }
    PeerId GetPeerId() const { return mPeerId; }
    CHIP_ERROR SetPeerId(ByteSpan rcac, ByteSpan noc) override;

    bool MatchesSession(const SessionHandle & session) const { return mSecureSession.Contains(session); }

    SessionHolder & GetSecureSessionHolder() { return mSecureSession; }
    chip::Optional<SessionHandle> GetSecureSession() const override { return mSecureSession.ToOptional(); }

    Messaging::ExchangeManager * GetExchangeManager() const override { return mExchangeMgr; }

    void SetAddress(const Inet::IPAddress & deviceAddr) { mDeviceAddress.SetSingleIPAddress(deviceAddr); }

    PASESession & GetPairing() { return mPairing; }

    uint8_t GetNextSequenceNumber() override { return mSequenceNumber++; };

    CHIP_ERROR LoadSecureSessionParametersIfNeeded()
    {
        bool loadedSecureSession = false;
        return LoadSecureSessionParametersIfNeeded(loadedSecureSession);
    };

private:
    enum class ConnectionState
    {
        NotConnected,
        Connecting,
        SecureConnected,
    };

    enum class ResetTransport
    {
        kYes,
        kNo,
    };

    /* Compressed fabric ID and node ID assigned to the device. */
    PeerId mPeerId;

    /** Address used to communicate with the device.
     */
    Transport::PeerAddress mDeviceAddress = Transport::PeerAddress::UDP(Inet::IPAddress::Any);

    Inet::EndPointManager<Inet::UDPEndPoint> * mUDPEndPointManager = nullptr;

    bool mActive           = false;
    ConnectionState mState = ConnectionState::NotConnected;

#if CONFIG_NETWORK_LAYER_BLE
    Ble::BleLayer * mBleLayer = nullptr;
#endif

    PASESession mPairing;

    SessionManager * mSessionManager = nullptr;

    Messaging::ExchangeManager * mExchangeMgr = nullptr;

    SessionHolderWithDelegate mSecureSession;

    uint8_t mSequenceNumber = 0;

    /**
     * @brief
     *   This function loads the secure session object from the serialized operational
     *   credentials corresponding to the device. This is typically done when the device
     *   does not have an active secure channel.
     */
    CHIP_ERROR LoadSecureSessionParameters();

    /**
     * @brief
     *   This function loads the secure session object from the serialized operational
     *   credentials corresponding if needed, based on the current state of the device and
     *   underlying transport object.
     *
     * @param[out] didLoad   Were the secure session params loaded by the call to this function.
     */
    CHIP_ERROR LoadSecureSessionParametersIfNeeded(bool & didLoad);

    FabricIndex mFabricIndex = kUndefinedFabricIndex;

    SessionIDAllocator * mIDAllocator = nullptr;
};

} // namespace chip
