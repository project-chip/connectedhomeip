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
#include <optional>

#include <arpa/inet.h>
#include <strings.h>

#include <TracingCommandLineArgument.h>
#include <lib/dnssd/Advertiser.h>
#include <lib/support/CHIPArgParser.hpp>
#include <lib/support/CHIPMem.h>
#include <lib/support/Span.h>
#include <platform/CHIPDeviceLayer.h>

using namespace chip;

namespace {

enum class AdvertisingMode
{
    kCommissionableNode,
    kOperational,
    kOperationalMultiAdmin,
    kCommissioner,
};

struct Options
{
    bool enableIpV4                 = false;
    AdvertisingMode advertisingMode = AdvertisingMode::kCommissionableNode;

    // commissionable node / commissioner params
    std::optional<uint16_t> vendorId;
    std::optional<uint16_t> productId;
    std::optional<uint32_t> deviceType;
    std::optional<const char *> deviceName;

    // commissionable node params
    uint8_t shortDiscriminator                 = 52;
    uint16_t longDiscriminator                 = 840;
    Dnssd::CommissioningMode commissioningMode = Dnssd::CommissioningMode::kDisabled;
    std::optional<const char *> rotatingId;
    std::optional<const char *> pairingInstr;
    std::optional<uint16_t> pairingHint;

    // operational params
    uint64_t fabricId = 12345;
    uint64_t nodeId   = 6789;
    uint8_t mac[6]    = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06 };

} gOptions;

using namespace chip::ArgParser;

constexpr uint16_t kOptionEnableIpV4      = '4';
constexpr uint16_t kOptionAdvertisingMode = 'm';

constexpr uint16_t kOptionCommissioningShortDiscriminator = 's';
constexpr uint16_t kOptionCommissioningLongDiscriminaotr  = 'l';
constexpr uint16_t kOptionCommissioningVendorId           = 0x100; // v is used by 'version'
constexpr uint16_t kOptionCommissioningProductId          = 'p';
constexpr uint16_t kOptionCommissioningPairingInstr       = 0x200; // Just use the long format
constexpr uint16_t kOptionCommissioningPairingHint        = 0x300;
constexpr uint16_t kOptionCommissioningDeviceType         = 0x400;
constexpr uint16_t kOptionCommissioningDeviceName         = 0x500;
constexpr uint16_t kOptionCommissioningMode               = 0x600;
constexpr uint16_t kOptionCommissioningRotatingId         = 0x700;

constexpr uint16_t kOptionOperationalFabricId = 'f';
constexpr uint16_t kOptionOperationalNodeId   = 'n';
constexpr uint16_t kOptionTraceTo             = 't';

// Only used for argument parsing. Tracing setup owned by the main loop.
chip::CommandLineApp::TracingSetup * tracing_setup_for_argparse = nullptr;

