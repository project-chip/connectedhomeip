/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
