/*
 *
 *    Copyright (c) 2020 Google LLC.
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

// ================================================================================
// Main Code
// ================================================================================

#include <openthread/cli.h>
#include <openthread/error.h>
#include <openthread/heap.h>
#include "openthread/platform/uart.h"
#include "openthread/platform/logging.h"
#include <openthread/platform/openthread-system.h>

#include <Weave/DeviceLayer/WeaveDeviceLayer.h>
#include <Weave/DeviceLayer/ThreadStackManager.h>
#include <Weave/Core/WeaveError.h>

#include "app_config.h"
#include "FreeRtosMbedtlsMutex.h"

using namespace ::nl;
using namespace ::nl::Inet;
using namespace ::nl::Weave;
using namespace ::nl::Weave::DeviceLayer;
using namespace ::nl::Weave::Logging;

#include <AppTask.h>

void k32wLog(const char *aFormat, ...)
{
    va_list args;

    va_start(args, aFormat);
    otPlatLog(OT_LOG_LEVEL_NONE, OT_LOG_REGION_API, aFormat, args);
    va_end(args);
}

typedef void (*InitFunc)(void);
extern InitFunc __init_array_start;
extern InitFunc __init_array_end;

extern "C" void main_task(void const *argument)
{
    WEAVE_ERROR ret;

	/* Call C++ constructors */
	InitFunc *pFunc = &__init_array_start;
	for ( ; pFunc < &__init_array_end; ++pFunc )
	{
		(*pFunc)();
	}

    /* Used for HW initializations */
    otSysInit(0, NULL);

    /* UART needs to be enabled so early for getting the Weave Init Logs.
     * Otherwise, some logs are lost because the UART gets enabled later
     * during the initialization of the Thread stack */
    otPlatUartEnable();

    k32wLog("Welcome to NXP ELock Demo App");

    /* Using OT Heap is deprecated so use instead the FreeRTOS
     * allocation system - which is also thread-safe */
    otHeapSetCAllocFree(calloc, free);

    /* Mbedtls Threading support is needed because both
     * Thread and Weave tasks are using it */
    freertos_mbedtls_mutex_init();

    ret = PlatformMgr().InitWeaveStack();
    if (ret != WEAVE_NO_ERROR)
    {
	k32wLog("Error during PlatformMgr().InitWeaveStack()");
        goto exit;
    }

    ret = ThreadStackMgr().InitThreadStack();
    if (ret != WEAVE_NO_ERROR)
    {
	k32wLog("Error during ThreadStackMgr().InitThreadStack()");
        goto exit;
    }

    // Configure device to operate as a Thread sleepy end-device.
    ret = ConnectivityMgr().SetThreadDeviceType(ConnectivityManager::kThreadDeviceType_SleepyEndDevice);
    if (ret != WEAVE_NO_ERROR)
    {
        goto exit;
    }

    // Configure the Thread polling behavior for the device.
    {
        ConnectivityManager::ThreadPollingConfig pollingConfig;
        pollingConfig.Clear();
        pollingConfig.ActivePollingIntervalMS = THREAD_ACTIVE_POLLING_INTERVAL_MS;
        pollingConfig.InactivePollingIntervalMS = THREAD_INACTIVE_POLLING_INTERVAL_MS;

        ret = ConnectivityMgr().SetThreadPollingConfig(pollingConfig);
        if (ret != WEAVE_NO_ERROR)
        {
		k32wLog("Error during ConnectivityMgr().SetThreadPollingConfig(pollingConfig)");
            goto exit;
        }
    }

    ret = PlatformMgr().StartEventLoopTask();
    if (ret != WEAVE_NO_ERROR)
    {
	k32wLog("Error during PlatformMgr().StartEventLoopTask();");
        goto exit;
    }

    // Start OpenThread task
    ret = ThreadStackMgrImpl().StartThreadTask();
    if (ret != WEAVE_NO_ERROR)
    {
	k32wLog("Error during ThreadStackMgrImpl().StartThreadTask()");
        goto exit;
    }

    ret = GetAppTask().StartAppTask();
    if (ret != WEAVE_NO_ERROR)
    {
	k32wLog("Error during GetAppTask().StartAppTask()");
        goto exit;
    }

    GetAppTask().AppTaskMain(NULL);

exit:
    return;
}
