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
#include <controller-clusters/zap-generated/CHIPClientCallbacks.h>
#include <credentials/CHIPOperationalCredentials.h>
#include <lib/core/CHIPCallback.h>
#include <lib/core/CHIPCore.h>
#include <lib/support/Base64.h>
#include <lib/support/DLLUtil.h>
#include <lib/support/TypeTraits.h>
#include <messaging/ExchangeContext.h>
#include <messaging/ExchangeDelegate.h>
#include <messaging/ExchangeMgr.h>
#include <messaging/Flags.h>
#include <protocols/secure_channel/CASESession.h>
#include <protocols/secure_channel/PASESession.h>
#include <protocols/secure_channel/SessionIDAllocator.h>
#include <setup_payload/SetupPayload.h>
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
    DeviceTransportMgr * transportMgr           = nullptr;
    SecureSessionMgr * sessionMgr               = nullptr;
    Messaging::ExchangeManager * exchangeMgr    = nullptr;
    Inet::InetLayer * inetLayer                 = nullptr;
    PersistentStorageDelegate * storageDelegate = nullptr;
    SessionIDAllocator * idAllocator            = nullptr;
#if CONFIG_NETWORK_LAYER_BLE
    Ble::BleLayer * bleLayer = nullptr;
#endif
    Transport::FabricTable * fabricsTable = nullptr;
};

class Device;

typedef void (*OnDeviceConnected)(void * context, Device * device);
typedef void (*OnDeviceConnectionFailure)(void * context, NodeId deviceId, CHIP_ERROR error);

class DLL_EXPORT Device : public Messaging::ExchangeDelegate, public SessionEstablishmentDelegate
{
public:
    ~Device();
    Device() :
        mOpenPairingSuccessCallback(OnOpenPairingWindowSuccessResponse, this),
        mOpenPairingFailureCallback(OnOpenPairingWindowFailureResponse, this)
    {}
    Device(const Device &) = delete;

    enum class CommissioningWindowOption
    {
        kOriginalSetupCode = 0,
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
     * @param [in] sendFlags  SendMessageFlags::kExpectResponse or SendMessageFlags::kNone
     * @param[in] message     The message payload to be sent.
     *
     * @return CHIP_ERROR   CHIP_NO_ERROR on success, or corresponding error
     */
    CHIP_ERROR SendMessage(Protocols::Id protocolId, uint8_t msgType, Messaging::SendFlags sendFlags,
                           System::PacketBufferHandle && message);

    /**
     * A strongly-message-typed version of SendMessage.
     */
    template <typename MessageType, typename = std::enable_if_t<std::is_enum<MessageType>::value>>
    CHIP_ERROR SendMessage(MessageType msgType, Messaging::SendFlags sendFlags, System::PacketBufferHandle && message)
    {
        return SendMessage(Protocols::MessageTypeTraits<MessageType>::ProtocolId(), to_underlying(msgType), sendFlags,
                           std::move(message));
    }

    CHIP_ERROR SendReadAttributeRequest(app::AttributePathParams aPath, Callback::Cancelable * onSuccessCallback,
                                        Callback::Cancelable * onFailureCallback, app::TLVDataFilter aTlvDataFilter);

