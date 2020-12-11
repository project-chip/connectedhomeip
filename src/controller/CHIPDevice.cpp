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

CHIP_ERROR Device::Serialize(SerializedDevice & output)
{
    CHIP_ERROR error       = CHIP_NO_ERROR;
    uint16_t serializedLen = 0;
    SerializableDevice serializable;

    static_assert(BASE64_ENCODED_LEN(sizeof(serializable)) <= sizeof(output.inner),
                  "Size of serializable should be <= size of output");

    CHIP_ZERO_AT(serializable);

    memmove(&serializable.mOpsCreds, &mSecureSessionParameters, sizeof(serializable.mOpsCreds));
    serializable.mDeviceId   = Encoding::LittleEndian::HostSwap64(mDeviceId);

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

    memmove(&mSecureSessionParameters, &serializable.mOpsCreds, sizeof(mSecureSessionParameters));
    mDeviceId   = Encoding::LittleEndian::HostSwap64(serializable.mDeviceId);
exit:
    return error;
}

CHIP_ERROR Device::EstablishCaseSession()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    if (mExchangeManager == nullptr || mState == ConnectionState::SecureConnected)
    {
        ExitNow(err = CHIP_ERROR_INCORRECT_STATE);
    }

    {
        Messaging::ChannelBuilder builder;
        builder.SetPeerNodeId(mDeviceId)
            //.SetLocalKeyId(mSecureSessionParameters.mLocalKeyId)
            //.SetPeerKeyId(mSecureSessionParameters.mPeerKeyId)
        ;
        mCaseChannel = mExchangeManager->EstablishChannel(builder, this);
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "LoadSecureSessionParameters returning error %d\n", err);
    }
    return err;
}

void Device::OnEstablished()
{
    // device connected
    mStatusDelegate->OnStatusChange();
}

void Device::OnClosed()
{
    // device disconnected
    mStatusDelegate->OnStatusChange();
}

void Device::OnFail(CHIP_ERROR err)
{
    // device failure
    mStatusDelegate->OnStatusChange();
}

} // namespace Controller
} // namespace chip
