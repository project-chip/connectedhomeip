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

#if CONFIG_DEVICE_LAYER
#include <platform/CHIPDeviceLayer.h>
#endif

#if CHIP_SYSTEM_CONFIG_USE_LWIP
#include <lwip/tcp.h>
#include <lwip/tcpip.h>
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

#include <app/CommandSender.h>
#include <app/server/DataModelHandler.h>
#include <core/CHIPCore.h>
#include <core/CHIPEncoding.h>
#include <core/CHIPSafeCasts.h>
#include <protocols/Protocols.h>
#include <support/Base64.h>
#include <support/CHIPMem.h>
#include <support/CodeUtils.h>
#include <support/ErrorStr.h>
#include <support/SafeInt.h>
#include <support/logging/CHIPLogging.h>
#include <system/TLVPacketBufferBackingStore.h>

using namespace chip::Inet;
using namespace chip::System;
using namespace chip::Callback;

namespace chip {
namespace Controller {
// TODO: This is a placeholder delegate for exchange context created in Device::SendMessage()
//       Delete this class when Device::SendMessage() is obsoleted.
class DeviceExchangeDelegate : public Messaging::ExchangeDelegate
{
    void OnMessageReceived(Messaging::ExchangeContext * ec, const PacketHeader & packetHeader, const PayloadHeader & payloadHeader,
                           System::PacketBufferHandle payload) override
    {}
    void OnResponseTimeout(Messaging::ExchangeContext * ec) override {}
};

CHIP_ERROR Device::SendMessage(Protocols::Id protocolId, uint8_t msgType, System::PacketBufferHandle buffer)
{
    System::PacketBufferHandle resend;
    bool loadedSecureSession = false;
    Messaging::SendFlags sendFlags;

    VerifyOrReturnError(!buffer.IsNull(), CHIP_ERROR_INVALID_ARGUMENT);

    ReturnErrorOnFailure(LoadSecureSessionParametersIfNeeded(loadedSecureSession));

    Messaging::ExchangeContext * exchange = mExchangeMgr->NewContext(mSecureSession, nullptr);
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

    // TODO(#5675): This code is temporary, and must be updated to use the IM API. Currenlty, we use a temporary Protocol
    // TempZCL to carry over legacy ZCL messages, use an ephemeral exchange to send message and use its unsolicited message
    // handler to receive messages. We need to set flag kFromInitiator to allow receiver to deliver message to corresponding
    // unsolicited message handler, and we also need to set flag kNoAutoRequestAck since there is no persistent exchange to
    // receive the ack message. This logic need to be deleted after we converting all legacy ZCL messages to IM messages.
    sendFlags.Set(Messaging::SendMessageFlags::kFromInitiator).Set(Messaging::SendMessageFlags::kNoAutoRequestAck);

    DeviceExchangeDelegate delegate;
    exchange->SetDelegate(&delegate);

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
        ChipLogDetail(Controller, "Re-SendMessage returned %d", err);
        ReturnErrorOnFailure(err);
    }

    if (exchange != nullptr)
    {
        exchange->Close();
    }

    return CHIP_NO_ERROR;
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
        Transport::PeerConnectionState * connectionState = nullptr;
        connectionState                                  = mSessionManager->GetPeerConnectionState(mSecureSession);

