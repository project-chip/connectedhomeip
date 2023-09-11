/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <stdlib.h>

#include "openiotsdk_platform.h"
#include <NlTestLogger.h>
#include <lib/support/UnitTestRegistration.h>
#include <platform/CHIPDeviceLayer.h>

constexpr nl_test_output_logger_t NlTestLogger::nl_test_logger;

using namespace ::chip;

int main()
{
    if (openiotsdk_platform_init())
    {
        ChipLogAutomation("ERROR: Open IoT SDK platform initialization failed");
        return EXIT_FAILURE;
    }

    nlTestSetLogger(&NlTestLogger::nl_test_logger);

    ChipLogAutomation("Open IoT SDK unit-tests start");

    if (openiotsdk_network_init(true))
    {
        ChipLogAutomation("ERROR: Network initialization failed");
        return EXIT_FAILURE;
    }

    ChipLogAutomation("Open IoT SDK unit-tests run...");
    int status = RunRegisteredUnitTests();
    ChipLogAutomation("Test status: %d", status);
    ChipLogAutomation("Open IoT SDK unit-tests completed");

    return EXIT_SUCCESS;
}
