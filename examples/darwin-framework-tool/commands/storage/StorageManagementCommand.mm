/*
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#include "../common/CHIPCommandStorageDelegate.h"

#include "StorageManagementCommand.h"

#import <Matter/Matter.h>

static CHIPToolPersistentStorageDelegate * storage = nil;

CHIP_ERROR StorageClearAll::Run()
{
    storage = [[CHIPToolPersistentStorageDelegate alloc] init];
    if (![storage deleteAllStorage]) {
        return CHIP_ERROR_INTERNAL;
    }
    return CHIP_NO_ERROR;
}
