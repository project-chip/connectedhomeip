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

#include <cstdio>
#include <memory>
#include <string>
#include <vector>

#include <inet/InetInterface.h>
#include <inet/UDPEndPoint.h>
#include <lib/dnssd/MinimalMdnsServer.h>
#include <lib/dnssd/minimal_mdns/AddressPolicy.h>
#include <lib/dnssd/minimal_mdns/QueryBuilder.h>
#include <lib/dnssd/minimal_mdns/Server.h>
#include <lib/dnssd/minimal_mdns/core/QName.h>
#include <lib/support/CHIPArgParser.hpp>
#include <lib/support/CHIPMem.h>
#include <platform/CHIPDeviceLayer.h>
#include <system/SystemPacketBuffer.h>

#include "PacketReporter.h"

using namespace chip;

namespace {

struct Options
{
    bool unicastAnswers       = true;
    uint32_t runtimeMs        = 500;
    uint16_t querySendPort    = 5353;
    uint16_t listenPort       = 5388;
    const char * query        = "_services._dns-sd._udp.local";
    mdns::Minimal::QType type = mdns::Minimal::QType::ANY;
} gOptions;

constexpr uint32_t kTestMessageId   = 0;
constexpr size_t kMdnsMaxPacketSize = 1'024;

using namespace chip::ArgParser;

constexpr uint16_t kOptionQuery = 'q';
constexpr uint16_t kOptionType  = 't';

// non-ascii options have no short option version
constexpr uint16_t kOptionListenPort       = 0x100;
constexpr uint16_t kOptionQueryPort        = 0x101;
constexpr uint16_t kOptionRuntimeMs        = 0x102;
constexpr uint16_t kOptionMulticastReplies = 0x103;

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
    case kOptionQuery:
        gOptions.query = aValue;
        return true;
    case kOptionType:
        if (strcasecmp(aValue, "ANY") == 0)
        {
            gOptions.type = mdns::Minimal::QType::ANY;
        }
        else if (strcasecmp(aValue, "A") == 0)
        {
            gOptions.type = mdns::Minimal::QType::A;
        }
        else if (strcasecmp(aValue, "AAAA") == 0)
        {
            gOptions.type = mdns::Minimal::QType::AAAA;
        }
        else if (strcasecmp(aValue, "PTR") == 0)
        {
            gOptions.type = mdns::Minimal::QType::PTR;
        }
        else if (strcasecmp(aValue, "TXT") == 0)
        {
            gOptions.type = mdns::Minimal::QType::TXT;
        }
        else if (strcasecmp(aValue, "SRV") == 0)
        {
            gOptions.type = mdns::Minimal::QType::SRV;
        }
        else if (strcasecmp(aValue, "CNAME") == 0)
        {
            gOptions.type = mdns::Minimal::QType::CNAME;
        }
        else
        {
            PrintArgError("%s: invalid value for query type: %s\n", aProgram, aValue);
            return false;
        }
        return true;

    case kOptionQueryPort:
        if (!ParseInt(aValue, gOptions.querySendPort))
        {
            PrintArgError("%s: invalid value for query send port: %s\n", aProgram, aValue);
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

    case kOptionMulticastReplies:
        gOptions.unicastAnswers = false;
        return true;

    default:
        PrintArgError("%s: INTERNAL ERROR: Unhandled option: %s\n", aProgram, aName);
        return false;
    }
}

OptionDef cmdLineOptionsDef[] = {
    { "listen-port", kArgumentRequired, kOptionListenPort },
    { "query", kArgumentRequired, kOptionQuery },
    { "type", kArgumentRequired, kOptionType },
    { "query-port", kArgumentRequired, kOptionQueryPort },
    { "timeout-ms", kArgumentRequired, kOptionRuntimeMs },
    { "multicast-reply", kNoArgument, kOptionMulticastReplies },
    {},
};

