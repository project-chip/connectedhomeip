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
/*****************************************************************************
 * @file
 * @brief Interface for the application event mechanism
 *******************************************************************************
 ******************************************************************************/

#pragma once

#include "af.h"

#define MAX_TIMER_UNITS_HOST 0x7fff
#define MAX_TIMER_MILLISECONDS_HOST (MAX_TIMER_UNITS_HOST * MILLISECOND_TICKS_PER_MINUTE)

/** @brief Complete events with a control and a handler procedure.
 *
 * An application typically creates an array of events
 * along with their handlers.
 * The main loop passes the array to ::emberRunEvents() to call
 * the handlers of any events whose time has arrived.
 */
typedef struct EmberEventData EmberEventData;

// A function used to retrieve the proper NCP timer duration and unit based on a given
// passed number of milliseconds.
void emAfGetTimerDurationAndUnitFromMS(uint32_t durationMs, uint16_t * duration, EmberEventUnits * units);

// A function (inverse of the above) to retrieve the number of milliseconds
// represented by a given timer duration and unit.
uint32_t emAfGetMSFromTimerDurationAndUnit(uint16_t duration, EmberEventUnits units);

const char * emberAfGetEventString(uint8_t index);

void emAfInitEvents(void);

/** @brief Sets this ::EmberEventControl as inactive (no pending event).
 */
void emberEventControlSetInactive(EmberEventControl * control);

/** @brief Returns true is ::EmberEventControl is active.
 */
bool emberEventControlGetActive(EmberEventControl * control);

/** @brief Sets this ::EmberEventControl to run as soon as possible.
 */
void emberEventControlSetActive(EmberEventControl * control);
