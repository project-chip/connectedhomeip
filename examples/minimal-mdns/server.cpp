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

#include <TracingCommandLineArgument.h>
#include <inet/IPAddress.h>
#include <inet/InetInterface.h>
#include <inet/UDPEndPoint.h>
#include <lib/dnssd/MinimalMdnsServer.h>
#include <lib/dnssd/ServiceNaming.h>
#include <lib/dnssd/minimal_mdns/AddressPolicy.h>
#include <lib/dnssd/minimal_mdns/QueryBuilder.h>
#include <lib/dnssd/minimal_mdns/ResponseSender.h>
#include <lib/dnssd/minimal_mdns/Server.h>
#include <lib/dnssd/minimal_mdns/responders/IP.h>
#include <lib/dnssd/minimal_mdns/responders/Ptr.h>
#include <lib/dnssd/minimal_mdns/responders/Srv.h>
#include <lib/dnssd/minimal_mdns/responders/Txt.h>
#include <lib/dnssd/wire/QName.h>
#include <lib/support/CHIPArgParser.hpp>
#include <lib/support/CHIPMem.h>
#include <platform/CHIPDeviceLayer.h>
#include <system/SystemPacketBuffer.h>

#include "PacketReporter.h"

namespace {

using namespace chip;

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
constexpr uint16_t kOptionTraceTo      = 't';

// Only used for argument parsing. Tracing setup owned by the main loop.
chip::CommandLineApp::TracingSetup * tracing_setup_for_argparse = nullptr;

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

    case kOptionTraceTo:
        tracing_setup_for_argparse->EnableTracingFor(aValue);
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
    { "trace-to", kArgumentRequired, kOptionTraceTo },
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
                             "  -t <dest>\n"
                             "  --trace-to <dest>\n"
                             "        trace to the given destination (supported: " SUPPORTED_COMMAND_LINE_TRACING_TARGETS ").\n"
                             "\n" };

HelpOptions helpOptions("minimal-mdns-server", "Usage: minimal-mdns-server [options]", "1.0");

OptionSet * allOptions[] = { &cmdLineOptions, &helpOptions, nullptr };

class ReplyDelegate : public mdns::Minimal::ServerDelegate, public chip::Dnssd::ParserDelegate
{
public:
    ReplyDelegate(mdns::Minimal::ResponseSender * responder) : mResponder(responder) {}

    void OnQuery(const chip::Dnssd::BytesRange & data, const Inet::IPPacketInfo * info) override
    {
        char addr[Inet::IPAddress::kMaxStringLength];
        info->SrcAddress.ToString(addr, sizeof(addr));

        char ifName[Inet::InterfaceId::kMaxIfNameLength];
        VerifyOrDie(info->Interface.GetInterfaceName(ifName, sizeof(ifName)) == CHIP_NO_ERROR);

        printf("QUERY from: %-15s on port %d, via interface %s\n", addr, info->SrcPort, ifName);
        Report("QUERY: ", data);

        mCurrentSource = info;
        if (!chip::Dnssd::ParseMdnsPacket(data, this))
        {
            printf("Parsing failure may result in reply failure!\n");
        }
        mCurrentSource = nullptr;
    }

    void OnResponse(const chip::Dnssd::BytesRange & data, const Inet::IPPacketInfo * info) override
    {
        char addr[Inet::IPAddress::kMaxStringLength];
        info->SrcAddress.ToString(addr, sizeof(addr));

        char ifName[Inet::InterfaceId::kMaxIfNameLength];
        VerifyOrDie(info->Interface.GetInterfaceName(ifName, sizeof(ifName)) == CHIP_NO_ERROR);

        printf("RESPONSE from: %-15s on port %d, via interface %s\n", addr, info->SrcPort, ifName);
    }

    // ParserDelegate
    void OnHeader(chip::Dnssd::ConstHeaderRef & header) override { mMessageId = header.GetMessageId(); }
    void OnResource(chip::Dnssd::ResourceType type, const chip::Dnssd::ResourceData & data) override {}

    void OnQuery(const chip::Dnssd::QueryData & data) override
    {
        if (mResponder->Respond(mMessageId, data, mCurrentSource, mdns::Minimal::ResponseConfiguration()) != CHIP_NO_ERROR)
        {
            printf("FAILED to respond!\n");
        }
    }

private:
    void Report(const char * prefix, const chip::Dnssd::BytesRange & data)
    {
        MdnsExample::PacketReporter reporter(prefix, data);
        if (!chip::Dnssd::ParseMdnsPacket(data, &reporter))
        {
            printf("INVALID PACKET!!!!!!\n");
        }
    }

    mdns::Minimal::ResponseSender * mResponder;
    const Inet::IPPacketInfo * mCurrentSource = nullptr;
    uint16_t mMessageId                       = 0;
};

