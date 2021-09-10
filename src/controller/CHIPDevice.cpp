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
 *    This file contains implementation of Device class. The objects of this
 *    class will be used by Controller applications to interact with CHIP
 *    devices. The class provides mechanism to construct, send and receive
 *    messages to and from the corresponding CHIP devices.
 */

#include <controller/CHIPDevice.h>

#include <controller-clusters/zap-generated/CHIPClusters.h>

#if CONFIG_DEVICE_LAYER
#include <platform/CHIPDeviceLayer.h>
#endif

#if CHIP_SYSTEM_CONFIG_USE_LWIP
#include <lwip/tcp.h>
#include <lwip/tcpip.h>
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

#include <app/CommandSender.h>
#include <app/ReadPrepareParams.h>
#include <app/util/DataModelHandler.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/CHIPEncoding.h>
#include <lib/core/CHIPSafeCasts.h>
#include <lib/support/Base64.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/ErrorStr.h>
#include <lib/support/PersistentStorageMacros.h>
#include <lib/support/SafeInt.h>
#include <lib/support/TypeTraits.h>
#include <lib/support/logging/CHIPLogging.h>
#include <protocols/Protocols.h>
#include <protocols/service_provisioning/ServiceProvisioning.h>
#include <system/TLVPacketBufferBackingStore.h>
#include <transport/MessageCounter.h>
#include <transport/PeerMessageCounter.h>

using namespace chip::Inet;
using namespace chip::System;
using namespace chip::Callback;

