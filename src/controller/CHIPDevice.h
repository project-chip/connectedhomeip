/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
 *    This file contains definitions for Device class. The objects of this
 *    class will be used by Controller applications to interact with CHIP
 *    devices. The class provides mechanism to construct, send and receive
 *    messages to and from the corresponding CHIP devices.
 */

#pragma once

#include <core/CHIPCore.h>
#include <support/Base64.h>
#include <support/DLLUtil.h>
#include <transport/SecurePairingSession.h>
#include <transport/SecureSessionMgr.h>
#include <transport/raw/MessageHeader.h>
#include <transport/raw/UDP.h>

namespace chip {
namespace Controller {

class DeviceController;
class DeviceStatusDelegate;
struct SerializedDevice;

class DLL_EXPORT Device
{
public:
    Device() {}
    ~Device() {}

    void SetDelegate(DeviceStatusDelegate * delegate) { mStatusDelegate = delegate; }

    // ----- Messaging -----
    /**
     * @brief
     *   Send the provided message to the device
     *
     * @param[in] message   The message to be sent. The ownership of the message buffer
     *                      is handed over to Device object. SendMessage() will
     *                      decrement the reference count of the message buffer before
     *                      returning.
     *
     * @return CHIP_ERROR   CHIP_NO_ERROR on success, or corresponding error
     */
    CHIP_ERROR SendMessage(System::PacketBuffer * message);

    /**
     * @brief
     *   Get the IP address assigned to the device.
     *
     * @param[out] addr   The reference to the IP address.
     *
     * @return true, if the IP address was filled in the out parameter, false otherwise
     */
    bool GetIpAddress(Inet::IPAddress & addr) const;

    void Init(SecureSessionMgr<Transport::UDP> * sessionMgr, Inet::InetLayer * inetLayer)
    {
        mSessionManager = sessionMgr;
        mInetLayer      = inetLayer;
    }

    void Init(SecureSessionMgr<Transport::UDP> * sessionMgr, Inet::InetLayer * inetLayer, NodeId deviceId, uint16_t devicePort,
              Inet::InterfaceId interfaceId)
    {
        Init(sessionMgr, inetLayer);
        mDeviceId   = deviceId;
        mDevicePort = devicePort;
        mInterface  = interfaceId;
        mState      = kConnectionState_Connecting;
    }

    /** @brief Serialize the Pairing Session to a string.
     *
     * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
     **/
    CHIP_ERROR Serialize(SerializedDevice & output);

    /** @brief Deserialize the Pairing Session from the string.
     *
     * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
     **/
    CHIP_ERROR Deserialize(const SerializedDevice & input);

    void OnMessageReceived(const PacketHeader & header, const PayloadHeader & payloadHeader, Transport::PeerConnectionState * state,
                           System::PacketBuffer * msgBuf, SecureSessionMgrBase * mgr);

    bool IsActive() { return mActive; }

    void SetActive(bool active) { mActive = active; }

    NodeId GetDeviceId() { return mDeviceId; }

    void SetAddress(Inet::IPAddress deviceAddr) { mDeviceAddr = deviceAddr; }

    SecurePairingSessionSerializable & GetPairing() { return mPairing; }

private:
    enum ConnectionState
    {
        kConnectionState_NotConnected,
        kConnectionState_Connecting,
        kConnectionState_SecureConnected,
    };

    NodeId mDeviceId;
    Inet::IPAddress mDeviceAddr;
    uint16_t mDevicePort;
    Inet::InterfaceId mInterface;

    Inet::InetLayer * mInetLayer;

    bool mActive = false;
    ConnectionState mState;

    SecurePairingSessionSerializable mPairing;

    DeviceStatusDelegate * mStatusDelegate;

    SecureSessionMgr<Transport::UDP> * mSessionManager;

    CHIP_ERROR LoadSecureSessionParameters();
    CHIP_ERROR ResumeSecureSession();
};

/**
 * This class defines an interface for an object that the user of Device
 * can register as a delegate. The delegate object will be called by the
 * Device when a new message or status update is received from the corresponding
 * CHIP device.
 */
class DLL_EXPORT DeviceStatusDelegate
{
public:
    virtual ~DeviceStatusDelegate() {}

    /**
     * @brief
     *   Called when a message is received from the device.
     *
     * @param[in] msg Received message buffer.
     */
    virtual void OnMessage(System::PacketBuffer * msg) = 0;

    /**
     * @brief
     *   Called when device status is updated.
     *
     */
    virtual void OnStatusChange(void){};
};

typedef struct SerializableDevice
{
    SecurePairingSessionSerializable mOpsCreds;
    uint64_t mDeviceId;
    uint8_t mDeviceAddr[INET6_ADDRSTRLEN];
    uint16_t mDevicePort;
} SerializableDevice;

typedef struct SerializedDevice
{
    // Extra uint64_t to account for padding bytes (NULL termination, and some decoding overheads)
    uint8_t inner[BASE64_ENCODED_LEN(sizeof(SerializableDevice) + sizeof(uint64_t))];
} SerializedDevice;

} // namespace Controller
} // namespace chip
