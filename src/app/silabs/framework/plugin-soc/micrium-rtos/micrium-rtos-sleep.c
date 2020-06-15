/**
 *
 *    Copyright (c) 2020 Silicon Labs
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
/***************************************************************************//**
 * @file
 * @brief Code related to letting the chip sleep when using Micrium RTOS.
 *******************************************************************************
   ******************************************************************************/

#include PLATFORM_HEADER
#ifdef EMBER_AF_NCP
#include "app/framework/include/af-ncp.h"
#else // !EMBER_AF_NCP
#include "app/framework/include/af.h"
#endif // EMBER_AF_NCP
#include "sl_sleeptimer.h"
#include <kernel/include/os.h>
#include "em_cmu.h"
#include "em_emu.h"
#include "sleep.h"
#include "rail.h"

#if (defined(EMBER_AF_PLUGIN_MICRIUM_RTOS_POLL_CLI) && defined(ZA_CLI_FULL))
#define ENABLE_POLL_CLI
#define POLL_CLI_TASK_PERIOD_MS                 250
#endif // EMBER_AF_PLUGIN_MICRIUM_RTOS_POLL_CLI && ZA_CLI_FULL

//------------------------------------------------------------------------------
// Static variables.

#ifndef EMBER_AF_NCP
// Only idling is supported on NCP.
static bool zigbeeTaskDeepSleepAllowed = false;
static sl_sleeptimer_timer_handle_t wakeupTimerId;
#endif // EMBER_AF_NCP
static uint32_t zigbeeTaskNextWakeUpTimeMs;

//------------------------------------------------------------------------------
// Extern and forward declarations.

extern void emAfPluginMicriumRtosWakeUpZigbeeStackTask(void);
static uint16_t EM2WakeupProcessTime(void);
#ifdef EMBER_AF_NCP
// This flag is to avoid posting multiple zigbee task semaphores or
// posting a semaphore if zigbee task hasn't been pended.
// This could happen due to uart interrupts occur on every byte.
boolean isZigbeeTaskPended = true;
#endif // EMBER_AF_NCP
//------------------------------------------------------------------------------
// Sleep handler - invoked from the idle task hook.
// On NCP - this handler only idle the processor.

void emAfPluginMicriumRtosSleepHandler(void)
{
#ifndef EMBER_AF_NCP
  // To fix never referenced warning
  bool railCanDeepSleep = false;
  bool bleCanDeepSleep = true;
#endif // EMBER_AF_NCP
  uint32_t zigbeeSleepTimeMs, callTimeMs;

  INTERRUPTS_OFF();

#if defined(EMBER_AF_PLUGIN_BLE) && !defined(EMBER_AF_NCP)
  // The BLE stacks keeps the SLEEP manager updated with the current lowest
  // energy mode allowed, so we just query the SLEEP manager to determine
  // whether BLE can deep sleep or not.
  bleCanDeepSleep = (SLEEP_LowestEnergyModeGet() != sleepEM1);
#endif

  callTimeMs = halCommonGetInt32uMillisecondTick();

  if (timeGTorEqualInt32u(callTimeMs, zigbeeTaskNextWakeUpTimeMs)) {
    // ZigBee next wake up time is in the past, don't sleep at all and post the
    // ZigBee task so that it can refresh the next wake up time.
    INTERRUPTS_ON();
    emAfPluginMicriumRtosWakeUpZigbeeStackTask();
    return;
  } else {
    zigbeeSleepTimeMs =
      elapsedTimeInt32u(callTimeMs, zigbeeTaskNextWakeUpTimeMs);
  }

#ifndef EMBER_AF_NCP
  if (bleCanDeepSleep && zigbeeTaskDeepSleepAllowed) {
    RAIL_Status_t status = RAIL_Sleep(EM2WakeupProcessTime(),
                                      &railCanDeepSleep);
    // RAIL_Sleep() returns a non-success status when the radio is not idle. We
    // do not deep sleep in that case.
    if (status != RAIL_STATUS_NO_ERROR) {
      railCanDeepSleep = false;
    }
  }

  if (zigbeeTaskDeepSleepAllowed && bleCanDeepSleep && railCanDeepSleep) {
    RTOS_ERR err;

    // Lock the OS scheduler so that we can get to the RAIL_Wake() call once we
    // are done deep sleeping.
    OSSchedLock(&err);
    assert(RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE);

    halPowerDown();

    // Start an RTC timer to get us out of EM2.
    assert(sl_sleeptimer_start_timer_ms(&wakeupTimerId,
                                        zigbeeSleepTimeMs,
                                        NULL,
                                        NULL,
                                        0u,
                                        0u) == SL_STATUS_OK);

    // Enter EM2.
    halSleepPreserveInts(SLEEPMODE_WAKETIMER);

    assert(RAIL_Wake(0) == RAIL_STATUS_NO_ERROR);

    INTERRUPTS_ON();

    halPowerUp();

    sl_sleeptimer_stop_timer(&wakeupTimerId);

    OSSchedUnlock(&err);
    assert(RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE);
  } else {
#endif // EMBER_AF_NCP
  halCommonIdleForMilliseconds(&zigbeeSleepTimeMs);

  // Before Gecko SDK 2.7 this function (ie. emAfPluginMicriumRtosSleepHandler())
  // was called periodically from the idle task through the idle hook, meaning
  // that all sleep requirements could have been periodically reevaluated.
  // Micrium in Gecko SDK 2.7 has changed this, meaning that
  // emAfPluginMicriumRtosSleepHandler() is only called once from the idle state.
  // We post the zigbee task here, in order to make sure that the zigbee task can
  // still reevaluate the sleep requirements.
  emAfPluginMicriumRtosWakeUpZigbeeStackTask();
#ifndef EMBER_AF_NCP
}
#endif  // EMBER_AF_NCP
}

