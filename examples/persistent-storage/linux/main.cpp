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

#include <platform/CHIPDeviceLayer.h>

#include <lib/core/CHIPError.h>
#include <lib/support/CHIPMem.h>

#include "KeyValueStorageTest.h"
#include <platform/KeyValueStoreManager.h>

#include <iostream>

using namespace chip;

int main(int argc, char * argv[])
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    err = chip::Platform::MemoryInit();
    SuccessOrExit(err);

    chip::DeviceLayer::PersistedStorage::KeyValueStoreMgrImpl().Init("/tmp/chip_example_kvs");

    printf("=============================================\n");
    printf("chip-linux-persitent-storage-example starting\n");
    printf("=============================================\n");

    while (true)
    {
        printf("Running Tests:\n");
        chip::RunKvsTest();
        sleep(60); // Run every minute
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        std::cerr << "Failed to run Linux App: " << ErrorStr(err) << std::endl;
        // End the program with non zero error code to indicate a error.
        return 1;
    }
    return 0;
}
