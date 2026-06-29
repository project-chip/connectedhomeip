/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
 *    All rights reserved.
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

#include <app_options/AppOptions.h>
#include <device-factory/DeviceFactory.h>
#include <lib/support/CodeUtils.h>
#include <platform/CHIPDeviceConfig.h>

#include <algorithm>
#include <cstdlib>
#include <cstring>

using namespace chip;
using namespace chip::ArgParser;

namespace {

bool IsExcludedFromWildcard(const std::string & type)
{
    // These device types are excluded from the wildcard (*) expansion to prevent redundant,
    // invalid, or non-leaf endpoint structures.
    static const std::vector<std::string> kExcludedDevices = {
        "aggregator",   // Top-level container representing a bridge; not a standalone leaf device.
        "bridged-node", // Base class representing a bridged endpoint wrapper; not a standalone leaf device.
    };
    return std::any_of(kExcludedDevices.begin(), kExcludedDevices.end(),
                       [&type](const auto & excluded) { return excluded == type; });
}

} // namespace

// App custom argument handling
constexpr uint16_t kOptionDeviceType    = 0xffd0;
constexpr uint16_t kOptionWiFi          = 0xffd2;
constexpr uint16_t kOptionKVS           = 0xffd3;
constexpr uint16_t kOptionDiscriminator = 0xffd4;
constexpr uint16_t kOptionVendorId      = 0xffd5;
constexpr uint16_t kOptionProductId     = 0xffd6;
constexpr uint16_t kOptionPort          = 0xffd7;
constexpr uint16_t kOptionInterfaceId   = 0xffd8;
constexpr uint16_t kOptionBLE           = 0xffd9;
constexpr uint16_t kOptionGroupcast     = 0xffda;
constexpr uint16_t kOptionAppPipe       = 0xffdb;
constexpr uint16_t kOptionTraceTo       = 0xffdc;

DeviceTypeParser AppOptions::sParser;
AppOptions::AppConfig AppOptions::mConfig;
bool AppOptions::sIsConfigValidated = false;

const AppOptions::AppConfig & AppOptions::GetConfig()
{
    VerifyOrDie(sIsConfigValidated);
    return mConfig;
}

CHIP_ERROR AppOptions::ValidateConfig()
{
    // Default device fallback if no devices are configured
    if (mConfig.deviceTypeEntries.empty())
    {
        mConfig.deviceTypeEntries.push_back({
            .type     = chip::app::DeviceFactory::GetInstance().GetDefaultDevice(),
            .endpoint = 1,
            .parentId = chip::kInvalidEndpointId,
        });
    }
    else
    {
        // Expand wildcards using the supported device types from DeviceFactory
        std::vector<std::string> supportedTypes;
        for (const auto & deviceType : chip::app::DeviceFactory::GetInstance().SupportedDeviceTypes())
        {
            if (!IsExcludedFromWildcard(deviceType))
            {
                supportedTypes.push_back(deviceType);
            }
        }

        sParser.ExpandWildcards(supportedTypes);
        mConfig.deviceTypeEntries = sParser.GetDeviceTypeEntries();
    }

    ReturnErrorOnFailure(DeviceTypeParser::ValidateConfig(mConfig.deviceTypeEntries));
    sIsConfigValidated = true;
    return CHIP_NO_ERROR;
}

bool AppOptions::AllDevicesAppOptionHandler(const char * program, OptionSet * options, int identifier, const char * name,
                                            const char * value)
{
    switch (identifier)
    {
    case kOptionDeviceType: {
        sIsConfigValidated = false;
        if (sParser.ParseSingleDeviceString(value) != CHIP_NO_ERROR)
        {
            return false;
        }
        mConfig.deviceTypeEntries = sParser.GetDeviceTypeEntries();
        return true;
    }
    case kOptionBLE:
        if (!ParseInt(value, mConfig.bleController))
        {
            ChipLogError(Support, "Invalid BLE controller specified: %s", value);
            return false;
        }
        return true;
    case kOptionWiFi:
        mConfig.enableWiFi = true;
        ChipLogProgress(AppServer, "WiFi usage enabled");
        return true;
    case kOptionKVS:
        mConfig.kvsPath = value;
        return true;
    case kOptionDiscriminator: {
        char * endptr;
        unsigned long val = strtoul(value, &endptr, 0);
        if (*endptr != '\0' || val > 0xFFF)
        {
            ChipLogError(Support, "Invalid discriminator: %s", value);
            return false;
        }
        mConfig.discriminator = static_cast<uint16_t>(val);
        return true;
    }
    case kOptionVendorId:
        mConfig.vendorId = static_cast<uint16_t>(strtoul(value, nullptr, 0));
        return true;
    case kOptionProductId:
        mConfig.productId = static_cast<uint16_t>(strtoul(value, nullptr, 0));
        return true;
    case kOptionPort: {
        char * endptr;
        unsigned long val = strtoul(value, &endptr, 0);
        if (*endptr != '\0' || val > 0xFFFF)
        {
            ChipLogError(Support, "Invalid port: %s", value);
            return false;
        }
        mConfig.port = static_cast<uint16_t>(val);
        ChipLogProgress(AppServer, "Port option set to %u", static_cast<uint16_t>(val));
        return true;
    }
    case kOptionInterfaceId:
        mConfig.interfaceId = static_cast<uint32_t>(strtoul(value, nullptr, 0));
        return true;
    case kOptionGroupcast:
        mConfig.enableGroupcast = true;
        ChipLogProgress(AppServer, "Groupcast usage enabled");
        return true;
    case kOptionAppPipe:
        mConfig.appPipePath = value;
        ChipLogProgress(AppServer, "App pipe path set to %s", value);
        return true;
    case kOptionTraceTo:
        mConfig.traceTo.push_back(value);
        ChipLogProgress(AppServer, "Added trace destination: %s", value);
        return true;
    default:
        ChipLogError(Support, "%s: INTERNAL ERROR: Unhandled option: %s\n", program, name);
        return false;
    }

    return true;
}

