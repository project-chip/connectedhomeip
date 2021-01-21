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
    kCommisioning,
    kOperational,
    kCommisionable,
};

struct Options
{
    bool enableIpV4                 = false;
    AdvertisingMode advertisingMode = AdvertisingMode::kCommisioning;

    // commisioning/commisionable params
    uint8_t shortDiscriminator = 52;
    uint16_t longDiscriminator = 840;
    Optional<uint16_t> vendorId;
    Optional<uint16_t> productId;

    // commisionable params
    Optional<const char *> pairingInstr;
    Optional<uint8_t> pairingHint;

    // operational params
    uint64_t fabricId = 12345;
    uint64_t nodeId   = 6789;

} gOptions;

using namespace chip::ArgParser;

constexpr uint16_t kOptionEnableIpV4      = '4';
constexpr uint16_t kOptionAdvertisingMode = 'm';

constexpr uint16_t kOptionCommisioningShordDiscriminator = 's';
constexpr uint16_t kOptionCommisioningLongDiscriminaotr  = 'l';
constexpr uint16_t kOptionCommisioningVendorId           = 0x100; // v is used by 'version'
constexpr uint16_t kOptionCommisioningProductId          = 'p';
constexpr uint16_t kOptionCommisioningPairingInstr       = 0x200; // Just use the long format
constexpr uint16_t kOptionCommisioningPairingHint        = 0x300;

constexpr uint16_t kOptionOperationalFabricId = 'f';
constexpr uint16_t kOptionOperationalNodeId   = 'n';

