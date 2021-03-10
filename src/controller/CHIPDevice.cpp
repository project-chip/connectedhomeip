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
#include <support/Base64.h>
#include <support/CHIPMem.h>
#include <support/CodeUtils.h>
#include <support/ErrorStr.h>
#include <support/ReturnMacros.h>
#include <support/SafeInt.h>
#include <support/logging/CHIPLogging.h>
#include <system/TLVPacketBufferBackingStore.h>

using namespace chip::Inet;
using namespace chip::System;
using namespace chip::Callback;

namespace chip {
namespace Controller {

CHIP_ERROR Device::SendMessage(System::PacketBufferHandle buffer, PayloadHeader & payloadHeader)
{
    System::PacketBufferHandle resend;
    bool loadedSecureSession = false;

    VerifyOrReturnError(mSessionManager != nullptr, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(!buffer.IsNull(), CHIP_ERROR_INVALID_ARGUMENT);

    ReturnErrorOnFailure(LoadSecureSessionParametersIfNeeded(loadedSecureSession));

    if (!loadedSecureSession)
    {
        // Secure connection already existed
        // Hold on to the buffer, in case session resumption and resend is needed
        // Cloning data, instead of increasing the ref count, as the original
        // buffer might get modified by lower layers before the send fails. So,
        // that buffer cannot be used for resends.
        resend = buffer.CloneData();
    }

    CHIP_ERROR err = mSessionManager->SendMessage(mSecureSession, payloadHeader, std::move(buffer));

    buffer = nullptr;
    ChipLogDetail(Controller, "SendMessage returned %d", err);

    // The send could fail due to network timeouts (e.g. broken pipe)
    // Try session resumption if needed
    if (err != CHIP_NO_ERROR && !resend.IsNull() && mState == ConnectionState::SecureConnected)
    {
        mState = ConnectionState::NotConnected;

        ReturnErrorOnFailure(LoadSecureSessionParameters(ResetTransport::kYes));

        err = mSessionManager->SendMessage(mSecureSession, std::move(resend));
        ChipLogDetail(Controller, "Re-SendMessage returned %d", err);
        ReturnErrorOnFailure(err);
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

CHIP_ERROR Device::SendMessage(System::PacketBufferHandle buffer)
{
    PayloadHeader unusedHeader;
    return SendMessage(std::move(buffer), unusedHeader);
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
    serializable.mDevicePort = Encoding::LittleEndian::HostSwap16(mDeviceUdpAddress.GetPort());
    serializable.mAdminId    = Encoding::LittleEndian::HostSwap16(mAdminId);
    SuccessOrExit(error = Inet::GetInterfaceName(mDeviceUdpAddress.GetInterface(), Uint8::to_char(serializable.mInterfaceName),
                                                 sizeof(serializable.mInterfaceName)));
    static_assert(sizeof(serializable.mDeviceAddr) <= INET6_ADDRSTRLEN, "Size of device address must fit within INET6_ADDRSTRLEN");
    mDeviceUdpAddress.GetIPAddress().ToString(Uint8::to_char(serializable.mDeviceAddr), sizeof(serializable.mDeviceAddr));

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

    mDeviceUdpAddress = Transport::PeerAddress::UDP(ipAddress, port, interfaceId);

exit:
    return error;
}

void Device::OnNewConnection(SecureSessionHandle session, SecureSessionMgr * mgr)
{
    mState         = ConnectionState::SecureConnected;
    mSecureSession = session;
}

void Device::OnConnectionExpired(SecureSessionHandle session, SecureSessionMgr * mgr)
{
    mState         = ConnectionState::NotConnected;
    mSecureSession = SecureSessionHandle{};
}

void Device::OnMessageReceived(const PacketHeader & header, const PayloadHeader & payloadHeader, SecureSessionHandle session,
                               System::PacketBufferHandle msgBuf, SecureSessionMgr * mgr)
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
    writer.ImplicitProfileId = chip::Protocols::kProtocol_ServiceProvisioning;

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

    PayloadHeader header;

    header.SetMessageType(chip::Protocols::kProtocol_ServiceProvisioning, 0);

    ReturnErrorOnFailure(SendMessage(std::move(outBuffer), header));

    setupPayload.version               = 1;
    setupPayload.rendezvousInformation = RendezvousInformationFlags::kBLE;

    return CHIP_NO_ERROR;
}

CHIP_ERROR Device::UpdateAddress(const Transport::PeerAddress & addr)
{
    bool didLoad;

    VerifyOrReturnError(addr.GetTransportType() == Transport::Type::kUdp, CHIP_ERROR_INVALID_ADDRESS);
    ReturnErrorOnFailure(LoadSecureSessionParametersIfNeeded(didLoad));

    Transport::PeerConnectionState * connectionState = mSessionManager->GetPeerConnectionState(mSecureSession);
    VerifyOrReturnError(connectionState != nullptr, CHIP_ERROR_INCORRECT_STATE);

    mDeviceUdpAddress = addr;
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
        );
        SuccessOrExit(err);
    }

    err = mSessionManager->NewPairing(Optional<Transport::PeerAddress>::Value(mDeviceUdpAddress), mDeviceId, &pairingSession,
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

    addr = mDeviceUdpAddress.GetIPAddress();
    port = mDeviceUdpAddress.GetPort();
    return true;
}

void Device::AddResponseHandler(uint8_t seqNum, Callback::Cancelable * onSuccessCallback, Callback::Cancelable * onFailureCallback)
{
    mCallbacksMgr.AddResponseCallback(mDeviceId, seqNum, onSuccessCallback, onFailureCallback);
}

void Device::AddReportHandler(EndpointId endpoint, ClusterId cluster, AttributeId attribute,
                              Callback::Cancelable * onReportCallback)
{
    mCallbacksMgr.AddReportCallback(mDeviceId, endpoint, cluster, attribute, onReportCallback);
}

} // namespace Controller
} // namespace chip