namespace chip {
namespace Controller {
CHIP_ERROR Device::SendMessage(Protocols::Id protocolId, uint8_t msgType, Messaging::SendFlags sendFlags,
                               System::PacketBufferHandle && buffer)
{
    System::PacketBufferHandle resend;
    bool loadedSecureSession = false;

    VerifyOrReturnError(!buffer.IsNull(), CHIP_ERROR_INVALID_ARGUMENT);

    ReturnErrorOnFailure(LoadSecureSessionParametersIfNeeded(loadedSecureSession));

    Messaging::ExchangeContext * exchange = mExchangeMgr->NewContext(mSecureSession.Value(), nullptr);
    VerifyOrReturnError(exchange != nullptr, CHIP_ERROR_NO_MEMORY);

    if (!loadedSecureSession)
    {
        // Secure connection already existed
        // Hold on to the buffer, in case session resumption and resend is needed
        // Cloning data, instead of increasing the ref count, as the original
        // buffer might get modified by lower layers before the send fails. So,
        // that buffer cannot be used for resends.
        resend = buffer.CloneData();
    }

    // TODO(#5675): This code is temporary, and must be updated to use the IM API. Currently, we use a temporary Protocol
    // TempZCL to carry over legacy ZCL messages.  We need to set flag kFromInitiator to allow receiver to deliver message to
    // corresponding unsolicited message handler.
    sendFlags.Set(Messaging::SendMessageFlags::kFromInitiator);
    exchange->SetDelegate(this);

    CHIP_ERROR err = exchange->SendMessage(protocolId, msgType, std::move(buffer), sendFlags);

    buffer = nullptr;
    ChipLogDetail(Controller, "SendMessage returned %s", ErrorStr(err));

    // The send could fail due to network timeouts (e.g. broken pipe)
    // Try session resumption if needed
    if (err != CHIP_NO_ERROR && !resend.IsNull() && mState == ConnectionState::SecureConnected)
    {
        mState = ConnectionState::NotConnected;

        ReturnErrorOnFailure(LoadSecureSessionParameters(ResetTransport::kYes));

        err = exchange->SendMessage(protocolId, msgType, std::move(resend), sendFlags);
        ChipLogDetail(Controller, "Re-SendMessage returned %s", ErrorStr(err));
    }

    if (err != CHIP_NO_ERROR)
    {
        exchange->Close();
    }

    return err;
}

CHIP_ERROR Device::LoadSecureSessionParametersIfNeeded(bool & didLoad)
{
    didLoad = false;

    // If there is no secure connection to the device, try establishing it
    if (mState != ConnectionState::SecureConnected)
    {
        ReturnErrorOnFailure(LoadSecureSessionParameters(ResetTransport::kNo));
        didLoad = true;
    }
    else
    {
        if (mSecureSession.HasValue())
        {
            Transport::PeerConnectionState * connectionState = mSessionManager->GetPeerConnectionState(mSecureSession.Value());
            // Check if the connection state has the correct transport information
            if (connectionState->GetPeerAddress().GetTransportType() == Transport::Type::kUndefined)
            {
                mState = ConnectionState::NotConnected;
                ReturnErrorOnFailure(LoadSecureSessionParameters(ResetTransport::kNo));
                didLoad = true;
            }
        }
        else
        {
            mState = ConnectionState::NotConnected;
            ReturnErrorOnFailure(LoadSecureSessionParameters(ResetTransport::kNo));
            didLoad = true;
        }
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR Device::SendCommands(app::CommandSender * commandObj)
{
    bool loadedSecureSession = false;
    ReturnErrorOnFailure(LoadSecureSessionParametersIfNeeded(loadedSecureSession));
    VerifyOrReturnError(commandObj != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    return commandObj->SendCommandRequest(mDeviceId, mFabricIndex, mSecureSession);
}

CHIP_ERROR Device::Serialize(SerializedDevice & output)
{
    SerializableDevice serializable;

    static_assert(BASE64_ENCODED_LEN(sizeof(serializable)) <= sizeof(output.inner),
                  "Size of serializable should be <= size of output");

    CHIP_ZERO_AT(serializable);
    CHIP_ZERO_AT(output);

    serializable.mOpsCreds    = mPairing;
    serializable.mDeviceId    = Encoding::LittleEndian::HostSwap64(mDeviceId);
    serializable.mDevicePort  = Encoding::LittleEndian::HostSwap16(mDeviceAddress.GetPort());
    serializable.mFabricIndex = Encoding::LittleEndian::HostSwap16(mFabricIndex);

    // The connection state could be null if the device is moving from PASE connection to CASE connection.
    // The device parameters (e.g. mDeviceOperationalCertProvisioned) are updated during this transition.
    // The state during this transistion is being persisted so that the next access of the device will
    // trigger the CASE based secure session.
    if (mSecureSession.HasValue())
    {
        Transport::PeerConnectionState * connectionState = mSessionManager->GetPeerConnectionState(mSecureSession.Value());
        const uint32_t localMessageCounter = connectionState->GetSessionMessageCounter().GetLocalMessageCounter().Value();
        const uint32_t peerMessageCounter  = connectionState->GetSessionMessageCounter().GetPeerMessageCounter().GetCounter();

        serializable.mLocalMessageCounter = Encoding::LittleEndian::HostSwap32(localMessageCounter);
        serializable.mPeerMessageCounter  = Encoding::LittleEndian::HostSwap32(peerMessageCounter);
    }
    else
    {
        serializable.mLocalMessageCounter = 0;
        serializable.mPeerMessageCounter  = 0;
    }

    serializable.mDeviceOperationalCertProvisioned = (mDeviceOperationalCertProvisioned) ? 1 : 0;

    serializable.mDeviceTransport = to_underlying(mDeviceAddress.GetTransportType());

    ReturnErrorOnFailure(Inet::GetInterfaceName(mDeviceAddress.GetInterface(), Uint8::to_char(serializable.mInterfaceName),
                                                sizeof(serializable.mInterfaceName)));
    static_assert(sizeof(serializable.mDeviceAddr) <= INET6_ADDRSTRLEN, "Size of device address must fit within INET6_ADDRSTRLEN");
    mDeviceAddress.GetIPAddress().ToString(Uint8::to_char(serializable.mDeviceAddr), sizeof(serializable.mDeviceAddr));

    const uint16_t serializedLen = chip::Base64Encode(Uint8::to_const_uchar(reinterpret_cast<uint8_t *>(&serializable)),
                                                      static_cast<uint16_t>(sizeof(serializable)), Uint8::to_char(output.inner));
    VerifyOrReturnError(serializedLen > 0, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(serializedLen < sizeof(output.inner), CHIP_ERROR_INVALID_ARGUMENT);
    output.inner[serializedLen] = '\0';

    return CHIP_NO_ERROR;
}

CHIP_ERROR Device::Deserialize(const SerializedDevice & input)
{
    SerializableDevice serializable;
    constexpr size_t maxlen = BASE64_ENCODED_LEN(sizeof(serializable));
    const size_t len        = strnlen(Uint8::to_const_char(&input.inner[0]), maxlen);

    VerifyOrReturnError(len < sizeof(SerializedDevice), CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(CanCastTo<uint16_t>(len), CHIP_ERROR_INVALID_ARGUMENT);

    CHIP_ZERO_AT(serializable);
    const uint16_t deserializedLen = Base64Decode(Uint8::to_const_char(input.inner), static_cast<uint16_t>(len),
                                                  Uint8::to_uchar(reinterpret_cast<uint8_t *>(&serializable)));

    VerifyOrReturnError(deserializedLen > 0, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(deserializedLen <= sizeof(serializable), CHIP_ERROR_INVALID_ARGUMENT);

    // The second parameter to FromString takes the strlen value. We are subtracting 1
    // from the sizeof(serializable.mDeviceAddr) to account for null termination, since
    // strlen doesn't include null character in the size.
    Inet::IPAddress ipAddress = {};
    VerifyOrReturnError(
        IPAddress::FromString(Uint8::to_const_char(serializable.mDeviceAddr), sizeof(serializable.mDeviceAddr) - 1, ipAddress),
        CHIP_ERROR_INVALID_ADDRESS);

    mPairing             = serializable.mOpsCreds;
    mDeviceId            = Encoding::LittleEndian::HostSwap64(serializable.mDeviceId);
    const uint16_t port  = Encoding::LittleEndian::HostSwap16(serializable.mDevicePort);
    const uint16_t index = Encoding::LittleEndian::HostSwap16(serializable.mFabricIndex);
    mLocalMessageCounter = Encoding::LittleEndian::HostSwap32(serializable.mLocalMessageCounter);
    mPeerMessageCounter  = Encoding::LittleEndian::HostSwap32(serializable.mPeerMessageCounter);

    VerifyOrReturnError(CanCastTo<FabricIndex>(index), CHIP_ERROR_INVALID_ARGUMENT);
    mFabricIndex = static_cast<FabricIndex>(index);

    // TODO - Remove the hack that's incrementing message counter while deserializing device
    // This hack was added as a quick workaround for TE3 testing. The commissioning code
    // is closing the exchange after the device has already been serialized and persisted to the storage.
    // While closing the exchange, the outstanding ack gets sent to the device, thus incrementing
    // the local message counter. As the device information was stored prior to sending the ack, it now has
    // the old counter value (which is 1 less than the updated counter).
    mLocalMessageCounter++;

    mDeviceOperationalCertProvisioned = (serializable.mDeviceOperationalCertProvisioned != 0);

    // The InterfaceNameToId() API requires initialization of mInterface, and lock/unlock of
    // LwIP stack.
    Inet::InterfaceId interfaceId = INET_NULL_INTERFACEID;
    if (serializable.mInterfaceName[0] != '\0')
    {
#if CHIP_SYSTEM_CONFIG_USE_LWIP
        LOCK_TCPIP_CORE();
#endif
        CHIP_ERROR inetErr = Inet::InterfaceNameToId(Uint8::to_const_char(serializable.mInterfaceName), interfaceId);
#if CHIP_SYSTEM_CONFIG_USE_LWIP
        UNLOCK_TCPIP_CORE();
#endif
        ReturnErrorOnFailure(inetErr);
    }

    static_assert(std::is_same<std::underlying_type<decltype(mDeviceAddress.GetTransportType())>::type, uint8_t>::value,
                  "The underlying type of Transport::Type is not uint8_t.");
    switch (static_cast<Transport::Type>(serializable.mDeviceTransport))
    {
    case Transport::Type::kUdp:
        mDeviceAddress = Transport::PeerAddress::UDP(ipAddress, port, interfaceId);
        break;
    case Transport::Type::kBle:
        mDeviceAddress = Transport::PeerAddress::BLE();
        break;
    case Transport::Type::kTcp:
    case Transport::Type::kUndefined:
    default:
        return CHIP_ERROR_INTERNAL;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR Device::Persist()
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    if (mStorageDelegate != nullptr)
    {
        SerializedDevice serialized;
        ReturnErrorOnFailure(Serialize(serialized));

        // TODO: no need to base-64 the serialized values AGAIN
        PERSISTENT_KEY_OP(GetDeviceId(), kPairedDeviceKeyPrefix, key,
                          error = mStorageDelegate->SyncSetKeyValue(key, serialized.inner, sizeof(serialized.inner)));
        if (error != CHIP_NO_ERROR)
        {
            ChipLogError(Controller, "Failed to persist device %" CHIP_ERROR_FORMAT, error.Format());
        }
    }
    return error;
}

void Device::OnNewConnection(SessionHandle session)
{
    mState = ConnectionState::SecureConnected;
    mSecureSession.SetValue(session);

    // Reset the message counters here because this is the first time we get a handle to the secure session.
    // Since CHIPDevices can be serialized/deserialized in the middle of what is conceptually a single PASE session
    // we need to restore the session counters along with the session information.
    Transport::PeerConnectionState * connectionState = mSessionManager->GetPeerConnectionState(mSecureSession.Value());
    VerifyOrReturn(connectionState != nullptr);
    MessageCounter & localCounter = connectionState->GetSessionMessageCounter().GetLocalMessageCounter();
    if (localCounter.SetCounter(mLocalMessageCounter) != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "Unable to restore local counter to %" PRIu32, mLocalMessageCounter);
    }
    Transport::PeerMessageCounter & peerCounter = connectionState->GetSessionMessageCounter().GetPeerMessageCounter();
    peerCounter.SetCounter(mPeerMessageCounter);
}

void Device::OnConnectionExpired(SessionHandle session)
{
    VerifyOrReturn(mSecureSession.HasValue() && mSecureSession.Value() == session,
                   ChipLogDetail(Controller, "Connection expired, but it doesn't match the current session"));
    mState = ConnectionState::NotConnected;
    mSecureSession.ClearValue();
}

CHIP_ERROR Device::OnMessageReceived(Messaging::ExchangeContext * exchange, const PacketHeader & header,
                                     const PayloadHeader & payloadHeader, System::PacketBufferHandle && msgBuf)
{
    if (mState == ConnectionState::SecureConnected)
    {
        if (mStatusDelegate != nullptr)
        {
            mStatusDelegate->OnMessage(std::move(msgBuf));
        }
        else
        {
            HandleDataModelMessage(exchange, std::move(msgBuf));
        }
    }
    return CHIP_NO_ERROR;
}

void Device::OnResponseTimeout(Messaging::ExchangeContext * ec) {}

void Device::OnOpenPairingWindowSuccessResponse(void * context)
{
    ChipLogProgress(Controller, "Successfully opened pairing window on the device");
}

void Device::OnOpenPairingWindowFailureResponse(void * context, uint8_t status)
{
    ChipLogError(Controller, "Failed to open pairing window on the device. Status %d", status);
}

CHIP_ERROR Device::OpenCommissioningWindow(uint16_t timeout, uint32_t iteration, CommissioningWindowOption option,
                                           const ByteSpan & salt, SetupPayload & setupPayload)
{
    constexpr EndpointId kAdministratorCommissioningClusterEndpoint = 0;

    chip::Controller::AdministratorCommissioningCluster cluster;
    cluster.Associate(this, kAdministratorCommissioningClusterEndpoint);

    Callback::Cancelable * successCallback = mOpenPairingSuccessCallback.Cancel();
    Callback::Cancelable * failureCallback = mOpenPairingFailureCallback.Cancel();

    if (option != CommissioningWindowOption::kOriginalSetupCode)
    {
        bool randomSetupPIN = (option == CommissioningWindowOption::kTokenWithRandomPIN);
        PASEVerifier verifier;

        ReturnErrorOnFailure(
            PASESession::GeneratePASEVerifier(verifier, iteration, salt, randomSetupPIN, setupPayload.setUpPINCode));

        uint8_t serializedVerifier[2 * kSpake2p_WS_Length];
        VerifyOrReturnError(sizeof(serializedVerifier) == sizeof(verifier), CHIP_ERROR_INTERNAL);

        memcpy(serializedVerifier, verifier.mW0, kSpake2p_WS_Length);
        memcpy(&serializedVerifier[kSpake2p_WS_Length], verifier.mL, kSpake2p_WS_Length);

        ReturnErrorOnFailure(cluster.OpenCommissioningWindow(successCallback, failureCallback, timeout,
                                                             ByteSpan(serializedVerifier, sizeof(serializedVerifier)),
                                                             setupPayload.discriminator, iteration, salt, mPAKEVerifierID++));
    }
    else
    {
        ReturnErrorOnFailure(cluster.OpenBasicCommissioningWindow(successCallback, failureCallback, timeout));
    }

    setupPayload.version               = 0;
    setupPayload.rendezvousInformation = RendezvousInformationFlags(RendezvousInformationFlag::kBLE);

    return CHIP_NO_ERROR;
}

CHIP_ERROR Device::OpenPairingWindow(uint16_t timeout, CommissioningWindowOption option, SetupPayload & setupPayload)
{
    ByteSpan salt(reinterpret_cast<const uint8_t *>(kSpake2pKeyExchangeSalt), strlen(kSpake2pKeyExchangeSalt));

    return OpenCommissioningWindow(timeout, kPBKDFMinimumIterations, option, salt, setupPayload);
}

CHIP_ERROR Device::CloseSession()
{
    ReturnErrorCodeIf(mState != ConnectionState::SecureConnected, CHIP_ERROR_INCORRECT_STATE);
    if (mSecureSession.HasValue())
    {
        mSessionManager->ExpirePairing(mSecureSession.Value());
    }
    mState = ConnectionState::NotConnected;
    return CHIP_NO_ERROR;
}

CHIP_ERROR Device::UpdateAddress(const Transport::PeerAddress & addr)
{
    bool didLoad;

    mDeviceAddress = addr;

    ReturnErrorOnFailure(LoadSecureSessionParametersIfNeeded(didLoad));

    if (!mSecureSession.HasValue())
    {
        // Nothing needs to be done here.  It's not an error to not have a
        // connectionState.  For one thing, we could have gotten an different
        // UpdateAddress already and that caused connections to be torn down and
        // whatnot.
        return CHIP_NO_ERROR;
    }

    Transport::PeerConnectionState * connectionState = mSessionManager->GetPeerConnectionState(mSecureSession.Value());
    connectionState->SetPeerAddress(addr);

    return CHIP_NO_ERROR;
}

void Device::Reset()
{
    if (IsActive() && mStorageDelegate != nullptr && mSessionManager != nullptr)
    {
        // If a session can be found, persist the device so that we track the newest message counter values

        if (mSecureSession.HasValue())
        {
            Persist();
        }
    }

    SetActive(false);
    mCASESession.Clear();

    mState          = ConnectionState::NotConnected;
    mSessionManager = nullptr;
    mStatusDelegate = nullptr;
    mInetLayer      = nullptr;
#if CONFIG_NETWORK_LAYER_BLE
    mBleLayer = nullptr;
#endif
    if (mExchangeMgr)
    {
        // Ensure that any exchange contexts we have open get closed now,
        // because we don't want them to call back in to us after this
        // point.
        mExchangeMgr->CloseAllContextsForDelegate(this);
    }
    mExchangeMgr = nullptr;
}

CHIP_ERROR Device::LoadSecureSessionParameters(ResetTransport resetNeeded)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    PASESession pairingSession;

    if (mSessionManager == nullptr || mState == ConnectionState::SecureConnected)
    {
        ExitNow(err = CHIP_ERROR_INCORRECT_STATE);
    }

    if (mState == ConnectionState::Connecting)
    {
        ExitNow(err = CHIP_NO_ERROR);
    }

    if (resetNeeded == ResetTransport::kYes)
    {
        err = mTransportMgr->ResetTransport(
            Transport::UdpListenParameters(mInetLayer).SetAddressType(kIPAddressType_IPv6).SetListenPort(mListenPort)
#if INET_CONFIG_ENABLE_IPV4
                ,
            Transport::UdpListenParameters(mInetLayer).SetAddressType(kIPAddressType_IPv4).SetListenPort(mListenPort)
#endif
#if CONFIG_NETWORK_LAYER_BLE
                ,
            Transport::BleListenParameters(mBleLayer)
#endif
        );
        SuccessOrExit(err);
    }

    if (IsOperationalCertProvisioned())
    {
        err = WarmupCASESession();
        SuccessOrExit(err);
    }
    else
    {
        err = pairingSession.FromSerializable(mPairing);
        SuccessOrExit(err);

        err = mSessionManager->NewPairing(Optional<Transport::PeerAddress>::Value(mDeviceAddress), mDeviceId, &pairingSession,
                                          SecureSession::SessionRole::kInitiator, mFabricIndex);
        SuccessOrExit(err);
    }

exit:

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "LoadSecureSessionParameters returning error %" CHIP_ERROR_FORMAT, err.Format());
    }
    return err;
}

bool Device::GetAddress(Inet::IPAddress & addr, uint16_t & port) const
{
    if (mState == ConnectionState::NotConnected)
        return false;

    addr = mDeviceAddress.GetIPAddress();
    port = mDeviceAddress.GetPort();
    return true;
}

void Device::OperationalCertProvisioned()
{
    VerifyOrReturn(!mDeviceOperationalCertProvisioned,
                   ChipLogDetail(Controller, "Operational certificates already provisioned for this device"));

    ChipLogDetail(Controller, "Enabling CASE session establishment for the device");
    mDeviceOperationalCertProvisioned = true;

    Persist();
    CloseSession();
    mState = ConnectionState::NotConnected;
}

CHIP_ERROR Device::WarmupCASESession()
{
    VerifyOrReturnError(mDeviceOperationalCertProvisioned, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mState == ConnectionState::NotConnected, CHIP_NO_ERROR);

    Messaging::ExchangeContext * exchange =
        mExchangeMgr->NewContext(SessionHandle::TemporaryUnauthenticatedSession(), &mCASESession);
    VerifyOrReturnError(exchange != nullptr, CHIP_ERROR_INTERNAL);

    ReturnErrorOnFailure(mCASESession.MessageDispatch().Init(mSessionManager->GetTransportManager()));
    mCASESession.MessageDispatch().SetPeerAddress(mDeviceAddress);

    uint16_t keyID = 0;
    ReturnErrorOnFailure(mIDAllocator->Allocate(keyID));

    mLocalMessageCounter = 0;
    mPeerMessageCounter  = 0;

    Transport::FabricInfo * fabric = mFabricsTable->FindFabricWithIndex(mFabricIndex);
    ReturnErrorCodeIf(fabric == nullptr, CHIP_ERROR_INCORRECT_STATE);

    ReturnErrorOnFailure(mCASESession.EstablishSession(mDeviceAddress, fabric, mDeviceId, keyID, exchange, this));

    mState = ConnectionState::Connecting;

    return CHIP_NO_ERROR;
}

void Device::OnSessionEstablishmentError(CHIP_ERROR error)
{
    mState = ConnectionState::NotConnected;
    mIDAllocator->Free(mCASESession.GetLocalKeyId());

    Cancelable ready;
    mConnectionFailure.DequeueAll(ready);
    while (ready.mNext != &ready)
    {
        Callback::Callback<OnDeviceConnectionFailure> * cb =
            Callback::Callback<OnDeviceConnectionFailure>::FromCancelable(ready.mNext);

        cb->Cancel();
        cb->mCall(cb->mContext, GetDeviceId(), error);
    }
}

void Device::OnSessionEstablished()
{
    CHIP_ERROR err = mSessionManager->NewPairing(Optional<Transport::PeerAddress>::Value(mDeviceAddress), mDeviceId, &mCASESession,
                                                 SecureSession::SessionRole::kInitiator, mFabricIndex);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "Failed in setting up CASE secure channel: err %s", ErrorStr(err));
        OnSessionEstablishmentError(err);
        return;
    }

    Cancelable ready;
    mConnectionSuccess.DequeueAll(ready);
    while (ready.mNext != &ready)
    {
        Callback::Callback<OnDeviceConnected> * cb = Callback::Callback<OnDeviceConnected>::FromCancelable(ready.mNext);

        cb->Cancel();
        cb->mCall(cb->mContext, this);
    }
}

CHIP_ERROR Device::EstablishConnectivity(Callback::Callback<OnDeviceConnected> * onConnection,
                                         Callback::Callback<OnDeviceConnectionFailure> * onFailure)
{
    bool loadedSecureSession = false;
    ReturnErrorOnFailure(LoadSecureSessionParametersIfNeeded(loadedSecureSession));

    if (loadedSecureSession)
    {
        if (IsOperationalCertProvisioned())
        {
            if (onConnection != nullptr)
            {
                mConnectionSuccess.Enqueue(onConnection->Cancel());
            }

            if (onFailure != nullptr)
            {
                mConnectionFailure.Enqueue(onFailure->Cancel());
            }
        }
        else
        {
            if (onConnection != nullptr)
            {
                onConnection->mCall(onConnection->mContext, this);
            }
        }
    }

    return CHIP_NO_ERROR;
}

void Device::AddResponseHandler(uint8_t seqNum, Callback::Cancelable * onSuccessCallback, Callback::Cancelable * onFailureCallback,
                                app::TLVDataFilter tlvDataFilter)
{
    mCallbacksMgr.AddResponseCallback(mDeviceId, seqNum, onSuccessCallback, onFailureCallback, tlvDataFilter);
}

void Device::CancelResponseHandler(uint8_t seqNum)
{
    mCallbacksMgr.CancelResponseCallback(mDeviceId, seqNum);
}

void Device::AddIMResponseHandler(app::CommandSender * commandObj, Callback::Cancelable * onSuccessCallback,
                                  Callback::Cancelable * onFailureCallback)
{
    // We are using the pointer to command sender object as the identifier of command transactions. This makes sense as long as
    // there are only one active command transaction on one command sender object. This is a bit tricky, we try to assume that
    // chip::NodeId is uint64_t so the pointer can be used as a NodeId for CallbackMgr.
    static_assert(std::is_same<chip::NodeId, uint64_t>::value, "chip::NodeId is not uint64_t");
    chip::NodeId transactionId = reinterpret_cast<chip::NodeId>(commandObj);
    mCallbacksMgr.AddResponseCallback(transactionId, 0 /* seqNum, always 0 for IM before #6559 */, onSuccessCallback,
                                      onFailureCallback);
}

void Device::CancelIMResponseHandler(app::CommandSender * commandObj)
{
    // We are using the pointer to command sender object as the identifier of command transactions. This makes sense as long as
    // there are only one active command transaction on one command sender object. This is a bit tricky, we try to assume that
    // chip::NodeId is uint64_t so the pointer can be used as a NodeId for CallbackMgr.
    static_assert(std::is_same<chip::NodeId, uint64_t>::value, "chip::NodeId is not uint64_t");
    chip::NodeId transactionId = reinterpret_cast<chip::NodeId>(commandObj);
    mCallbacksMgr.CancelResponseCallback(transactionId, 0 /* seqNum, always 0 for IM before #6559 */);
}

void Device::AddReportHandler(EndpointId endpoint, ClusterId cluster, AttributeId attribute,
                              Callback::Cancelable * onReportCallback)
{
    mCallbacksMgr.AddReportCallback(mDeviceId, endpoint, cluster, attribute, onReportCallback);
}

CHIP_ERROR Device::SendReadAttributeRequest(app::AttributePathParams aPath, Callback::Cancelable * onSuccessCallback,
                                            Callback::Cancelable * onFailureCallback, app::TLVDataFilter aTlvDataFilter)
{
    bool loadedSecureSession = false;
    uint8_t seqNum           = GetNextSequenceNumber();
    aPath.mNodeId            = GetDeviceId();

    ReturnErrorOnFailure(LoadSecureSessionParametersIfNeeded(loadedSecureSession));

    if (onSuccessCallback != nullptr || onFailureCallback != nullptr)
    {
        AddResponseHandler(seqNum, onSuccessCallback, onFailureCallback, aTlvDataFilter);
    }
    // The application context is used to identify different requests from client applicaiton the type of it is intptr_t, here we
    // use the seqNum.
    chip::app::ReadPrepareParams readPrepareParams(mSecureSession.Value());
    readPrepareParams.mpAttributePathParamsList    = &aPath;
    readPrepareParams.mAttributePathParamsListSize = 1;
    CHIP_ERROR err =
        chip::app::InteractionModelEngine::GetInstance()->SendReadRequest(readPrepareParams, seqNum /* application context */);
    if (err != CHIP_NO_ERROR)
    {
        CancelResponseHandler(seqNum);
    }
    return err;
}

CHIP_ERROR Device::SendWriteAttributeRequest(app::WriteClientHandle aHandle, Callback::Cancelable * onSuccessCallback,
                                             Callback::Cancelable * onFailureCallback)
{
    bool loadedSecureSession = false;
    uint8_t seqNum           = GetNextSequenceNumber();
    CHIP_ERROR err           = CHIP_NO_ERROR;

    aHandle->SetAppIdentifier(seqNum);
    ReturnErrorOnFailure(LoadSecureSessionParametersIfNeeded(loadedSecureSession));

    if (onSuccessCallback != nullptr || onFailureCallback != nullptr)
    {
        AddResponseHandler(seqNum, onSuccessCallback, onFailureCallback);
    }
    if ((err = aHandle.SendWriteRequest(GetDeviceId(), 0, mSecureSession)) != CHIP_NO_ERROR)
    {
        CancelResponseHandler(seqNum);
    }
    return err;
}

Device::~Device()
{
    if (mExchangeMgr)
    {
        // Ensure that any exchange contexts we have open get closed now,
        // because we don't want them to call back in to us after this
        // point.
        mExchangeMgr->CloseAllContextsForDelegate(this);
    }
}

CHIP_ERROR Device::ReduceNOCChainBufferSize(size_t new_size)
{
    ReturnErrorCodeIf(new_size > sizeof(mNOCChainBuffer), CHIP_ERROR_INVALID_ARGUMENT);
    mNOCChainBufferSize = new_size;
    return CHIP_NO_ERROR;
}

} // namespace Controller
} // namespace chip
