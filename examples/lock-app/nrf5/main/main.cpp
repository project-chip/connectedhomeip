/*
 *
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

#include <stdbool.h>
#include <stdint.h>

#include "boards.h"
#include "nrf_log.h"
#include "nrf_delay.h"
#ifdef SOFTDEVICE_PRESENT
#include "nrf_sdh.h"
#include "nrf_sdh_ble.h"
#include "nrf_sdh_soc.h"
#endif
#include "nrf_drv_clock.h"
#if NRF_CRYPTO_ENABLED
#include "nrf_crypto.h"
#endif
#include "mem_manager.h"
extern "C" {
#include "freertos_mbedtls_mutex.h"
}

#if NRF_LOG_ENABLED
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "nrf_log_backend_uart.h"
#endif // NRF_LOG_ENABLED

#include <mbedtls/platform.h>

#include <openthread/instance.h>
#include <openthread/thread.h>
#include <openthread/tasklet.h>
#include <openthread/link.h>
#include <openthread/dataset.h>
#include <openthread/error.h>
#include <openthread/icmp6.h>
#include <openthread/platform/openthread-system.h>
extern "C" {
#include <openthread/platform/platform-softdevice.h>
}

#include <Weave/DeviceLayer/WeaveDeviceLayer.h>
#include <Weave/DeviceLayer/ThreadStackManager.h>
#include <Weave/DeviceLayer/nRF5/GroupKeyStoreImpl.h>
#include <Weave/DeviceLayer/internal/testing/ConfigUnitTest.h>
#include <Weave/DeviceLayer/internal/testing/GroupKeyStoreUnitTest.h>
#include <Weave/DeviceLayer/internal/testing/SystemClockUnitTest.h>

#include <AppTask.h>

using namespace ::nl;
using namespace ::nl::Inet;
using namespace ::nl::Weave;
using namespace ::nl::Weave::DeviceLayer;

extern "C" size_t GetHeapTotalSize(void);

// ================================================================================
// Logging Support
// ================================================================================

#if NRF_LOG_ENABLED

#if NRF_LOG_USES_TIMESTAMP

uint32_t LogTimestamp(void)
{
    return static_cast<uint32_t>(nl::Weave::System::Platform::Layer::GetClock_MonotonicMS());
}

#define LOG_TIMESTAMP_FUNC LogTimestamp
#define LOG_TIMESTAMP_FREQ 1000

#else // NRF_LOG_USES_TIMESTAMP

#define LOG_TIMESTAMP_FUNC NULL
#define LOG_TIMESTAMP_FREQ 0

#endif // NRF_LOG_USES_TIMESTAMP

#endif // NRF_LOG_ENABLED

// ================================================================================
// SoftDevice Support
// ================================================================================

#if defined(SOFTDEVICE_PRESENT) && SOFTDEVICE_PRESENT

static void OnSoCEvent(uint32_t sys_evt, void * p_context)
{
    UNUSED_PARAMETER(p_context);

    otSysSoftdeviceSocEvtHandler(sys_evt);
}

#endif // defined(SOFTDEVICE_PRESENT) && SOFTDEVICE_PRESENT

// ================================================================================
// J-Link Monitor Mode Debugging Support
// ================================================================================

#if JLINK_MMD

extern "C" void JLINK_MONITOR_OnExit(void)
{

}

extern "C" void JLINK_MONITOR_OnEnter(void)
{

}

extern "C" void JLINK_MONITOR_OnPoll(void)
{

}

#endif // JLINK_MMD

// ================================================================================
// Main Code
// ================================================================================

int main(void)
{
    ret_code_t ret;

#if JLINK_MMD
    NVIC_SetPriority(DebugMonitor_IRQn, _PRIO_SD_LOWEST);
#endif

    // Initialize clock driver.
    ret = nrf_drv_clock_init();
    APP_ERROR_CHECK(ret);

    nrf_drv_clock_lfclk_request(NULL);

    // Wait for the clock to be ready.
    while (!nrf_clock_lf_is_running()) { }

#if NRF_LOG_ENABLED

    // Initialize logging component
    ret = NRF_LOG_INIT(LOG_TIMESTAMP_FUNC, LOG_TIMESTAMP_FREQ);
    APP_ERROR_CHECK(ret);

    // Initialize logging backends
    NRF_LOG_DEFAULT_BACKENDS_INIT();

#endif

    NRF_LOG_INFO("==================================================");
    NRF_LOG_INFO("openweave-nrf52840-lock-example starting");
#if BUILD_RELEASE
    NRF_LOG_INFO("*** PSEUDO-RELEASE BUILD ***");
#else
    NRF_LOG_INFO("*** DEVELOPMENT BUILD ***");
#endif
    NRF_LOG_INFO("==================================================");

#if defined(SOFTDEVICE_PRESENT) && SOFTDEVICE_PRESENT

    NRF_LOG_INFO("Enabling SoftDevice");

    ret = nrf_sdh_enable_request();
    if (ret != NRF_SUCCESS)
    {
        NRF_LOG_INFO("nrf_sdh_enable_request() failed");
        APP_ERROR_HANDLER(ret);
    }

    NRF_LOG_INFO("Waiting for SoftDevice to be enabled");

    while (!nrf_sdh_is_enabled()) { }

    // Register a handler for SOC events.
    NRF_SDH_SOC_OBSERVER(m_soc_observer, NRF_SDH_SOC_STACK_OBSERVER_PRIO, OnSoCEvent, NULL);

    NRF_LOG_INFO("SoftDevice enable complete");

#endif // defined(SOFTDEVICE_PRESENT) && SOFTDEVICE_PRESENT

    ret = nrf_mem_init();
    if (ret != NRF_SUCCESS)
    {
        NRF_LOG_INFO("nrf_mem_init() failed");
        APP_ERROR_HANDLER(ret);
    }

#if NRF_CRYPTO_ENABLED
    ret = nrf_crypto_init();
    if (ret != NRF_SUCCESS)
    {
        NRF_LOG_INFO("nrf_crypto_init() failed");
        APP_ERROR_HANDLER(ret);
    }
#endif

#if defined(SOFTDEVICE_PRESENT) && SOFTDEVICE_PRESENT

    {
        uint32_t appRAMStart = 0;

        // Configure the BLE stack using the default settings.
        // Fetch the start address of the application RAM.
        ret = nrf_sdh_ble_default_cfg_set(WEAVE_DEVICE_LAYER_BLE_CONN_CFG_TAG, &appRAMStart);
        APP_ERROR_CHECK(ret);

        // Enable BLE stack.
        ret = nrf_sdh_ble_enable(&appRAMStart);
        APP_ERROR_CHECK(ret);
    }

#endif // defined(SOFTDEVICE_PRESENT) && SOFTDEVICE_PRESENT

    // Configure mbedTLS to use FreeRTOS-based mutexes.  This ensures that mbedTLS can be used
    // simultaneously from multiple FreeRTOS tasks (e.g. OpenThread, OpenWeave and the application).
    freertos_mbedtls_mutex_init();

    NRF_LOG_INFO("Initializing Weave stack");

    ret = PlatformMgr().InitWeaveStack();
    if (ret != WEAVE_NO_ERROR)
    {
        NRF_LOG_INFO("PlatformMgr().InitWeaveStack() failed");
        APP_ERROR_HANDLER(ret);
    }

    NRF_LOG_INFO("Initializing OpenThread stack");

    otSysInit(0, NULL);

    ret = ThreadStackMgr().InitThreadStack();
    if (ret != WEAVE_NO_ERROR)
    {
        NRF_LOG_INFO("ThreadStackMgr().InitThreadStack() failed");
        APP_ERROR_HANDLER(ret);
    }

    // Reconfigure mbedTLS to use regular calloc and free.
    //
    // By default, OpenThread configures mbedTLS to use its private heap at initialization time.  However,
    // the OpenThread heap is not thread-safe, effectively preventing other threads from using mbedTLS
    // functions.
    //
    // Note that this presumes that the system heap is itself thread-safe.  On newlib-based systems
    // this requires a proper implementation of __malloc_lock()/__malloc_unlock() for the applicable
    // RTOS.  It also requires the heap to be provisioned with enough storage to accommodate OpenThread's
    // needs.
    //
    mbedtls_platform_set_calloc_free(calloc, free);

    // Configure device to operate as a Thread sleepy end-device.
    ret = ConnectivityMgr().SetThreadDeviceType(ConnectivityManager::kThreadDeviceType_SleepyEndDevice);
    if (ret != WEAVE_NO_ERROR)
    {
        NRF_LOG_INFO("ConnectivityMgr().SetThreadDeviceType() failed");
        APP_ERROR_HANDLER(ret);
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
            NRF_LOG_INFO("ConnectivityMgr().SetThreadPollingConfig() failed");
            APP_ERROR_HANDLER(ret);
        }
    }

    NRF_LOG_INFO("Starting Weave task");

    ret = PlatformMgr().StartEventLoopTask();
    if (ret != WEAVE_NO_ERROR)
    {
        NRF_LOG_INFO("PlatformMgr().StartEventLoopTask() failed");
        APP_ERROR_HANDLER(ret);
    }

    NRF_LOG_INFO("Starting OpenThread task");

    // Start OpenThread task
    ret = ThreadStackMgrImpl().StartThreadTask();
    if (ret != WEAVE_NO_ERROR)
    {
        NRF_LOG_INFO("ThreadStackMgr().StartThreadTask() failed");
        APP_ERROR_HANDLER(ret);
    }

    ret = GetAppTask().StartAppTask();
    if (ret != NRF_SUCCESS)
    {
        NRF_LOG_INFO("GetAppTask().Init() failed");
        APP_ERROR_HANDLER(ret);
    }

    // Activate deep sleep mode
    SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;

    {
        struct mallinfo minfo = mallinfo();
        NRF_LOG_INFO("System Heap Utilization: heap size %" PRId32 ", arena size %" PRId32 ", in use %" PRId32 ", free %" PRId32,
                GetHeapTotalSize(), minfo.arena, minfo.uordblks, minfo.fordblks);
    }

    NRF_LOG_INFO("Starting FreeRTOS scheduler");

    /* Start FreeRTOS scheduler. */
    vTaskStartScheduler();

    // Should never get here
    NRF_LOG_INFO("vTaskStartScheduler() failed");
    APP_ERROR_HANDLER(0);
}
