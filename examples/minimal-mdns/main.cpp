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
#include <memory>

#include <inet/UDPEndPoint.h>
#include <mdns/minimal/DnsHeader.h>
#include <mdns/minimal/Parser.h>
#include <mdns/minimal/QName.h>
#include <mdns/minimal/QueryBuilder.h>
#include <mdns/minimal/RecordData.h>
#include <platform/CHIPDeviceLayer.h>
#include <support/CHIPMem.h>
#include <system/SystemPacketBuffer.h>
#include <system/SystemTimer.h>

using namespace chip;

namespace {

constexpr uint32_t kRunTimeMs    = 500;
constexpr uint16_t kMdnsPort     = 5353;
constexpr uint16_t kFakeMdnsPort = 53333;

constexpr uint32_t kTestMessageId = 0x1234;

constexpr size_t kMdnsMaxPacketSize = 1'024;

const mdns::Minimal::QNamePart kCastQnames[] = { "_googlecast", "_tcp", "local" };

const char * ToString(mdns::Minimal::QType t)
{
    static char buff[32];

    switch (t)
    {
    case mdns::Minimal::QType::A:
        return "A";
    case mdns::Minimal::QType::NS:
        return "NS";
    case mdns::Minimal::QType::CNAME:
        return "CNAME";
    case mdns::Minimal::QType::SOA:
        return "SOA";
    case mdns::Minimal::QType::WKS:
        return "WKS";
    case mdns::Minimal::QType::PTR:
        return "PTR";
    case mdns::Minimal::QType::MX:
        return "MX";
    case mdns::Minimal::QType::SRV:
        return "SRV";
    case mdns::Minimal::QType::AAAA:
        return "AAAA";
    case mdns::Minimal::QType::ANY:
        return "ANY";
    case mdns::Minimal::QType::TXT:
        return "TXT";
    default:
        sprintf(buff, "UNKNOWN (%d)!!", static_cast<int>(t));
        return buff;
    }
}

class TxtReport : public mdns::Minimal::TxtRecordDelegate
{
public:
    void OnRecord(const mdns::Minimal::BytesRange & name, const mdns::Minimal::BytesRange * value)
    {
        std::unique_ptr<char[]> nameBuff(new char[name.Size() + 1]);
        std::unique_ptr<char[]> valueBuff(new char[value ? (value->Size() + 1) : 32]);

        memcpy(nameBuff.get(), name.Start(), name.Size());
        nameBuff.get()[name.Size()] = 0;

        if (value)
        {
            memcpy(valueBuff.get(), value->Start(), value->Size());
            valueBuff.get()[value->Size()] = 0;
        }
        else
        {
            strcpy(valueBuff.get(), "!!NULL!!");
        }

        printf("            TXT:  '%s' = '%s'\n", nameBuff.get(), valueBuff.get());
    }
};

class PacketReporter : public mdns::Minimal::ParserDelegate
{
public:
    void Header(const mdns::Minimal::HeaderRef & header) override
    {
        printf("   Queries:     %d\n", header.GetQueryCount());
        printf("   Answers:     %d\n", header.GetAnswerCount());
        printf("   Authorities: %d\n", header.GetAuthorityCount());
        printf("   Additionals: %d\n", header.GetAdditionalCount());
    }

    void Query(const mdns::Minimal::QueryData & data) override
    {
        printf("       QUERY TYPE:  %s\n", ToString(data.GetType()));
        printf("       QUERY CLASS: %d\n", static_cast<int>(data.GetClass()));
        printf("       UNICAST:     %s\n", data.GetUnicastAnswer() ? "true" : "false");
        printf("       QUERY FOR:   ");

        mdns::Minimal::SerializedQNameIterator it = data.GetName();
        while (it.Next())
        {
            printf("%s.", it.Value());
        }
        if (!it.IsValid())
        {
            printf("   (INVALID!)");
        }
        printf("\n");
    }

    void Resource(ResourceType type, const mdns::Minimal::ResourceData & data) override
    {
        printf("       RESOURCE %d\n", static_cast<int>(type));
        printf("          Type:      %s\n", ToString(data.GetType()));
        printf("          Class:     %d\n", static_cast<int>(data.GetClass()));
        printf("          TTL:       %ld\n", static_cast<long>(data.GetTtlSeconds()));
        printf("          Data size: %ld\n", static_cast<long>(data.GetData().Size()));
        printf("          NAME:  ");
        mdns::Minimal::SerializedQNameIterator it = data.GetName();
        while (it.Next())
        {
            printf("%s.", it.Value());
        }
        if (!it.IsValid())
        {
            printf("   (INVALID!)");
        }
        printf("\n");

        if (data.GetType() == mdns::Minimal::QType::TXT)
        {
            TxtReport txtReport;
            mdns::Minimal::ParseTxtRecord(data.GetData(), &txtReport);
        }
    }
};

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

    mdns::Minimal::QueryBuilder builder(buffer);

    builder.Header().SetMessageId(kTestMessageId);
    builder.AddQuery(mdns::Minimal::Query(kCastQnames, ArraySize(kCastQnames))
                         .SetClass(mdns::Minimal::QClass::IN)
                         .SetType(mdns::Minimal::QType::ANY)
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

    PacketReporter reporter;
    if (!mdns::Minimal::ParsePacket(mdns::Minimal::BytesRange(buffer->Start(), buffer->Start() + buffer->DataLength()), &reporter))
    {
        printf("INVALID PACKET!!!!!!\n");
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