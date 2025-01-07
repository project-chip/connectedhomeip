/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
 *    Copyright (c) 2019 Nest Labs, Inc.
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

#include <crypto/CHIPCryptoPAL.h>
#include <platform/internal/CHIPDeviceLayerInternal.h>
#include <stdlib.h>

#include <platform/FreeRTOS/SystemTimeSupport.h>
#include <platform/PlatformManager.h>
#include <platform/bouffalolab/BL616/NetworkCommissioningDriver.h>
#include <platform/bouffalolab/common/DiagnosticDataProviderImpl.h>
#include <platform/internal/GenericPlatformManagerImpl_FreeRTOS.ipp>

#if CHIP_SYSTEM_CONFIG_USE_LWIP
#include <lwip/tcpip.h>
#endif

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
#include <wifi_mgmr_portable.h>
#endif

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
#include <openthread_port.h>
#include <utils_list.h>
#endif

#include <bflb_flash.h>
#include <bflb_sec_trng.h>
extern "C" {
#include <partition.h>
#include <rfparam_adapter.h>
}

namespace chip {
namespace DeviceLayer {

static int app_entropy_source(void * data, unsigned char * output, size_t len, size_t * olen)
{
    bflb_trng_readlen(reinterpret_cast<uint8_t *>(output), static_cast<int>(len));
    *olen = len;
    return 0;
}

CHIP_ERROR PlatformManagerImpl::_InitChipStack(void)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    TaskHandle_t backup_eventLoopTask;
    int iret_rfInit = -1;

    pt_table_set_flash_operation(bflb_flash_erase, bflb_flash_write, bflb_flash_read);

    VerifyOrDieWithMsg(0 == (iret_rfInit = rfparam_init(0, NULL, 0)), DeviceLayer, "rfparam_init failed with %d", iret_rfInit);

#if CHIP_SYSTEM_CONFIG_USE_LWIP
    tcpip_init(NULL, NULL);
#endif

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
    wifi_start_firmware_task();
#endif

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
    otRadio_opt_t opt;
#if CHIP_DEVICE_CONFIG_THREAD_FTD
    opt.bf.isFtd = true;
#else
    opt.bf.isFtd = false;
#endif
    opt.bf.isCoexEnable = true;

    ot_alarmInit();
    ot_radioInit(opt);
#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD

    ReturnErrorOnFailure(System::Clock::InitClock_RealTime());

    err = chip::Crypto::add_entropy_source(app_entropy_source, NULL, 16);
    SuccessOrExit(err);

    // Call _InitChipStack() on the generic implementation base class
    // to finish the initialization process.
    /** weiyin, backup mEventLoopTask which is reset in _InitChipStack */
    backup_eventLoopTask = Internal::GenericPlatformManagerImpl_FreeRTOS<PlatformManagerImpl>::mEventLoopTask;
    err                  = Internal::GenericPlatformManagerImpl_FreeRTOS<PlatformManagerImpl>::_InitChipStack();
    SuccessOrExit(err);
    Internal::GenericPlatformManagerImpl_FreeRTOS<PlatformManagerImpl>::mEventLoopTask = backup_eventLoopTask;

exit:
    return err;
}

} // namespace DeviceLayer
} // namespace chip
