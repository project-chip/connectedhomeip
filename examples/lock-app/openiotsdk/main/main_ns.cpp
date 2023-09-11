/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <stdlib.h>

#include <lib/support/logging/CHIPLogging.h>

#include "openiotsdk_platform.h"

int main()
{
    if (openiotsdk_platform_init())
    {
        ChipLogError(NotSpecified, "Open IoT SDK platform initialization failed");
        return EXIT_FAILURE;
    }

    if (openiotsdk_chip_init())
    {
        ChipLogError(NotSpecified, "Open IoT SDK CHIP stack initialization failed");
        return EXIT_FAILURE;
    }

    ChipLogProgress(NotSpecified, "Open IoT SDK lock-app example application start");

    if (openiotsdk_network_init(true))
    {
        ChipLogError(NotSpecified, "Network initialization failed");
        return EXIT_FAILURE;
    }

    if (openiotsdk_chip_run())
    {
        ChipLogError(NotSpecified, "CHIP stack run failed");
        return EXIT_FAILURE;
    }

    ChipLogProgress(NotSpecified, "Open IoT SDK lock-app example application run");

    while (true)
    {
        // Add forever delay to ensure proper workload for this thread
        osDelay(osWaitForever);
    }

    openiotsdk_chip_shutdown();

    return EXIT_SUCCESS;
}
