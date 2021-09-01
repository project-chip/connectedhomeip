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

#include <arpa/inet.h>

#include <inet/InetInterface.h>
#include <inet/UDPEndPoint.h>
#include <lib/mdns/ServiceNaming.h>
#include <lib/mdns/minimal/QueryBuilder.h>
#include <lib/mdns/minimal/ResponseSender.h>
#include <lib/mdns/minimal/Server.h>
#include <lib/mdns/minimal/core/QName.h>
#include <lib/mdns/minimal/responders/IP.h>
#include <lib/mdns/minimal/responders/Ptr.h>
#include <lib/mdns/minimal/responders/Srv.h>
#include <lib/mdns/minimal/responders/Txt.h>
#include <lib/support/CHIPArgParser.hpp>
#include <lib/support/CHIPMem.h>
#include <platform/CHIPDeviceLayer.h>
#include <system/SystemPacketBuffer.h>

#include "AllInterfaceListener.h"
#include "PacketReporter.h"

using namespace chip;

namespace {

struct Options
{
    bool enableIpV4           = false;
    uint16_t listenPort       = 5353;
    const char * instanceName = "chip-mdns-demo";
} gOptions;

using namespace ArgParser;

constexpr uint16_t kOptionEnableIpV4   = '4';
constexpr uint16_t kOptionListenPort   = 'p';
constexpr uint16_t kOptionInstanceName = 'i';

bool HandleOptions(const char * aProgram, OptionSet * aOptions, int aIdentifier, const char * aName, const char * aValue)
{
    switch (aIdentifier)
    {
    case kOptionEnableIpV4:
        gOptions.enableIpV4 = true;
        return true;

    case kOptionInstanceName:
        gOptions.instanceName = aValue;
        return true;

    case kOptionListenPort:
        if (!ParseInt(aValue, gOptions.listenPort))
        {
            PrintArgError("%s: invalid value for port: %s\n", aProgram, aValue);
            return false;
        }
        return true;

    default:
        PrintArgError("%s: INTERNAL ERROR: Unhandled option: %s\n", aProgram, aName);
        return false;
    }
}

OptionDef cmdLineOptionsDef[] = {
    { "listen-port", kArgumentRequired, kOptionListenPort },
    { "enable-ip-v4", kNoArgument, kOptionEnableIpV4 },
    { "instance-name", kArgumentRequired, kOptionInstanceName },
    {},
};

OptionSet cmdLineOptions = { HandleOptions, cmdLineOptionsDef, "PROGRAM OPTIONS",
                             "  -p <number>\n"
                             "  --listen-port <number>\n"
                             "        The port number to listen on\n"
                             "  -4\n"
                             "  --enable-ip-v4\n"
                             "        enable listening on IPv4\n"
                             "  -i <name>\n"
                             "  --instance-name <name>\n"
                             "        instance name to advertise.\n"
                             "\n" };

HelpOptions helpOptions("minimal-mdns-server", "Usage: minimal-mdns-server [options]", "1.0");

OptionSet * allOptions[] = { &cmdLineOptions, &helpOptions, nullptr };

class ReplyDelegate : public mdns::Minimal::ServerDelegate, public mdns::Minimal::ParserDelegate
{
public:
    ReplyDelegate(mdns::Minimal::ResponseSender * responder) : mResponder(responder) {}

    void OnQuery(const mdns::Minimal::BytesRange & data, const Inet::IPPacketInfo * info) override
    {
        char addr[INET6_ADDRSTRLEN];
        info->SrcAddress.ToString(addr, sizeof(addr));

        printf("QUERY from: %-15s on port %d, via interface %d\n", addr, info->SrcPort, info->Interface);
        Report("QUERY: ", data);

        mCurrentSource = info;
        if (!mdns::Minimal::ParsePacket(data, this))
        {
            printf("Parsing failure may result in reply failure!\n");
        }
        mCurrentSource = nullptr;
    }

    void OnResponse(const mdns::Minimal::BytesRange & data, const Inet::IPPacketInfo * info) override
    {
        char addr[INET6_ADDRSTRLEN];
        info->SrcAddress.ToString(addr, sizeof(addr));

        printf("RESPONSE from: %-15s on port %d, via interface %d\n", addr, info->SrcPort, info->Interface);
    }

    // ParserDelegate
    void OnHeader(mdns::Minimal::ConstHeaderRef & header) override { mMessageId = header.GetMessageId(); }
    void OnResource(mdns::Minimal::ResourceType type, const mdns::Minimal::ResourceData & data) override {}

    void OnQuery(const mdns::Minimal::QueryData & data) override
    {
        if (mResponder->Respond(mMessageId, data, mCurrentSource) != CHIP_NO_ERROR)
        {
            printf("FAILED to respond!\n");
        }
    }

private:
    void Report(const char * prefix, const mdns::Minimal::BytesRange & data)
    {
        MdnsExample::PacketReporter reporter(prefix, data);
        if (!mdns::Minimal::ParsePacket(data, &reporter))
        {
            printf("INVALID PACKET!!!!!!\n");
        }
    }

    mdns::Minimal::ResponseSender * mResponder;
    const Inet::IPPacketInfo * mCurrentSource = nullptr;
    uint32_t mMessageId                       = 0;
};

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

    if (!ArgParser::ParseArgs(args[0], argc, args, allOptions))
    {
        return 1;
    }

    printf("Running on port %d using %s...\n", gOptions.listenPort, gOptions.enableIpV4 ? "IPv4 AND IPv6" : "IPv6 ONLY");

