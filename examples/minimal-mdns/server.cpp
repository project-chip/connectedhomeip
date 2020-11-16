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
#include <mdns/minimal/QName.h>
#include <mdns/minimal/QueryBuilder.h>
#include <mdns/minimal/Server.h>
#include <platform/CHIPDeviceLayer.h>
#include <support/CHIPArgParser.hpp>
#include <support/CHIPMem.h>
#include <system/SystemPacketBuffer.h>
#include <system/SystemTimer.h>

#include "AllInterfaceListener.h"
#include "PacketReporter.h"

using namespace chip;

namespace {

struct Options
{
    bool enableIpV4     = false;
    uint16_t listenPort = 5353;
} gOptions;

using namespace chip::ArgParser;

constexpr uint16_t kOptionEnableIpV4 = '4';
constexpr uint16_t kOptionListenPort = 'p';

bool HandleOptions(const char * aProgram, OptionSet * aOpotions, int aIdentifier, const char * aName, const char * aValue)
{
    switch (aIdentifier)
    {
    case kOptionEnableIpV4:
        gOptions.enableIpV4 = true;
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
    nullptr,
};

OptionSet cmdLineOptions = { HandleOptions, cmdLineOptionsDef, "PROGRAM OPTIONS",
                             "  -p <number>\n"
                             "  --listen-port <number>\n"
                             "        The port number to listen on\n"
                             "  -4\n"
                             "  --enable-ip-v4\n"
                             "        enable listening on IPv4\n"
                             "\n" };

HelpOptions helpOptions("minimal-mdns-server", "Usage: minimal-mdns-server [options]", "1.0");

OptionSet * allOptions[] = { &cmdLineOptions, &helpOptions, nullptr };

class ReplyDelegate : public mdns::Minimal::ServerDelegate
{
public:
    void OnQuery(const mdns::Minimal::BytesRange & data, const chip::Inet::IPPacketInfo * info) override
    {
        char addr[32];
        info->SrcAddress.ToString(addr, sizeof(addr));

        printf("QUERY from: %-15s on port %d, via interface %d\n", addr, info->SrcPort, info->Interface);
        Report("QUERY: ", data);
    }

    void OnResponse(const mdns::Minimal::BytesRange & data, const chip::Inet::IPPacketInfo * info) override
    {
        char addr[32];
        info->SrcAddress.ToString(addr, sizeof(addr));

        printf("RESPONSE from: %-15s on port %d, via interface %d\n", addr, info->SrcPort, info->Interface);
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

    printf("Running on port %d using %s...\n", gOptions.listenPort, gOptions.enableIpV4 ? "IPv4 AND IPv6" : "IPv6 ONLY");

    mdns::Minimal::Server<10> mdnsServer;

    ReplyDelegate delegate;
    mdnsServer.SetDelegate(&delegate);

    {
        MdnsExample::AllInterfaces allInterfaces(gOptions.enableIpV4);

        // if (mdnsServer.Listen(&allInterfaces, kFakeMdnsPort) != CHIP_NO_ERROR)
        if (mdnsServer.Listen(&allInterfaces, gOptions.listenPort) != CHIP_NO_ERROR)
        {
            printf("Server failed to listen on all interfaces\n");
            return 1;
        }
    }

    DeviceLayer::PlatformMgr().RunEventLoop();

    printf("Done...\n");
    return 0;
}
