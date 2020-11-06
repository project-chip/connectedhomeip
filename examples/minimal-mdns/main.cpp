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
#include <platform/CHIPDeviceLayer.h>
#include <support/CHIPMem.h>
#include <system/SystemPacketBuffer.h>
#include <system/SystemTimer.h>

using namespace chip;

namespace {

constexpr uint32_t kRunTimeMs = 3'000;
constexpr uint16_t kMdnsPort  = 5353;

constexpr uint32_t kTestMessageId = 0x1234;

constexpr size_t kMdnsMaxPacketSize = 1'024;

const mdns::Minimal::QNamePart kCastQnames[] = { "_googlecast", "_tcp", "local" };

const auto kQuestion = mdns::Minimal::Question(kCastQnames, ArraySize(kCastQnames))
                           .SetClass(mdns::Minimal::Question::QClass::IN)
                           .SetType(mdns::Minimal::Question::QType::ANY)
                           .SetAnswerViaUnicast(true);

// static uint8_t kHardcodedQuestion[] = { 0x0b, 0x5f, 0x67, 0x6f, 0x6f, 0x67, 0x6c, 0x65, 0x63, 0x61, 0x73, 0x74, 0x04, 0x5f,
//                                        0x74, 0x63, 0x70, 0x05, 0x6c, 0x6f, 0x63, 0x61, 0x6c, 0x00, 0x00, 0xff, 0x80, 0x01 };

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

    buffer->SetDataLength(mdns::Minimal::HeaderRef::kSizeBytes);

    mdns::Minimal::HeaderRef hdr(buffer->Start());
    hdr.Clear().SetFlags(hdr.GetFlags().SetQuery()).SetMessageId(kTestMessageId);

    buffer->SetDataLength(static_cast<uint16_t>(buffer->DataLength() + kQuestion.WriteSizeBytes()));

    if (kQuestion.Append(hdr, buffer->Start() + mdns::Minimal::HeaderRef::kSizeBytes, kQuestion.WriteSizeBytes()) == nullptr)
    {
        printf("Failed to encode buffer");
        System::PacketBuffer::Free(buffer);
        return;
    }

    if (udp->SendTo(destIpAddr, kMdnsPort, buffer) != CHIP_NO_ERROR)
    {
        printf("Error sending");
        return;
    }
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
        printf("Failed to create the shutdown timer. Kill with ^C.");
    }

    // IPV6: FF02::FB
    SendPacket(udp, "224.0.0.251");

    // TODO:
    //   - MDNS:
    //     - build a query (TODO header + query)
    //     - Send it out (where? IPV4 works but what about V6?)
    //     - Listen
    //     - timeout

    DeviceLayer::PlatformMgr().RunEventLoop();

    udp->Free();
    printf("Done...\n");
    return 0;
}