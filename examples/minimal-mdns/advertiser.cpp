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
#include <strings.h>

#include <mdns/Advertiser.h>
#include <platform/CHIPDeviceLayer.h>
#include <support/CHIPArgParser.hpp>
#include <support/CHIPMem.h>

using namespace chip;

namespace {

enum class AdvertisingMode
{
    kCommisionable,
    kOperational,
};

struct Options
{
    bool enableIpV4                 = false;
    AdvertisingMode advertisingMode = AdvertisingMode::kCommisionable;

    // commisionable params
    uint8_t shortDiscriminator = 12;
    uint16_t longDiscriminator = 2233;
    Optional<uint64_t> vendorId;
    Optional<uint64_t> productId;

    // operational params
    uint64_t fabricId = 12345;
    uint64_t nodeId   = 6789;

} gOptions;

using namespace chip::ArgParser;

constexpr uint16_t kOptionEnableIpV4      = '4';
constexpr uint16_t kOptionAdvertisingMode = 'm';

constexpr uint16_t kOptionCommisionableShordDiscriminator = 's';
constexpr uint16_t kOptionCommisionableLongDiscriminaotr  = 'l';
constexpr uint16_t kOptionCommisionableVendorId           = 'v';
constexpr uint16_t kOptionCommisionableProductId          = 'p';

constexpr uint16_t kOptionOperationalFabricId = 'f';
constexpr uint16_t kOptionOperationalNodeId   = 'n';

bool HandleOptions(const char * aProgram, OptionSet * aOpotions, int aIdentifier, const char * aName, const char * aValue)
{
    switch (aIdentifier)
    {
    case kOptionEnableIpV4:
        gOptions.enableIpV4 = true;
        return true;
    case kOptionAdvertisingMode:
        if ((strcmp(aValue, "operational") == 0) || (strcmp(aValue, "o") == 0))
        {
            gOptions.advertisingMode = AdvertisingMode::kOperational;
        }
        else if ((strcmp(aValue, "commisionable") == 0) || (strcmp(aValue, "c") == 0))
        {
            gOptions.advertisingMode = AdvertisingMode::kCommisionable;
        }
        else
        {
            PrintArgError("%s: Invalid advertising mode %s\n", aProgram, aValue);

            return false;
        }
        return true;
    case kOptionCommisionableShordDiscriminator:
        gOptions.shortDiscriminator = static_cast<uint8_t>(atoi(aValue));
        return true;
    case kOptionCommisionableLongDiscriminaotr:
        gOptions.longDiscriminator = static_cast<uint16_t>(atoi(aValue));
        return true;
    case kOptionCommisionableVendorId:
        gOptions.vendorId = Optional<uint64_t>::Value(atoll(aValue));
        return true;
    case kOptionCommisionableProductId:
        gOptions.productId = Optional<uint64_t>::Value(atoll(aValue));
        return true;
    case kOptionOperationalFabricId:
        gOptions.fabricId = atoll(aValue);
        return true;
    case kOptionOperationalNodeId:
        gOptions.nodeId = atoll(aValue);
        return true;
    default:
        PrintArgError("%s: INTERNAL ERROR: Unhandled option: %s\n", aProgram, aName);
        return false;
    }
}

OptionDef cmdLineOptionsDef[] = {
    { "enable-ip-v4", kNoArgument, kOptionEnableIpV4 },
    { "advertising-mode", kArgumentRequired, kOptionAdvertisingMode },

    { "short-discriminator", kArgumentRequired, kOptionCommisionableShordDiscriminator },
    { "long-discriminator", kArgumentRequired, kOptionCommisionableLongDiscriminaotr },
    { "vendor-id", kArgumentRequired, kOptionCommisionableVendorId },
    { "product-id", kArgumentRequired, kOptionCommisionableProductId },

    { "fabrid-id", kArgumentRequired, kOptionOperationalFabricId },
    { "node-id", kArgumentRequired, kOptionOperationalNodeId },
    nullptr,
};

OptionSet cmdLineOptions = { HandleOptions, cmdLineOptionsDef, "PROGRAM OPTIONS",
                             "  -4\n"
                             "  --enable-ip-v4\n"
                             "        enable listening on IPv4\n"
                             "  -m <mode>\n"
                             "  --advertising-mode <mode>\n"
                             "        Advertise in this mode (o/operational or c/commisionable).\n"
                             "  --short-discriminator <value>\n"
                             "  -s <value>\n"
                             "        Commisionable short discriminator.\n"
                             "  --long-discriminator <value>\n"
                             "  -l <value>\n"
                             "        Commisionable long discriminator.\n"
                             "  --vendor-id <value>\n"
                             "  -v <value>\n"
                             "        Commisionable vendor id.\n"
                             "  --product-id <value>\n"
                             "  -p <value>\n"
                             "        Commisionable product id.\n"
                             "  --fabrid-id <value>\n"
                             "  -f <value>\n"
                             "        Operational fabric id.\n"
                             "  --node-id <value>\n"
                             "  -n <value>\n"
                             "        Operational node id.\n"
                             "\n" };

HelpOptions helpOptions("advertiser", "Usage: advertiser [options]", "1.0");

OptionSet * allOptions[] = { &cmdLineOptions, &helpOptions, nullptr };

} // namespace

int main(int argc, char ** args)
{
    if (Platform::MemoryInit() != CHIP_NO_ERROR)
    {
        printf("FAILED to initialize memory\n");
        return 1;
    }

    if (DeviceLayer::PlatformMgr().InitChipStack() != CHIP_NO_ERROR)
    {
        printf("FAILED to initialize chip stack\n");
        return 1;
    }

    if (!chip::ArgParser::ParseArgs(args[0], argc, args, allOptions))
    {
        return 1;
    }

    chip::Mdns::ServiceAdvertiser::Instance().Start(&DeviceLayer::InetLayer, Mdns::kMdnsPort);

    if (gOptions.advertisingMode == AdvertisingMode::kCommisionable)
    {
        chip::Mdns::ServiceAdvertiser::Instance().Advertise(chip::Mdns::CommisionableAdvertisingParameters()
                                                                .EnableIpV4(gOptions.enableIpV4)
                                                                .SetPort(CHIP_PORT)
                                                                .SetShortDiscriminator(gOptions.shortDiscriminator)
                                                                .SetLongDiscrimininator(gOptions.longDiscriminator)
                                                                .SetVendorId(gOptions.vendorId)
                                                                .SetProductId(gOptions.productId));
    }
    else if (gOptions.advertisingMode == AdvertisingMode::kOperational)
    {
        chip::Mdns::ServiceAdvertiser::Instance().Advertise(chip::Mdns::OperationalAdvertisingParameters()
                                                                .EnableIpV4(gOptions.enableIpV4)
                                                                .SetPort(CHIP_PORT)
                                                                .SetFabricId(gOptions.fabricId)
                                                                .SetNodeId(gOptions.nodeId));
    }
    else
    {
        printf("FAILED to determine advertising type.\n");
        return 1;
    }

    DeviceLayer::PlatformMgr().RunEventLoop();

    printf("Done...\n");
    return 0;
}
