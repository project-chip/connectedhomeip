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

#include <core/CHIPCore.h>
#include <core/CHIPEncoding.h>
#include <core/CHIPSafeCasts.h>
#include <support/Base64.h>
#include <support/CHIPMem.h>
#include <support/CodeUtils.h>
#include <support/ErrorStr.h>
#include <support/SafeInt.h>
#include <support/logging/CHIPLogging.h>

using namespace chip::Inet;
using namespace chip::System;
using namespace chip::Callback;

namespace chip {
namespace Controller {

CHIP_ERROR Device::SendMessage(System::PacketBufferHandle buffer)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    System::PacketBufferHandle resend;

    VerifyOrExit(mSessionManager != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(!buffer.IsNull(), err = CHIP_ERROR_INVALID_ARGUMENT);

    // If there is no secure connection to the device, try establishing it
    if (mState != ConnectionState::SecureConnected)
    {
        err = LoadSecureSessionParameters(ResetTransport::kNo);
        SuccessOrExit(err);
    }
    else
    {
        // Secure connection already exists
        // Hold on to the buffer, in case session resumption and resend is needed
        resend = buffer.Retain();
    }

    err    = mSessionManager->SendMessage(mSecureSession, std::move(buffer));
    buffer = nullptr;
    ChipLogDetail(Controller, "SendMessage returned %d", err);

    // The send could fail due to network timeouts (e.g. broken pipe)
    // Try sesion resumption if needed
    if (err != CHIP_NO_ERROR && !resend.IsNull() && mState == ConnectionState::SecureConnected)
    {
        mState = ConnectionState::NotConnected;

        err = LoadSecureSessionParameters(ResetTransport::kYes);
        SuccessOrExit(err);

        err = mSessionManager->SendMessage(mSecureSession, std::move(resend));
        ChipLogDetail(Controller, "Re-SendMessage returned %d", err);
        SuccessOrExit(err);
    }

exit:
    return err;
}

CHIP_ERROR Device::Serialize(SerializedDevice & output)
{
    CHIP_ERROR error       = CHIP_NO_ERROR;
    uint16_t serializedLen = 0;
    SerializableDevice serializable;

    static_assert(BASE64_ENCODED_LEN(sizeof(serializable)) <= sizeof(output.inner),
                  "Size of serializable should be <= size of output");

    CHIP_ZERO_AT(serializable);

    memmove(&serializable.mOpsCreds, &mPairing, sizeof(mPairing));
    serializable.mDeviceId   = Encoding::LittleEndian::HostSwap64(mDeviceId);
    serializable.mDevicePort = Encoding::LittleEndian::HostSwap16(mDevicePort);
    VerifyOrExit(
        CHIP_NO_ERROR ==
            Inet::GetInterfaceName(mInterface, Uint8::to_char(serializable.mInterfaceName), sizeof(serializable.mInterfaceName)),
        error = CHIP_ERROR_INTERNAL);
    static_assert(sizeof(serializable.mDeviceAddr) <= INET6_ADDRSTRLEN, "Size of device address must fit within INET6_ADDRSTRLEN");
    mDeviceAddr.ToString(Uint8::to_char(serializable.mDeviceAddr), sizeof(serializable.mDeviceAddr));

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

    // The second parameter to FromString takes the strlen value. We are subtracting 1
    // from the sizeof(serializable.mDeviceAddr) to account for null termination, since
    // strlen doesn't include null character in the size.
    VerifyOrExit(
        IPAddress::FromString(Uint8::to_const_char(serializable.mDeviceAddr), sizeof(serializable.mDeviceAddr) - 1, mDeviceAddr),
        error = CHIP_ERROR_INVALID_ADDRESS);

    memmove(&mPairing, &serializable.mOpsCreds, sizeof(mPairing));
    mDeviceId   = Encoding::LittleEndian::HostSwap64(serializable.mDeviceId);
    mDevicePort = Encoding::LittleEndian::HostSwap16(serializable.mDevicePort);

    // The InterfaceNameToId() API requires initialization of mInterface, and lock/unlock of
    // LwIP stack.
    mInterface = INET_NULL_INTERFACEID;
    if (serializable.mInterfaceName[0] != '\0')
    {
#if CHIP_SYSTEM_CONFIG_USE_LWIP
        LOCK_TCPIP_CORE();
#endif
        INET_ERROR inetErr = Inet::InterfaceNameToId(Uint8::to_const_char(serializable.mInterfaceName), mInterface);
#if CHIP_SYSTEM_CONFIG_USE_LWIP
        UNLOCK_TCPIP_CORE();
#endif
        VerifyOrExit(CHIP_NO_ERROR == inetErr, error = CHIP_ERROR_INTERNAL);
    }

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

        // TODO: The following callback processing will need further work
        //       1. The response needs to be parsed as per cluster definition. The response callback
        //          should carry the parsed response values.
        //       2. The reports callbacks should also be called with the parsed reports.
        //       3. The callbacks would be tracked using exchange context. On receiving the
        //          message, the exchange context in the message should be matched against
        //          the registered callbacks.
        // GitHub issue: https://github.com/project-chip/connectedhomeip/issues/3910
        Cancelable * ca = mResponses.mNext;
        while (ca != &mResponses)
        {
            Callback::Callback<> * cb = Callback::Callback<>::FromCancelable(ca);
            // Let's advance to the next cancelable, as the current one will get removed
            // from the list (and once removed, its next will point to itself)
            ca = ca->mNext;
            if (cb != nullptr)
            {
                ChipLogProgress(Controller, "Dispatching response callback %p", cb);
                cb->Cancel();
                cb->mCall(cb->mContext);
            }
        }

        ca = mReports.mNext;
        while (ca != &mReports)
        {
            Callback::Callback<> * cb = Callback::Callback<>::FromCancelable(ca);
            // Let's advance to the next cancelable, as the current one might get removed
            // from the list in the callback (and if removed, its next will point to itself)
            ca = ca->mNext;
            if (cb != nullptr)
            {
                ChipLogProgress(Controller, "Dispatching report callback %p", cb);
                cb->mCall(cb->mContext);
            }
        }
    }
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

