/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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
    kDeviceOption_BleDevice = 0x1000,
    kDeviceOption_WiFi      = 0x1001,
    kDeviceOption_Thread    = 0x1002,
};

OptionDef sDeviceOptionDefs[] = { { "ble-device", kArgumentRequired, kDeviceOption_BleDevice },
#if CHIP_DEVICE_CONFIG_ENABLE_WPA
                                  { "wifi", kNoArgument, kDeviceOption_WiFi },
#endif // CHIP_DEVICE_CONFIG_ENABLE_WPA
#if CHIP_ENABLE_OPENTHREAD
                                  { "thread", kNoArgument, kDeviceOption_Thread },
#endif // CHIP_ENABLE_OPENTHREAD
                                  {} };

const char sDeviceOptionHelp[] = "  --ble-device <number>\n"
                                 "       The device number for CHIPoBLE, without 'hci' prefix, can be found by hciconfig.\n"
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

    default:
        PrintArgError("%s: INTERNAL ERROR: Unhandled option: %s\n", aProgram, aName);
        retval = false;
        break;
    }

    return (retval);
}

OptionSet sDeviceOptions = { HandleOption, sDeviceOptionDefs, "GENERAL OPTIONS", sDeviceOptionHelp };

OptionSet * sLinuxDeviceOptionSets[] = { &sDeviceOptions, nullptr };
} // namespace

CHIP_ERROR ParseArguments(int argc, char * argv[])
{
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
