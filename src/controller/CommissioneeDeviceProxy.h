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
 *    This file contains definitions for Device class. The objects of this
 *    class will be used by Controller applications to interact with CHIP
 *    devices. The class provides mechanism to construct, send and receive
 *    messages to and from the corresponding CHIP devices.
 */

#pragma once

#include <app/CommandSender.h>
#include <app/DeviceControllerInteractionModelDelegate.h>
#include <app/DeviceProxy.h>
#include <app/InteractionModelEngine.h>
#include <app/util/CHIPDeviceCallbacksMgr.h>
#include <app/util/attribute-filter.h>
#include <app/util/basic-types.h>
#include <controller-clusters/zap-generated/CHIPClientCallbacks.h>
#include <controller/CHIPDeviceControllerSystemState.h>
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
#include <transport/SessionManager.h>
#include <transport/TransportMgr.h>
#include <transport/raw/MessageHeader.h>
#include <transport/raw/UDP.h>

#if CONFIG_NETWORK_LAYER_BLE
#include <ble/BleLayer.h>
#include <transport/raw/BLE.h>
#endif

namespace chip {

class DeviceStatusDelegate;
struct SerializedDevice;

constexpr size_t kOpCSRNonceLength       = 32;
constexpr size_t kAttestationNonceLength = 32;

using DeviceIPTransportMgr = TransportMgr<Transport::UDP /* IPv6 */
#if INET_CONFIG_ENABLE_IPV4
                                          ,
                                          Transport::UDP /* IPv4 */
#endif
                                          >;

struct ControllerDeviceInitParams
{
    DeviceTransportMgr * transportMgr           = nullptr;
    SessionManager * sessionManager             = nullptr;
    Messaging::ExchangeManager * exchangeMgr    = nullptr;
    Inet::InetLayer * inetLayer                 = nullptr;
    PersistentStorageDelegate * storageDelegate = nullptr;
    SessionIDAllocator * idAllocator            = nullptr;
#if CONFIG_NETWORK_LAYER_BLE
    Ble::BleLayer * bleLayer = nullptr;
#endif
    FabricTable * fabricsTable = nullptr;

    Controller::DeviceControllerInteractionModelDelegate * imDelegate = nullptr;
};

class CommissioneeDeviceProxy;

class DLL_EXPORT CommissioneeDeviceProxy : public DeviceProxy, Messaging::ExchangeDelegate
{
public:
    ~CommissioneeDeviceProxy();
    CommissioneeDeviceProxy() {}
    CommissioneeDeviceProxy(const CommissioneeDeviceProxy &) = delete;

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
    CHIP_ERROR SendReadAttributeRequest(app::AttributePathParams aPath, Callback::Cancelable * onSuccessCallback,
                                        Callback::Cancelable * onFailureCallback, app::TLVDataFilter aTlvDataFilter) override;

    CHIP_ERROR SendSubscribeAttributeRequest(app::AttributePathParams aPath, uint16_t mMinIntervalFloorSeconds,
                                             uint16_t mMaxIntervalCeilingSeconds, Callback::Cancelable * onSuccessCallback,
                                             Callback::Cancelable * onFailureCallback) override;

    CHIP_ERROR SendWriteAttributeRequest(app::WriteClientHandle aHandle, Callback::Cancelable * onSuccessCallback,
                                         Callback::Cancelable * onFailureCallback) override;

    /**
     * @brief
     *   Send the command in internal command sender.
     */
    CHIP_ERROR SendCommands(app::CommandSender * commandObj) override;

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
        mSessionManager  = params.sessionManager;
        mExchangeMgr     = params.exchangeMgr;
        mInetLayer       = params.inetLayer;
        mFabricIndex     = fabric;
        mStorageDelegate = params.storageDelegate;
        mIDAllocator     = params.idAllocator;
        mFabricsTable    = params.fabricsTable;
        mpIMDelegate     = params.imDelegate;
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
        mDeviceId = deviceId;
        mState    = ConnectionState::Connecting;

        mDeviceAddress = peerAddress;
    }

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
    void OnConnectionExpired(SessionHandle session) override;

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
     * @param[in] payloadHeader Reference to payload header in the message
     * @param[in] msgBuf        The message buffer
     */
    CHIP_ERROR OnMessageReceived(Messaging::ExchangeContext * exchange, const PayloadHeader & payloadHeader,
                                 System::PacketBufferHandle && msgBuf) override;

    /**
     * @brief ExchangeDelegate implementation of OnResponseTimeout.
     */
    void OnResponseTimeout(Messaging::ExchangeContext * exchange) override;

    /**
     *  In case there exists an open session to the device, mark it as expired.
     */
    CHIP_ERROR CloseSession();

    CHIP_ERROR Disconnect() override { return CloseSession(); }

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
    bool IsActive() const override { return mActive; }

    void SetActive(bool active) { mActive = active; }

    bool IsSecureConnected() const { return IsActive() && mState == ConnectionState::SecureConnected; }

    bool IsSessionSetupInProgress() const { return IsActive() && mState == ConnectionState::Connecting; }

    void Reset();

    NodeId GetDeviceId() const override { return mDeviceId; }

    bool MatchesSession(SessionHandle session) const { return mSecureSession.HasValue() && mSecureSession.Value() == session; }

    chip::Optional<SessionHandle> GetSecureSession() const override { return mSecureSession; }

