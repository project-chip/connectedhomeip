/*
 *
 *    Copyright (c) 2022-2023 Project CHIP Authors
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

#pragma once

#include "AppMain.h"

#include <app/tests/suites/credentials/TestHarnessDACProvider.h>

class AppOptions
{
public:
    static chip::ArgParser::OptionSet * GetOptions();
    static chip::Credentials::DeviceAttestationCredentialsProvider * GetDACProvider();
    static chip::Optional<std::string> GetEndUserSupportLogFilePath();
    static chip::Optional<std::string> GetNetworkDiagnosticsLogFilePath();
    static chip::Optional<std::string> GetCrashLogFilePath();

private:
    static bool HandleOptions(const char * program, chip::ArgParser::OptionSet * options, int identifier, const char * name,
                              const char * value);

    static bool IsEmptyString(const char * value);
};
