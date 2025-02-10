/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include <AppMain.h>
#include <app/server/TermsAndConditionsManager.h>
#include <app/server/TermsAndConditionsProvider.h>

using namespace chip;
using namespace chip::ArgParser;

static uint16_t sTcMinRequiredVersion       = 0;
static uint16_t sTcRequiredAcknowledgements = 0;

static OptionDef sTermsAndConditionsOptions[] = {
    { "tc-min-required-version", kArgumentRequired, 'm' },
    { "tc-required-acknowledgements", kArgumentRequired, 'r' },
    { nullptr },
};

static const char * const sTermsAndConditionsOptionHelp =
    "-m, --tc-min-required-version <number>\n      Configure the minimum required TC version.\n\n"
    "-r, --tc-required-acknowledgements <number (bitfield)>\n      Configure the required TC acknowledgements.\n\n";

static bool HandleOption(const char * progName, OptionSet * optSet, int id, const char * name, const char * arg)
{
    switch (id)
    {
    case 'm':
        sTcMinRequiredVersion = static_cast<uint16_t>(atoi(arg));
        break;
    case 'r':
        sTcRequiredAcknowledgements = static_cast<uint16_t>(atoi(arg));
        break;
    default:
        PrintArgError("%s: INTERNAL ERROR: Unhandled option: %s\n", progName, name);
        return false;
    }

    return true;
}

static OptionSet sTermsAndConditionsCmdLineOptions = {
    HandleOption,
    sTermsAndConditionsOptions,
    "TERMS AND CONDITIONS OPTIONS",
    sTermsAndConditionsOptionHelp,
};

void ApplicationInit()
{
    const app::TermsAndConditions termsAndConditions = app::TermsAndConditions(sTcRequiredAcknowledgements, sTcMinRequiredVersion);
    PersistentStorageDelegate & persistentStorageDelegate = Server::GetInstance().GetPersistentStorage();
    app::TermsAndConditionsManager::GetInstance()->Init(&persistentStorageDelegate, MakeOptional(termsAndConditions));
}

void ApplicationShutdown() {}

int main(int argc, char * argv[])
{
    if (ChipLinuxAppInit(argc, argv, &sTermsAndConditionsCmdLineOptions) != 0)
    {
        return -1;
    }

    ChipLinuxAppMainLoop();
    return 0;
}
