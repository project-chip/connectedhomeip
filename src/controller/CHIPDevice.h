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

struct SerializedDevice;

class DLL_EXPORT Device
{
public:
    Device() {}
    ~Device() {}

    void SetDelegate(DeviceStatusDelegate * delegate) { mStatusDelegate = delegate; }

    // ----- Messaging -----
    CHIP_ERROR SendMessage(System::PacketBuffer * message);

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
