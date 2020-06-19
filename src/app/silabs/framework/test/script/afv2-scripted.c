/**
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
 * @brief Stubs for AFV2 scripted unit tests.
 *******************************************************************************
   ******************************************************************************/

#include "afv2-scripted.h"

#define EMBER_APPLICATION_HAS_GET_ENDPOINT
#include "config/ember-configuration.c"

#ifdef EMBER_STACK_PROFILE
#undef EMBER_STACK_PROFILE
#endif

// -----------------------------------------------------------------------------
// TICK.

static uint32_t systemTick = 0;

uint32_t scriptTime(void)
{
  return systemTick;
}

void scriptTick(void)
{
  systemTick += 1;
  microSetSystemTime(systemTick);
  scriptTickCallback();
}

// -----------------------------------------------------------------------------
// STUBS.

EmberStatus halCommonIdleForMilliseconds(uint32_t *duration)
{
  return EMBER_SUCCESS;
}
void emCallCounterHandler(EmberCounterType type, uint8_t data)
{
}
void emBuildAndSendCounterInfo(EmberCounterType counter, EmberNodeId dst, uint8_t data)
{
}

// -----------------------------------------------------------------------------
// PRINT.

// These can be implemented like in test-framework.c if we want to.

void emberAfPrint(uint16_t functionality, const char * formatString, ...)
{
}

void emberAfPrintln(uint16_t functionality, const char * formatString, ...)
{
}

void emberAfPrintBuffer(uint16_t area,
                        const uint8_t *buffer,
                        uint16_t bufferLen,
                        bool withSpace)
{
}

bool emberAfPrintEnabled(uint16_t area)
{
  return true;
}

void emberAfPrintBigEndianEui64(const EmberEUI64 eui64)
{
}