bool HandleOptions(const char * aProgram, OptionSet * aOptions, int aIdentifier, const char * aName, const char * aValue)
{
    switch (aIdentifier)
    {
    case kOptionEnableIpV4:
        gOptions.enableIpV4 = true;
        return true;
    case kOptionAdvertisingMode:
        if (strcmp(aValue, "operational") == 0)
        {
            gOptions.advertisingMode = AdvertisingMode::kOperational;
        }
        else if (strcmp(aValue, "commisioning") == 0)
        {
            gOptions.advertisingMode = AdvertisingMode::kCommisioning;
        }
        else if (strcmp(aValue, "commisionable") == 0)
        {
            gOptions.advertisingMode = AdvertisingMode::kCommisionable;
        }
        else
        {
            PrintArgError("%s: Invalid advertising mode %s\n", aProgram, aValue);

            return false;
        }
        return true;
    case kOptionCommisioningShordDiscriminator:
        gOptions.shortDiscriminator = static_cast<uint8_t>(atoi(aValue));
        return true;
    case kOptionCommisioningLongDiscriminaotr:
        gOptions.longDiscriminator = static_cast<uint16_t>(atoi(aValue));
        return true;
    case kOptionCommisioningVendorId:
        gOptions.vendorId = Optional<uint16_t>::Value(static_cast<uint16_t>(atoi(aValue)));
        return true;
    case kOptionCommisioningProductId:
        gOptions.productId = Optional<uint16_t>::Value(static_cast<uint16_t>(atoi(aValue)));
        return true;
    case kOptionCommisioningPairingInstr:
        gOptions.pairingInstr = Optional<const char *>::Value(static_cast<const char *>(aValue));
        return true;
    case kOptionCommisioningPairingHint:
        gOptions.pairingHint = Optional<uint8_t>::Value(static_cast<uint8_t>(atoi(aValue)));
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
#if INET_CONFIG_ENABLE_IPV4
    { "enable-ip-v4", kNoArgument, kOptionEnableIpV4 },
#endif
    { "advertising-mode", kArgumentRequired, kOptionAdvertisingMode },

    { "short-discriminator", kArgumentRequired, kOptionCommisioningShordDiscriminator },
    { "long-discriminator", kArgumentRequired, kOptionCommisioningLongDiscriminaotr },
    { "vendor-id", kArgumentRequired, kOptionCommisioningVendorId },
    { "product-id", kArgumentRequired, kOptionCommisioningProductId },
    { "pairing-instruction", kArgumentRequired, kOptionCommisioningPairingInstr },
    { "pairing-hint", kArgumentRequired, kOptionCommisioningPairingHint },

    { "fabrid-id", kArgumentRequired, kOptionOperationalFabricId },
    { "node-id", kArgumentRequired, kOptionOperationalNodeId },
    nullptr,
};

OptionSet cmdLineOptions = { HandleOptions, cmdLineOptionsDef, "PROGRAM OPTIONS",
#if INET_CONFIG_ENABLE_IPV4
                             "  -4\n"
                             "  --enable-ip-v4\n"
                             "        enable listening on IPv4\n"
#endif
                             "  -m <mode>\n"
                             "  --advertising-mode <mode>\n"
                             "        Advertise in this mode (operational or commisioning or commisionable).\n"
                             "  --short-discriminator <value>\n"
                             "  -s <value>\n"
                             "        Commisioning/commisionable short discriminator.\n"
                             "  --long-discriminator <value>\n"
                             "  -l <value>\n"
                             "        Commisioning/commisionable long discriminator.\n"
                             "  --vendor-id <value>\n"
                             "        Commisioning/commisionable vendor id.\n"
                             "  --product-id <value>\n"
                             "  -p <value>\n"
                             "        Commisioning/commisionable product id.\n"
                             "  --pairing-instruction <value>\n"
                             "        Commisionable pairing instruction.\n"
                             "  --pairing-hint <value>\n"
                             "        Commisionable pairing hint.\n"
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
        fprintf(stderr, "FAILED to initialize memory\n");
        return 1;
    }

    if (DeviceLayer::PlatformMgr().InitChipStack() != CHIP_NO_ERROR)
    {
        fprintf(stderr, "FAILED to initialize chip stack\n");
        return 1;
    }

    if (!chip::ArgParser::ParseArgs(args[0], argc, args, allOptions))
    {
        return 1;
    }

    if (chip::Mdns::ServiceAdvertiser::Instance().Start(&DeviceLayer::InetLayer, Mdns::kMdnsPort) != CHIP_NO_ERROR)
    {
        fprintf(stderr, "FAILED to start MDNS advertisement\n");
        return 1;
    }

    CHIP_ERROR err;

    if (gOptions.advertisingMode == AdvertisingMode::kCommisioning)
    {
        err = chip::Mdns::ServiceAdvertiser::Instance().Advertise(chip::Mdns::CommissionAdvertisingParameters()
                                                                      .EnableIpV4(gOptions.enableIpV4)
                                                                      .SetPort(CHIP_PORT)
                                                                      .SetShortDiscriminator(gOptions.shortDiscriminator)
                                                                      .SetLongDiscrimininator(gOptions.longDiscriminator)
                                                                      .SetVendorId(gOptions.vendorId)
                                                                      .SetProductId(gOptions.productId));
    }
    else if (gOptions.advertisingMode == AdvertisingMode::kOperational)
    {
        err = chip::Mdns::ServiceAdvertiser::Instance().Advertise(chip::Mdns::OperationalAdvertisingParameters()
                                                                      .EnableIpV4(gOptions.enableIpV4)
                                                                      .SetPort(CHIP_PORT)
                                                                      .SetFabricId(gOptions.fabricId)
                                                                      .SetNodeId(gOptions.nodeId));
    }
    else if (gOptions.advertisingMode == AdvertisingMode::kCommisionable)
    {
        err = chip::Mdns::ServiceAdvertiser::Instance().Advertise(
            chip::Mdns::CommissionAdvertisingParameters()
                .EnableIpV4(gOptions.enableIpV4)
                .SetPort(CHIP_PORT)
                .SetShortDiscriminator(gOptions.shortDiscriminator)
                .SetLongDiscrimininator(gOptions.longDiscriminator)
                .SetVendorId(gOptions.vendorId)
                .SetProductId(gOptions.productId)
                .SetPairingInstr(gOptions.pairingInstr)
                .SetPairingHint(gOptions.pairingHint)
                .SetCommissionAdvertiseMode(chip::Mdns::CommssionAdvertiseMode::kCommissionable));
    }
    else
    {
        fprintf(stderr, "FAILED to determine advertising type.\n");
        return 1;
    }

    if (err != CHIP_NO_ERROR)
    {
        fprintf(stderr, "FAILED to setup advertisement parameters\n");
        return 1;
    }

    DeviceLayer::PlatformMgr().RunEventLoop();

    printf("Done...\n");
    return 0;
}
