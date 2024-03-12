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
#include <transport/SessionHolder.h>
#include <transport/SessionManager.h>
#include <transport/TransportMgr.h>
#include <transport/raw/MessageHeader.h>
#include <transport/raw/UDP.h>

namespace chip {

inline constexpr size_t kAttestationNonceLength = 32;

struct ControllerDeviceInitParams
{
    SessionManager * sessionManager          = nullptr;
    Messaging::ExchangeManager * exchangeMgr = nullptr;
};

class CommissioneeDeviceProxy : public DeviceProxy, public SessionDelegate
{
public:
    ~CommissioneeDeviceProxy() override;
    CommissioneeDeviceProxy() : mSecureSession(*this) {}
    CommissioneeDeviceProxy(const CommissioneeDeviceProxy &) = delete;

    /**
     * @brief
     *   Send the command in internal command sender.
     */
    CHIP_ERROR SendCommands(app::CommandSender * commandObj, Optional<System::Clock::Timeout> timeout) override;

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
     */
    void Init(ControllerDeviceInitParams params, NodeId deviceId, const Transport::PeerAddress & peerAddress)
    {
        mSessionManager = params.sessionManager;
        mExchangeMgr    = params.exchangeMgr;
        mPeerId         = PeerId().SetNodeId(deviceId);
        mState          = ConnectionState::Connecting;

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
    void CloseSession();

    /**
     *  Detaches the underlying session (if any) from this proxy and returns it.
     */
    chip::Optional<SessionHandle> DetachSecureSession();

    void Disconnect() override { CloseSession(); }

    /**
     * @brief
     *   Update data of the device.
     *
     *   This function will set new IP address, port and MRP retransmission intervals of the device.
     *
     * @param[in] addr   Address of the device to be set.
     * @param[in] config MRP parameters
     *
     * @return CHIP_NO_ERROR if the data has been updated, an error code otherwise.
     */
    CHIP_ERROR UpdateDeviceData(const Transport::PeerAddress & addr, const ReliableMessageProtocolConfig & config);

    /**
     * @brief
     * Called to indicate this proxy has been paired successfully.
     *
     * This stores the session details in the session manager.
     */
    CHIP_ERROR SetConnected(const SessionHandle & session);

    bool IsSecureConnected() const override { return mState == ConnectionState::SecureConnected; }

    bool IsSessionSetupInProgress() const { return mState == ConnectionState::Connecting; }

    NodeId GetDeviceId() const override { return mPeerId.GetNodeId(); }
    PeerId GetPeerId() const { return mPeerId; }
    CHIP_ERROR SetPeerId(ByteSpan rcac, ByteSpan noc) override;
    const Transport::PeerAddress & GetPeerAddress() const { return mDeviceAddress; }

    chip::Optional<SessionHandle> GetSecureSession() const override { return mSecureSession.Get(); }

    Messaging::ExchangeManager * GetExchangeManager() const override { return mExchangeMgr; }

    PASESession & GetPairing() { return mPairing; }

    Transport::Type GetDeviceTransportType() const { return mDeviceAddress.GetTransportType(); }

private:
    enum class ConnectionState
    {
        NotConnected,
        Connecting,
        SecureConnected,
    };

    /* Compressed fabric ID and node ID assigned to the device. */
    PeerId mPeerId;

    /** Address used to communicate with the device.
     */
    Transport::PeerAddress mDeviceAddress = Transport::PeerAddress::UDP(Inet::IPAddress::Any);

    ConnectionState mState = ConnectionState::NotConnected;

    PASESession mPairing;

    SessionManager * mSessionManager = nullptr;

    Messaging::ExchangeManager * mExchangeMgr = nullptr;

    SessionHolderWithDelegate mSecureSession;
};

} // namespace chip