        // Check if the connection state has the correct transport information
        if (connectionState == nullptr || connectionState->GetPeerAddress().GetTransportType() == Transport::Type::kUndefined ||
            connectionState->GetTransport() != nullptr)
        {
            mState = ConnectionState::NotConnected;
            ReturnErrorOnFailure(LoadSecureSessionParameters(ResetTransport::kNo));
            didLoad = true;
        }
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR Device::SendCommands()
{
    bool loadedSecureSession = false;
    ReturnErrorOnFailure(LoadSecureSessionParametersIfNeeded(loadedSecureSession));
    VerifyOrReturnError(mCommandSender != nullptr, CHIP_ERROR_INCORRECT_STATE);
    return mCommandSender->SendCommandRequest(mDeviceId, mAdminId);
}

CHIP_ERROR Device::Serialize(SerializedDevice & output)
{
    CHIP_ERROR error       = CHIP_NO_ERROR;
    uint16_t serializedLen = 0;
    SerializableDevice serializable;

    static_assert(BASE64_ENCODED_LEN(sizeof(serializable)) <= sizeof(output.inner),
                  "Size of serializable should be <= size of output");

    CHIP_ZERO_AT(serializable);

    serializable.mOpsCreds   = mPairing;
    serializable.mDeviceId   = Encoding::LittleEndian::HostSwap64(mDeviceId);
    serializable.mDevicePort = Encoding::LittleEndian::HostSwap16(mDeviceAddress.GetPort());
    serializable.mAdminId    = Encoding::LittleEndian::HostSwap16(mAdminId);

    static_assert(std::is_same<std::underlying_type<decltype(mDeviceAddress.GetTransportType())>::type, uint8_t>::value,
                  "The underlying type of Transport::Type is not uint8_t.");
    serializable.mDeviceTransport = static_cast<uint8_t>(mDeviceAddress.GetTransportType());

    SuccessOrExit(error = Inet::GetInterfaceName(mDeviceAddress.GetInterface(), Uint8::to_char(serializable.mInterfaceName),
                                                 sizeof(serializable.mInterfaceName)));
    static_assert(sizeof(serializable.mDeviceAddr) <= INET6_ADDRSTRLEN, "Size of device address must fit within INET6_ADDRSTRLEN");
    mDeviceAddress.GetIPAddress().ToString(Uint8::to_char(serializable.mDeviceAddr), sizeof(serializable.mDeviceAddr));

    serializedLen = chip::Base64Encode(Uint8::to_const_uchar(reinterpret_cast<uint8_t *>(&serializable)),
                                       static_cast<uint16_t>(sizeof(serializable)), Uint8::to_char(output.inner));
    VerifyOrExit(serializedLen > 0, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(serializedLen < sizeof(output.inner), error = CHIP_ERROR_INVALID_ARGUMENT);
    output.inner[serializedLen] = '\0';

exit:
    return error;
}

CHIP_ERROR Device::Deserialize(const SerializedDevice & input)
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    SerializableDevice serializable;
    size_t maxlen            = BASE64_ENCODED_LEN(sizeof(serializable));
    size_t len               = strnlen(Uint8::to_const_char(&input.inner[0]), maxlen);
    uint16_t deserializedLen = 0;

    VerifyOrExit(len < sizeof(SerializedDevice), error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(CanCastTo<uint16_t>(len), error = CHIP_ERROR_INVALID_ARGUMENT);

    CHIP_ZERO_AT(serializable);
    deserializedLen = Base64Decode(Uint8::to_const_char(input.inner), static_cast<uint16_t>(len),
                                   Uint8::to_uchar(reinterpret_cast<uint8_t *>(&serializable)));

    VerifyOrExit(deserializedLen > 0, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(deserializedLen <= sizeof(serializable), error = CHIP_ERROR_INVALID_ARGUMENT);

    Inet::IPAddress ipAddress;
    uint16_t port;
    Inet::InterfaceId interfaceId;

    // The second parameter to FromString takes the strlen value. We are subtracting 1
    // from the sizeof(serializable.mDeviceAddr) to account for null termination, since
    // strlen doesn't include null character in the size.
    VerifyOrExit(
        IPAddress::FromString(Uint8::to_const_char(serializable.mDeviceAddr), sizeof(serializable.mDeviceAddr) - 1, ipAddress),
        error = CHIP_ERROR_INVALID_ADDRESS);

    mPairing  = serializable.mOpsCreds;
    mDeviceId = Encoding::LittleEndian::HostSwap64(serializable.mDeviceId);
    port      = Encoding::LittleEndian::HostSwap16(serializable.mDevicePort);
    mAdminId  = Encoding::LittleEndian::HostSwap16(serializable.mAdminId);

    // The InterfaceNameToId() API requires initialization of mInterface, and lock/unlock of
    // LwIP stack.
    interfaceId = INET_NULL_INTERFACEID;
    if (serializable.mInterfaceName[0] != '\0')
    {
#if CHIP_SYSTEM_CONFIG_USE_LWIP
        LOCK_TCPIP_CORE();
#endif
        INET_ERROR inetErr = Inet::InterfaceNameToId(Uint8::to_const_char(serializable.mInterfaceName), interfaceId);
#if CHIP_SYSTEM_CONFIG_USE_LWIP
        UNLOCK_TCPIP_CORE();
#endif
        VerifyOrExit(CHIP_NO_ERROR == inetErr, error = CHIP_ERROR_INTERNAL);
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
        ExitNow(error = CHIP_ERROR_INTERNAL);
    }

exit:
    return error;
}

void Device::OnNewConnection(SecureSessionHandle session)
{
    mState         = ConnectionState::SecureConnected;
    mSecureSession = session;
}

void Device::OnConnectionExpired(SecureSessionHandle session)
{
    mState         = ConnectionState::NotConnected;
    mSecureSession = SecureSessionHandle{};
}

void Device::OnMessageReceived(const PacketHeader & header, const PayloadHeader & payloadHeader, System::PacketBufferHandle msgBuf)
{
    if (mState == ConnectionState::SecureConnected)
    {
        if (mStatusDelegate != nullptr)
        {
            mStatusDelegate->OnMessage(std::move(msgBuf));
        }
        else
        {
            HandleDataModelMessage(mDeviceId, std::move(msgBuf));
        }
    }
}

CHIP_ERROR Device::OpenPairingWindow(uint32_t timeout, PairingWindowOption option, SetupPayload & setupPayload)
{
    // TODO: This code is temporary, and must be updated to use the Cluster API.
    // Issue: https://github.com/project-chip/connectedhomeip/issues/4725

    // Construct and send "open pairing window" message to the device
    System::PacketBufferHandle buf = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);
    System::PacketBufferTLVWriter writer;

    writer.Init(std::move(buf));
    writer.ImplicitProfileId = chip::Protocols::ServiceProvisioning::Id.ToTLVProfileId();

    ReturnErrorOnFailure(writer.Put(TLV::ProfileTag(writer.ImplicitProfileId, 1), timeout));

    if (option != PairingWindowOption::kOriginalSetupCode)
    {
        ReturnErrorOnFailure(writer.Put(TLV::ProfileTag(writer.ImplicitProfileId, 2), setupPayload.discriminator));

        PASEVerifier verifier;
        bool randomSetupPIN = (option == PairingWindowOption::kTokenWithRandomPIN);
        ReturnErrorOnFailure(PASESession::GeneratePASEVerifier(verifier, randomSetupPIN, setupPayload.setUpPINCode));
        ReturnErrorOnFailure(writer.PutBytes(TLV::ProfileTag(writer.ImplicitProfileId, 3),
                                             reinterpret_cast<const uint8_t *>(verifier), sizeof(verifier)));
    }

    System::PacketBufferHandle outBuffer;
    ReturnErrorOnFailure(writer.Finalize(&outBuffer));

    ReturnErrorOnFailure(SendMessage(Protocols::ServiceProvisioning::Id, 0, std::move(outBuffer)));

    setupPayload.version               = 0;
    setupPayload.rendezvousInformation = RendezvousInformationFlags(RendezvousInformationFlag::kBLE);

    return CHIP_NO_ERROR;
}

CHIP_ERROR Device::UpdateAddress(const Transport::PeerAddress & addr)
{
    bool didLoad;

    ReturnErrorOnFailure(LoadSecureSessionParametersIfNeeded(didLoad));

    Transport::PeerConnectionState * connectionState = mSessionManager->GetPeerConnectionState(mSecureSession);
    VerifyOrReturnError(connectionState != nullptr, CHIP_ERROR_INCORRECT_STATE);

    mDeviceAddress = addr;
    connectionState->SetPeerAddress(addr);

    return CHIP_NO_ERROR;
}

CHIP_ERROR Device::LoadSecureSessionParameters(ResetTransport resetNeeded)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    PASESession pairingSession;

    if (mSessionManager == nullptr || mState == ConnectionState::SecureConnected)
    {
        ExitNow(err = CHIP_ERROR_INCORRECT_STATE);
    }

    err = pairingSession.FromSerializable(mPairing);
    SuccessOrExit(err);

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

    err = mSessionManager->NewPairing(Optional<Transport::PeerAddress>::Value(mDeviceAddress), mDeviceId, &pairingSession,
                                      SecureSessionMgr::PairingDirection::kInitiator, mAdminId);
    SuccessOrExit(err);

exit:

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "LoadSecureSessionParameters returning error %d\n", err);
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

void Device::AddResponseHandler(uint8_t seqNum, Callback::Cancelable * onSuccessCallback, Callback::Cancelable * onFailureCallback)
{
    mCallbacksMgr.AddResponseCallback(mDeviceId, seqNum, onSuccessCallback, onFailureCallback);
}

void Device::CancelResponseHandler(uint8_t seqNum)
{
    mCallbacksMgr.CancelResponseCallback(mDeviceId, seqNum);
}

void Device::AddReportHandler(EndpointId endpoint, ClusterId cluster, AttributeId attribute,
                              Callback::Cancelable * onReportCallback)
{
    mCallbacksMgr.AddReportCallback(mDeviceId, endpoint, cluster, attribute, onReportCallback);
}

void Device::InitCommandSender()
{
    if (mCommandSender != nullptr)
    {
        mCommandSender->Shutdown();
        mCommandSender = nullptr;
    }
#if CHIP_ENABLE_INTERACTION_MODEL
    CHIP_ERROR err = chip::app::InteractionModelEngine::GetInstance()->NewCommandSender(&mCommandSender);
    ChipLogFunctError(err);
#endif
}

} // namespace Controller
} // namespace chip
