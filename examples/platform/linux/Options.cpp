/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include "Options.h"

#include <app/server/OnboardingCodesUtil.h>
#include <platform/CHIPDeviceLayer.h>

#include <lib/core/CHIPError.h>
#include <lib/support/CHIPArgParser.hpp>

using namespace chip;
using namespace chip::ArgParser;

namespace {
LinuxDeviceOptions gDeviceOptions;

// Follow the code style of command line arguments in case we need to add more options in the future.
enum
{
    kDeviceOption_BleDevice                 = 0x1000,
    kDeviceOption_WiFi                      = 0x1001,
    kDeviceOption_Thread                    = 0x1002,
    kDeviceOption_Version                   = 0x1003,
    kDeviceOption_VendorID                  = 0x1004,
    kDeviceOption_ProductID                 = 0x1005,
    kDeviceOption_CustomFlow                = 0x1006,
    kDeviceOption_Capabilities              = 0x1007,
    kDeviceOption_Discriminator             = 0x1008,
    kDeviceOption_Passcode                  = 0x1009,
    kDeviceOption_SecuredDevicePort         = 0x100a,
    kDeviceOption_SecuredCommissionerPort   = 0x100b,
    kDeviceOption_UnsecuredCommissionerPort = 0x100c
};

constexpr unsigned kAppUsageLength = 64;

OptionDef sDeviceOptionDefs[] = {
#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
    { "ble-device", kArgumentRequired, kDeviceOption_BleDevice },
#endif // CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
#if CHIP_DEVICE_CONFIG_ENABLE_WPA
    { "wifi", kNoArgument, kDeviceOption_WiFi },
#endif // CHIP_DEVICE_CONFIG_ENABLE_WPA
#if CHIP_ENABLE_OPENTHREAD
    { "thread", kNoArgument, kDeviceOption_Thread },
#endif // CHIP_ENABLE_OPENTHREAD
    { "version", kArgumentRequired, kDeviceOption_Version },
    { "vendor-id", kArgumentRequired, kDeviceOption_VendorID },
    { "product-id", kArgumentRequired, kDeviceOption_ProductID },
    { "custom-flow", kArgumentRequired, kDeviceOption_CustomFlow },
    { "capabilities", kArgumentRequired, kDeviceOption_Capabilities },
    { "discriminator", kArgumentRequired, kDeviceOption_Discriminator },
    { "passcode", kArgumentRequired, kDeviceOption_Passcode },
    { "secured-device-port", kArgumentRequired, kDeviceOption_SecuredDevicePort },
    { "secured-commissioner-port", kArgumentRequired, kDeviceOption_SecuredCommissionerPort },
    { "unsecured-commissioner-port", kArgumentRequired, kDeviceOption_UnsecuredCommissionerPort },
    {}
};

const char * sDeviceOptionHelp =
#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
    "  --ble-device <number>\n"
    "       The device number for CHIPoBLE, without 'hci' prefix, can be found by hciconfig.\n"
#endif // CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
#if CHIP_DEVICE_CONFIG_ENABLE_WPA
    "\n"
    "  --wifi\n"
    "       Enable WiFi management via wpa_supplicant.\n"
#endif // CHIP_DEVICE_CONFIG_ENABLE_WPA
#if CHIP_ENABLE_OPENTHREAD
    "\n"
    "  --thread\n"
    "       Enable Thread management via ot-agent.\n"
#endif // CHIP_ENABLE_OPENTHREAD
    "\n"
    "  --version <version>\n"
    "       The version indication provides versioning of the setup payload.\n"
    "\n"
    "  --vendor-id <id>\n"
    "       The Vendor ID is assigned by the Connectivity Standards Alliance.\n"
    "\n"
    "  --product-id <id>\n"
    "       The Product ID is specified by vendor.\n"
    "\n"
    "  --custom-flow <Standard = 0 | UserActionRequired = 1 | Custom = 2>\n"
    "       A 2-bit unsigned enumeration specifying manufacturer-specific custom flow options.\n"
    "\n"
    "  --capabilities <None = 0, SoftAP = 1 << 0, BLE = 1 << 1, OnNetwork = 1 << 2>\n"
    "       Discovery Capabilities Bitmask which contains information about Device’s available technologies for device discovery.\n"
    "\n"
    "  --discriminator <discriminator>\n"
    "       A 12-bit unsigned integer match the value which a device advertises during commissioning.\n"
    "\n"
    "  --passcode <passcode>\n"
    "       A 27-bit unsigned integer, which serves as proof of possession during commissioning.\n"
    "\n"
    "  --secured-device-port <port>\n"
    "       A 16-bit unsigned integer specifying the listen port to use for secure device messages (default is 5540).\n"
    "\n"
    "  --secured-commissioner-port <port>\n"
    "       A 16-bit unsigned integer specifying the listen port to use for secure commissioner messages (default is 5542). Only "
    "valid when app is both device and commissioner\n"
    "\n"
    "  --unsecured-commissioner-port <port>\n"
    "       A 16-bit unsigned integer specifying the port to use for unsecured commissioner messages (default is 5550).\n"
    "\n";

bool HandleOption(const char * aProgram, OptionSet * aOptions, int aIdentifier, const char * aName, const char * aValue)
{
    bool retval = true;

    switch (aIdentifier)
    {

    case kDeviceOption_BleDevice:
        if (!ParseInt(aValue, LinuxDeviceOptions::GetInstance().mBleDevice))
        {
            PrintArgError("%s: invalid value specified for ble device number: %s\n", aProgram, aValue);
            retval = false;
        }
        break;

    case kDeviceOption_WiFi:
        LinuxDeviceOptions::GetInstance().mWiFi = true;
        break;

    case kDeviceOption_Thread:
        LinuxDeviceOptions::GetInstance().mThread = true;
        break;

    case kDeviceOption_Version:
        LinuxDeviceOptions::GetInstance().payload.version = static_cast<uint8_t>(atoi(aValue));
        break;

    case kDeviceOption_VendorID:
        LinuxDeviceOptions::GetInstance().payload.vendorID = static_cast<uint16_t>(atoi(aValue));
        break;

    case kDeviceOption_ProductID:
        LinuxDeviceOptions::GetInstance().payload.productID = static_cast<uint16_t>(atoi(aValue));
        break;

    case kDeviceOption_CustomFlow:
        LinuxDeviceOptions::GetInstance().payload.commissioningFlow = static_cast<CommissioningFlow>(atoi(aValue));
        break;

    case kDeviceOption_Capabilities:
        LinuxDeviceOptions::GetInstance().payload.rendezvousInformation.SetRaw(static_cast<uint8_t>(atoi(aValue)));
        break;

    case kDeviceOption_Discriminator:
        LinuxDeviceOptions::GetInstance().payload.discriminator = static_cast<uint16_t>(atoi(aValue));
        break;

    case kDeviceOption_Passcode:
        LinuxDeviceOptions::GetInstance().payload.setUpPINCode = static_cast<uint32_t>(atoi(aValue));
        break;

    case kDeviceOption_SecuredDevicePort:
        LinuxDeviceOptions::GetInstance().securedDevicePort = static_cast<uint16_t>(atoi(aValue));
        break;

    case kDeviceOption_SecuredCommissionerPort:
        LinuxDeviceOptions::GetInstance().securedCommissionerPort = static_cast<uint16_t>(atoi(aValue));
        break;

    case kDeviceOption_UnsecuredCommissionerPort:
        LinuxDeviceOptions::GetInstance().unsecuredCommissionerPort = static_cast<uint16_t>(atoi(aValue));
        break;

    default:
        PrintArgError("%s: INTERNAL ERROR: Unhandled option: %s\n", aProgram, aName);
        retval = false;
        break;
    }

    return (retval);
}

OptionSet sDeviceOptions = { HandleOption, sDeviceOptionDefs, "GENERAL OPTIONS", sDeviceOptionHelp };

OptionSet * sLinuxDeviceOptionSets[] = { &sDeviceOptions, nullptr, nullptr };
} // namespace

CHIP_ERROR ParseArguments(int argc, char * argv[])
{
    char usage[kAppUsageLength];
    snprintf(usage, kAppUsageLength, "Usage: %s [options]", argv[0]);

    HelpOptions helpOptions(argv[0], usage, "1.0");
    sLinuxDeviceOptionSets[1] = &helpOptions;

    if (!ParseArgs(argv[0], argc, argv, sLinuxDeviceOptionSets))
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    return CHIP_NO_ERROR;
}

LinuxDeviceOptions & LinuxDeviceOptions::GetInstance()
{
    return gDeviceOptions;
}
