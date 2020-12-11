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

#include <app/util/basic-types.h>
#include <core/CHIPCallback.h>
#include <core/CHIPCore.h>
#include <messaging/Channel.h>
#include <messaging/ExchangeContext.h>
#include <messaging/ExchangeMgr.h>
#include <support/Base64.h>
#include <support/DLLUtil.h>
#include <transport/SecurePairingSession.h>
#include <transport/SecureSessionMgr.h>
#include <transport/TransportMgr.h>
#include <transport/raw/MessageHeader.h>
#include <transport/raw/UDP.h>

namespace chip {
namespace Controller {

class DeviceController;
class DeviceStatusDelegate;
struct SerializedDevice;

using DeviceTransportMgr = TransportMgr<Transport::UDP /* IPv6 */
#if INET_CONFIG_ENABLE_IPV4
                                        ,
                                        Transport::UDP /* IPv4 */
#endif
                                        >;

class DLL_EXPORT Device : public Messaging::ChannelDelegate
{
public:
    Device() : mActive(false), mState(ConnectionState::NotConnected) {}
    ~Device() {}

    /**
     * @brief
     *   Set the delegate object which will be called when a message is received.
     *   The user of this Device object must reset the delegate (by calling
     *   SetDelegate(nullptr)) before releasing their delegate object.
     *
     * @param[in] delegate   The pointer to the delegate object.
     */
    void SetDelegate(DeviceStatusDelegate * delegate) { mStatusDelegate = delegate; }

    /**
     * @brief
     *   Get the IP address assigned to the device.
     *
     * @param[out] addr   The reference to the IP address.
     *
     * @return true, if the IP address was filled in the out parameter, false otherwise
     */
    bool GetIpAddress(Inet::IPAddress & addr) const;

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
     * @param[in] transportMgr Transport manager object pointer
     * @param[in] sessionMgr   Secure session manager object pointer
     * @param[in] inetLayer    InetLayer object pointer
     * @param[in] listenPort   Port on which controller is listening (typically CHIP_PORT)
     */
    void Init(Messaging::ExchangeManager * exchangeManager) { mExchangeManager = exchangeManager; }

    /**
     * @brief
     *   Initialize a new device object with secure session manager, inet layer object,
     *   and other device specific parameters. This variant of function is typically used when
     *   a new device is paired, and the corresponding device object needs to updated with
     *   all device specifc parameters (address, port, interface etc).
     *
     *   This is not done as part of constructor so that the controller can have a list of
     *   uninitialzed/unpaired device objects. The object is initialized only when the device
     *   is actually paired.
     *
     * @param[in] transportMgr Transport manager object pointer
     * @param[in] sessionMgr   Secure session manager object pointer
     * @param[in] deviceId     Node ID of the device
     */
    void Init(Messaging::ExchangeManager * exchangeManager, NodeId deviceId)
    {
        Init(exchangeManager);
        mDeviceId = deviceId;
        mState    = ConnectionState::Connecting;
    }

    /** @brief Serialize the Pairing Session to a string. It's guaranteed that the string
     *         will be null terminated, and there won't be any embedded null characters.
     *
     * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
     **/
    CHIP_ERROR Serialize(SerializedDevice & output);

    /** @brief Deserialize the Pairing Session from the string. It's expected that the string
     *         will be null terminated, and there won't be any embedded null characters.
     *
     * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
     **/
    CHIP_ERROR Deserialize(const SerializedDevice & input);

    /**
     * @brief
     *   Return whether the current device object is actively associated with a paired CHIP
     *   device. An active object can be used to communicate with the corresponding device.
     */
    bool IsActive() const { return mActive; }

    void SetActive(bool active) { mActive = active; }

    void Reset()
    {
        SetActive(false);
        mState          = ConnectionState::NotConnected;
        mStatusDelegate = nullptr;
    }

    NodeId GetDeviceId() const { return mDeviceId; }

    SecurePairingSessionSerializable GetSecureSessoinParameters() { return mSecureSessionParameters; }

    CHIP_ERROR EstablishPaseSession();
    CHIP_ERROR EstablishCaseSession();
    void CloseSession();

    Messaging::ExchangeContext * NewExchange();

    // Messaging::ChannelDelegate interface
    void OnEstablished() override;
    void OnClosed() override;
    void OnFail(CHIP_ERROR err) override;

private:
    enum class ConnectionState
    {
        NotConnected,
        Connecting,
        SecureConnected,
    };

    struct CallbackInfo
    {
        EndpointId endpoint;
        ClusterId cluster;
    };

    enum class ResetTransport
    {
        kYes,
        kNo,
    };
    /* Node ID assigned to the CHIP device */
    NodeId mDeviceId;

    SecurePairingSessionSerializable mSecureSessionParameters;

    bool mActive;
    ConnectionState mState;

    DeviceStatusDelegate * mStatusDelegate;

    Messaging::ExchangeManager * mExchangeManager;
    Messaging::ChannelHandle mCaseChannel;

    /**
     * @brief
     *   This function loads the secure session object from the serialized operational
     *   credentials corresponding to the device. This is typically done when the device
     *   does not have an active secure channel.
     *
     * @param[in] resetNeeded   Does the underlying network socket require a reset
     */
    CHIP_ERROR LoadSecureSessionParameters(ResetTransport resetNeeded);
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
     *   Called when device status is updated.
     *
     */
    virtual void OnStatusChange(void){};
};

#ifdef IFNAMSIZ
constexpr uint16_t kMaxInterfaceName = IFNAMSIZ;
#else
constexpr uint16_t kMaxInterfaceName = 32;
#endif

typedef struct SerializableDevice
{
    SecurePairingSessionSerializable mOpsCreds;
    uint64_t mDeviceId; /* This field is serialized in LittleEndian byte order */
} SerializableDevice;

typedef struct SerializedDevice
{
    // Extra uint64_t to account for padding bytes (NULL termination, and some decoding overheads)
    // The encoder may not include a NULL character, and there are maximum 2 bytes of padding.
    // So extra 8 bytes should be sufficient to absorb this overhead.
    uint8_t inner[BASE64_ENCODED_LEN(sizeof(SerializableDevice) + sizeof(uint64_t))];
} SerializedDevice;

} // namespace Controller
} // namespace chip