OptionSet * AppOptions::GetOptions()
{
    static OptionDef sAllDevicesAppOptionDefs[] = {
        { "device", kArgumentRequired, kOptionDeviceType },
#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
        { "ble-controller", kArgumentRequired, kOptionBLE },
#endif
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
        { "wifi", kNoArgument, kOptionWiFi },
#endif
        { "KVS", kArgumentRequired, kOptionKVS },
        { "discriminator", kArgumentRequired, kOptionDiscriminator },
        { "vendor-id", kArgumentRequired, kOptionVendorId },
        { "product-id", kArgumentRequired, kOptionProductId },
        { "port", kArgumentRequired, kOptionPort },
        { "interface-id", kArgumentRequired, kOptionInterfaceId },
        { "groupcast", kNoArgument, kOptionGroupcast },
        { "app-pipe", kArgumentRequired, kOptionAppPipe },
        { "trace-to", kArgumentRequired, kOptionTraceTo },
        {}, // need empty terminator
    };

    static const std::string gHelpText = []() {
        // Device option - this is dynamic
        std::string result = "  --device <";
        for (auto & name : app::DeviceFactory::GetInstance().SupportedDeviceTypes())
        {
            result.append(name);
            result.append("|");
        }
        result.append("*");
        result.append(">");
        result += "\n";
        result += "       Select the device to start up. Format: 'type' or 'type:endpoint' or "
                  "'type:endpoint,parent=parentId[,bridged]'.\n";
        result += "       Use '*' to select all supported leaf devices (e.g. --device \"*:1\").\n";
        result += "       Use 'bridged' to automatically create a parent bridged-node endpoint for the device.\n";
        result += "       Can be specified multiple times for multi-endpoint devices.\n";
        result += "       Example: --device aggregator:1 --device \"chime:2,parent=1,bridged\"\n\n";

#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
        result += "  --ble-controller <number>\n";
        result += "       Select the BLE controller to use (default: 0)\n\n";
#endif

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
        result += "  --wifi\n";
        result += "       Enable wifi support for commissioning\n\n";
#endif

        result += "  --KVS <path>\n";
        result += "       Path to the Key Value Store file (default: " CHIP_CONFIG_KVS_PATH ")\n\n";

        result += "  --discriminator <number>\n";
        result += "       Discriminator value for commissioning (default: 3840)\n\n";

        result += "  --vendor-id <number>\n";
        result += "       Vendor ID value for commissioning\n\n";

        result += "  --product-id <number>\n";
        result += "       Product ID value for commissioning\n\n";

        result += "  --port <number>\n";
        result += "       Listen port for secure device messages (default: 5540)\n\n";

        result += "  --interface-id <number>\n";
        result += "       Interface ID to use for multicast multicast DNS\n\n";

        result += "  --groupcast\n";
        result += "       Enable groupcast messaging support\n\n";

        result += "  --app-pipe <path>\n";
        result += "       Path to the named pipe for receiving runtime commands\n\n";

        result += "  --trace-to <destination>\n";
        result += "       Enable tracing destination (e.g., json:log, json:file_path)\n\n";

        return result;
    }();

    static OptionSet sCmdLineOptions = { AllDevicesAppOptionHandler, // handler function
                                         sAllDevicesAppOptionDefs,   // array of option definitions
                                         "PROGRAM OPTIONS",          // help group
                                         gHelpText.c_str() };

    return &sCmdLineOptions;
}
