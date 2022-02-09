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

#include "AppConfig.h"
#include "KeyValueStorageTest.h"
#include "init_p6Platform.h"
#include <platform/CHIPDeviceLayer.h>
#include <platform/KeyValueStoreManager.h>

using namespace chip;

int main(int argc, char * argv[])
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    init_p6Platform();

    err = chip::Platform::MemoryInit();
    SuccessOrExit(err);

    err = chip::DeviceLayer::PersistedStorage::KeyValueStoreMgrImpl().Init();
    SuccessOrExit(err);

    P6_LOG("=============================================\n");
    P6_LOG("chip-p6-persistent-storage-example starting\n");
    P6_LOG("=============================================\n");

    while (1)
    {
        P6_LOG("Running Tests:\n");
        chip::RunKvsTest();
        cyhal_system_delay_ms(60000); // Run every minute
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        return 1;
    }
    return 0;
}
