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
#include <app/util/CHIPDeviceCallbacksMgr.h>
#include <app/util/attribute-filter.h>
#include <app/util/basic-types.h>
#include <controller-clusters/zap-generated/CHIPClientCallbacks.h>
#include <controller/CHIPDeviceControllerSystemState.h>
#include <lib/core/CHIPCallback.h>
#include <lib/core/CHIPCore.h>
#include <lib/support/DLLUtil.h>
#include <messaging/ExchangeContext.h>
#include <messaging/ExchangeMgr.h>
#include <messaging/Flags.h>
#include <protocols/secure_channel/PASESession.h>
#include <protocols/secure_channel/SessionIDAllocator.h>
#include <transport/SessionManager.h>
#include <transport/TransportMgr.h>
#include <transport/raw/MessageHeader.h>
#include <transport/raw/UDP.h>

#if CONFIG_NETWORK_LAYER_BLE
#include <ble/BleLayer.h>
#include <transport/raw/BLE.h>
#endif

namespace chip {

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

class CommissioneeDeviceProxy : public DeviceProxy, public SessionReleaseDelegate
{
public:
    ~CommissioneeDeviceProxy();
    CommissioneeDeviceProxy() {}
    CommissioneeDeviceProxy(const CommissioneeDeviceProxy &) = delete;

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
        mSessionManager = params.sessionManager;
        mExchangeMgr    = params.exchangeMgr;
        mInetLayer      = params.inetLayer;
        mFabricIndex    = fabric;
        mIDAllocator    = params.idAllocator;
        mpIMDelegate    = params.imDelegate;
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
     *   Called when the associated session is released
     *
     *   The receiver should release all resources associated with the connection.
     *
     * @param session A handle to the secure session
     */
    void OnSessionReleased(SessionHandle session) override;

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

    bool IsSecureConnected() const override { return IsActive() && mState == ConnectionState::SecureConnected; }

    bool IsSessionSetupInProgress() const { return IsActive() && mState == ConnectionState::Connecting; }

    void Reset();

    NodeId GetDeviceId() const override { return mDeviceId; }

    bool MatchesSession(SessionHandle session) const { return mSecureSession.HasValue() && mSecureSession.Value() == session; }

    chip::Optional<SessionHandle> GetSecureSession() const override { return mSecureSession; }

    Messaging::ExchangeManager * GetExchangeManager() const override { return mExchangeMgr; }

    void SetAddress(const Inet::IPAddress & deviceAddr) { mDeviceAddress.SetIPAddress(deviceAddr); }

    PASESession & GetPairing() { return mPairing; }

    uint8_t GetNextSequenceNumber() override { return mSequenceNumber++; };

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

    PASESession mPairing;

    SessionManager * mSessionManager = nullptr;

    Messaging::ExchangeManager * mExchangeMgr = nullptr;

    Optional<SessionHandle> mSecureSession = Optional<SessionHandle>::Missing();

    Controller::DeviceControllerInteractionModelDelegate * mpIMDelegate = nullptr;

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

    void ReleaseDAC();
    void ReleasePAI();

    FabricIndex mFabricIndex = kUndefinedFabricIndex;

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

} // namespace chip