//------------------------------------------------------------------------------
// Idle/sleep rtos callback - invoked from the idle/sleep plugin whenever the
// Zigbee stack can idle or sleep.

bool emberAfPluginIdleSleepRtosCallback(uint32_t *durationMs, bool sleepOk)
{
  uint32_t actualDurationMs = *durationMs;
  uint32_t deltaMs, startTicks = sl_sleeptimer_get_tick_count();
  OS_TICK yieldTimeTicks;
  RTOS_ERR err;
  CPU_TS ts;

#if defined(ENABLE_POLL_CLI)
  if (actualDurationMs > POLL_CLI_TASK_PERIOD_MS) {
    actualDurationMs = POLL_CLI_TASK_PERIOD_MS;
  }
#endif // ENABLE_POLL_CLI

  yieldTimeTicks = (OSTimeTickRateHzGet(&err) * actualDurationMs) / 1000;

#ifndef EMBER_AF_NCP
  zigbeeTaskDeepSleepAllowed = sleepOk;
#endif // EMBER_AF_NCP
  zigbeeTaskNextWakeUpTimeMs
    = halCommonGetInt32uMillisecondTick() + actualDurationMs;

  INTERRUPTS_ON();

#ifdef EMBER_AF_NCP
  // Set the flag just before yielding the zigbee task.
  ATOMIC(isZigbeeTaskPended = true; )

  // Enable the UART interrupt(s) to active zigbee stack task
  // if there is serial port activity. This is mainly to process
  // ezsp frames from zigbee host if something comes up while zigbee
  // task is suspended. UART irq handler post the semaphore to run
  // the zigbee stack task.
  COM_InternalPowerDown(true);
#endif  // EMBER_AF_NCP

  // Yield the stack task.
  OSTaskSemPend(yieldTimeTicks, OS_OPT_PEND_BLOCKING, &ts, &err);

#ifdef EMBER_AF_NCP
  // Disable back the UART interrupt(s).
  COM_InternalPowerUp(true);
#endif // EMBER_AF_NCP

#ifndef EMBER_AF_NCP
  zigbeeTaskDeepSleepAllowed = false;
#endif // EMBER_AF_NCP

  deltaMs = sl_sleeptimer_tick_to_ms(sl_sleeptimer_get_tick_count() - startTicks);

  if ( deltaMs <= actualDurationMs ) {
    *durationMs = actualDurationMs - deltaMs;
  } else {
    *durationMs = 0;
  }

  return true;
}

//------------------------------------------------------------------------------
// Static functions.

