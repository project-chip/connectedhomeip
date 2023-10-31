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
#include <app/clusters/diagnostic-logs-server/diagnostic-logs-server.h>

using namespace chip::ArgParser;
using namespace chip::app::Clusters::DiagnosticLogs;

using chip::ArgParser::OptionDef;
using chip::ArgParser::OptionSet;
using chip::ArgParser::PrintArgError;

constexpr uint16_t kOptionDacProviderFilePath        = 0xFF01;
constexpr uint16_t kOptionMinCommissioningTimeout    = 0xFF02;
constexpr uint16_t kOptionEndUserSupportFilePath     = 0xFF03;
constexpr uint16_t kOptionNetworkDiagnosticsFilePath = 0xFF04;
constexpr uint16_t kOptionCrashFilePath              = 0xFF05;

static chip::Credentials::Examples::TestHarnessDACProvider mDacProvider;

static char mEndUserSupportLogFileDesignator[kLogFileDesignatorMaxLen];
static char mNetworkDiagnosticsLogFileDesignator[kLogFileDesignatorMaxLen];
static char mCrashLogFileDesignator[kLogFileDesignatorMaxLen];

bool AppOptions::HandleOptions(const char * program, OptionSet * options, int identifier, const char * name, const char * value)
{
    bool retval = true;
    switch (identifier)
    {
    case kOptionDacProviderFilePath:
        ChipLogError(BDX, "dac provider path");
        mDacProvider.Init(value);
        break;
    case kOptionMinCommissioningTimeout: {
        auto & commissionMgr = chip::Server::GetInstance().GetCommissioningWindowManager();
        commissionMgr.OverrideMinCommissioningTimeout(chip::System::Clock::Seconds16(static_cast<uint16_t>(atoi(value))));
        break;
    }
    case kOptionEndUserSupportFilePath: {
        ChipLogError(BDX, "kOptionEndUserSupportFilePath setting end user fd");
        if (strlen(value) > kLogFileDesignatorMaxLen)
        {
            PrintArgError("%s: Invalid file path length. Must be less that %d: %d\n", program, kLogFileDesignatorMaxLen, strlen(value));
        }
        strncpy(mEndUserSupportLogFileDesignator, value, strlen(value));
        break;
    }
    case kOptionNetworkDiagnosticsFilePath: {
        if (strlen(value) > kLogFileDesignatorMaxLen)
        {
            PrintArgError("%s: Invalid file path length. Must be less that %d: %d\n", program, kLogFileDesignatorMaxLen, strlen(value));
        }
        strncpy(mNetworkDiagnosticsLogFileDesignator, value, strlen(value));
        break;
    }
    case kOptionCrashFilePath: {
        if (strlen(value) > kLogFileDesignatorMaxLen)
        {
            PrintArgError("%s: Invalid file path length. Must be less that %d: %d\n", program, kLogFileDesignatorMaxLen, strlen(value));
        }
        strncpy(mCrashLogFileDesignator, value, strlen(value));
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
        { "dac_provider", kArgumentRequired, kOptionDacProviderFilePath },
        { "min_commissioning_timeout", kArgumentRequired, kOptionMinCommissioningTimeout },
        { "end_user_support_log", kArgumentRequired, kOptionEndUserSupportFilePath },
        { "network_diagnostics_log", kArgumentRequired, kOptionNetworkDiagnosticsFilePath },
        { "crash_log", kArgumentRequired, kOptionCrashFilePath },
        {},
    };

    static OptionSet options = {
        AppOptions::HandleOptions, optionsDef, "PROGRAM OPTIONS",
        "  --dac_provider <filepath>\n"
        "       A json file with data used by the example dac provider to validate device attestation procedure.\n"
        "  --min_commissioning_timeout <value>\n"
        "       The minimum time in seconds during which commissioning session establishment is allowed by the Node.\n"
        "  --end_user_support_log <value>\n"
        "       The end user support log file to be used for diagnostic logs transfer.\n"
        "  --network_diagnostics_log <value>\n"
        "       The network diagnostics log file to be used for diagnostic logs transfer.\n"
        "  --crash_log <value>\n"
        "       The crash log file to be used for diagnostic logs transfer\n"
    };

    return &options;
}

chip::Credentials::DeviceAttestationCredentialsProvider * AppOptions::GetDACProvider()
{
    return &mDacProvider;
}

char * AppOptions::GetEndUserSupportLogFileDesignator()
{
    return mEndUserSupportLogFileDesignator;
}

char * AppOptions::GetNetworkDiagnosticsLogFileDesignator()
{
    return mNetworkDiagnosticsLogFileDesignator;
}

char * AppOptions::GetCrashLogFileDesignator()
{
    return mCrashLogFileDesignator;
}
