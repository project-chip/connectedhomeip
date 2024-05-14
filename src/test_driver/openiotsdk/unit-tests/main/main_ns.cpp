/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <stdio.h>
#include <stdlib.h>

#include "openiotsdk_platform.h"
#include <lib/support/UnitTest.h>
#include <platform/CHIPDeviceLayer.h>

using namespace ::chip;

int main()
{
    if (openiotsdk_platform_init())
    {
        ChipLogAutomation("ERROR: Open IoT SDK platform initialization failed");
        return EXIT_FAILURE;
    }

    ChipLogAutomation("Open IoT SDK unit-tests start");

    if (openiotsdk_network_init(true))
    {
        ChipLogAutomation("ERROR: Network initialization failed");
        return EXIT_FAILURE;
    }

    ChipLogAutomation("Open IoT SDK unit-tests run...");
    int status = chip::test::RunAllTests();
    ChipLogAutomation("Test status: %d", status);
    ChipLogAutomation("Open IoT SDK unit-tests completed");

    return EXIT_SUCCESS;
}
