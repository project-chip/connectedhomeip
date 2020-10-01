/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include <core/CHIPEncoding.h>
#include <core/CHIPSafeCasts.h>
#include <platform/internal/DeviceNetworkInfo.h>
#include <protocols/CHIPProtocols.h>
#include <support/CodeUtils.h>
#include <support/ErrorStr.h>
#include <support/SafeInt.h>
#include <transport/NetworkProvisioning.h>

#if CONFIG_DEVICE_LAYER
#include <platform/CHIPDeviceLayer.h>
#endif

namespace chip {

CHIP_ERROR NetworkProvisioning::HandleNetworkProvisioningMessage(uint8_t msgType, PacketBuffer * msgBuf,
                                                                 RendezvousSessionDelegate * delegate)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    VerifyOrExit(delegate != nullptr, err = CHIP_ERROR_INCORRECT_STATE);

    switch (msgType)
    {
    case NetworkProvisioning::MsgTypes::kWiFiAssociationRequest: {
        char SSID[chip::DeviceLayer::Internal::kMaxWiFiSSIDLength];
        char passwd[chip::DeviceLayer::Internal::kMaxWiFiKeyLength];
        BufBound bbufSSID(Uint8::from_char(SSID), chip::DeviceLayer::Internal::kMaxWiFiSSIDLength);
        BufBound bbufPW(Uint8::from_char(passwd), chip::DeviceLayer::Internal::kMaxWiFiKeyLength);

        const uint8_t * buffer = msgBuf->Start();
        size_t len             = msgBuf->DataLength();
        size_t offset          = 0;

        err = DecodeString(&buffer[offset], len - offset, bbufSSID, offset);
        SuccessOrExit(err);

        err = DecodeString(&buffer[offset], len - offset, bbufPW, offset);
        SuccessOrExit(err);

        delegate->OnRendezvousProvisionNetwork(SSID, passwd);
    }
    break;

    case NetworkProvisioning::MsgTypes::kIPAddressAssigned: {
        if (!IPAddress::FromString(Uint8::to_const_char(msgBuf->Start()), msgBuf->DataLength(), mDeviceAddress))
        {
            ExitNow(err = CHIP_ERROR_INVALID_ADDRESS);
        }
    }
    break;

    default:
        ExitNow(err = CHIP_ERROR_INVALID_MESSAGE_TYPE);
        break;
    };

exit:
    if (err != CHIP_NO_ERROR)
    {
        mDelegate->OnNetworkProvisioningError(err);
    }
    else
    {
        // Network provisioning handshake requires only one message exchange in either direction.
        // If the current message handling did not result in an error, network provisioning is
        // complete.
        mDelegate->OnNetworkProvisioningComplete();
    }
    return err;
}

CHIP_ERROR NetworkProvisioning::EncodeString(const char * str, BufBound & bbuf)
{
    CHIP_ERROR err  = CHIP_NO_ERROR;
    size_t length   = strlen(str);
    uint16_t u16len = static_cast<uint16_t>(length);
    VerifyOrExit(CanCastTo<uint16_t>(length), err = CHIP_ERROR_INVALID_ARGUMENT);

    bbuf.PutLE16(u16len);
    bbuf.Put(str);

exit:
    return err;
}

CHIP_ERROR NetworkProvisioning::DecodeString(const uint8_t * input, size_t input_len, BufBound & bbuf, size_t & consumed)
{
    CHIP_ERROR err  = CHIP_NO_ERROR;
    uint16_t length = 0;

    VerifyOrExit(input_len >= sizeof(uint16_t), err = CHIP_ERROR_BUFFER_TOO_SMALL);
    length   = chip::Encoding::LittleEndian::Get16(input);
    consumed = sizeof(uint16_t);

    VerifyOrExit(input_len - consumed >= length, err = CHIP_ERROR_BUFFER_TOO_SMALL);
    bbuf.Put(&input[consumed], length);

    consumed += bbuf.Written();
    bbuf.Put('\0');

    VerifyOrExit(bbuf.Fit(), err = CHIP_ERROR_BUFFER_TOO_SMALL);

exit:
    return err;
}

CHIP_ERROR NetworkProvisioning::SendIPAddress(const IPAddress & addr)
{
    CHIP_ERROR err                = CHIP_NO_ERROR;
    System::PacketBuffer * buffer = System::PacketBuffer::New();
    char * addrStr                = addr.ToString(Uint8::to_char(buffer->Start()), buffer->AvailableDataLength());
    size_t addrLen                = 0;

    VerifyOrExit(mDelegate != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(addrStr != nullptr, err = CHIP_ERROR_INVALID_ADDRESS);

    addrLen = strlen(addrStr) + 1;

    VerifyOrExit(CanCastTo<uint16_t>(addrLen), err = CHIP_ERROR_INVALID_ARGUMENT);
    buffer->SetDataLength(static_cast<uint16_t>(addrLen));

    err = mDelegate->SendSecureMessage(Protocols::kChipProtocol_NetworkProvisioning,
                                       NetworkProvisioning::MsgTypes::kIPAddressAssigned, buffer);
    SuccessOrExit(err);

exit:
    if (CHIP_NO_ERROR != err)
    {
        PacketBuffer::Free(buffer);
    }
    return err;
}

CHIP_ERROR NetworkProvisioning::SendNetworkCredentials(const char * ssid, const char * passwd)
{
    CHIP_ERROR err                = CHIP_NO_ERROR;
    System::PacketBuffer * buffer = System::PacketBuffer::New();
    BufBound bbuf(buffer->Start(), buffer->TotalLength());

    VerifyOrExit(mDelegate != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    SuccessOrExit(EncodeString(ssid, bbuf));
    SuccessOrExit(EncodeString(passwd, bbuf));
    VerifyOrExit(bbuf.Fit(), err = CHIP_ERROR_BUFFER_TOO_SMALL);

    VerifyOrExit(CanCastTo<uint16_t>(bbuf.Written()), err = CHIP_ERROR_INVALID_ARGUMENT);
    buffer->SetDataLength(static_cast<uint16_t>(bbuf.Written()));

    err = mDelegate->SendSecureMessage(Protocols::kChipProtocol_NetworkProvisioning,
                                       NetworkProvisioning::MsgTypes::kWiFiAssociationRequest, buffer);
    SuccessOrExit(err);

exit:
    if (CHIP_NO_ERROR != err)
    {
        PacketBuffer::Free(buffer);
    }
    return err;
}

#if CONFIG_DEVICE_LAYER
void NetworkProvisioning::ConnectivityHandler(const DeviceLayer::ChipDeviceEvent * event, intptr_t arg)
{
    NetworkProvisioning * session = reinterpret_cast<NetworkProvisioning *>(arg);

    VerifyOrExit(session != nullptr, /**/);
    VerifyOrExit(event->Type == DeviceLayer::DeviceEventType::kInternetConnectivityChange, /**/);
    VerifyOrExit(event->InternetConnectivityChange.IPv4 == DeviceLayer::kConnectivity_Established, /**/);

    IPAddress addr;
    IPAddress::FromString(event->InternetConnectivityChange.address, addr);
    (void) session->SendIPAddress(addr);

exit:
    return;
}
#endif

} // namespace chip
