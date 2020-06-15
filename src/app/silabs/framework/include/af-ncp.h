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
 * @brief This file is used to define includes and declarations to adapt
 * afv2 plugins on afv6 framework.
 * This file can be extended to make more afv2 plugins adapt to afv6.
 ******************************************************************************/

#include "ncp-callbacks.h"
#include "hal/plugin/serial/serial.h"
#include "stack/include/ember.h"
#include "hal/hal.h"
#include "stack/include/event.h"

// Renaming afv2 application events to afv6.
// Afv2 framework defines application events as emAfEvents
// where as emAppEvents for afv6.
#define emAfEvents emAppEvents
extern EmberTaskId emAfTaskId;
// Enable this to get prints on virtual uart if needed.
//#define emberAfDebugPrintln(...)  emberSerialPrintfLine(0, __VA_ARGS__)
#define emberAfDebugPrintln(...)

// This is being used by idle-sleep plugin
typedef enum {
  EMBER_AF_OK_TO_SLEEP,
  /** @deprecated. */
  EMBER_AF_OK_TO_HIBERNATE = EMBER_AF_OK_TO_SLEEP,
  /** @deprecated. */
  EMBER_AF_OK_TO_NAP,
  EMBER_AF_STAY_AWAKE,
} EmberAfEventSleepControl;

// Get length of data in a callback queue.
uint16_t emberGetCallbackQueueLength(void);
