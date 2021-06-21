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

#include <app/CommandSender.h>
#include <app/InteractionModelEngine.h>
#include <app/util/CHIPDeviceCallbacksMgr.h>
#include <app/util/basic-types.h>
#include <core/CHIPCallback.h>
#include <core/CHIPCore.h>
#include <credentials/CHIPOperationalCredentials.h>
#include <messaging/ExchangeContext.h>
#include <messaging/ExchangeDelegate.h>
#include <messaging/ExchangeMgr.h>
#include <protocols/secure_channel/CASESession.h>
#include <protocols/secure_channel/PASESession.h>
#include <setup_payload/SetupPayload.h>
#include <support/Base64.h>
#include <support/DLLUtil.h>
#include <transport/SecureSessionMgr.h>
#include <transport/TransportMgr.h>
#include <transport/raw/MessageHeader.h>
#include <transport/raw/UDP.h>

#if CONFIG_NETWORK_LAYER_BLE
#include <ble/BleLayer.h>
#include <transport/raw/BLE.h>
#endif

namespace chip {
namespace Controller {

class DeviceController;
class DeviceStatusDelegate;
struct SerializedDevice;

constexpr size_t kMaxBlePendingPackets = 1;
constexpr uint32_t kOpCSRNonceLength   = 32;

using DeviceTransportMgr = TransportMgr<Transport::UDP /* IPv6 */
#if INET_CONFIG_ENABLE_IPV4
                                        ,
                                        Transport::UDP /* IPv4 */
#endif
#if CONFIG_NETWORK_LAYER_BLE
                                        ,
                                        Transport::BLE<kMaxBlePendingPackets> /* BLE */
#endif
                                        >;

struct ControllerDeviceInitParams
{
    DeviceTransportMgr * transportMgr                   = nullptr;
    SecureSessionMgr * sessionMgr                       = nullptr;
    Messaging::ExchangeManager * exchangeMgr            = nullptr;
    Inet::InetLayer * inetLayer                         = nullptr;
    PersistentStorageDelegate * storageDelegate         = nullptr;
    Credentials::OperationalCredentialSet * credentials = nullptr;
#if CONFIG_NETWORK_LAYER_BLE
    Ble::BleLayer * bleLayer = nullptr;
#endif
};

class DLL_EXPORT Device : public Messaging::ExchangeDelegate, public SessionEstablishmentDelegate
{
public:
    ~Device();
    Device()               = default;
    Device(const Device &) = delete;

    enum class PairingWindowOption
    {
        kOriginalSetupCode,
        kTokenWithRandomPIN,
        kTokenWithProvidedPIN,
    };

    /**
     * @brief
     *   Set the delegate object which will be called when a message is received.
     *   The user of this Device object must reset the delegate (by calling
     *   SetDelegate(nullptr)) before releasing their delegate object.
     *
     * @param[in] delegate   The pointer to the delegate object.
     */
    void SetDelegate(DeviceStatusDelegate * delegate) { mStatusDelegate = delegate; }

    // ----- Messaging -----
    /**
     * @brief
     *   Send the provided message to the device
     *
     * @param[in] protocolId  The protocol identifier of the CHIP message to be sent.
     * @param[in] msgType     The message type of the message to be sent.  Must be a valid message type for protocolId.
     * @param[in] message     The message payload to be sent.
     *
     * @return CHIP_ERROR   CHIP_NO_ERROR on success, or corresponding error
     */
    CHIP_ERROR SendMessage(Protocols::Id protocolId, uint8_t msgType, System::PacketBufferHandle && message);

    /**
     * A strongly-message-typed version of SendMessage.
     */
    template <typename MessageType, typename = std::enable_if_t<std::is_enum<MessageType>::value>>
    CHIP_ERROR SendMessage(MessageType msgType, System::PacketBufferHandle && message)
    {
        static_assert(std::is_same<std::underlying_type_t<MessageType>, uint8_t>::value, "Enum is wrong size; cast is not safe");
        return SendMessage(Protocols::MessageTypeTraits<MessageType>::ProtocolId(), static_cast<uint8_t>(msgType),
                           std::move(message));
    }

    /**
     * @brief
     *   Send the command in internal command sender.
     */
    CHIP_ERROR SendCommands(app::CommandSender * commandObj);

