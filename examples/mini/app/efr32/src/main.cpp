/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2019 Google LLC.
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

#include <AppConfig.h>
#include <FreeRTOS.h>
#include <MiniApp.h>
#include <app/server/Server.h>
#include <init_efrPlatform.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CHIPPlatformMemory.h>
#include <mbedtls/threading.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/KeyValueStoreManager.h>

#if CHIP_ENABLE_OPENTHREAD
#include <mbedtls/platform.h>
#include <openthread/cli.h>
#include <openthread/dataset.h>
#include <openthread/error.h>
#include <openthread/heap.h>
#include <openthread/icmp6.h>
#include <openthread/instance.h>
#include <openthread/link.h>
#include <openthread/platform/openthread-system.h>
#include <openthread/tasklet.h>
#include <openthread/thread.h>
#endif // CHIP_ENABLE_OPENTHREAD

#if PW_RPC_ENABLED
#include <Rpc.h>
#endif

using namespace ::chip::DeviceLayer;

// ================================================================================
// FreeRTOS Callbacks
// ================================================================================
extern "C" void vApplicationIdleHook(void)
{
    // FreeRTOS Idle callback

    // Check CHIP Config nvm3 and repack flash if necessary.
    Internal::EFR32Config::RepackNvm3Flash();
}

// ================================================================================
// App Error
//=================================================================================

void appError(CHIP_ERROR err)
{
    // appError(static_cast<int>(err.AsInteger()));
    EFR32_LOG("!!!!!!!!!!!! App Critical Error: %d !!!!!!!!!!!", err);
    portDISABLE_INTERRUPTS();
    while (1)
        ;
}

// ================================================================================
// Main Code
// ================================================================================
int main(void)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    example::MiniApp & app = example::MiniApp::Instance();

    printf("\n----Starting App\n");
    err = app.Init();
    if (err != CHIP_NO_ERROR)
    {
        printf("\n----App Init failed\n");
        appError(err);
    }

    err = app.Start();
    if (err != CHIP_NO_ERROR)
    {
        printf("\n----App Start failed\n");
        appError(err);
    }

    app.Finish();
    return err.AsInteger();
}
