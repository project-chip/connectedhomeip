/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "TestCHIPoBLEStackMgr.h"
#include <stdlib.h>

int main(int argc, char * argv[])
{
#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
    if (argc == 2 && atoi(argv[1]) == 1)
    {
        return TestCHIPoBLEStackManager();
    }
    return 0;
#endif // CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
}
