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
 * @brief APIs and defines for the DRLC plugin.
 *******************************************************************************
   ******************************************************************************/

#include "load-control-event-table.h"

// needed for EmberSignatureData
#include "stack/include/ember-types.h"

#define EVENT_OPT_IN_DEFAULT  0x01

void afReportEventStatusHandler(uint32_t eventId,
                                uint8_t eventCode,
                                uint32_t startTime,
                                uint8_t criticalityLevelApplied,
                                int16_t coolingTempSetPointApplied,
                                int16_t heatingTempSetPointApplied,
                                int8_t avgLoadAdjPercent,
                                uint8_t dutyCycle,
                                uint8_t eventControl,
                                uint8_t messageLength,
                                uint8_t* message,
                                EmberSignatureData* signature);
