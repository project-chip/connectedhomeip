/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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
#include <ota-provider-common/OTAProviderExample.h>

namespace chip {
namespace Shell {

class OTAProviderCommands
{
public:
    // delete the copy constructor
    OTAProviderCommands(const OTAProviderCommands &) = delete;
    // delete the move constructor
    OTAProviderCommands(OTAProviderCommands &&) = delete;
    // delete the assignment operator
    OTAProviderCommands & operator=(const OTAProviderCommands &) = delete;

    static OTAProviderCommands & GetInstance()
    {
        static OTAProviderCommands instance;
        return instance;
    }

    // Register the OTA provider commands
    void Register();

    // Set Example OTA provider
    void SetExampleOTAProvider(OTAProviderExample * otaProvider);

private:
    OTAProviderCommands() {}
};

} // namespace Shell
} // namespace chip
