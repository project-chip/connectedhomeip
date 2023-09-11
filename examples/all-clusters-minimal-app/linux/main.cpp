/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "AppMain.h"
#include "AppOptions.h"
#include "binding-handler.h"

// Network commissioning
namespace {
constexpr chip::EndpointId kNetworkCommissioningEndpointSecondary = 0xFFFE;
} // anonymous namespace

int main(int argc, char * argv[])
{
    VerifyOrDie(
        ChipLinuxAppInit(argc, argv, AppOptions::GetOptions(), chip::MakeOptional(kNetworkCommissioningEndpointSecondary)) == 0);
    VerifyOrDie(InitBindingHandlers() == CHIP_NO_ERROR);

    LinuxDeviceOptions::GetInstance().dacProvider = AppOptions::GetDACProvider();

    ChipLinuxAppMainLoop();
    return 0;
}