    err = mSessionManager->NewPairing(
        Optional<Transport::PeerAddress>::Value(Transport::PeerAddress::UDP(mDeviceAddr, mDevicePort, mInterface)), mDeviceId,
        &pairingSession);
    SuccessOrExit(err);

exit:

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "LoadSecureSessionParameters returning error %d\n", err);
    }
    return err;
}

bool Device::GetIpAddress(Inet::IPAddress & addr) const
{
    if (mState == ConnectionState::NotConnected)
        return false;
    addr = mDeviceAddr;
    return true;
}

void Device::AddResponseHandler(EndpointId endpoint, ClusterId cluster, Callback::Callback<> * onResponse)
{
    CallbackInfo info                 = { endpoint, cluster };
    Callback::Cancelable * cancelable = onResponse->Cancel();

    static_assert(sizeof(info) <= sizeof(cancelable->mInfoScalar), "Size of CallbackInfo should be <= size of mInfoScalar");

    cancelable->mInfoScalar = 0;
    memmove(&cancelable->mInfoScalar, &info, sizeof(info));
    mResponses.Enqueue(cancelable);
}

void Device::AddReportHandler(EndpointId endpoint, ClusterId cluster, Callback::Callback<> * onReport)
{
    CallbackInfo info                 = { endpoint, cluster };
    Callback::Cancelable * cancelable = onReport->Cancel();

    static_assert(sizeof(info) <= sizeof(cancelable->mInfoScalar), "Size of CallbackInfo should be <= size of mInfoScalar");

    cancelable->mInfoScalar = 0;
    memmove(&cancelable->mInfoScalar, &info, sizeof(info));
    mReports.Enqueue(cancelable);
}

} // namespace Controller
} // namespace chip