    mdns::Minimal::Server<10 /* endpoints */> mdnsServer;
    mdns::Minimal::QueryResponder<16 /* maxRecords */> queryResponder;

    mdns::Minimal::QNamePart tcpServiceName[] = { Mdns::kOperationalServiceName, Mdns::kOperationalProtocol, Mdns::kLocalDomain };
    mdns::Minimal::QNamePart tcpServerServiceName[] = { gOptions.instanceName, Mdns::kOperationalServiceName,
                                                        Mdns::kOperationalProtocol, Mdns::kLocalDomain };
    mdns::Minimal::QNamePart udpServiceName[] = { Mdns::kCommissionableServiceName, Mdns::kCommissionProtocol, Mdns::kLocalDomain };
    mdns::Minimal::QNamePart udpServerServiceName[] = { gOptions.instanceName, Mdns::kCommissionableServiceName,
                                                        Mdns::kCommissionProtocol, Mdns::kLocalDomain };

    // several UDP versions for discriminators
    mdns::Minimal::QNamePart udpDiscriminator1[] = { "S52", Mdns::kSubtypeServiceNamePart, Mdns::kCommissionableServiceName,
                                                     Mdns::kCommissionProtocol, Mdns::kLocalDomain };
    mdns::Minimal::QNamePart udpDiscriminator2[] = { "V123", Mdns::kSubtypeServiceNamePart, Mdns::kCommissionableServiceName,
                                                     Mdns::kCommissionProtocol, Mdns::kLocalDomain };
    mdns::Minimal::QNamePart udpDiscriminator3[] = { "L840", Mdns::kSubtypeServiceNamePart, Mdns::kCommissionableServiceName,
                                                     Mdns::kCommissionProtocol, Mdns::kLocalDomain };

    mdns::Minimal::QNamePart serverName[] = { gOptions.instanceName, Mdns::kLocalDomain };

    mdns::Minimal::IPv4Responder ipv4Responder(serverName);
    mdns::Minimal::IPv6Responder ipv6Responder(serverName);
    mdns::Minimal::SrvResourceRecord srvRecord(tcpServerServiceName, serverName, CHIP_PORT);
    mdns::Minimal::SrvResponder tcpSrvResponder(mdns::Minimal::SrvResourceRecord(tcpServerServiceName, serverName, CHIP_PORT));
    mdns::Minimal::SrvResponder udpSrvResponder(mdns::Minimal::SrvResourceRecord(udpServerServiceName, serverName, CHIP_PORT));
    mdns::Minimal::PtrResponder ptrTcpResponder(tcpServiceName, tcpServerServiceName);
    mdns::Minimal::PtrResponder ptrUdpResponder(udpServiceName, udpServerServiceName);
    mdns::Minimal::PtrResponder ptrUdpDiscriminator1Responder(udpDiscriminator1, udpServerServiceName);
    mdns::Minimal::PtrResponder ptrUdpDiscriminator2Responder(udpDiscriminator2, udpServerServiceName);
    mdns::Minimal::PtrResponder ptrUdpDiscriminator3Responder(udpDiscriminator3, udpServerServiceName);

    // report TXT records for our service.
    const char * txtEntries[] = {
        "D0840=yes",
        "VP=123+456",
        "PH=3",
        "OTH=Some text here...",
    };
    mdns::Minimal::TxtResponder tcpTxtResponder(mdns::Minimal::TxtResourceRecord(tcpServerServiceName, txtEntries));
    mdns::Minimal::TxtResponder udpTxtResponder(mdns::Minimal::TxtResourceRecord(udpServerServiceName, txtEntries));

    queryResponder.AddResponder(&ptrTcpResponder).SetReportInServiceListing(true).SetReportAdditional(tcpServerServiceName);
    queryResponder.AddResponder(&ptrUdpResponder).SetReportInServiceListing(true).SetReportAdditional(udpServerServiceName);
    queryResponder.AddResponder(&ptrUdpDiscriminator1Responder).SetReportAdditional(udpServerServiceName);
    queryResponder.AddResponder(&ptrUdpDiscriminator2Responder).SetReportAdditional(udpServerServiceName);
    queryResponder.AddResponder(&ptrUdpDiscriminator3Responder).SetReportAdditional(udpServerServiceName);
    queryResponder.AddResponder(&tcpTxtResponder);
    queryResponder.AddResponder(&udpTxtResponder);
    queryResponder.AddResponder(&tcpSrvResponder).SetReportAdditional(serverName);
    queryResponder.AddResponder(&udpSrvResponder).SetReportAdditional(serverName);
    queryResponder.AddResponder(&ipv6Responder);

    if (gOptions.enableIpV4)
    {
        queryResponder.AddResponder(&ipv4Responder);
    }

    mdns::Minimal::ResponseSender responseSender(&mdnsServer);
    responseSender.AddQueryResponder(&queryResponder);

    ReplyDelegate delegate(&responseSender);
    mdnsServer.SetDelegate(&delegate);

    {
        MdnsExample::AllInterfaces allInterfaces(gOptions.enableIpV4);

        if (mdnsServer.Listen(&DeviceLayer::InetLayer, &allInterfaces, gOptions.listenPort) != CHIP_NO_ERROR)
        {
            printf("Server failed to listen on all interfaces\n");
            return 1;
        }
    }

    DeviceLayer::PlatformMgr().RunEventLoop();

    printf("Done...\n");
    return 0;
}