OptionSet cmdLineOptions = { HandleOptions, cmdLineOptionsDef, "PROGRAM OPTIONS",
                             "  --listen-port <number>\n"
                             "        The port number to listen on\n"
                             "  -q\n"
                             "  --query\n"
                             "        The query to send\n"
                             "  -t\n"
                             "  --type\n"
                             "        The query type to ask for (ANY/PTR/A/AAAA/SRV/TXT)\n"
                             "  --query-port\n"
                             "        On what port to multicast the query\n"
                             "  --timeout-ms\n"
                             "        How long to wait for replies\n"
                             "  --multicast-reply\n"
                             "        Do not request unicast replies\n"
                             "\n" };

HelpOptions helpOptions("minimal-mdns-client", "Usage: minimal-mdns-client [options]", "1.0");

OptionSet * allOptions[] = { &cmdLineOptions, &helpOptions, nullptr };

class ReportDelegate : public mdns::Minimal::ServerDelegate
{
public:
    void OnQuery(const mdns::Minimal::BytesRange & data, const chip::Inet::IPPacketInfo * info) override
    {
        char addr[32];
        info->SrcAddress.ToString(addr, sizeof(addr));

        char ifName[64];
        VerifyOrDie(info->Interface.GetInterfaceName(ifName, sizeof(ifName)) == CHIP_NO_ERROR);

        printf("QUERY from: %-15s on port %d, via interface %s\n", addr, info->SrcPort, ifName);
        Report("QUERY: ", data);
    }

    void OnResponse(const mdns::Minimal::BytesRange & data, const chip::Inet::IPPacketInfo * info) override
    {
        char addr[32];
        info->SrcAddress.ToString(addr, sizeof(addr));

        char ifName[64];
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

class QuerySplitter
{
public:
    void Split(const char * query)
    {
        mStorage.clear();
        mParts.clear();

        const char * dot = nullptr;
        while (nullptr != (dot = strchr(query, '.')))
        {
            mStorage.push_back(std::string(query, dot));
            query = dot + 1;
        }

        mStorage.push_back(query);

        for (unsigned i = 0; i < mStorage.size(); i++)
        {
            mParts.push_back(mStorage[i].c_str());
        }
    }

    mdns::Minimal::Query MdnsQuery() const
    {
        mdns::Minimal::FullQName qName;

        qName.nameCount = mParts.size();
        qName.names     = mParts.data();

        return mdns::Minimal::Query(qName);
    }

private:
    std::vector<mdns::Minimal::QNamePart> mParts;
    std::vector<std::string> mStorage;
};

void BroadcastPacket(mdns::Minimal::ServerBase * server)
{
    System::PacketBufferHandle buffer = System::PacketBufferHandle::New(kMdnsMaxPacketSize);
    VerifyOrDie(!buffer.IsNull());

    QuerySplitter query;
    query.Split(gOptions.query);

    mdns::Minimal::QueryBuilder builder(std::move(buffer));

    builder.Header().SetMessageId(kTestMessageId);
    builder.AddQuery(query
                         .MdnsQuery()                                  //
                         .SetClass(mdns::Minimal::QClass::IN)          //
                         .SetType(gOptions.type)                       //
                         .SetAnswerViaUnicast(gOptions.unicastAnswers) //
    );

    if (!builder.Ok())
    {
        printf("Failed to build the question");
        return;
    }

    if (gOptions.unicastAnswers)
    {
        if (server->BroadcastUnicastQuery(builder.ReleasePacket(), gOptions.querySendPort) != CHIP_NO_ERROR)
        {
            printf("Error sending\n");
            return;
        }
    }
    else
    {
        if (server->BroadcastSend(builder.ReleasePacket(), gOptions.querySendPort) != CHIP_NO_ERROR)
        {
            printf("Error sending\n");
            return;
        }
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

    if (!chip::ArgParser::ParseArgs(args[0], argc, args, allOptions))
    {
        return 1;
    }

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
            DeviceLayer::PlatformMgr().Shutdown();
        },
        nullptr);
    if (err != CHIP_NO_ERROR)
    {
        printf("Failed to create the shutdown timer. Kill with ^C. %" CHIP_ERROR_FORMAT "\n", err.Format());
    }

    DeviceLayer::PlatformMgr().RunEventLoop();

    printf("Done...\n");
    return 0;
}
