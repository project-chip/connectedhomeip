/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
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
#include <protocols/Protocols.h>
#include <protocols/secure_channel/NetworkProvisioning.h>
#include <support/CodeUtils.h>
#include <support/ErrorStr.h>
#include <support/SafeInt.h>
#include <support/ThreadOperationalDataset.h>
#include <transport/SecureSessionMgr.h>

#if CONFIG_DEVICE_LAYER
#include <platform/CHIPDeviceLayer.h>
#if defined(CHIP_DEVICE_LAYER_TARGET)
#define DEVICENETWORKPROVISIONING_HEADER <platform/CHIP_DEVICE_LAYER_TARGET/DeviceNetworkProvisioningDelegateImpl.h>
#include DEVICENETWORKPROVISIONING_HEADER
#endif
#endif

namespace chip {

constexpr char kAPInterfaceNamePrefix[]      = "ap";
constexpr char kLoobackInterfaceNamePrefix[] = "lo";

void NetworkProvisioning::Init(Messaging::ExchangeManager * exchangeMgr, SecureSessionHandle session,
                               NetworkProvisioningDelegate * delegate)
{
    mExchangeMgr = exchangeMgr;
    mDelegate    = delegate;
#if CONFIG_DEVICE_LAYER
#if defined(CHIP_DEVICE_LAYER_TARGET)
    DeviceLayer::PlatformMgr().AddEventHandler(ConnectivityHandler, reinterpret_cast<intptr_t>(this));
#endif
#endif
    exchangeMgr->RegisterUnsolicitedMessageHandlerForProtocol(Protocols::NetworkProvisioning::Id, this);
    mSession = session;
}

NetworkProvisioning::~NetworkProvisioning()
{
#if CONFIG_DEVICE_LAYER
#if defined(CHIP_DEVICE_LAYER_TARGET)
    DeviceLayer::PlatformMgr().RemoveEventHandler(ConnectivityHandler, reinterpret_cast<intptr_t>(this));
#endif
#endif
}

void NetworkProvisioning::OnMessageReceived(Messaging::ExchangeContext * exchangeContext, const PacketHeader & packetHeader,
                                            const PayloadHeader & payloadHeader, System::PacketBufferHandle payload)
{
    HandleNetworkProvisioningMessage(payloadHeader.GetMessageType(), payload);

    // Currently, the only mechanism to get this callback is via unsolicited message handler.
    // That means that we now own the reference to exchange context. Let's free the reference since we no longer
    // need it.
    exchangeContext->Close();
}

CHIP_ERROR NetworkProvisioning::HandleNetworkProvisioningMessage(uint8_t msgType, const System::PacketBufferHandle & msgBuf)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    switch (msgType)
    {
    case NetworkProvisioning::MsgTypes::kWiFiAssociationRequest: {
        char SSID[chip::DeviceLayer::Internal::kMaxWiFiSSIDLength];
        char passwd[chip::DeviceLayer::Internal::kMaxWiFiKeyLength];
        Encoding::LittleEndian::BufferWriter bbufSSID(Uint8::from_char(SSID), chip::DeviceLayer::Internal::kMaxWiFiSSIDLength);
        Encoding::LittleEndian::BufferWriter bbufPW(Uint8::from_char(passwd), chip::DeviceLayer::Internal::kMaxWiFiKeyLength);

        const uint8_t * buffer = msgBuf->Start();
        size_t len             = msgBuf->DataLength();
        size_t offset          = 0;

        ChipLogProgress(NetworkProvisioning, "Received kWiFiAssociationRequest.\n");

        err = DecodeString(&buffer[offset], len - offset, bbufSSID, offset);
        // TODO: Check for the error once network provisioning is moved to delegate calls

        err = DecodeString(&buffer[offset], len - offset, bbufPW, offset);
        // TODO: Check for the error once network provisioning is moved to delegate calls

#if CONFIG_DEVICE_LAYER
#if defined(CHIP_DEVICE_LAYER_TARGET)
        DeviceLayer::DeviceNetworkProvisioningDelegateImpl deviceDelegate;
        err = deviceDelegate.ProvisionWiFi(SSID, passwd);
        SuccessOrExit(err);

        if (DeviceLayer::ConnectivityMgr().IsWiFiStationConnected())
        {
            err = SendCurrentIPv4Address();
        }
#endif
#endif
    }
    break;

    case NetworkProvisioning::MsgTypes::kThreadAssociationRequest:
        ChipLogProgress(NetworkProvisioning, "Received kThreadAssociationRequest");
        err = DecodeThreadAssociationRequest(msgBuf);
        break;

    case NetworkProvisioning::MsgTypes::kIPAddressAssigned: {
        ChipLogProgress(NetworkProvisioning, "Received kIPAddressAssigned");
        if (!Inet::IPAddress::FromString(Uint8::to_const_char(msgBuf->Start()), msgBuf->DataLength(), mDeviceAddress))
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
    if (mDelegate != nullptr)
    {
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(NetworkProvisioning, "Failed in HandleNetworkProvisioningMessage. error %s\n", ErrorStr(err));
            mDelegate->OnNetworkProvisioningError(err);
        }
        else
        {
            // Network provisioning handshake requires only one message exchange in either direction.
            // If the current message handling did not result in an error, network provisioning is
            // complete.
            mDelegate->OnNetworkProvisioningComplete();
        }
    }
    return err;
}

size_t NetworkProvisioning::EncodedStringSize(const char * str)
{
    return strlen(str) + sizeof(uint16_t);
}

CHIP_ERROR NetworkProvisioning::EncodeString(const char * str, Encoding::LittleEndian::BufferWriter & bbuf)
{
    const size_t length = strlen(str);
    VerifyOrReturnError(CanCastTo<uint16_t>(length), CHIP_ERROR_INVALID_ARGUMENT);
    const uint16_t u16len = static_cast<uint16_t>(length);

    bbuf.Put16(u16len);
    bbuf.Put(str);

    return CHIP_NO_ERROR;
}

CHIP_ERROR NetworkProvisioning::DecodeString(const uint8_t * input, size_t input_len, Encoding::LittleEndian::BufferWriter & bbuf,
                                             size_t & consumed)
{
    VerifyOrReturnError(input_len >= sizeof(uint16_t), CHIP_ERROR_BUFFER_TOO_SMALL);
    const uint16_t length = chip::Encoding::LittleEndian::Get16(input);
    consumed              = sizeof(uint16_t);

    VerifyOrReturnError(input_len - consumed >= length, CHIP_ERROR_BUFFER_TOO_SMALL);
    bbuf.Put(&input[consumed], length);

    consumed += bbuf.Needed();
    bbuf.Put(static_cast<uint8_t>('\0'));

    VerifyOrReturnError(bbuf.Fit(), CHIP_ERROR_BUFFER_TOO_SMALL);

    return CHIP_NO_ERROR;
}

CHIP_ERROR NetworkProvisioning::SendMessageUsingExchange(uint8_t msgType, System::PacketBufferHandle msgPayload)
{
    Messaging::ExchangeContext * exchangeContext = mExchangeMgr->NewContext(mSession, this);
    VerifyOrReturnError(exchangeContext != nullptr, CHIP_ERROR_INTERNAL);
    CHIP_ERROR err = exchangeContext->SendMessage(Protocols::NetworkProvisioning::Id, msgType, std::move(msgPayload),
                                                  Messaging::SendMessageFlags::kNoAutoRequestAck);
    exchangeContext->Close();
    return err;
}

CHIP_ERROR NetworkProvisioning::SendIPAddress(const Inet::IPAddress & addr)
{
    char * addrStr;
    CHIP_ERROR err                    = CHIP_NO_ERROR;
    System::PacketBufferHandle buffer = System::PacketBufferHandle::New(Inet::kMaxIPAddressStringLength);
    VerifyOrExit(!buffer.IsNull(), err = CHIP_ERROR_NO_MEMORY);
    addrStr = addr.ToString(Uint8::to_char(buffer->Start()), buffer->AvailableDataLength());
    buffer->SetDataLength(static_cast<uint16_t>(strlen(addrStr) + 1));

    ChipLogProgress(NetworkProvisioning, "Sending IP Address\n");
    VerifyOrExit(addrStr != nullptr, err = CHIP_ERROR_INVALID_ADDRESS);

    err = SendMessageUsingExchange(NetworkProvisioning::MsgTypes::kIPAddressAssigned, std::move(buffer));
    SuccessOrExit(err);

exit:
    if (CHIP_NO_ERROR != err)
        ChipLogError(NetworkProvisioning, "Failed in sending IP address. error %s\n", ErrorStr(err));
    return err;
}

CHIP_ERROR NetworkProvisioning::SendCurrentIPv4Address()
{
    for (chip::Inet::InterfaceAddressIterator it; it.HasCurrent(); it.Next())
    {
        char ifName[chip::Inet::InterfaceIterator::kMaxIfNameLength];
        if (it.IsUp() && CHIP_NO_ERROR == it.GetInterfaceName(ifName, sizeof(ifName)) &&
            memcmp(ifName, kAPInterfaceNamePrefix, sizeof(kAPInterfaceNamePrefix) - 1) &&
            memcmp(ifName, kLoobackInterfaceNamePrefix, sizeof(kLoobackInterfaceNamePrefix) - 1))
        {
            chip::Inet::IPAddress addr = it.GetAddress();
            if (addr.IsIPv4())
            {
                return SendIPAddress(addr);
            }
        }
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR NetworkProvisioning::SendNetworkCredentials(const char * ssid, const char * passwd)
{
    CHIP_ERROR err          = CHIP_NO_ERROR;
    const size_t bufferSize = EncodedStringSize(ssid) + EncodedStringSize(passwd);
    VerifyOrExit(CanCastTo<uint16_t>(bufferSize), err = CHIP_ERROR_INVALID_ARGUMENT);
    {
        Encoding::LittleEndian::PacketBufferWriter bbuf(MessagePacketBuffer::New(bufferSize), bufferSize);

        ChipLogProgress(NetworkProvisioning, "Sending Network Creds\n");
        VerifyOrExit(!bbuf.IsNull(), err = CHIP_ERROR_NO_MEMORY);
        SuccessOrExit(EncodeString(ssid, bbuf));
        SuccessOrExit(EncodeString(passwd, bbuf));
        VerifyOrExit(bbuf.Fit(), err = CHIP_ERROR_BUFFER_TOO_SMALL);

        err = SendMessageUsingExchange(NetworkProvisioning::MsgTypes::kWiFiAssociationRequest, bbuf.Finalize());
        SuccessOrExit(err);
    }

exit:
    if (CHIP_NO_ERROR != err)
        ChipLogError(NetworkProvisioning, "Failed in sending Network Creds. error %s\n", ErrorStr(err));
    return err;
}

CHIP_ERROR NetworkProvisioning::SendThreadCredentials(ByteSpan threadData)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    Encoding::LittleEndian::PacketBufferWriter bbuf(MessagePacketBuffer::New(threadData.size()), threadData.size());

    ChipLogProgress(NetworkProvisioning, "Sending Thread Credentials");
    VerifyOrExit(!bbuf.IsNull(), err = CHIP_ERROR_NO_MEMORY);

    bbuf.Put(threadData.data(), threadData.size());

    VerifyOrExit(bbuf.Fit(), err = CHIP_ERROR_BUFFER_TOO_SMALL);

    err = SendMessageUsingExchange(NetworkProvisioning::MsgTypes::kThreadAssociationRequest, bbuf.Finalize());

exit:
    if (CHIP_NO_ERROR != err)
        ChipLogError(NetworkProvisioning, "Failed to send Thread Credentials: %s", ErrorStr(err));
    return err;
}

#ifdef CHIP_ENABLE_OPENTHREAD
CHIP_ERROR NetworkProvisioning::DecodeThreadAssociationRequest(const System::PacketBufferHandle & msgBuf)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

#if CONFIG_DEVICE_LAYER
    // Start listening for OpenThread changes to be able to respond with SLAAC/On-Mesh IP Address
    DeviceLayer::PlatformMgr().AddEventHandler(ConnectivityHandler, reinterpret_cast<intptr_t>(this));
#if defined(CHIP_DEVICE_LAYER_TARGET)
    {
        DeviceLayer::DeviceNetworkProvisioningDelegateImpl deviceDelegate;
        err = deviceDelegate.ProvisionThread(ByteSpan(msgBuf->Start(), msgBuf->DataLength()));
    }
#endif
#endif

    return err;
}
#else  // CHIP_ENABLE_OPENTHREAD
CHIP_ERROR NetworkProvisioning::DecodeThreadAssociationRequest(const System::PacketBufferHandle &)
{
    return CHIP_ERROR_INVALID_MESSAGE_TYPE;
}
#endif // CHIP_ENABLE_OPENTHREAD

#if CONFIG_DEVICE_LAYER
void NetworkProvisioning::ConnectivityHandler(const DeviceLayer::ChipDeviceEvent * event, intptr_t arg)
{
    NetworkProvisioning * session = reinterpret_cast<NetworkProvisioning *>(arg);

    VerifyOrReturn(session != nullptr);

    if (event->Type == DeviceLayer::DeviceEventType::kInternetConnectivityChange &&
        event->InternetConnectivityChange.IPv4 == DeviceLayer::kConnectivity_Established)
    {
        Inet::IPAddress addr;
        Inet::IPAddress::FromString(event->InternetConnectivityChange.address, addr);
        (void) session->SendIPAddress(addr);
    }

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
    if (event->Type == DeviceLayer::DeviceEventType::kThreadStateChange && event->ThreadStateChange.AddressChanged)
    {
        Inet::IPAddress addr;
        ReturnOnFailure(DeviceLayer::ThreadStackMgr().GetExternalIPv6Address(addr));
        (void) session->SendIPAddress(addr);
    }
#endif
}
#endif

} // namespace chip