// Jira:EMZIGBEE-2690: EM2_WAKEUP_PROCESS_TIME_OVERHEAD_US value is set to 215
// in the BLE code (which is where this code comes from). On Jumbo, this
// translates to EM2WakeupProcessTime() returning 575us, which turned out to be
// too close. Bumping the overall EM2WakeupProcessTime() to 750 (by bumbing
// EM2_WAKEUP_PROCESS_TIME_OVERHEAD_US to 390) gives RAIL enough time to keep up
// with BLE events.

// Time required by the hardware to come out of EM2 in microseconds.
// This value includes HW startup, emlib and sleepdrv execution time.
// Voltage scaling, HFXO startup and HFXO steady times are excluded from
// this because they are handled separately. RTCCSYNC time is also
// excluded and it is handled by RTCCSYNC code itself.
#define EM2_WAKEUP_PROCESS_TIME_OVERHEAD_US (390)

// Time it takes to upscale voltage after EM2 in microseconds.
#define EM2_WAKEUP_VSCALE_OVERHEAD_US (30)

/* one cycle is 83 ns */
#define TIMEOUT_PERIOD_US(cycles) (cycles * 83 / 1000)
static const uint16_t timeoutPeriodTable[] =
{
  TIMEOUT_PERIOD_US(2),       /* 0 = 2 cycles */
  TIMEOUT_PERIOD_US(4),       /* 1 = 4 cycles */
  TIMEOUT_PERIOD_US(16),      /* 2 = 16 cycles */
  TIMEOUT_PERIOD_US(32),      /* 3 = 32 cycles */
  TIMEOUT_PERIOD_US(256),     /* 4 = 256 cycles */
  TIMEOUT_PERIOD_US(1024),    /* 5 = 1024 cycles */
  TIMEOUT_PERIOD_US(2048),    /* 6 = 2048 cycles */
  TIMEOUT_PERIOD_US(4096),    /* 7 = 4096 cycles */
  TIMEOUT_PERIOD_US(8192),    /* 8 = 8192 cycles */
  TIMEOUT_PERIOD_US(16384),   /* 9 = 16384 cycles */
  TIMEOUT_PERIOD_US(32768),   /* 10 = 32768 cycles */
};

static bool isHfxoAutoSelectEnabled(void)
{
#ifdef _CMU_HFXOCTRL_AUTOSTARTSELEM0EM1_MASK
  if (CMU->HFXOCTRL & _CMU_HFXOCTRL_AUTOSTARTSELEM0EM1_MASK) {
    return true;
  }
#endif
  return false;
}

static bool isEm23VScaleEnabled(void)
{
#ifdef _EMU_CTRL_EM23VSCALE_MASK
  if (EMU->CTRL & _EMU_CTRL_EM23VSCALE_MASK) {
    return true;
  }
#endif
  return false;
}

static uint16_t em2WakeupVScaleOverhead(void)
{
  if (!isEm23VScaleEnabled()) {
    return 0;
  }

  if (!isHfxoAutoSelectEnabled()) {
    // If HFXO auto select is disabled, the voltage upscaling is done in
    // EMLIB while waiting for HFXO to stabilize, thus adding no overhead
    // to the overall wakeup time.
    return 0;
  }

  return EM2_WAKEUP_VSCALE_OVERHEAD_US;
}

static uint16_t EM2WakeupProcessTime(void)
{
#ifndef _SILICON_LABS_32B_SERIES_2
  uint8_t steady = ((CMU->HFXOTIMEOUTCTRL
                     & _CMU_HFXOTIMEOUTCTRL_STEADYTIMEOUT_MASK)
                    >> _CMU_HFXOTIMEOUTCTRL_STEADYTIMEOUT_SHIFT);
  uint8_t startup = ((CMU->HFXOTIMEOUTCTRL
                      & _CMU_HFXOTIMEOUTCTRL_STARTUPTIMEOUT_MASK)
                     >> _CMU_HFXOTIMEOUTCTRL_STARTUPTIMEOUT_SHIFT);
  return timeoutPeriodTable[steady] + timeoutPeriodTable[startup]
         + EM2_WAKEUP_PROCESS_TIME_OVERHEAD_US + em2WakeupVScaleOverhead();
#else
  // Note: (EMHAL-1521) return some safe value (1ms) until we have all the
  // proper registers for EFR series 2.
  return 1000;
#endif
}
