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
};

OptionDef sDeviceOptionDefs[] = { { "ble-device", kArgumentRequired, kDeviceOption_BleDevice }, {} };

const char * sDeviceOptionHelp = "  --ble-device <number>\n"
                                 "       The device number for CHIPoBLE, without 'hci' prefix, can be found by hciconfig.\n"
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
