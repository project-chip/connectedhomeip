/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "AppMain.h"

#include <app/tests/suites/credentials/TestHarnessDACProvider.h>

class AppOptions
{
public:
    static chip::ArgParser::OptionSet * GetOptions();
    static chip::Credentials::DeviceAttestationCredentialsProvider * GetDACProvider();
    static bool GetInteractiveMode();
    static chip::Optional<uint16_t> GetInteractiveModePort();

private:
    static bool HandleOptions(const char * program, chip::ArgParser::OptionSet * options, int identifier, const char * name,
                              const char * value);
};
