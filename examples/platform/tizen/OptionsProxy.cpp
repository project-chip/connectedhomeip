/*
 *
 *    Copyright (c) 2021-2022 Project CHIP Authors
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

#include "OptionsProxy.h"

#include <cstring>

#include <app_control.h>

#include <lib/core/CHIPConfig.h>
#include <platform/CHIPDeviceConfig.h>

namespace {

struct Option
{
    const char * name;
    bool isBoolean;
};

// NOTE: Keep this list in sync with the sDeviceOptionDefs
//       from examples/platform/linux/Options.cpp
static constexpr Option sOptions[] = {
#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
    { "ble-device", false },
#endif
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
    { "wifi", true },
#endif
#if CHIP_ENABLE_OPENTHREAD
    { "thread", true },
#endif
    { "version", false },
    { "vendor-id", false },
    { "product-id", false },
    { "custom-flow", false },
    { "capabilities", false },
    { "discriminator", false },
    { "passcode", false },
    { "spake2p-verifier-base64", false },
    { "spake2p-salt-base64", false },
    { "spake2p-iterations", false },
    { "secured-device-port", false },
    { "secured-commissioner-port", false },
    { "unsecured-commissioner-port", false },
    { "command", false },
    { "PICS", false },
    { "KVS", false },
    { "interface-id", false },
#if CHIP_CONFIG_TRANSPORT_TRACE_ENABLED
    { "trace_file", false },
    { "trace_log", false },
#endif
};

bool ParseAppExtraData(app_control_h app_control, const char * key, void * userData)
{
    auto * args = static_cast<std::vector<std::string> *>(userData);

    for (const auto & option : sOptions)
    {
        if (strcmp(key, option.name) != 0)
        {
            continue;
        }

        char * value = nullptr;
        if (app_control_get_extra_data(app_control, option.name, &value) == APP_CONTROL_ERROR_NONE && value != nullptr)
        {
            if (!option.isBoolean)
            {
                args->push_back(std::string("--") + option.name);
                args->push_back(value);
            }
            else if (strcmp(value, "true") == 0)
            {
                args->push_back(std::string("--") + option.name);
            }
            // Release memory allocated by app_control_get_extra_data()
            free(value);
        }
    }

    // Continue iterating over all extra data
    return true;
}

}; // namespace

void OptionsProxy::Parse(const char * argv0, app_control_h app_control)
{
    // Insert argv[0] commonly used as a process name
    if (argv0 != nullptr)
    {
        mArgs.push_back(argv0);
    }

    app_control_foreach_extra_data(app_control, ParseAppExtraData, &mArgs);

    // Convert vector of strings into NULL-terminated vector of char pointers
    mArgv.reserve(mArgs.size() + 1);
    for (auto & arg : mArgs)
    {
        mArgv.push_back(arg.c_str());
    }
    mArgv.push_back(nullptr);
}
