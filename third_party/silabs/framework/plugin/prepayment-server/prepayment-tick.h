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
 * @brief Event APIs and defines for the Prepayment Server plugin.
 *******************************************************************************
   ******************************************************************************/

#ifndef SILABS_PREPAYMENT_TICK_H
#define SILABS_PREPAYMENT_TICK_H

//
enum {
  PREPAYMENT_TICK_CHANGE_DEBT_EVENT         = (1 << 0),
  PREPAYMENT_TICK_EMERGENCY_CREDIT_EVENT    = (1 << 1),
  PREPAYMENT_TICK_CREDIT_ADJUSTMENT_EVENT   = (1 << 2),
  PREPAYMENT_TICK_CHANGE_PAYMENT_MODE_EVENT = (1 << 3),
  PREPAYMENT_TICK_MAX_CREDIT_LIMIT_EVENT    = (1 << 4),
  PREPAYMENT_TICK_DEBT_CAP_EVENT            = (1 << 5),
};

#define EVENT_TIME_NO_PENDING_EVENTS  0xFFFFFFFF

/**
 * @brief Sets the event flag and schedules the prepayment tick based on the next expiring pending event.
 * @param endpoint The endpoint of the device that supports the prepayment server.
 * @event The event bit that should be set when scheduling the next tick.
 *
 **/
void emberAfPrepaymentClusterScheduleTickCallback(uint8_t endpoint, uint16_t event);

/**
 * @brief This function clears an event flag from the pending events bitmask.
 * @event The event bit that should be cleared.
 *
 **/
void emberAfPrepaymentClusterClearPendingEvent(uint16_t event);

#endif  // #ifndef _PREPAYMENT_TICK_H_
