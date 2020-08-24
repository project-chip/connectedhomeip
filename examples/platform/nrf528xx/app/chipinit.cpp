#include "chipinit.h"

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
#include <core/CHIPError.h>
#include <platform/CHIPDeviceLayer.h>
#include <support/logging/CHIPLogging.h>

#include "Server.h"

using namespace ::chip;
using namespace ::chip::DeviceLayer;

DemoSessionManager sessions;

#if CHIP_ENABLE_OPENTHREAD
static void * ot_calloc(size_t n, size_t size)
{
    void * p_ptr = NULL;

    p_ptr = pvPortMalloc(n * size);

    memset(p_ptr, 0, n * size);

    return p_ptr;
}

static void ot_free(void * p_ptr)
{
    vPortFree(p_ptr);
}
#endif

ret_code_t ChipInit()
{
    ret_code_t ret = CHIP_NO_ERROR;

    NRF_LOG_INFO("Init CHIP stack");
    ret = PlatformMgr().InitChipStack();
    if (ret != CHIP_NO_ERROR)
    {
        NRF_LOG_INFO("PlatformMgr().InitChipStack() failed");
        APP_ERROR_HANDLER(ret);
    }

#if CHIP_ENABLE_OPENTHREAD
    NRF_LOG_INFO("Initializing OpenThread stack");

    mbedtls_platform_set_calloc_free(ot_calloc, ot_free);
    nrf_cc310_platform_abort_init();
    nrf_cc310_platform_mutex_init();
    mbedtls_platform_setup(NULL);
    otHeapSetCAllocFree(ot_calloc, ot_free);

    otSysInit(0, NULL);

    // Configure multiprotocol to work with BLE.
    {
        uint32_t retval = multiprotocol_802154_mode_set(MULTIPROTOCOL_802154_MODE_FAST_SWITCHING_TIMES);

        if (retval != NRF_SUCCESS)
        {
            NRF_LOG_INFO("multiprotocol 15.4 failed");
            APP_ERROR_HANDLER(CHIP_ERROR_INTERNAL);
        }
    }

    ret = ThreadStackMgr().InitThreadStack();
    if (ret != CHIP_NO_ERROR)
    {
        NRF_LOG_INFO("ThreadStackMgr().InitThreadStack() failed");
        APP_ERROR_HANDLER(ret);
    }

    // Configure device to operate as a Thread sleepy end-device.
    ret = ConnectivityMgr().SetThreadDeviceType(ConnectivityManager::kThreadDeviceType_Router);
    if (ret != CHIP_NO_ERROR)
    {
        NRF_LOG_INFO("ConnectivityMgr().SetThreadDeviceType() failed");
        APP_ERROR_HANDLER(ret);
    }
#endif // CHIP_ENABLE_OPENTHREAD

    NRF_LOG_INFO("Starting CHIP task");
    ret = PlatformMgr().StartEventLoopTask();
    if (ret != CHIP_NO_ERROR)
    {
        NRF_LOG_INFO("PlatformMgr().StartEventLoopTask() failed");
        APP_ERROR_HANDLER(ret);
    }

#if CHIP_ENABLE_OPENTHREAD
    NRF_LOG_INFO("Starting OpenThread task");

    // Start OpenThread task
    ret = ThreadStackMgrImpl().StartThreadTask();
    if (ret != CHIP_NO_ERROR)
    {
        NRF_LOG_INFO("ThreadStackMgr().StartThreadTask() failed");
        APP_ERROR_HANDLER(ret);
    }
#endif // CHIP_ENABLE_OPENTHREAD

    // Init ZCL Data Model
    InitDataModelHandler();
    StartServer(&sessions);

    return ret;
}