bool HandleOptions(const char * aProgram, OptionSet * aOptions, int aIdentifier, const char * aName, const char * aValue)
{
    uint8_t cm;
    switch (aIdentifier)
    {
    case kOptionEnableIpV4:
        gOptions.enableIpV4 = true;
        return true;
    case kOptionTraceTo:
        tracing_setup_for_argparse->EnableTracingFor(aValue);
        return true;
    case kOptionAdvertisingMode:
        if (strcmp(aValue, "operational") == 0)
        {
            gOptions.advertisingMode = AdvertisingMode::kOperational;
        }
        else if (strcmp(aValue, "operational-multi-admin") == 0)
        {
            gOptions.advertisingMode = AdvertisingMode::kOperationalMultiAdmin;
        }
        else if (strcmp(aValue, "commissionable-node") == 0)
        {
            gOptions.advertisingMode = AdvertisingMode::kCommissionableNode;
        }
        else if (strcmp(aValue, "commissioner") == 0)
        {
            gOptions.advertisingMode = AdvertisingMode::kCommissioner;
        }
        else
        {
            PrintArgError("%s: Invalid advertising mode %s\n", aProgram, aValue);

            return false;
        }
        return true;
    case kOptionCommissioningShortDiscriminator:
        gOptions.shortDiscriminator = static_cast<uint8_t>(atoi(aValue));
        return true;
    case kOptionCommissioningLongDiscriminaotr:
        gOptions.longDiscriminator = static_cast<uint16_t>(atoi(aValue));
        return true;
    case kOptionCommissioningVendorId:
        gOptions.vendorId = std::make_optional<uint16_t>(static_cast<uint16_t>(atoi(aValue)));
        return true;
    case kOptionCommissioningProductId:
        gOptions.productId = std::make_optional<uint16_t>(static_cast<uint16_t>(atoi(aValue)));
        return true;
    case kOptionCommissioningMode:
        cm = static_cast<uint8_t>(atoi(aValue));
        if (cm == 1)
        {
            gOptions.commissioningMode = Dnssd::CommissioningMode::kEnabledBasic;
        }
        if (cm == 2)
        {
            gOptions.commissioningMode = Dnssd::CommissioningMode::kEnabledEnhanced;
        }
        return true;
    case kOptionCommissioningDeviceType:
        gOptions.deviceType = std::make_optional<uint32_t>(static_cast<uint32_t>(atoi(aValue)));
        return true;
    case kOptionCommissioningDeviceName:
        gOptions.deviceName = std::make_optional<const char *>(static_cast<const char *>(aValue));
        return true;
    case kOptionCommissioningRotatingId:
        gOptions.rotatingId = std::make_optional<const char *>(static_cast<const char *>(aValue));
        return true;
    case kOptionCommissioningPairingInstr:
        gOptions.pairingInstr = std::make_optional<const char *>(static_cast<const char *>(aValue));
        return true;
    case kOptionCommissioningPairingHint:
        gOptions.pairingHint = std::make_optional<uint16_t>(static_cast<uint16_t>(atoi(aValue)));
        return true;
    case kOptionOperationalFabricId:
        if (sscanf(aValue, "%" SCNx64, &gOptions.fabricId) != 1)
        {
            PrintArgError("%s: Invalid fabric id: %s\n", aProgram, aValue);
            return false;
        }
        return true;
    case kOptionOperationalNodeId:
        if (sscanf(aValue, "%" SCNx64, &gOptions.nodeId) != 1)
        {
            PrintArgError("%s: Invalid node id: %s\n", aProgram, aValue);
            return false;
        }
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

    { "short-discriminator", kArgumentRequired, kOptionCommissioningShortDiscriminator },
    { "long-discriminator", kArgumentRequired, kOptionCommissioningLongDiscriminaotr },
    { "vendor-id", kArgumentRequired, kOptionCommissioningVendorId },
    { "product-id", kArgumentRequired, kOptionCommissioningProductId },
    { "commissioning-mode", kNoArgument, kOptionCommissioningMode },
    { "device-type", kArgumentRequired, kOptionCommissioningDeviceType },
    { "device-name", kArgumentRequired, kOptionCommissioningDeviceName },
    { "rotating-id", kArgumentRequired, kOptionCommissioningRotatingId },
    { "pairing-instruction", kArgumentRequired, kOptionCommissioningPairingInstr },
    { "pairing-hint", kArgumentRequired, kOptionCommissioningPairingHint },

    { "fabric-id", kArgumentRequired, kOptionOperationalFabricId },
    { "node-id", kArgumentRequired, kOptionOperationalNodeId },
    { "trace-to", kArgumentRequired, kOptionTraceTo },
    {},
};

OptionSet cmdLineOptions = {
    HandleOptions, cmdLineOptionsDef, "PROGRAM OPTIONS",
#if INET_CONFIG_ENABLE_IPV4
    "  -4\n"
    "  --enable-ip-v4\n"
    "        enable listening on IPv4\n"
#endif
    "  -m <mode>\n"
    "  --advertising-mode <mode>\n"
    "        Advertise in this mode (operational, operational-multi-admin, commissionable-node or commissioner).\n"
    "  --short-discriminator <value>\n"
    "  -s <value>\n"
    "        Commissioning/commissionable short discriminator.\n"
    "  --long-discriminator <value>\n"
    "  -l <value>\n"
    "        Commissioning/commissionable long discriminator.\n"
    "  --vendor-id <value>\n"
    "        Commissioning/commissionable vendor id.\n"
    "  --product-id <value>\n"
    "  -p <value>\n"
    "        Commissioning/commissionable product id.\n"
    "  --commissioning-mode <value>\n"
    "        Commissioning Mode (0=disabled, 1=basic, 2=enhanced).\n"
    "  --device-type <value>\n"
    "        Device type id.\n"
    "  --device-name <value>\n"
    "        Name of device.\n"
    "  --rotating-id <value>\n"
    "        Rotating Id.\n"
    "  --pairing-instruction <value>\n"
    "        Commissionable pairing instruction.\n"
    "  --pairing-hint <value>\n"
    "        Commissionable pairing hint.\n"
    "  --fabric-id <value>\n"
    "  -f <value>\n"
    "        Operational fabric id.\n"
    "  --node-id <value>\n"
    "  -n <value>\n"
    "        Operational node id.\n"
    "  -t <dest>\n"
    "  --trace-to <dest>\n"
    "        trace to the given destination (supported: " SUPPORTED_COMMAND_LINE_TRACING_TARGETS ").\n"
    "\n"
};

HelpOptions helpOptions("advertiser", "Usage: advertiser [options]", "1.0");

OptionSet * allOptions[] = { &cmdLineOptions, &helpOptions, nullptr };

void StopSignalHandler(int signal)
{
    DeviceLayer::PlatformMgr().StopEventLoopTask();
}

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

    chip::CommandLineApp::TracingSetup tracing_setup;

    tracing_setup_for_argparse = &tracing_setup;
    if (!chip::ArgParser::ParseArgs(args[0], argc, args, allOptions))
    {
        return 1;
    }
    tracing_setup_for_argparse = nullptr;

    if (chip::Dnssd::ServiceAdvertiser::Instance().Init(DeviceLayer::UDPEndPointManager()) != CHIP_NO_ERROR)
    {
        fprintf(stderr, "FAILED to start MDNS advertisement\n");
        return 1;
    }

    CHIP_ERROR err;

    if (gOptions.advertisingMode == AdvertisingMode::kCommissionableNode)
    {
        printf("Advertise Commissionable Node\n");
        err = chip::Dnssd::ServiceAdvertiser::Instance().Advertise(chip::Dnssd::CommissionAdvertisingParameters()
                                                                       .EnableIpV4(gOptions.enableIpV4)
                                                                       .SetPort(CHIP_PORT)
                                                                       .SetShortDiscriminator(gOptions.shortDiscriminator)
                                                                       .SetLongDiscriminator(gOptions.longDiscriminator)
                                                                       .SetMac(chip::ByteSpan(gOptions.mac, 6))
                                                                       .SetVendorId(gOptions.vendorId)
                                                                       .SetProductId(gOptions.productId)
                                                                       .SetCommissioningMode(gOptions.commissioningMode)
                                                                       .SetDeviceType(gOptions.deviceType)
                                                                       .SetDeviceName(gOptions.deviceName)
                                                                       .SetRotatingDeviceId(gOptions.rotatingId)
                                                                       .SetPairingInstruction(gOptions.pairingInstr)
                                                                       .SetPairingHint(gOptions.pairingHint));
    }
    else if (gOptions.advertisingMode == AdvertisingMode::kOperational)
    {
        err = chip::Dnssd::ServiceAdvertiser::Instance().Advertise(
            chip::Dnssd::OperationalAdvertisingParameters()
                .EnableIpV4(gOptions.enableIpV4)
                .SetPort(CHIP_PORT)
                .SetMac(chip::ByteSpan(gOptions.mac, 6))
                .SetPeerId(PeerId().SetCompressedFabricId(gOptions.fabricId).SetNodeId(gOptions.nodeId)));
    }
    else if (gOptions.advertisingMode == AdvertisingMode::kOperationalMultiAdmin)
    {
        err = chip::Dnssd::ServiceAdvertiser::Instance().Advertise(
            chip::Dnssd::OperationalAdvertisingParameters()
                .EnableIpV4(gOptions.enableIpV4)
                .SetPort(CHIP_PORT)
                .SetMac(chip::ByteSpan(gOptions.mac, 6))
                .SetPeerId(PeerId().SetCompressedFabricId(gOptions.fabricId).SetNodeId(gOptions.nodeId)));

        if (err == CHIP_NO_ERROR)
        {
            err = chip::Dnssd::ServiceAdvertiser::Instance().Advertise(
                chip::Dnssd::OperationalAdvertisingParameters()
                    .EnableIpV4(gOptions.enableIpV4)
                    .SetPort(CHIP_PORT + 1)
                    .SetMac(chip::ByteSpan(gOptions.mac, 6))
                    .SetPeerId(PeerId().SetCompressedFabricId(gOptions.fabricId + 1).SetNodeId(gOptions.nodeId + 1)));
        }

        if (err == CHIP_NO_ERROR)
        {
            err = chip::Dnssd::ServiceAdvertiser::Instance().Advertise(
                chip::Dnssd::OperationalAdvertisingParameters()
                    .EnableIpV4(gOptions.enableIpV4)
                    .SetPort(CHIP_PORT + 2)
                    .SetMac(chip::ByteSpan(gOptions.mac, 6))
                    .SetPeerId(PeerId().SetCompressedFabricId(gOptions.fabricId + 2).SetNodeId(gOptions.nodeId + 2)));
        }
    }
    else if (gOptions.advertisingMode == AdvertisingMode::kCommissioner)
    {
        printf("Advertise Commissioner\n");
        err = chip::Dnssd::ServiceAdvertiser::Instance().Advertise(
            chip::Dnssd::CommissionAdvertisingParameters()
                .EnableIpV4(gOptions.enableIpV4)
                .SetPort(CHIP_PORT)
                .SetMac(chip::ByteSpan(gOptions.mac, 6))
                .SetVendorId(gOptions.vendorId)
                .SetProductId(gOptions.productId)
                .SetDeviceType(gOptions.deviceType)
                .SetDeviceName(gOptions.deviceName)
                .SetCommissionAdvertiseMode(chip::Dnssd::CommssionAdvertiseMode::kCommissioner));
    }
    else
    {
        fprintf(stderr, "FAILED to determine advertising type.\n");
        return 1;
    }

    if (err != CHIP_NO_ERROR)
    {
        fprintf(stderr, "FAILED to setup advertisement parameters err=%s\n", chip::ErrorStr(err));
        return 1;
    }

    signal(SIGTERM, StopSignalHandler);
    signal(SIGINT, StopSignalHandler);

    DeviceLayer::PlatformMgr().RunEventLoop();

    tracing_setup.StopTracing();
    Dnssd::Resolver::Instance().Shutdown();
    DeviceLayer::PlatformMgr().Shutdown();

    printf("Done...\n");
    return 0;
}
