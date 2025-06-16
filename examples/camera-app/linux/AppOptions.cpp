/*
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

#include "AppOptions.h"
#include "camera-device.h"

using namespace chip::ArgParser;
using namespace chip::System;

using chip::ArgParser::OptionDef;
using chip::ArgParser::OptionSet;
using chip::ArgParser::PrintArgError;

constexpr uint16_t kOptionVideoDevicePath = 0xFF01;

static std::string sVideoDevicePath = Camera::kDefaultVideoDevicePath;

bool AppOptions::IsEmptyString(const char * value)
{
    return (value == nullptr || strlen(value) == 0);
}

bool AppOptions::HandleOptions(const char * program, OptionSet * options, int identifier, const char * name, const char * value)
{
    bool retval = true;
    switch (identifier)
    {
    case kOptionVideoDevicePath: {
        if (!IsEmptyString(value))
        {
            sVideoDevicePath = value;
        }
        break;
    }
    default:
        PrintArgError("%s: INTERNAL ERROR: Unhandled option: %s\n", program, name);
        retval = false;
        break;
    }

    return retval;
}

OptionSet * AppOptions::GetOptions()
{
    static OptionDef optionsDef[] = {
        { "video-device", kArgumentRequired, kOptionVideoDevicePath },
        {},
    };

    // Build the help string using the constant
    static const std::string helpString = "  --video-device <value>\n"
                                          "      Path to a V4L2 video capture device (default: " +
        std::string(Camera::kDefaultVideoDevicePath) + ").\n";

    static OptionSet options = { AppOptions::HandleOptions, optionsDef, "PROGRAM OPTIONS", helpString.c_str() };

    return &options;
}

std::string AppOptions::GetVideoDevicePath()
{
    return sVideoDevicePath;
}
