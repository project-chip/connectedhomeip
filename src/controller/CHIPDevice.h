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
    friend class DeviceController;
    friend class DeviceCommissioner;

    void SetDelegate(DeviceStatusDelegate * delegate, void * state)
    {
        mStatusDelegate = delegate;
        mAppReqState    = state;
    }

    // ----- Messaging -----
    /**
     * @brief
     *   Send a message to a connected CHIP device
     *
     * @param[in] buffer        The Data Buffer to trasmit to the device
     * @return CHIP_ERROR   The return status
     */
    CHIP_ERROR SendMessage(System::PacketBuffer * buffer);

    bool GetIpAddress(Inet::IPAddress & addr) const;

private:
    enum ConnectionState
    {
        kConnectionState_NotConnected    = 0,
        kConnectionState_Connecting      = 1,
        kConnectionState_SecureConnected = 2,
    };

    NodeId mDeviceId;
    Inet::IPAddress mDeviceAddr;
    uint16_t mDevicePort;
    Inet::InterfaceId mInterface;

    Inet::InetLayer * mInetLayer;

    Device() {}
    ~Device() {}

    bool mActive = false;
    ConnectionState mState;

    SecurePairingSessionSerializable mPairing;

    DeviceStatusDelegate * mStatusDelegate;
    void * mAppReqState;

    SecureSessionMgr<Transport::UDP> * mSessionManager;

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

    CHIP_ERROR EstablishSecureSession();
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
