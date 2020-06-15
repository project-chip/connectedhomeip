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
 * @brief CLI for the Compact Logger plugin.
 *******************************************************************************
   ******************************************************************************/

#include "af.h"
#include "compact-logger.h"

// plugin compact-logger clear
void emAfCompactLoggerClearLog(void)
{
  emberAfPluginCompactLoggerInit();
}

// plugin compact-logger set-severity
void emAfCompactLoggerSetSeverity(void)
{
  EmberStatus status;
  uint8_t severity = (uint8_t)emberUnsignedCommandArgument(0);
  status = emberAfPluginCompactLoggerSetSeverity(severity);
  emberAfCorePrintln("Set severity to %d, status=0x%x", severity, status);
}

// plugin compact-logger set-facility
void emAfCompactLoggerSetFacility(void)
{
  uint16_t facility = (uint16_t)emberUnsignedCommandArgument(0);
  emberAfPluginCompactLoggerSetFacility(facility);
  emberAfCorePrintln("Set facility to 0x%2x", facility);
}
