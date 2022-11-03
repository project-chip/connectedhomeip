/*
 * SPDX-FileCopyrightText: (c) 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "AppOptions.h"

using chip::ArgParser::OptionDef;
using chip::ArgParser::OptionSet;
using chip::ArgParser::PrintArgError;

constexpr uint16_t kOptionDacProviderFilePath = 0xFF01;

static chip::Credentials::Examples::TestHarnessDACProvider mDacProvider;

bool AppOptions::HandleOptions(const char * program, OptionSet * options, int identifier, const char * name, const char * value)
{
    bool retval = true;
    switch (identifier)
    {
    case kOptionDacProviderFilePath:
        mDacProvider.Init(value);
        break;
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
        {},
    };

    static OptionSet options = {
        AppOptions::HandleOptions, optionsDef, "PROGRAM OPTIONS",
        "  --dac_provider <filepath>\n"
        "       A json file with data used by the example dac provider to validate device attestation procedure.\n"
    };

    return &options;
}

chip::Credentials::DeviceAttestationCredentialsProvider * AppOptions::GetDACProvider()
{
    return &mDacProvider;
}
