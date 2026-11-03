/*
 *    Copyright (c) 2022 Project CHIP Authors
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

#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <platform/OpenThread/OpenThreadUtils.h>
#include <platform/ThreadStackManager.h>

#include <platform/FreeRTOS/GenericThreadStackManagerImpl_FreeRTOS.hpp>
#include <platform/OpenThread/GenericThreadStackManagerImpl_OpenThread.hpp>

#include <lib/support/CHIPPlatformMemory.h>
#include <lib/support/logging/CHIPLogging.h>

#include <openthread/platform/entropy.h>

#include <mbedtls/platform.h>

#include <openthread_port.h>
#include <utils_list.h>

extern "C" {

#if CHIP_DEVICE_LAYER_TARGET_BFLB
#if defined(BL616)
#include <bl616.h>
#elif defined(BL616CL)
#include <bl616cl.h>
#elif defined(BL618DG)
#include <bl618dg.h>
#endif
#include <bflb_irq.h>
#include <bflb_mtimer.h>
#else
#if defined(BL702L)
#include <bl702l.h>
#elif defined(BL702)
#include <bl702.h>
#endif
#include <bl_irq.h>
#include <bl_timer.h>
#ifdef CFG_PDS_ENABLE
#include <bl_rtc.h>
#endif
#endif

#include <lmac154.h>
#include <lmac154_fpt.h>
#include <zb_timer.h>
}

namespace chip {
namespace DeviceLayer {

using namespace ::chip::DeviceLayer::Internal;

ThreadStackManagerImpl ThreadStackManagerImpl::sInstance;

namespace Internal {

#if !CHIP_DEVICE_LAYER_TARGET_BFLB && defined(CFG_PDS_ENABLE)
static inline uint32_t GetRtcSymbolCounter()
{
    constexpr uint32_t kRtcFrequencyBits = 15;
    constexpr uint32_t kRtcCounterMask   = (1UL << kRtcFrequencyBits) - 1;
    constexpr uint32_t kSymbolsPerSecond = 1000000UL >> LMAC154_US_PER_SYMBOL_BITS;

    uint64_t rtcCounter = bl_rtc_get_counter();
    uint32_t seconds    = static_cast<uint32_t>(rtcCounter >> kRtcFrequencyBits);
    uint32_t remainder  = static_cast<uint32_t>(rtcCounter & kRtcCounterMask);

    return (seconds * kSymbolsPerSecond) +
        (((remainder * kSymbolsPerSecond) + (1UL << (kRtcFrequencyBits - 1))) >> kRtcFrequencyBits);
}
#endif

CHIP_ERROR InitThreadPlatform()
{
    lmac154_init();
    lmac154_enableCoex();
    lmac154_setStd2015Extra(true);
    lmac154_setTxRetry(0);
    lmac154_fptClear();

    lmac154_setEnhAckWaitTime((LMAC154_AIFS + (6 + 39) * 2 + 20) << LMAC154_US_PER_SYMBOL_BITS);
    lmac154_setRxStateWhenIdle(true);

#if CHIP_DEVICE_LAYER_TARGET_BL702 || CHIP_DEVICE_LAYER_TARGET_BL702L
    lmac154_setTxRxTransTime(0xA0);
#endif

#if CHIP_DEVICE_CONFIG_THREAD_FTD && ((CHIP_DEVICE_LAYER_TARGET_BFLB && defined(BL616)) || CHIP_DEVICE_LAYER_TARGET_BL702L)
    lmac154_setFramePendingMode(LMAC154_FPT_ANY);
#endif

#if CHIP_DEVICE_LAYER_TARGET_BFLB
    zb_timer_cfg(bflb_mtimer_get_time_us() >> LMAC154_US_PER_SYMBOL_BITS);
#else
#ifdef CFG_PDS_ENABLE
    zb_timer_cfg(GetRtcSymbolCounter());
#else
    zb_timer_cfg(bl_timer_now_us64() >> LMAC154_US_PER_SYMBOL_BITS);
#endif
#endif
    lmac154_disableRx();

#if CHIP_DEVICE_LAYER_TARGET_BFLB
    bflb_irq_attach(M154_INT_IRQn, (irq_callback) lmac154_getInterruptCallback(), nullptr);
    bflb_irq_enable(M154_INT_IRQn);
#else
    bl_irq_register(M154_IRQn, (void *) lmac154_getInterruptCallback());
    bl_irq_enable(M154_IRQn);
#endif
    return CHIP_NO_ERROR;
}

} // namespace Internal

CHIP_ERROR ThreadStackManagerImpl::_InitThreadStack(void)
{
    return InitThreadStack(NULL);
}

CHIP_ERROR ThreadStackManagerImpl::InitThreadStack(otInstance * otInst)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // Initialize the generic implementation base classes.
    err = GenericThreadStackManagerImpl_FreeRTOS<ThreadStackManagerImpl>::DoInit();
    SuccessOrExit(err);
    err = GenericThreadStackManagerImpl_OpenThread<ThreadStackManagerImpl>::DoInit(otInst);
    SuccessOrExit(err);

    ot_radioSetInstance(OTInstance());

#if CHIP_DEVICE_LAYER_TARGET_BL702 || CHIP_DEVICE_LAYER_TARGET_BL702L
    mbedtls_platform_set_calloc_free(pvPortCalloc, vPortFree);
#endif

exit:
    return err;
}

bool ThreadStackManagerImpl::IsInitialized()
{
    return sInstance.mThreadStackLock != NULL;
}

void ThreadStackManagerImpl::FactoryResetThreadStack(void)
{
    VerifyOrReturn(ThreadStackMgrImpl().OTInstance() != NULL);
    otInstanceFactoryReset(ThreadStackMgrImpl().OTInstance());
}

} // namespace DeviceLayer
} // namespace chip

using namespace ::chip::DeviceLayer;

extern "C" ot_system_event_t otrGetNotifyEvent(void);

ot_system_event_t ot_system_event_var = OT_SYSTEM_EVENT_NONE;

void otSysProcessDrivers(otInstance * aInstance)
{
    ot_system_event_t sevent = otrGetNotifyEvent();

    if (sevent)
    {
        ot_alarmTask(sevent);
        ot_radioTask(sevent);
    }
}

extern "C" void otTaskletsSignalPending(otInstance * p_instance)
{
    otSysEventSignalPending();
}

extern "C" void otSysEventSignalPending(void)
{
    if (xPortIsInsideInterrupt())
    {
        BaseType_t yieldRequired = ThreadStackMgrImpl().SignalThreadActivityPendingFromISR();
        portYIELD_FROM_ISR(yieldRequired);
    }
    else
    {
        ThreadStackMgrImpl().SignalThreadActivityPending();
    }
}

extern "C" otInstance * otrGetInstance()
{
    return ThreadStackMgrImpl().OTInstance();
}

extern "C" void otrLock(void)
{
    ThreadStackMgrImpl().LockThreadStack();
}

extern "C" void otrUnlock(void)
{
    ThreadStackMgrImpl().UnlockThreadStack();
}

extern "C" uint32_t otrEnterCrit(void)
{
    if (xPortIsInsideInterrupt())
    {
        return taskENTER_CRITICAL_FROM_ISR();
    }
    else
    {
        taskENTER_CRITICAL();
        return 0;
    }
}

extern "C" void otrExitCrit(uint32_t tag)
{
    if (xPortIsInsideInterrupt())
    {
        taskEXIT_CRITICAL_FROM_ISR(tag);
    }
    else
    {
        taskEXIT_CRITICAL();
    }
}

extern "C" ot_system_event_t otrGetNotifyEvent(void)
{
    ot_system_event_t sevent = OT_SYSTEM_EVENT_NONE;

    taskENTER_CRITICAL();
    sevent              = ot_system_event_var;
    ot_system_event_var = OT_SYSTEM_EVENT_NONE;
    taskEXIT_CRITICAL();

    return sevent;
}

extern "C" void otrNotifyEvent(ot_system_event_t sevent)
{
    uint32_t tag        = otrEnterCrit();
    ot_system_event_var = (ot_system_event_t) (ot_system_event_var | sevent);
    otrExitCrit(tag);

    otSysEventSignalPending();
}
