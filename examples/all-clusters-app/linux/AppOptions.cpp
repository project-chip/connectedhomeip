/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#include <app/server/CommissioningWindowManager.h>
#include <app/server/Server.h>

using chip::ArgParser::OptionDef;
using chip::ArgParser::OptionSet;
using chip::ArgParser::PrintArgError;

constexpr uint16_t kOptionDacProviderFilePath     = 0xFF01;
constexpr uint16_t kOptionMinCommissioningTimeout = 0xFF02;

static chip::Credentials::Examples::TestHarnessDACProvider mDacProvider;

bool AppOptions::HandleOptions(const char * program, OptionSet * options, int identifier, const char * name, const char * value)
{
    bool retval = true;
    switch (identifier)
    {
    case kOptionDacProviderFilePath:
        mDacProvider.Init(value);
        break;
    case kOptionMinCommissioningTimeout: {
        auto & commissionMgr = chip::Server::GetInstance().GetCommissioningWindowManager();
        commissionMgr.OverrideMinCommissioningTimeout(chip::System::Clock::Seconds16(static_cast<uint16_t>(atoi(value))));
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
        { "dac_provider", chip::ArgParser::kArgumentRequired, kOptionDacProviderFilePath },
        { "min_commissioning_timeout", chip::ArgParser::kArgumentRequired, kOptionMinCommissioningTimeout },
        {},
    };

    static OptionSet options = {
        AppOptions::HandleOptions, optionsDef, "PROGRAM OPTIONS",
        "  --dac_provider <filepath>\n"
        "       A json file with data used by the example dac provider to validate device attestation procedure.\n"
        "  --min_commissioning_timeout <value>\n"
        "       The minimum time in seconds during which commissioning session establishment is allowed by the Node.\n"
    };

    return &options;
}

chip::Credentials::DeviceAttestationCredentialsProvider * AppOptions::GetDACProvider()
{
    return &mDacProvider;
}
