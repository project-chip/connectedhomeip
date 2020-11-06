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

#include <cstdio>

#include <inet/UDPEndPoint.h>
#include <mdns/minimal/DnsHeader.h>
#include <mdns/minimal/QName.h>
#include <mdns/minimal/QuestionBuilder.h>
#include <platform/CHIPDeviceLayer.h>
#include <support/CHIPMem.h>
#include <system/SystemPacketBuffer.h>
#include <system/SystemTimer.h>

using namespace chip;

namespace {

constexpr uint32_t kRunTimeMs    = 3'000;
constexpr uint16_t kMdnsPort     = 5353;
constexpr uint16_t kFakeMdnsPort = 53333;

constexpr uint32_t kTestMessageId = 0x1234;

constexpr size_t kMdnsMaxPacketSize = 1'024;

const mdns::Minimal::QNamePart kCastQnames[] = { "_googlecast", "_tcp", "local" };

void SendPacket(Inet::UDPEndPoint * udp, const char * destIpString)
{
    Inet::IPAddress destIpAddr;

    if (!Inet::IPAddress::FromString(destIpString, destIpAddr))
    {
        printf("Cannot parse IP address: '%s'", destIpString);
        return;
    }

    System::PacketBuffer * buffer = System::PacketBuffer::NewWithAvailableSize(kMdnsMaxPacketSize);
    if (buffer == nullptr)
    {
        printf("Buffer allocation failure.");
        return;
    }

    mdns::Minimal::QuestionBuilder builder(buffer);

    builder.Header().SetMessageId(kTestMessageId);
    builder.AddQuestion(mdns::Minimal::Question(kCastQnames, ArraySize(kCastQnames))
                            .SetClass(mdns::Minimal::Question::QClass::IN)
                            .SetType(mdns::Minimal::Question::QType::ANY)
                            .SetAnswerViaUnicast(true));

    if (!builder.Ok())
    {
        printf("Failed to build the question");
        System::PacketBuffer::Free(buffer);
        return;
    }

    if (udp->SendTo(destIpAddr, kMdnsPort, buffer) != CHIP_NO_ERROR)
    {
        printf("Error sending");
        return;
    }
}

void OnUdpPacketReceived(chip::Inet::IPEndPointBasis * endPoint, chip::System::PacketBuffer * buffer,
                         const chip::Inet::IPPacketInfo * info)
{
    char addr[32];
    info->SrcAddress.ToString(addr, sizeof(addr));

    printf("Packet received from: %-15s on port %d\n", addr, info->SrcPort);
}

} // namespace

int main(int argc, char ** args)
{
    if (Platform::MemoryInit() != CHIP_NO_ERROR)
    {
        printf("FAILED to initialize memory");
        return 1;
    }

    if (DeviceLayer::PlatformMgr().InitChipStack() != CHIP_NO_ERROR)
    {
        printf("FAILED to initialize chip stack");
        return 1;
    }

    printf("Running...\n");

    Inet::UDPEndPoint * udp = nullptr;

    if (DeviceLayer::InetLayer.NewUDPEndPoint(&udp) != CHIP_NO_ERROR)
    {
        printf("FAILED to create new udp endpoint");
        return 1;
    }

    System::Timer * timer = nullptr;

    if (DeviceLayer::SystemLayer.NewTimer(timer) == CHIP_NO_ERROR)
    {
        timer->Start(
            kRunTimeMs, [](System::Layer *, void *, System::Error err) { DeviceLayer::PlatformMgr().Shutdown(); }, nullptr);
    }
    else
    {
        printf("Failed to create the shutdown timer. Kill with ^C.\n");
    }

    if (udp->Bind(chip::Inet::IPAddressType::kIPAddressType_IPv4, chip::Inet::IPAddress::Any, kFakeMdnsPort) != CHIP_NO_ERROR)
    {
        printf("Failed to bind\n");
    }

    udp->OnMessageReceived = OnUdpPacketReceived;
    if (udp->Listen() != CHIP_NO_ERROR)
    {
        printf("Failed to listen\n");
    }

    SendPacket(udp, "224.0.0.251");
    // SendPacket(udp, "FF02::FB");

    DeviceLayer::PlatformMgr().RunEventLoop();

    udp->Free();
    printf("Done...\n");
    return 0;
}