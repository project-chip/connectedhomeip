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

#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif
#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif

// module header, comes first
#include <controller/CHIPDeviceController.h>

#if CONFIG_DEVICE_LAYER
#include <platform/CHIPDeviceLayer.h>
#endif

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

namespace chip {
namespace Controller {

CHIP_ERROR Device::SendMessage(System::PacketBuffer * buffer)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    System::PacketBuffer * resend = nullptr;

    VerifyOrExit(buffer != nullptr, err = CHIP_ERROR_INVALID_ARGUMENT);

    // If there is no secure connection to the device, try establishing it
    if (mState != kConnectionState_SecureConnected)
    {
        err = EstablishSecureSession();
        SuccessOrExit(err);
    }
    else
    {
        // Secure connection already exists
        // Hold on to the buffer, in case session resumption and resend is needed
        buffer->AddRef();
        resend = buffer;
    }

    err    = mSessionManager->SendMessage(mDeviceId, buffer);
    buffer = nullptr;
    ChipLogDetail(Controller, "SendMessage returned %d", err);

    // The send could fail due to network timeouts (e.g. broken pipe)
    // Try sesion resumption if needed
    if (err != CHIP_NO_ERROR && resend != nullptr)
    {
        err = ResumeSecureSession();
        SuccessOrExit(err);

        err    = mSessionManager->SendMessage(mDeviceId, resend);
        resend = nullptr;
        ChipLogDetail(Controller, "Re-SendMessage returned %d", err);
        SuccessOrExit(err);
    }

exit:

    if (buffer != nullptr)
    {
        PacketBuffer::Free(buffer);
    }

    if (resend != nullptr)
    {
        PacketBuffer::Free(resend);
    }

    return err;
}

CHIP_ERROR Device::Serialize(SerializedDevice & output)
{
    CHIP_ERROR error       = CHIP_NO_ERROR;
    uint16_t serializedLen = 0;
    SerializableDevice serializable;

    VerifyOrExit(BASE64_ENCODED_LEN(sizeof(serializable)) <= sizeof(output.inner), error = CHIP_ERROR_INVALID_ARGUMENT);

    CHIP_ZERO_AT(serializable);

    memmove(&serializable.mOpsCreds, &mPairing, sizeof(mPairing));
    serializable.mDeviceId   = Encoding::LittleEndian::HostSwap64(mDeviceId);
    serializable.mDevicePort = Encoding::LittleEndian::HostSwap16(mDevicePort);
    mDeviceAddr.ToString(Uint8::to_char(serializable.mDeviceAddr), INET6_ADDRSTRLEN);

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
    deserializedLen =
        Base64Decode(Uint8::to_const_char(input.inner), static_cast<uint16_t>(len), Uint8::to_uchar((uint8_t *) &serializable));

    VerifyOrExit(deserializedLen > 0, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(deserializedLen <= sizeof(serializable), error = CHIP_ERROR_INVALID_ARGUMENT);

    VerifyOrExit(IPAddress::FromString(Uint8::to_const_char(serializable.mDeviceAddr), mDeviceAddr),
                 error = CHIP_ERROR_INVALID_ADDRESS);

    memmove(&mPairing, &serializable.mOpsCreds, sizeof(mPairing));
    mDeviceId   = Encoding::LittleEndian::HostSwap64(serializable.mDeviceId);
    mDevicePort = Encoding::LittleEndian::HostSwap16(serializable.mDevicePort);

exit:
    return error;
}

void Device::OnMessageReceived(const PacketHeader & header, const PayloadHeader & payloadHeader,
                               Transport::PeerConnectionState * state, System::PacketBuffer * msgBuf, SecureSessionMgrBase * mgr)
{
    if (mState == kConnectionState_SecureConnected && mStatusDelegate != nullptr)
    {
        mStatusDelegate->OnMessage(msgBuf);
    }
}

CHIP_ERROR Device::EstablishSecureSession()
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    SecurePairingSession pairingSession;

    if (mSessionManager == nullptr || mState == kConnectionState_SecureConnected)
    {
        ExitNow(err = CHIP_ERROR_INCORRECT_STATE);
    }

    err = pairingSession.FromSerializable(mPairing);
    SuccessOrExit(err);

    err = mSessionManager->NewPairing(
        Optional<Transport::PeerAddress>::Value(Transport::PeerAddress::UDP(mDeviceAddr, mDevicePort, mInterface)),
        &pairingSession);
    SuccessOrExit(err);

    mState = kConnectionState_SecureConnected;

exit:

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "EstablishSecureSession returning error %d\n", err);
    }
    return err;
}

CHIP_ERROR Device::ResumeSecureSession()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    if (mState == kConnectionState_SecureConnected)
    {
        mState = kConnectionState_NotConnected;
    }

    VerifyOrExit(mSessionManager != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    err = mSessionManager->ResetTransport(Transport::UdpListenParameters(mInetLayer).SetAddressType(mDeviceAddr.Type()));
    SuccessOrExit(err);

    err = EstablishSecureSession();
    SuccessOrExit(err);

exit:

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "ResumeSecureSession returning error %d\n", err);
    }
    return err;
}

} // namespace Controller
} // namespace chip
