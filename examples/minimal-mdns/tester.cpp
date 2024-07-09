/*
 *
 *    Copyright (c) 2020-2024 Project CHIP Authors
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

#include <cstdint>
#include <cstdio>
#include <memory>

#include <TracingCommandLineArgument.h>
#include <inet/InetInterface.h>
#include <inet/UDPEndPoint.h>
#include <lib/dnssd/MinimalMdnsServer.h>
#include <lib/dnssd/ServiceNaming.h>
#include <lib/dnssd/minimal_mdns/AddressPolicy.h>
#include <lib/dnssd/minimal_mdns/ResponseBuilder.h>
#include <lib/dnssd/minimal_mdns/Server.h>
#include <lib/dnssd/minimal_mdns/core/QName.h>
#include <lib/dnssd/minimal_mdns/records/IP.h>
#include <lib/dnssd/minimal_mdns/records/Ptr.h>
#include <lib/dnssd/minimal_mdns/records/Srv.h>
#include <lib/dnssd/minimal_mdns/records/Txt.h>
#include <lib/support/CHIPArgParser.hpp>
#include <lib/support/CHIPMem.h>
#include <platform/CHIPDeviceLayer.h>
#include <system/SystemPacketBuffer.h>

#include "PacketReporter.h"

using namespace chip;

namespace {

enum class TestType
{
    kSrv,
};

struct Options
{
    uint32_t runtimeMs  = 500;
    uint16_t listenPort = 5353;
    TestType type       = TestType::kSrv;
} gOptions;

constexpr size_t kMdnsMaxPacketSize = 1'024;

using namespace chip::ArgParser;

constexpr uint16_t kOptionType = 't';

// non-ascii options have no short option version
constexpr uint16_t kOptionListenPort = 0x100;
constexpr uint16_t kOptionRuntimeMs  = 0x102;
constexpr uint16_t kOptionTraceTo    = 0x104;

// Only used for argument parsing. Tracing setup owned by the main loop.
chip::CommandLineApp::TracingSetup * tracing_setup_for_argparse = nullptr;

bool HandleOptions(const char * aProgram, OptionSet * aOptions, int aIdentifier, const char * aName, const char * aValue)
{
    switch (aIdentifier)
    {
    case kOptionListenPort:
        if (!ParseInt(aValue, gOptions.listenPort))
        {
            PrintArgError("%s: invalid value for listen port: %s\n", aProgram, aValue);
            return false;
        }
        return true;
    case kOptionTraceTo:
        tracing_setup_for_argparse->EnableTracingFor(aValue);
        return true;
    case kOptionType:
        if (strcasecmp(aValue, "SRV") == 0)
        {
            gOptions.type = TestType::kSrv;
        }
        else
        {
            PrintArgError("%s: invalid value for query type: %s\n", aProgram, aValue);
            return false;
        }
        return true;

    case kOptionRuntimeMs:
        if (!ParseInt(aValue, gOptions.runtimeMs))
        {
            PrintArgError("%s: invalid value for runtime ms: %s\n", aProgram, aValue);
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
    { "type", kArgumentRequired, kOptionType },
    { "timeout-ms", kArgumentRequired, kOptionRuntimeMs },
    { "trace-to", kArgumentRequired, kOptionTraceTo },
    {},
};

OptionSet cmdLineOptions = { HandleOptions, cmdLineOptionsDef, "PROGRAM OPTIONS",
                             "  --listen-port <number>\n"
                             "        The port number to listen on\n"
                             "  -t\n"
                             "  --type\n"
                             "        The packet to test with: \n"
                             "  --timeout-ms\n"
                             "        How long to wait for replies\n"
                             "  --trace-to <dest>\n"
                             "        trace to the given destination (supported: " SUPPORTED_COMMAND_LINE_TRACING_TARGETS ").\n"
                             "\n" };

HelpOptions helpOptions("minimal-mdns-tester", "Usage: minimal-mdns-tester [options]", "1.0");

OptionSet * allOptions[] = { &cmdLineOptions, &helpOptions, nullptr };

class ReportDelegate : public mdns::Minimal::ServerDelegate
{
public:
    void OnQuery(const mdns::Minimal::BytesRange & data, const chip::Inet::IPPacketInfo * info) override
    {
        char addr[Inet::IPAddress::kMaxStringLength];
        info->SrcAddress.ToString(addr, sizeof(addr));

        char ifName[Inet::InterfaceId::kMaxIfNameLength];
        VerifyOrDie(info->Interface.GetInterfaceName(ifName, sizeof(ifName)) == CHIP_NO_ERROR);

        printf("QUERY from: %-15s on port %d, via interface %s\n", addr, info->SrcPort, ifName);
        Report("QUERY: ", data);
    }

    void OnResponse(const mdns::Minimal::BytesRange & data, const chip::Inet::IPPacketInfo * info) override
    {
        char addr[Inet::IPAddress::kMaxStringLength];
        info->SrcAddress.ToString(addr, sizeof(addr));

        char ifName[Inet::InterfaceId::kMaxIfNameLength];
        VerifyOrDie(info->Interface.GetInterfaceName(ifName, sizeof(ifName)) == CHIP_NO_ERROR);

        printf("RESPONSE from: %-15s on port %d, via interface %s\n", addr, info->SrcPort, ifName);
        Report("RESPONSE: ", data);
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
};

System::PacketBufferHandle BuildSrvTestPacket()
{
    System::PacketBufferHandle packet = System::PacketBufferHandle::New(kMdnsMaxPacketSize);
    VerifyOrReturnValue(!packet.IsNull(), packet);

    mdns::Minimal::ResponseBuilder builder(std::move(packet));

    constexpr uint16_t kSrvPort                          = 5540;
    constexpr const char * kNodeName                     = "ABCD1234ABCD1234";
    constexpr const char * kNodeFabricName               = "ABCD1234ABCD1234-0000000000000001";
    constexpr mdns::Minimal::QNamePart kServiceName[]    = { Dnssd::kOperationalServiceName, Dnssd::kOperationalProtocol,
                                                             Dnssd::kLocalDomain };
    constexpr mdns::Minimal::QNamePart kServerFullName[] = { kNodeFabricName, Dnssd::kOperationalServiceName,
                                                             Dnssd::kOperationalProtocol, Dnssd::kLocalDomain };
    constexpr mdns::Minimal::QNamePart kServerName[]     = { kNodeName, Dnssd::kLocalDomain };

    mdns::Minimal::PtrResourceRecord ptrRecord(kServiceName, kServerFullName);
    mdns::Minimal::SrvResourceRecord srvRecord(kServerFullName, kServerName, kSrvPort);
    srvRecord.SetCacheFlush(true);

    builder.AddRecord(mdns::Minimal::ResourceType::kAnswer, ptrRecord);
    builder.AddRecord(mdns::Minimal::ResourceType::kAnswer, srvRecord);

    if (!builder.Ok())
    {
        printf("Failed to build response packet for SRV (maybe out of space?)\n");
        return System::PacketBufferHandle();
    }

    return builder.ReleasePacket();
}

System::PacketBufferHandle BuildTestPacket(TestType type)
{
    switch (type)
    {
    case TestType::kSrv:
        return BuildSrvTestPacket();
    default:
        return System::PacketBufferHandle();
    }
}

void BroadcastPacket(mdns::Minimal::ServerBase * server)
{
    System::PacketBufferHandle buffer = BuildTestPacket(gOptions.type);
    VerifyOrDie(!buffer.IsNull());

    CHIP_ERROR err = server->BroadcastSend(std::move(buffer), 5353);
    if (err != CHIP_NO_ERROR)
    {
        printf("Error sending: %" CHIP_ERROR_FORMAT "\n", err.Format());
        return;
    }
}

mdns::Minimal::Server<20> gMdnsServer;

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
    if (!chip::ArgParser::ParseArgs(args[0], argc, args, allOptions))
    {
        return 1;
    }
    tracing_setup_for_argparse = nullptr;

    printf("Running...\n");

    ReportDelegate reporter;
    CHIP_ERROR err;

    // This forces the global MDNS instance to be loaded in, effectively setting
    // built in policies for addresses.
    (void) chip::Dnssd::GlobalMinimalMdnsServer::Instance();

    gMdnsServer.SetDelegate(&reporter);

    {
        auto endpoints = mdns::Minimal::GetAddressPolicy()->GetListenEndpoints();

        err = gMdnsServer.Listen(chip::DeviceLayer::UDPEndPointManager(), endpoints.get(), gOptions.listenPort);
        if (err != CHIP_NO_ERROR)
        {
            printf("Server failed to listen on all interfaces: %s\n", chip::ErrorStr(err));
            return 1;
        }
    }

    BroadcastPacket(&gMdnsServer);

    err = DeviceLayer::SystemLayer().StartTimer(
        chip::System::Clock::Milliseconds32(gOptions.runtimeMs),
        [](System::Layer *, void *) {
            // Close all sockets BEFORE system layer is shut down, otherwise
            // attempts to free UDP sockets with system layer down will segfault
            gMdnsServer.Shutdown();

            DeviceLayer::PlatformMgr().StopEventLoopTask();
        },
        nullptr);
    if (err != CHIP_NO_ERROR)
    {
        printf("Failed to create the shutdown timer. Kill with ^C. %" CHIP_ERROR_FORMAT "\n", err.Format());
    }

    DeviceLayer::PlatformMgr().RunEventLoop();

    tracing_setup.StopTracing();
    DeviceLayer::PlatformMgr().Shutdown();
    Platform::MemoryShutdown();

    printf("Done...\n");
    return 0;
}
