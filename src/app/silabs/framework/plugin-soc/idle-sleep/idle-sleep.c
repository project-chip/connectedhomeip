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
 * @brief Definitions for the Idle Sleep plugin, which handle chip idling and
 *        sleeping.
 *******************************************************************************
   ******************************************************************************/

#ifdef EMBER_AF_NCP
#include "app/framework/include/af-ncp.h"
#else
#include "app/framework/include/af.h"
#include "app/framework/util/af-event.h"
#endif
#include "idle-sleep.h"

#if defined(EMBER_AF_PLUGIN_BLE)
#include EMBER_AF_API_BLE
#endif // EMBER_AF_PLUGIN_BLE

#ifdef EZSP_HOST
  #define MAX_SLEEP_VALUE_MS MAX_TIMER_MILLISECONDS_HOST
#else
  #define MAX_SLEEP_VALUE_MS 0xFFFFFFFFUL
#endif

#ifdef EMBER_AF_PLUGIN_IDLE_SLEEP_STAY_AWAKE_WHEN_NOT_JOINED
  #define STAY_AWAKE_WHEN_NOT_JOINED_DEFAULT true
#else
  #define STAY_AWAKE_WHEN_NOT_JOINED_DEFAULT false
#endif
bool emAfStayAwakeWhenNotJoined = STAY_AWAKE_WHEN_NOT_JOINED_DEFAULT;

bool emAfForceEndDeviceToStayAwake = false;

// NO PRINTFS.  This may be called in ISR context.
void emberAfForceEndDeviceToStayAwake(bool stayAwake)
{
  emAfForceEndDeviceToStayAwake = stayAwake;
}

#ifdef EMBER_AF_PLUGIN_IDLE_SLEEP_USE_BUTTONS
void emberAfHalButtonIsrCallback(uint8_t button, uint8_t state)
{
  if (state == BUTTON_PRESSED) {
    emberAfForceEndDeviceToStayAwake(button == BUTTON0);
  }
}
#endif

#ifdef EMBER_TEST
extern bool doingSerialTx[];
  #define simulatorDoingSerialTx(port) doingSerialTx[port]
#else
  #define simulatorDoingSerialTx(port) false
#endif

bool emAfOkToIdleOrSleep(void)
{
  uint8_t i;

  if (emAfForceEndDeviceToStayAwake) {
    return false;
  }

#if defined(EMBER_AF_PLUGIN_BLE)
  if (emberAfPluginBleHasEventPending()) {
    return false;
  }
#endif

#ifndef EMBER_AF_NCP
  if (emAfStayAwakeWhenNotJoined) {
    bool awake = false;
    for (i = 0; !awake && i < EMBER_SUPPORTED_NETWORKS; i++) {
      if (emberAfPushNetworkIndex(i) == EMBER_SUCCESS) {
        awake = (emberAfNetworkState() != EMBER_JOINED_NETWORK);
        emberAfPopNetworkIndex();
      }
    }
    if (awake) {
      return false;
    }
  }
#else // EMBER_AF_NCP
  // Only idle support is available on NCP, it is mainly used for yielding
  // Zigbee task in multiprotocol ncp application. So idling can be done
  // any time irrespective of network state.
  (void)emAfStayAwakeWhenNotJoined; // to make compiler happy
#endif  // EMBER_AF_NCP

#ifdef EM_NUM_SERIAL_PORTS
  for (i = 0; i < EM_NUM_SERIAL_PORTS; i++) {
    if (!emberSerialUnused(i)
        && (emberSerialReadAvailable(i) != 0
            || emberSerialWriteUsed(i) != 0
            || simulatorDoingSerialTx(i))) {
      return false;
    }
  }
#else
  if (!emberSerialUnused(APP_SERIAL)
      && (emberSerialReadAvailable(APP_SERIAL) != 0
          || emberSerialWriteUsed(APP_SERIAL) != 0
          || simulatorDoingSerialTx(APP_SERIAL))) {
    return false;
  }
#endif

#ifdef EMBER_AF_NCP
  if (emberGetCallbackQueueLength()) {
    return false;
  }
#endif  // EMBER_AF_NCP

#ifndef EMBER_AF_NCP
  return (emberAfGetCurrentSleepControlCallback() != EMBER_AF_STAY_AWAKE);
#else // EMBER_AF_NCP
  // There is only idling on NCP, hence return true always.
  return true;
#endif  // EMBER_AF_NCP
}

#ifndef EMBER_AF_NCP
// There is only idling on NCP hence sleep control code is not needed.
static EmberAfEventSleepControl defaultSleepControl = EMBER_AF_OK_TO_SLEEP;

EmberAfEventSleepControl emberAfGetCurrentSleepControlCallback(void)
{
  EmberAfEventSleepControl sleepControl = defaultSleepControl;
#ifdef EMBER_AF_GENERATED_EVENT_CONTEXT
  uint8_t i;
  for (i = 0; i < emAfAppEventContextLength; i++) {
    EmberAfEventContext *context = &emAfAppEventContext[i];
    if (emberEventControlGetActive(*context->eventControl)
        && sleepControl < context->sleepControl) {
      sleepControl = context->sleepControl;
    }
  }
#endif
  return sleepControl;
}

EmberAfEventSleepControl emberAfGetDefaultSleepControlCallback(void)
{
  return defaultSleepControl;
}

void emberAfSetDefaultSleepControlCallback(EmberAfEventSleepControl sleepControl)
{
  defaultSleepControl = sleepControl;
}
#endif // EMBER_AF_NCP
