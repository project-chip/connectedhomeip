/*
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#include <controller/ExamplePersistentStorage.h>

#include "StorageManagementCommand.h"

CHIP_ERROR StorageClearAll::Run()
{
    PersistentStorage storage;
    ReturnErrorOnFailure(storage.Init());
    return storage.SyncClearAll();
}
