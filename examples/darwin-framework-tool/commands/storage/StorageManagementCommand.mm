/*
 *   Copyright (c) 2022 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
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
