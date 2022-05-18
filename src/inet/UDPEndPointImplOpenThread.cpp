/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
 *    Copyright (c) 2018 Google LLC.
 *    Copyright (c) 2013-2018 Nest Labs, Inc.
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

#include <inet/UDPEndPointImplOpenThread.h>

#include <lib/support/CodeUtils.h>
#include <lib/support/SafeInt.h>
#include <lib/support/logging/CHIPLogging.h>

#include <platform/OpenThread/OpenThreadUtils.h>

#include <system/SystemPacketBuffer.h>

namespace chip {
namespace Inet {

otInstance * globalOtInstance;

void UDPEndPointImplOT::handleUdpReceive(void * aContext, otMessage * aMessage, const otMessageInfo * aMessageInfo)
{
    UDPEndPointImplOT * ep = static_cast<UDPEndPointImplOT *>(aContext);
    IPPacketInfo pktInfo;
    uint16_t msgLen = otMessageGetLength(aMessage);
    System::PacketBufferHandle payload;
#if CHIP_DETAIL_LOGGING
    static uint16_t msgReceivedCount = 0;
    char sourceStr[Inet::IPAddress::kMaxStringLength];
    char destStr[Inet::IPAddress::kMaxStringLength];
#endif

    if (msgLen > System::PacketBuffer::kMaxSizeWithoutReserve)
    {
        ChipLogError(Inet, "UDP message too long, discarding. Size received %d", msgLen);
        return;
    }

    pktInfo.SrcAddress  = IPAddress::FromOtAddr(aMessageInfo->mPeerAddr);
    pktInfo.DestAddress = IPAddress::FromOtAddr(aMessageInfo->mSockAddr);
    pktInfo.SrcPort     = aMessageInfo->mPeerPort;
    pktInfo.DestPort    = aMessageInfo->mSockPort;

    payload = System::PacketBufferHandle::New(msgLen, 0);

    if (payload.IsNull())
    {
        ChipLogError(Inet, "Failed to allocate a System buffer of size %d for UDP Message reception.", msgLen);
        return;
    }

#if CHIP_DETAIL_LOGGING
    pktInfo.SrcAddress.ToString(sourceStr, Inet::IPAddress::kMaxStringLength);
    pktInfo.DestAddress.ToString(destStr, Inet::IPAddress::kMaxStringLength);

    ChipLogDetail(Inet,
                  "UDP Message Received packet nb : %d SrcAddr : %s[%d] DestAddr "
                  ": %s[%d] Payload Length %d",
                  ++msgReceivedCount, sourceStr, pktInfo.SrcPort, destStr, pktInfo.DestPort, msgLen);

#endif

    memcpy(payload->Start(), &pktInfo, sizeof(IPPacketInfo));

    if (otMessageRead(aMessage, 0, payload->Start() + sizeof(IPPacketInfo), msgLen) != msgLen)
    {
        ChipLogError(Inet, "Failed to copy OpenThread buffer into System Packet buffer");
        return;
    }
    payload->SetDataLength(static_cast<uint16_t>(msgLen + sizeof(IPPacketInfo)));

    ep->Retain();
    auto * buf     = std::move(payload).UnsafeRelease();
    CHIP_ERROR err = ep->GetSystemLayer().ScheduleLambda([ep, buf] {
        ep->HandleDataReceived(System::PacketBufferHandle::Adopt(buf));
        ep->Release();
    });
    if (err != CHIP_NO_ERROR)
    {
        // Make sure we properly clean up buf and ep, since our lambda will not
        // run.
        payload = System::PacketBufferHandle::Adopt(buf);
        ep->Release();
    }
}

CHIP_ERROR UDPEndPointImplOT::IPv6Bind(otUdpSocket & socket, const IPAddress & address, uint16_t port, InterfaceId interface)
{
    (void) interface;
    otError err = OT_ERROR_NONE;
    otSockAddr listenSockAddr;

    memset(&socket, 0, sizeof(socket));
    memset(&listenSockAddr, 0, sizeof(listenSockAddr));

    listenSockAddr.mPort    = port;
    listenSockAddr.mAddress = address.ToIPv6();

    LockOpenThread();
    otUdpOpen(mOTInstance, &socket, handleUdpReceive, this);
    otUdpBind(mOTInstance, &socket, &listenSockAddr, OT_NETIF_THREAD);
    UnlockOpenThread();

    return chip::DeviceLayer::Internal::MapOpenThreadError(err);
}

CHIP_ERROR UDPEndPointImplOT::BindImpl(IPAddressType addressType, const IPAddress & addr, uint16_t port, InterfaceId interface)
{

    if (addressType != IPAddressType::kIPv6)
    {
        return INET_ERROR_WRONG_ADDRESS_TYPE;
    }

    ReturnErrorOnFailure(IPv6Bind(mSocket, addr, port, interface));
    mBoundPort   = port;
    mBoundIntfId = interface;

    return CHIP_NO_ERROR;
}

InterfaceId UDPEndPointImplOT::GetBoundInterface() const
{
    return mBoundIntfId;
}

uint16_t UDPEndPointImplOT::GetBoundPort() const
{
    return mBoundPort;
}

CHIP_ERROR UDPEndPointImplOT::ListenImpl()
{
    // Nothing to do. Callback was set upon Binding call.
    return CHIP_NO_ERROR;
}

void UDPEndPointImplOT::HandleDataReceived(System::PacketBufferHandle && msg)
{
    if ((mState == State::kListening) && (OnMessageReceived != nullptr))
    {
        const IPPacketInfo * pktInfo = GetPacketInfo(msg);

        if (pktInfo != nullptr)
        {
            const IPPacketInfo pktInfoCopy = *pktInfo; // copy the address info so that the app can free the
                                                       // PacketBuffer without affecting access to address info.

            msg->ConsumeHead(sizeof(IPPacketInfo));
            OnMessageReceived(this, std::move(msg), &pktInfoCopy);
        }
        else
        {
            if (OnReceiveError != nullptr)
            {
                OnReceiveError(this, CHIP_ERROR_INBOUND_MESSAGE_TOO_BIG, nullptr);
            }
        }
    }
}

void UDPEndPointImplOT::SetNativeParams(void * params)
{
    if (params == nullptr)
    {
        ChipLogError(Inet, "FATAL!! No native parameters provided!!!!!");
        VerifyOrDie(false);
    }

    OpenThreadEndpointInitParam * initParams = static_cast<OpenThreadEndpointInitParam *>(params);
    mOTInstance                              = initParams->openThreadInstancePtr;
    globalOtInstance                         = mOTInstance;

    lockOpenThread   = initParams->lockCb;
    unlockOpenThread = initParams->unlockCb;
}

CHIP_ERROR UDPEndPointImplOT::SetMulticastLoopback(IPVersion aIPVersion, bool aLoopback)
{
    (void) aIPVersion;
    (void) aLoopback;
    // TODO
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR UDPEndPointImplOT::BindInterfaceImpl(IPAddressType addressType, InterfaceId interfaceId)
{
    (void) addressType;
    (void) interfaceId;
    return CHIP_NO_ERROR;
}

CHIP_ERROR UDPEndPointImplOT::SendMsgImpl(const IPPacketInfo * aPktInfo, System::PacketBufferHandle && msg)
{
    otError error = OT_ERROR_NONE;
    otMessage * message;
    otMessageInfo messageInfo;

    // For now the entire message must fit within a single buffer.
    VerifyOrReturnError(!msg->HasChainedBuffer(), CHIP_ERROR_MESSAGE_TOO_LONG);

    memset(&messageInfo, 0, sizeof(messageInfo));

    messageInfo.mSockAddr = aPktInfo->SrcAddress.ToIPv6();
    messageInfo.mPeerAddr = aPktInfo->DestAddress.ToIPv6();
    messageInfo.mPeerPort = aPktInfo->DestPort;

    LockOpenThread();
    message = otUdpNewMessage(mOTInstance, NULL);
    VerifyOrExit(message != NULL, error = OT_ERROR_NO_BUFS);

    error = otMessageAppend(message, msg->Start(), msg->DataLength());

    if (error == OT_ERROR_NONE)
    {
        error = otUdpSend(mOTInstance, &mSocket, message, &messageInfo);
    }

exit:
    if (error != OT_ERROR_NONE && message != NULL)
    {
        otMessageFree(message);
    }

    UnlockOpenThread();

    return chip::DeviceLayer::Internal::MapOpenThreadError(error);
}

void UDPEndPointImplOT::CloseImpl()
{
    LockOpenThread();
    if (otUdpIsOpen(mOTInstance, &mSocket))
    {
        otUdpClose(mOTInstance, &mSocket);
    }
    UnlockOpenThread();
}

void UDPEndPointImplOT::Free()
{
    Close();
    Release();
}

CHIP_ERROR UDPEndPointImplOT::IPv6JoinLeaveMulticastGroupImpl(InterfaceId aInterfaceId, const IPAddress & aAddress, bool join)
{
    const otIp6Address otAddress = aAddress.ToIPv6();
    otError err;

    LockOpenThread();
    if (join)
    {
        err = otIp6SubscribeMulticastAddress(mOTInstance, &otAddress);
    }
    else
    {
        err = otIp6UnsubscribeMulticastAddress(mOTInstance, &otAddress);
    }

    UnlockOpenThread();

    return chip::DeviceLayer::Internal::MapOpenThreadError(err);
}

IPPacketInfo * UDPEndPointImplOT::GetPacketInfo(const System::PacketBufferHandle & aBuffer)
{
    if (!aBuffer->EnsureReservedSize(sizeof(IPPacketInfo)))
    {
        return nullptr;
    }

    uintptr_t lStart           = (uintptr_t) aBuffer->Start();
    uintptr_t lPacketInfoStart = lStart - sizeof(IPPacketInfo);

    // Align to a 4-byte boundary
    return reinterpret_cast<IPPacketInfo *>(lPacketInfoStart & ~(sizeof(uint32_t) - 1));
}

} // namespace Inet
} // namespace chip