    Messaging::ExchangeManager * GetExchangeManager() const override { return mExchangeMgr; }

    void SetAddress(const Inet::IPAddress & deviceAddr) { mDeviceAddress.SetIPAddress(deviceAddr); }

    PASESessionSerializable & GetPairing() { return mPairing; }

    uint8_t GetNextSequenceNumber() { return mSequenceNumber++; };
    void AddResponseHandler(uint8_t seqNum, Callback::Cancelable * onSuccessCallback, Callback::Cancelable * onFailureCallback,
                            app::TLVDataFilter tlvDataFilter = nullptr);
    void CancelResponseHandler(uint8_t seqNum);
    void AddReportHandler(EndpointId endpoint, ClusterId cluster, AttributeId attribute, Callback::Cancelable * onReportCallback,
                          app::TLVDataFilter tlvDataFilter) override;

    // This two functions are pretty tricky, it is used to bridge the response, we need to implement interaction model delegate
    // on the app side instead of register callbacks here. The IM delegate can provide more infomation then callback and it is
    // type-safe.
    // TODO: Implement interaction model delegate in the application.
    void AddIMResponseHandler(app::CommandSender * commandObj, Callback::Cancelable * onSuccessCallback,
                              Callback::Cancelable * onFailureCallback) override;
    void CancelIMResponseHandler(app::CommandSender * commandObj) override;

    CHIP_ERROR LoadSecureSessionParametersIfNeeded()
    {
        bool loadedSecureSession = false;
        return LoadSecureSessionParametersIfNeeded(loadedSecureSession);
    };

    CHIP_ERROR SetCSRNonce(ByteSpan csrNonce)
    {
        VerifyOrReturnError(csrNonce.size() == sizeof(mCSRNonce), CHIP_ERROR_INVALID_ARGUMENT);
        memcpy(mCSRNonce, csrNonce.data(), csrNonce.size());
        return CHIP_NO_ERROR;
    }

    ByteSpan GetCSRNonce() const { return ByteSpan(mCSRNonce, sizeof(mCSRNonce)); }

    CHIP_ERROR SetAttestationNonce(ByteSpan attestationNonce)
    {
        VerifyOrReturnError(attestationNonce.size() == sizeof(mAttestationNonce), CHIP_ERROR_INVALID_ARGUMENT);
        memcpy(mAttestationNonce, attestationNonce.data(), attestationNonce.size());
        return CHIP_NO_ERROR;
    }

    ByteSpan GetAttestationNonce() const { return ByteSpan(mAttestationNonce, sizeof(mAttestationNonce)); }

    bool AreCredentialsAvailable() const { return (mDAC != nullptr && mDACLen != 0); }

    ByteSpan GetDAC() const { return ByteSpan(mDAC, mDACLen); }
    ByteSpan GetPAI() const { return ByteSpan(mPAI, mPAILen); }

    CHIP_ERROR SetDAC(const ByteSpan & dac);
    CHIP_ERROR SetPAI(const ByteSpan & pai);

    MutableByteSpan GetMutableNOCCert() { return MutableByteSpan(mNOCCertBuffer, sizeof(mNOCCertBuffer)); }

    CHIP_ERROR SetNOCCertBufferSize(size_t new_size);

    ByteSpan GetNOCCert() const { return ByteSpan(mNOCCertBuffer, mNOCCertBufferSize); }

    MutableByteSpan GetMutableICACert() { return MutableByteSpan(mICACertBuffer, sizeof(mICACertBuffer)); }

    CHIP_ERROR SetICACertBufferSize(size_t new_size);

    ByteSpan GetICACert() const { return ByteSpan(mICACertBuffer, mICACertBufferSize); }

    Controller::DeviceControllerInteractionModelDelegate * GetInteractionModelDelegate() override { return mpIMDelegate; };

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

    SessionManager * mSessionManager = nullptr;

    Messaging::ExchangeManager * mExchangeMgr = nullptr;

    Optional<SessionHandle> mSecureSession = Optional<SessionHandle>::Missing();

    Controller::DeviceControllerInteractionModelDelegate * mpIMDelegate = nullptr;

    uint8_t mSequenceNumber = 0;

    uint32_t mLocalMessageCounter = 0;
    uint32_t mPeerMessageCounter  = 0;

    app::CHIPDeviceCallbacksMgr & mCallbacksMgr = app::CHIPDeviceCallbacksMgr::GetInstance();

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

    void ReleaseDAC();
    void ReleasePAI();

    FabricIndex mFabricIndex = kUndefinedFabricIndex;

    FabricTable * mFabricsTable = nullptr;

    PersistentStorageDelegate * mStorageDelegate = nullptr;

    // TODO: Offload Nonces and DAC/PAI into a new struct
    uint8_t mCSRNonce[kOpCSRNonceLength];
    uint8_t mAttestationNonce[kAttestationNonceLength];

    uint8_t * mDAC   = nullptr;
    uint16_t mDACLen = 0;
    uint8_t * mPAI   = nullptr;
    uint16_t mPAILen = 0;

    uint8_t mNOCCertBuffer[Credentials::kMaxCHIPCertLength];
    size_t mNOCCertBufferSize = 0;

    uint8_t mICACertBuffer[Credentials::kMaxCHIPCertLength];
    size_t mICACertBufferSize = 0;

    SessionIDAllocator * mIDAllocator = nullptr;
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

} // namespace chip
