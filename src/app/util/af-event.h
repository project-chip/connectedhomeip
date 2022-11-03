/*
 * SPDX-FileCopyrightText: (c) 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <app/util/af.h>

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

/** @brief Sets this ::EmberEventControl as inactive (no pending event).
 */
void emberEventControlSetInactive(EmberEventControl * control);

/** @brief Returns true is ::EmberEventControl is active.
 */
bool emberEventControlGetActive(EmberEventControl * control);

/** @brief Sets this ::EmberEventControl to run as soon as possible.
 */
void emberEventControlSetActive(EmberEventControl * control);