    CHIP_ERROR SendWriteAttributeRequest(app::WriteClientHandle aHandle, Callback::Cancelable * onSuccessCallback,
                                         Callback::Cancelable * onFailureCallback);

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
     * @param[in] fabric        Local administrator that's initializing this device object
     */
    void Init(ControllerDeviceInitParams params, uint16_t listenPort, FabricIndex fabric)
    {
        mTransportMgr    = params.transportMgr;
        mSessionManager  = params.sessionMgr;
        mExchangeMgr     = params.exchangeMgr;
        mInetLayer       = params.inetLayer;
        mListenPort      = listenPort;
        mFabricIndex     = fabric;
        mStorageDelegate = params.storageDelegate;
        mIDAllocator     = params.idAllocator;
        mFabricsTable    = params.fabricsTable;
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
     * @param[in] fabric        Local administrator that's initializing this device object
     */
    void Init(ControllerDeviceInitParams params, uint16_t listenPort, NodeId deviceId, const Transport::PeerAddress & peerAddress,
              FabricIndex fabric)
    {
        Init(params, mListenPort, fabric);
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
    void OnNewConnection(SessionHandle session);

    /**
     * @brief
     *   Called when a connection is closing.
     *
     *   The receiver should release all resources associated with the connection.
     *
     * @param session A handle to the secure session
     */
    void OnConnectionExpired(SessionHandle session);

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
    CHIP_ERROR OnMessageReceived(Messaging::ExchangeContext * exchange, const PacketHeader & header,
                                 const PayloadHeader & payloadHeader, System::PacketBufferHandle && msgBuf) override;

    /**
     * @brief ExchangeDelegate implementation of OnResponseTimeout.
     */
    void OnResponseTimeout(Messaging::ExchangeContext * exchange) override;

    /**
     * @brief
     *   Trigger a paired device to re-enter the commissioning mode. If an onboarding token is provided, the device will use
     *   the provided setup PIN code and the discriminator to advertise itself for commissioning availability. If the token
     *   is not provided, the device will use the manufacturer assigned setup PIN code and discriminator.
     *
     *   The device will exit the commissioning mode after a successful commissioning, or after the given `timeout` time.
     *
     * @param[in] timeout         The commissioning mode should terminate after this much time.
     * @param[in] iteration       The PAKE iteration count associated with the PAKE Passcode ID and ephemeral
     *                            PAKE passcode verifier to be used for this commissioning.
     * @param[in] option          The commissioning window can be opened using the original setup code, or an
     *                            onboarding token can be generated using a random setup PIN code (or with
     *                            the PIN code provied in the setupPayload).
     * @param[in] salt            The PAKE Salt associated with the PAKE Passcode ID and ephemeral PAKE passcode
     *                            verifier to be used for this commissioning.
     * @param[out] setupPayload   The setup payload corresponding to the generated onboarding token.
     *
     * @return CHIP_ERROR         CHIP_NO_ERROR on success, or corresponding error
     */
    CHIP_ERROR OpenCommissioningWindow(uint16_t timeout, uint32_t iteration, CommissioningWindowOption option,
                                       const ByteSpan & salt, SetupPayload & setupPayload);

    /**
     * @brief
     *   Trigger a paired device to re-enter the commissioning mode. If an onboarding token is provided, the device will use
     *   the provided setup PIN code and the discriminator to advertise itself for commissioning availability. If the token
     *   is not provided, the device will use the manufacturer assigned setup PIN code and discriminator.
     *
     *   The device will exit the commissioning mode after a successful commissioning, or after the given `timeout` time.
     *
     * @param[in] timeout         The commissioning mode should terminate after this much time.
     * @param[in] option          The commissioning window can be opened using the original setup code, or an
     *                            onboarding token can be generated using a random setup PIN code (or with
     *                            the PIN code provied in the setupPayload). This argument selects one of these
     *                            methods.
     * @param[out] setupPayload   The setup payload corresponding to the generated onboarding token.
     *
     * @return CHIP_ERROR         CHIP_NO_ERROR on success, or corresponding error
     */
    CHIP_ERROR OpenPairingWindow(uint16_t timeout, CommissioningWindowOption option, SetupPayload & setupPayload);

    /**
     *  In case there exists an open session to the device, mark it as expired.
     */
    CHIP_ERROR CloseSession();

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

    bool IsSessionSetupInProgress() const { return IsActive() && mState == ConnectionState::Connecting; }

    void Reset();

    NodeId GetDeviceId() const { return mDeviceId; }

    bool MatchesSession(SessionHandle session) const { return mSecureSession.HasValue() && mSecureSession.Value() == session; }

    SessionHandle GetSecureSession() const { return mSecureSession.Value(); }

    void SetAddress(const Inet::IPAddress & deviceAddr) { mDeviceAddress.SetIPAddress(deviceAddr); }

    PASESessionSerializable & GetPairing() { return mPairing; }

    uint8_t GetNextSequenceNumber() { return mSequenceNumber++; };
    void AddResponseHandler(uint8_t seqNum, Callback::Cancelable * onSuccessCallback, Callback::Cancelable * onFailureCallback,
                            app::TLVDataFilter tlvDataFilter = nullptr);
    void CancelResponseHandler(uint8_t seqNum);
    void AddReportHandler(EndpointId endpoint, ClusterId cluster, AttributeId attribute, Callback::Cancelable * onReportCallback);

    // This two functions are pretty tricky, it is used to bridge the response, we need to implement interaction model delegate on
    // the app side instead of register callbacks here. The IM delegate can provide more infomation then callback and it is
    // type-safe.
    // TODO: Implement interaction model delegate in the application.
    void AddIMResponseHandler(app::CommandSender * commandObj, Callback::Cancelable * onSuccessCallback,
                              Callback::Cancelable * onFailureCallback);
    void CancelIMResponseHandler(app::CommandSender * commandObj);

    void OperationalCertProvisioned();
    bool IsOperationalCertProvisioned() const { return mDeviceOperationalCertProvisioned; }

    CHIP_ERROR LoadSecureSessionParametersIfNeeded()
    {
        bool loadedSecureSession = false;
        return LoadSecureSessionParametersIfNeeded(loadedSecureSession);
    };

    //////////// SessionEstablishmentDelegate Implementation ///////////////
    void OnSessionEstablishmentError(CHIP_ERROR error) override;
    void OnSessionEstablished() override;

    CASESession & GetCASESession() { return mCASESession; }

    CHIP_ERROR SetCSRNonce(ByteSpan csrNonce)
    {
        VerifyOrReturnError(csrNonce.size() == sizeof(mCSRNonce), CHIP_ERROR_INVALID_ARGUMENT);
        memcpy(mCSRNonce, csrNonce.data(), csrNonce.size());
        return CHIP_NO_ERROR;
    }

    ByteSpan GetCSRNonce() const { return ByteSpan(mCSRNonce, sizeof(mCSRNonce)); }

    MutableByteSpan GetMutableNOCChain() { return MutableByteSpan(mNOCChainBuffer, sizeof(mNOCChainBuffer)); }

    CHIP_ERROR ReduceNOCChainBufferSize(size_t new_size);

    ByteSpan GetNOCChain() const { return ByteSpan(mNOCChainBuffer, mNOCChainBufferSize); }

    /*
     * This function can be called to establish a secure session with the device.
     *
     * If the device doesn't have operational credentials, and is under commissioning process,
     * PASE keys will be used for secure session.
     *
     * If the device has been commissioned and has operational credentials, CASE session
     * setup will be triggered.
     *
     * On establishing the session, the callback function `onConnection` will be called. If the
     * session setup fails, `onFailure` will be called.
     *
     * If the session already exists, `onConnection` will be called immediately.
     */
    CHIP_ERROR EstablishConnectivity(Callback::Callback<OnDeviceConnected> * onConnection,
                                     Callback::Callback<OnDeviceConnectionFailure> * onFailure);

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

    Optional<SessionHandle> mSecureSession = Optional<SessionHandle>::Missing();

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

    /**
     *   This function triggers CASE session setup if the device has been provisioned with
     *   operational credentials, and there is no currently active session.
     */

    CHIP_ERROR WarmupCASESession();

    static void OnOpenPairingWindowSuccessResponse(void * context);
    static void OnOpenPairingWindowFailureResponse(void * context, uint8_t status);

    uint16_t mListenPort;

    FabricIndex mFabricIndex = Transport::kUndefinedFabricIndex;

    Transport::FabricTable * mFabricsTable = nullptr;

    bool mDeviceOperationalCertProvisioned = false;

    CASESession mCASESession;
    PersistentStorageDelegate * mStorageDelegate = nullptr;

    uint8_t mCSRNonce[kOpCSRNonceLength];

    // The chain can contain ICAC and OpCert
    uint8_t mNOCChainBuffer[Credentials::kMaxCHIPCertLength * 2];
    size_t mNOCChainBufferSize = 0;

    SessionIDAllocator * mIDAllocator = nullptr;

    uint16_t mPAKEVerifierID = 1;

    Callback::CallbackDeque mConnectionSuccess;
    Callback::CallbackDeque mConnectionFailure;

    Callback::Callback<DefaultSuccessCallback> mOpenPairingSuccessCallback;
    Callback::Callback<DefaultFailureCallback> mOpenPairingFailureCallback;
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
    uint16_t mDevicePort;  /* This field is serialized in LittleEndian byte order */
    uint16_t mFabricIndex; /* This field is serialized in LittleEndian byte order */
    uint8_t mDeviceTransport;
    uint8_t mDeviceOperationalCertProvisioned;
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