mdns::Minimal::Server<10 /* endpoints */> gMdnsServer;

void StopSignalHandler(int signal)
{
    gMdnsServer.Shutdown();

    TEMPORARY_RETURN_IGNORED DeviceLayer::PlatformMgr().StopEventLoopTask();
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

    chip::CommandLineApp::TracingSetup tracing_setup;

    tracing_setup_for_argparse = &tracing_setup;
    if (!ArgParser::ParseArgs(args[0], argc, args, allOptions))
    {
        return 1;
    }
    tracing_setup_for_argparse = nullptr;

    // This forces the global MDNS instance to be loaded in, effectively setting
    // built in policies for addresses.
    (void) chip::Dnssd::GlobalMinimalMdnsServer::Instance();

    printf("Running on port %d using %s...\n", gOptions.listenPort, gOptions.enableIpV4 ? "IPv4 AND IPv6" : "IPv6 ONLY");

    mdns::Minimal::QueryResponder<16 /* maxRecords */> queryResponder;

    chip::Dnssd::QNamePart tcpServiceName[] = { Dnssd::kOperationalServiceName, Dnssd::kOperationalProtocol, Dnssd::kLocalDomain };
    chip::Dnssd::QNamePart tcpServerServiceName[] = { gOptions.instanceName, Dnssd::kOperationalServiceName,
                                                      Dnssd::kOperationalProtocol, Dnssd::kLocalDomain };
    chip::Dnssd::QNamePart udpServiceName[]       = { Dnssd::kCommissionableServiceName, Dnssd::kCommissionProtocol,
                                                      Dnssd::kLocalDomain };
    chip::Dnssd::QNamePart udpServerServiceName[] = { gOptions.instanceName, Dnssd::kCommissionableServiceName,
                                                      Dnssd::kCommissionProtocol, Dnssd::kLocalDomain };

    // several UDP versions for discriminators
    chip::Dnssd::QNamePart udpDiscriminator1[] = { "S52", Dnssd::kSubtypeServiceNamePart, Dnssd::kCommissionableServiceName,
                                                   Dnssd::kCommissionProtocol, Dnssd::kLocalDomain };
    chip::Dnssd::QNamePart udpDiscriminator2[] = { "V123", Dnssd::kSubtypeServiceNamePart, Dnssd::kCommissionableServiceName,
                                                   Dnssd::kCommissionProtocol, Dnssd::kLocalDomain };
    chip::Dnssd::QNamePart udpDiscriminator3[] = { "L840", Dnssd::kSubtypeServiceNamePart, Dnssd::kCommissionableServiceName,
                                                   Dnssd::kCommissionProtocol, Dnssd::kLocalDomain };

    chip::Dnssd::QNamePart serverName[] = { gOptions.instanceName, Dnssd::kLocalDomain };

    mdns::Minimal::IPv4Responder ipv4Responder(serverName);
    mdns::Minimal::IPv6Responder ipv6Responder(serverName);
    chip::Dnssd::SrvResourceRecord srvRecord(tcpServerServiceName, serverName, CHIP_PORT);
    mdns::Minimal::SrvResponder tcpSrvResponder(chip::Dnssd::SrvResourceRecord(tcpServerServiceName, serverName, CHIP_PORT));
    mdns::Minimal::SrvResponder udpSrvResponder(chip::Dnssd::SrvResourceRecord(udpServerServiceName, serverName, CHIP_PORT));
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
    mdns::Minimal::TxtResponder tcpTxtResponder(chip::Dnssd::TxtResourceRecord(tcpServerServiceName, txtEntries));
    mdns::Minimal::TxtResponder udpTxtResponder(chip::Dnssd::TxtResourceRecord(udpServerServiceName, txtEntries));

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

    mdns::Minimal::ResponseSender responseSender(&gMdnsServer);
    TEMPORARY_RETURN_IGNORED responseSender.AddQueryResponder(&queryResponder);

    ReplyDelegate delegate(&responseSender);
    gMdnsServer.SetDelegate(&delegate);

    {
        auto endpoints = mdns::Minimal::GetAddressPolicy()->GetListenEndpoints();

        if (gMdnsServer.Listen(DeviceLayer::UDPEndPointManager(), endpoints.get(), gOptions.listenPort) != CHIP_NO_ERROR)
        {
            printf("Server failed to listen on all interfaces\n");
            return 1;
        }
    }

    signal(SIGTERM, StopSignalHandler);
    signal(SIGINT, StopSignalHandler);

    DeviceLayer::PlatformMgr().RunEventLoop();

    tracing_setup.StopTracing();
    DeviceLayer::PlatformMgr().Shutdown();

    printf("Done...\n");
    return 0;
}
