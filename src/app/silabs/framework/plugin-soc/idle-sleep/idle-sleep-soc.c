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
 * @brief SoC definitions for the Idle Sleep plugin.
 *******************************************************************************
   ******************************************************************************/

#ifdef EMBER_AF_NCP
#include "app/framework/include/af-ncp.h"
#else // !EMBER_AF_NCP
#include "app/framework/include/af.h"
#include "app/framework/util/af-event.h"
#endif  // EMBER_AF_NCP
#include "idle-sleep.h"

#if defined(EMBER_AF_HAS_RX_ON_WHEN_IDLE_NETWORK) && defined(EMBER_NO_IDLE_SUPPORT)
  #warning Idling and sleeping is not available on SoCs with RX-on-when-idle networks that do not support idling.
#else
  #define IDLE_AND_SLEEP_SUPPORTED
#endif

#if defined(EMBER_AF_PLUGIN_EM4) && ((EMBER_SUPPORTED_NETWORKS > 1))
#error Supported Networks must be set to 1, when em4 enabled
#endif

#if defined (EMBER_SCRIPTED_TEST) // em4 unit test
// User options for plugin Idle/Sleep
#include "app/framework/util/af-main.h"
#include "app/framework/plugin-soc/em4/em4-test.h"
#endif

#ifdef IDLE_AND_SLEEP_SUPPORTED
uint32_t lastWakeupMs = 0;

void emberAfPluginIdleSleepTickCallback(void)
{
  // Turn interrupts off so that we can safely determine whether we can sleep
  // or idle.  Both halSleepForMilliseconds and emberMarkTaskIdle forcibly
  // enable interrupts.  We can call neither function or exactly one of them
  // without risking a race condition.  Note that if we don't call either, we
  // have to enable interrupts ourselves.
  INTERRUPTS_OFF();

  // We can idle or sleep if some basic conditions are satisfied.  If not, we
  // stay awake and active.  Otherwise, we try to sleep or idle, in that order.
  bool inSleepBackoff;
#if (EMBER_AF_PLUGIN_IDLE_SLEEP_BACKOFF_SLEEP_MS > 0)
  inSleepBackoff = (halCommonGetInt32uMillisecondTick() - lastWakeupMs)
                   < EMBER_AF_PLUGIN_IDLE_SLEEP_BACKOFF_SLEEP_MS;
#else
  inSleepBackoff = false;
#endif
  if (emAfOkToIdleOrSleep() && !inSleepBackoff) {
#if !defined(EMBER_AF_HAS_RX_ON_WHEN_IDLE_NETWORK) && !defined(EMBER_AF_NCP)
    // If the stack says we can nap, it means that we may sleep for some amount
    // of time.  Otherwise, we can't sleep at all, although we can try to idle.
    // Also skip to idle if the time since last wakeup<sleep backoff

    if (emberOkToNap()) {
      // If the stack says we can hiberate, it means we can sleep as long as we
      // want.  Otherwise, we cannot sleep longer than the duration to its next
      // event.  In either case, we will never sleep longer than the duration
      // to our next event.
      // Note, emberOkToHibernate() should not be confused with the sleep mode
      // named similarly (SLEEPMODE_HIBERNATE). emberOkToHibernate() simply
      // tests if there is any active stack event that needs to taken into
      // account when calculating next sleep time.
      uint32_t durationMs = (emberOkToHibernate()
                             ? MAX_INT32U_VALUE
                             : emberMsToNextStackEvent());
      durationMs = emberMsToNextEvent(emAfEvents, durationMs);

      // If the sleep duration is below our minimum threshold, we don't bother
      // sleeping.  It takes time to shut everything down and bring everything
      // back up and, at some point, it becomes useless to sleep.  We also give
      // the application one last chance to stay awake if it wants.  If the
      // duration is long enough and the application says okay, we will sleep.
      // Otherwise, we will try to idle instead.
      if (EMBER_AF_PLUGIN_IDLE_SLEEP_MINIMUM_SLEEP_DURATION_MS <= durationMs
          && emberAfPluginIdleSleepOkToSleepCallback(durationMs)) {
        uint32_t attemptedDurationMs = durationMs;
        emberAfEepromShutdownCallback();
        emberStackPowerDown();
        // WARNING: The following block enables interrupts, so we will cease
        // to be atomic by the time it returns. One of
        // emberAfPluginIdleSleepRtosCallback() or halSleepForMilliseconds()
        // will enable interrupts.
        if (!emberAfPluginIdleSleepRtosCallback(&durationMs, true)) {
          halPowerDown();
          if (!emberAfPluginIdleSleepPreEm4ResetCallback(&durationMs)) {
            halSleepForMilliseconds(&durationMs);
          } else {
            halSleep(SLEEPMODE_HIBERNATE);
            return; // we won't continue the code path after em4 sleep
          }
          halPowerUp();
        }
        emberStackPowerUp();
        emberAfEepromNoteInitializedStateCallback(false);
        emberAfPluginIdleSleepWakeUpCallback(attemptedDurationMs - durationMs);
        emberAfDebugPrintln("slept for %l ms", (attemptedDurationMs - durationMs));
        lastWakeupMs = halCommonGetInt32uMillisecondTick();
        return;
      }
    }
#endif // !EMBER_AF_HAS_RX_ON_WHEN_IDLE_NETWORK && !EMBER_AF_NCP

#ifndef EMBER_NO_IDLE_SUPPORT
    // If we are here, it means we could not sleep, so we will try to idle
    // instead.  We give the application one last chance to stay active if it
    // wants.  Note that interrupts are still disabled at this point.
    if (emberAfPluginIdleSleepOkToIdleCallback()) {
      // We are going to try to idle.  Setting our task to idle will trigger a
      // check of every other task.  If the others are also idle, the device
      // will idle.  If not, it won't.  In either case, we set our task back to
      // active again.  This ensures that we are in control of idling and will
      // prevent the device from idling without our knowledge.
      // WARNING: The following function enables interrupts, so we will cease
      // to be atomic by the time it returns.
      bool idled = emberMarkTaskIdle(emAfTaskId);
      emberMarkTaskActive(emAfTaskId);
      if (idled) {
        emberAfPluginIdleSleepActiveCallback();
      }
      return;
    }
#endif // !EMBER_NO_IDLE_SUPPORT
  } // emAfOkToIdleOrSleep

  // If we are here, it means we did not sleep or idle.  Interrupts are still
  // disabled at this point, so we have to enable them again.
  INTERRUPTS_ON();
}

#else // !IDLE_AND_SLEEP_SUPPORTED

void emberAfPluginIdleSleepTickCallback(void)
{
}

#endif // IDLE_AND_SLEEP_SUPPORTED

void emberAfPluginIdleSleepInitCallback(void)
{
#ifdef EMBER_AF_NCP
  emberTaskEnableIdling(true);
#endif //EMBER_AF_NCP
}