    /**
     * @brief Get the IP address and port assigned to the device.
     *
     * @param[out] addr   IP address of the device.
     * @param[out] port   Port number of the device.
     *
     * @return true, if the IP address and port were filled in the out parameters, false otherwise
     */
    bool GetAddress(Inet::IPAddress & addr, uint16_t & port) const;

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
     * @param[in] listenPort   Port on which controller is listening (typically CHIP_PORT)
     * @param[in] admin        Local administrator that's initializing this device object
     */
    void Init(ControllerDeviceInitParams params, uint16_t listenPort, Transport::AdminId admin)
    {
        mTransportMgr    = params.transportMgr;
        mSessionManager  = params.sessionMgr;
        mExchangeMgr     = params.exchangeMgr;
        mInetLayer       = params.inetLayer;
        mListenPort      = listenPort;
        mAdminId         = admin;
        mStorageDelegate = params.storageDelegate;
        mCredentials     = params.credentials;
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
     * @param[in] listenPort   Port on which controller is listening (typically CHIP_PORT)
     * @param[in] deviceId     Node ID of the device
     * @param[in] peerAddress  The location of the peer. MUST be of type Transport::Type::kUdp
     * @param[in] admin        Local administrator that's initializing this device object
     */
    void Init(ControllerDeviceInitParams params, uint16_t listenPort, NodeId deviceId, const Transport::PeerAddress & peerAddress,
              Transport::AdminId admin)
    {
        Init(params, mListenPort, admin);
        mDeviceId = deviceId;
        mState    = ConnectionState::Connecting;

        mDeviceAddress = peerAddress;
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
     * @brief Serialize and store the Device in persistent storage
     *
     * @return Returns a CHIP_ERROR if either serialization or storage fails
     */
    CHIP_ERROR Persist();

    /**
     * @brief
     *   Called when a new pairing is being established
     *
     * @param session A handle to the secure session
     */
    void OnNewConnection(SecureSessionHandle session);

    /**
     * @brief
     *   Called when a connection is closing.
     *
     *   The receiver should release all resources associated with the connection.
     *
     * @param session A handle to the secure session
     */
    void OnConnectionExpired(SecureSessionHandle session);

    /**
     * @brief
     *   This function is called when a message is received from the corresponding CHIP
     *   device. The message ownership is transferred to the function, and it is expected
     *   to release the message buffer before returning.
     *
     * @param[in] exchange      The exchange context the message was received
     *                          on.  The Device guarantees that it will call
     *                          Close() on exchange when it's done processing
     *                          the message.
     * @param[in] header        Reference to common packet header of the received message
     * @param[in] payloadHeader Reference to payload header in the message
     * @param[in] msgBuf        The message buffer
     */
    void OnMessageReceived(Messaging::ExchangeContext * exchange, const PacketHeader & header, const PayloadHeader & payloadHeader,
                           System::PacketBufferHandle && msgBuf) override;

    /**
     * @brief ExchangeDelegate implementation of OnResponseTimeout.
     */
    void OnResponseTimeout(Messaging::ExchangeContext * exchange) override;

    /**
     * @brief
     *   Trigger a paired device to re-enter the pairing mode. If an onboarding token is provided, the device will use
     *   the provided setup PIN code and the discriminator to advertise itself for pairing availability. If the token
     *   is not provided, the device will use the manufacturer assigned setup PIN code and discriminator.
     *
     *   The device will exit the pairing mode after a successful pairing, or after the given `timeout` time.
     *
     * @param[in] timeout         The pairing mode should terminate after this much time.
     * @param[in] option          The pairing window can be opened using the original setup code, or an
     *                            onboarding token can be generated using a random setup PIN code (or with
     *                            the PIN code provied in the setupPayload). This argument selects one of these
     *                            methods.
     * @param[out] setupPayload   The setup payload corresponding to the generated onboarding token.
     *
     * @return CHIP_ERROR               CHIP_NO_ERROR on success, or corresponding error
     */
    CHIP_ERROR OpenPairingWindow(uint32_t timeout, PairingWindowOption option, SetupPayload & setupPayload);

    /**
     * @brief
     *   Update address of the device.
     *
     *   This function will set new IP address and port of the device. Since the device settings might
     *   have been moved from RAM to the persistent storage, the function will load the device settings
     *   first, before making the changes.
     *
     * @param[in] addr   Address of the device to be set.
     *
     * @return CHIP_NO_ERROR if the address has been updated, an error code otherwise.
     */
    CHIP_ERROR UpdateAddress(const Transport::PeerAddress & addr);
    /**
     * @brief
     *   Return whether the current device object is actively associated with a paired CHIP
     *   device. An active object can be used to communicate with the corresponding device.
     */
    bool IsActive() const { return mActive; }

    void SetActive(bool active) { mActive = active; }

    bool IsSecureConnected() const { return IsActive() && mState == ConnectionState::SecureConnected; }

    void Reset();

    NodeId GetDeviceId() const { return mDeviceId; }

    bool MatchesSession(SecureSessionHandle session) const { return mSecureSession == session; }

    void SetAddress(const Inet::IPAddress & deviceAddr) { mDeviceAddress.SetIPAddress(deviceAddr); }

    PASESessionSerializable & GetPairing() { return mPairing; }

    uint8_t GetNextSequenceNumber() { return mSequenceNumber++; };
    void AddResponseHandler(uint8_t seqNum, Callback::Cancelable * onSuccessCallback, Callback::Cancelable * onFailureCallback);
    void CancelResponseHandler(uint8_t seqNum);
    void AddReportHandler(EndpointId endpoint, ClusterId cluster, AttributeId attribute, Callback::Cancelable * onReportCallback);

    // This two functions are pretty tricky, it is used to bridge the response, we need to implement interaction model delegate on
    // the app side instead of register callbacks here. The IM delegate can provide more infomation then callback and it is
    // type-safe.
    // TODO: Implement interaction model delegate in the application.
    void AddIMResponseHandler(app::Command * commandObj, Callback::Cancelable * onSuccessCallback,
                              Callback::Cancelable * onFailureCallback);
    void CancelIMResponseHandler(app::Command * commandObj);

    void ProvisioningComplete(uint16_t caseKeyId)
    {
        mDeviceProvisioningComplete = true;
        mCASESessionKeyId           = caseKeyId;
    }
    bool IsProvisioningComplete() const { return mDeviceProvisioningComplete; }

    //////////// SessionEstablishmentDelegate Implementation ///////////////
    void OnSessionEstablishmentError(CHIP_ERROR error) override;
    void OnSessionEstablished() override;

    CASESession & GetCASESession() { return mCASESession; }

    CHIP_ERROR GenerateCSRNonce() { return Crypto::DRBG_get_bytes(mCSRNonce, sizeof(mCSRNonce)); }

    ByteSpan GetCSRNonce() const { return ByteSpan(mCSRNonce, sizeof(mCSRNonce)); }

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
    /* Node ID assigned to the CHIP device */
    NodeId mDeviceId;

    /** Address used to communicate with the device.
     */
    Transport::PeerAddress mDeviceAddress = Transport::PeerAddress::UDP(Inet::IPAddress::Any);

    Inet::InetLayer * mInetLayer = nullptr;

    bool mActive           = false;
    ConnectionState mState = ConnectionState::NotConnected;

#if CONFIG_NETWORK_LAYER_BLE
    Ble::BleLayer * mBleLayer = nullptr;
#endif

    PASESessionSerializable mPairing;

    DeviceStatusDelegate * mStatusDelegate = nullptr;

    SecureSessionMgr * mSessionManager = nullptr;

    DeviceTransportMgr * mTransportMgr = nullptr;

    Messaging::ExchangeManager * mExchangeMgr = nullptr;

    SecureSessionHandle mSecureSession = {};

    uint8_t mSequenceNumber = 0;

    uint32_t mLocalMessageCounter = 0;
    uint32_t mPeerMessageCounter  = 0;

    app::CHIPDeviceCallbacksMgr & mCallbacksMgr = app::CHIPDeviceCallbacksMgr::GetInstance();

    /**
     * @brief
     *   This function loads the secure session object from the serialized operational
     *   credentials corresponding to the device. This is typically done when the device
     *   does not have an active secure channel.
     *
     * @param[in] resetNeeded   Does the underlying network socket require a reset
     */
    CHIP_ERROR LoadSecureSessionParameters(ResetTransport resetNeeded);

    /**
     * @brief
     *   This function loads the secure session object from the serialized operational
     *   credentials corresponding if needed, based on the current state of the device and
     *   underlying transport object.
     *
     * @param[out] didLoad   Were the secure session params loaded by the call to this function.
     */
    CHIP_ERROR LoadSecureSessionParametersIfNeeded(bool & didLoad);

    CHIP_ERROR EstablishCASESession();

    uint16_t mListenPort;

    Transport::AdminId mAdminId = Transport::kUndefinedAdminId;

    bool mDeviceProvisioningComplete = false;

    CASESession mCASESession;
    uint16_t mCASESessionKeyId = 0;

    Credentials::OperationalCredentialSet * mCredentials = nullptr;

    PersistentStorageDelegate * mStorageDelegate = nullptr;

    uint8_t mCSRNonce[kOpCSRNonceLength];
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
    virtual void OnMessage(System::PacketBufferHandle && msg) = 0;

    /**
     * @brief
     *   Called when response to OpenPairingWindow is received from the device.
     *
     * @param[in] status CHIP_NO_ERROR on success, or corresponding error.
     */
    virtual void OnPairingWindowOpenStatus(CHIP_ERROR status){};

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
    PASESessionSerializable mOpsCreds;
    uint64_t mDeviceId; /* This field is serialized in LittleEndian byte order */
    uint8_t mDeviceAddr[INET6_ADDRSTRLEN];
    uint16_t mDevicePort;       /* This field is serialized in LittleEndian byte order */
    uint16_t mAdminId;          /* This field is serialized in LittleEndian byte order */
    uint16_t mCASESessionKeyId; /* This field is serialized in LittleEndian byte order */
    uint8_t mDeviceTransport;
    uint8_t mDeviceProvisioningComplete;
    uint8_t mInterfaceName[kMaxInterfaceName];
    uint32_t mLocalMessageCounter; /* This field is serialized in LittleEndian byte order */
    uint32_t mPeerMessageCounter;  /* This field is serialized in LittleEndian byte order */
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
