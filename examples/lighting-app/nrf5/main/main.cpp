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

#include <platform/CHIPDeviceConfig.h>

#include <stdbool.h>
#include <stdint.h>

#include "boards.h"
#include "nrf_delay.h"
#include "nrf_log.h"
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
#if CHIP_ENABLE_OPENTHREAD
extern "C" {
#include "multiprotocol_802154_config.h"
#include "nrf_802154.h"
#include "nrf_cc310_platform_abort.h"
#include "nrf_cc310_platform_mutex.h"
#include <openthread/platform/platform-softdevice.h>
}
#endif // CHIP_ENABLE_OPENTHREAD

#if NRF_LOG_ENABLED
#include "nrf_log_backend_uart.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#endif // NRF_LOG_ENABLED

#include "chipinit.h"
#include "nrf528xx/app/support/FreeRTOSNewlibLockSupport_test.h"
#include <AppTask.h>
#include <platform/CHIPDeviceLayer.h>

using namespace ::chip;
using namespace ::chip::Inet;
using namespace ::chip::DeviceLayer;

extern "C" size_t GetHeapTotalSize(void);

// ================================================================================
// Logging Support
// ================================================================================

#if NRF_LOG_ENABLED

#if NRF_LOG_USES_TIMESTAMP

uint32_t LogTimestamp(void)
{
    return 0;
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
#if CHIP_ENABLE_OPENTHREAD
    otSysSoftdeviceSocEvtHandler(sys_evt);
#endif
    UNUSED_PARAMETER(p_context);
}

#endif // defined(SOFTDEVICE_PRESENT) && SOFTDEVICE_PRESENT

// ================================================================================
// J-Link Monitor Mode Debugging Support
// ================================================================================

#if JLINK_MMD

extern "C" void JLINK_MONITOR_OnExit(void) {}

extern "C" void JLINK_MONITOR_OnEnter(void) {}

extern "C" void JLINK_MONITOR_OnPoll(void) {}

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
    while (!nrf_clock_lf_is_running())
    {
    }

#if NRF_LOG_ENABLED

    // Initialize logging component
    ret = NRF_LOG_INIT(LOG_TIMESTAMP_FUNC, LOG_TIMESTAMP_FREQ);
    APP_ERROR_CHECK(ret);

    // Initialize logging backends
    NRF_LOG_DEFAULT_BACKENDS_INIT();

#endif

    NRF_LOG_INFO("==================================================");
    NRF_LOG_INFO("chip-nrf52840-lighting-example starting");
#if BUILD_RELEASE
    NRF_LOG_INFO("*** PSEUDO-RELEASE BUILD ***");
#else
    NRF_LOG_INFO("*** DEVELOPMENT BUILD ***");
#endif
    NRF_LOG_INFO("==================================================");
    NRF_LOG_FLUSH();

#ifndef NDEBUG
    // TODO: Move this into a standalone test.
    freertos_newlib_lock_test();
#endif

#if defined(SOFTDEVICE_PRESENT) && SOFTDEVICE_PRESENT

    NRF_LOG_INFO("Enabling SoftDevice");

    ret = nrf_sdh_enable_request();
    if (ret != NRF_SUCCESS)
    {
        NRF_LOG_INFO("nrf_sdh_enable_request() failed");
        APP_ERROR_HANDLER(ret);
    }

    NRF_LOG_INFO("Waiting for SoftDevice to be enabled");

    while (!nrf_sdh_is_enabled())
    {
    }

    // Register a handler for SOC events.
    NRF_SDH_SOC_OBSERVER(m_soc_observer, NRF_SDH_SOC_STACK_OBSERVER_PRIO, OnSoCEvent, NULL);

    NRF_LOG_INFO("SoftDevice enable complete");

#endif // defined(SOFTDEVICE_PRESENT) && SOFTDEVICE_PRESENT

#if defined(SOFTDEVICE_PRESENT) && SOFTDEVICE_PRESENT

    {
        uint32_t appRAMStart = 0;

        // Configure the BLE stack using the default settings.
        // Fetch the start address of the application RAM.
        ret = nrf_sdh_ble_default_cfg_set(CHIP_DEVICE_LAYER_BLE_CONN_CFG_TAG, &appRAMStart);
        APP_ERROR_CHECK(ret);

        // Enable BLE stack.
        ret = nrf_sdh_ble_enable(&appRAMStart);
        APP_ERROR_CHECK(ret);
        NRF_LOG_INFO("SoftDevice BLE enabled");
    }

#endif // defined(SOFTDEVICE_PRESENT) && SOFTDEVICE_PRESENT

    ret = ChipInit();
    if (ret != NRF_SUCCESS)
    {
        NRF_LOG_INFO("ChipInit() failed");
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
