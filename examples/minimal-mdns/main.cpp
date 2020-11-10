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

#include <inet/InetInterface.h>
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
constexpr uint16_t kFakeMdnsPort = 5388;

constexpr uint32_t kTestMessageId = 0x1234;

constexpr size_t kMdnsMaxPacketSize = 1'024;

// const mdns::Minimal::QNamePart kCastQnames[] = { "_googlecast", "_tcp", "local" };
const mdns::Minimal::QNamePart kCastQnames[] = { "octopi", "local" };

// const char * kMdnsQueryDestination = "224.0.0.251"; // IPV4 address:
const char * kMdnsQueryDestination = "FF02::FB";

// Use `ip -6 maddr` to list these
#define FORCE_INTERFACE_ID 178

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
    void OnRecord(const mdns::Minimal::BytesRange & name, const mdns::Minimal::BytesRange & value)
    {
        std::unique_ptr<char[]> nameBuff(new char[name.Size() + 1]);
        std::unique_ptr<char[]> valueBuff(new char[value.Size() + 1]);

        memcpy(nameBuff.get(), name.Start(), name.Size());
        nameBuff.get()[name.Size()] = 0;

        memcpy(valueBuff.get(), value.Start(), value.Size());
        valueBuff.get()[value.Size()] = 0;

        printf("            TXT:  '%s' = '%s'\n", nameBuff.get(), valueBuff.get());
    }
};

void PrintQName(mdns::Minimal::SerializedQNameIterator it)
{
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

class PacketReporter : public mdns::Minimal::ParserDelegate
{
public:
    PacketReporter(const mdns::Minimal::BytesRange & packet) : mPacketRange(packet) {}

    void Header(const mdns::Minimal::HeaderRef & header) override
    {
        if (header.GetFlags().IsQuery())
        {
            printf("   QUERY:\n");
        }
        else
        {
            printf("   REPLY:\n");
        }
        printf("   MESSAGE ID : %d\n", header.GetMessageId());
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
        PrintQName(data.GetName());
    }

    void Resource(ResourceType type, const mdns::Minimal::ResourceData & data) override
    {
        printf("       RESOURCE %d\n", static_cast<int>(type));
        printf("          Type:      %s\n", ToString(data.GetType()));
        printf("          Class:     %d\n", static_cast<int>(data.GetClass()));
        printf("          TTL:       %ld\n", static_cast<long>(data.GetTtlSeconds()));
        printf("          Data size: %ld\n", static_cast<long>(data.GetData().Size()));
        printf("          NAME:  ");
        PrintQName(data.GetName());

        if (data.GetType() == mdns::Minimal::QType::TXT)
        {
            TxtReport txtReport;
            if (!mdns::Minimal::ParseTxtRecord(data.GetData(), &txtReport))
            {
                printf("FAILED TO PARSE TXT RECORD\n");
            }
        }
        else if (data.GetType() == mdns::Minimal::QType::SRV)
        {
            mdns::Minimal::SrvRecord srv;

            if (!srv.Parse(data.GetData(), mPacketRange))
            {
                printf("Failed to parse SRV record!!!!");
            }
            else
            {
                printf("             SRV on port %d, priority %d, weight %d:  ", srv.GetPort(), srv.GetPriority(), srv.GetWeight());
                PrintQName(data.GetName());
            }
        }
        else if (data.GetType() == mdns::Minimal::QType::A)
        {
            chip::Inet::IPAddress addr;

            if (!mdns::Minimal::ParseARecord(data.GetData(), &addr))
            {
                printf("FAILED TO PARSE A RECORD\n");
            }
            else
            {
                char buff[128];
                printf("             IP:  %s\n", addr.ToString(buff, sizeof(buff)));
            }
        }
        else if (data.GetType() == mdns::Minimal::QType::AAAA)
        {
            chip::Inet::IPAddress addr;

            if (!mdns::Minimal::ParseAAAARecord(data.GetData(), &addr))
            {
                printf("FAILED TO PARSE AAAA RECORD\n");
            }
            else
            {
                char buff[128];
                printf("             IP:  %s\n", addr.ToString(buff, sizeof(buff)));
            }
        }
        else if (data.GetType() == mdns::Minimal::QType::PTR)
        {
            mdns::Minimal::SerializedQNameIterator name;
            if (!mdns::Minimal::ParsePtrRecord(data.GetData(), mPacketRange, &name))
            {
                printf("FAILED TO PARSE AAAA RECORD\n");
            }
            else
            {
                printf("             PTR:  ");
                PrintQName(name);
            }
        }
    }

private:
    mdns::Minimal::BytesRange mPacketRange;
};

void SendPacket(Inet::UDPEndPoint * udp, const Inet::IPAddress & destIpAddr)
{
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

    mdns::Minimal::BytesRange packetRange(buffer->Start(), buffer->Start() + buffer->DataLength());
    PacketReporter reporter(packetRange);

    if (!mdns::Minimal::ParsePacket(packetRange, &reporter))
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

    Inet::IPAddress destIpAddr;
    if (!Inet::IPAddress::FromString(kMdnsQueryDestination, destIpAddr))
    {
        printf("Cannot parse IP address: '%s'", kMdnsQueryDestination);
        return 1;
    }

    chip::Inet::InterfaceId interfaceId = INET_NULL_INTERFACEID;
#ifdef FORCE_INTERFACE_ID
    interfaceId = FORCE_INTERFACE_ID;
    {
        char buff[64];
        if (chip::Inet::GetInterfaceName(interfaceId, buff, sizeof(buff)) == CHIP_NO_ERROR)
        {
            printf("USING interface: %s.\n", buff);
        }
        else
        {
            printf("FAILED to get inteface id name.\n");
        }
    }

#endif

    if (udp->Bind(destIpAddr.Type(), chip::Inet::IPAddress::Any, kFakeMdnsPort, interfaceId) != CHIP_NO_ERROR)
    {
        printf("Failed to bind\n");
        return 1;
    }

    udp->OnMessageReceived = OnUdpPacketReceived;
    if (udp->Listen() != CHIP_NO_ERROR)
    {
        printf("Failed to listen\n");
        return 1;
    }

    SendPacket(udp, destIpAddr);

    DeviceLayer::PlatformMgr().RunEventLoop();

    udp->Free();
    printf("Done...\n");
    return 0;
}
