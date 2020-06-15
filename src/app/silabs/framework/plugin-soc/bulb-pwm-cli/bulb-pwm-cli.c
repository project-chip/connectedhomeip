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
 * @brief CLI for the Bulb PWN CLI plugin.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "app/framework/util/af-main.h"
#include EMBER_AF_API_BULB_PWM_DRIVER

// -----------------------------------------------------------------------------
// Defines
#ifdef EMBER_AF_PLUGIN_BULB_PWM_DRIVER_BLINK_PATTERN_MAX_LENGTH
  #define BLINK_PATTERN_MAX_LENGTH EMBER_AF_PLUGIN_BULB_PWM_DRIVER_BLINK_PATTERN_MAX_LENGTH
#else
  #define BLINK_PATTERN_MAX_LENGTH 20
#endif

// -----------------------------------------------------------------------------
// Globals
static uint16_t ledPattern[BLINK_PATTERN_MAX_LENGTH];
static uint16_t statusPattern[BLINK_PATTERN_MAX_LENGTH];
static uint8_t patternBuffer8u[BLINK_PATTERN_MAX_LENGTH * 2];

// -----------------------------------------------------------------------------
// Forward Declarations

// -----------------------------------------------------------------------------
void emAfBulbPwmSetCommand(void)
{
  uint8_t channel = (uint8_t)emberUnsignedCommandArgument(0);
  uint16_t value = (uint16_t)emberUnsignedCommandArgument(1);

  halBulbPwmDriverSetPwmLevel(value, channel);
}

void emAfBulbPwmLedOnCommand(void)
{
  uint8_t time = (uint8_t)emberUnsignedCommandArgument(0);

  halBulbPwmDriverLedOn(time);
}

void emAfBulbPwmStatusOnCommand(void)
{
  uint8_t time = (uint8_t)emberUnsignedCommandArgument(0);

  halBulbPwmDriverStatusOn(time);
}

void emAfBulbPwmLedOffCommand(void)
{
  uint8_t time = (uint8_t)emberUnsignedCommandArgument(0);

  halBulbPwmDriverLedOff(time);
}

void emAfBulbPwmStatusOffCommand(void)
{
  uint8_t time = (uint8_t)emberUnsignedCommandArgument(0);

  halBulbPwmDriverStatusOff(time);
}

void emAfBulbPwmLedBlinkCommand(void)
{
  uint8_t count = (uint8_t)emberUnsignedCommandArgument(0);
  uint16_t time = (uint16_t)emberUnsignedCommandArgument(1);

  halBulbPwmDriverLedBlink(count, time);
}

void emAfBulbPwmStatusBlinkCommand(void)
{
  uint8_t count = (uint8_t)emberUnsignedCommandArgument(0);
  uint16_t time = (uint16_t)emberUnsignedCommandArgument(1);

  halBulbPwmDriverStatusBlink(count, time);
}

static uint8_t readPatternArgument(uint8_t argument, uint16_t *pattern)
{
  uint8_t bytesRead, patternLength, i;

  bytesRead = emberCopyStringArgument(argument,
                                      patternBuffer8u,
                                      BLINK_PATTERN_MAX_LENGTH,
                                      FALSE);

  // convert incoming bytes into words.
  patternLength = bytesRead / 2;

  for (i = 0; i < patternLength; i++) {
    pattern[i] = HIGH_LOW_TO_INT(patternBuffer8u[i * 2], patternBuffer8u[i * 2 + 1]);
  }

  return patternLength;
}

void emAfBulbPwmLedPatternCommand(void)
{
  uint8_t count = (uint8_t)emberUnsignedCommandArgument(0);
  uint8_t length = readPatternArgument(1, ledPattern);

  halBulbPwmDriverLedBlinkPattern(count, length, ledPattern);
}

void emAfBulbPwmStatusPatternCommand(void)
{
  uint8_t count = (uint8_t)emberUnsignedCommandArgument(0);
  uint8_t length = readPatternArgument(1, statusPattern);

  halBulbPwmDriverStatusBlinkPattern(count, length, statusPattern);
}
