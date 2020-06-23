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
 * @brief Tick APIs and defines for the Price Server plugin.
 *******************************************************************************
   ******************************************************************************/

#ifndef SILABS_PRICE_SERVER_TICK_H
#define SILABS_PRICE_SERVER_TICK_H

typedef enum {
  EMBER_AF_PRICE_SERVER_NO_PRICES_EVENT_MASK                 = (0 << 0),
  EMBER_AF_PRICE_SERVER_GET_SCHEDULED_PRICES_EVENT_MASK      = (1 << 0),
  EMBER_AF_PRICE_SERVER_CHANGE_BILLING_PERIOD_EVENT_MASK     = (1 << 1),
  EMBER_AF_PRICE_SERVER_CHANGE_BLOCK_PERIOD_EVENT_MASK       = (1 << 2),
  EMBER_AF_PRICE_SERVER_CHANGE_CALORIFIC_VALUE_EVENT_MASK    = (1 << 3),
  EMBER_AF_PRICE_SERVER_CHANGE_CO2_VALUE_EVENT_MASK          = (1 << 4),
  EMBER_AF_PRICE_SERVER_CHANGE_CONVERSION_FACTOR_EVENT_MASK  = (1 << 5),
  EMBER_AF_PRICE_SERVER_CHANGE_TARIFF_INFORMATION_EVENT_MASK = (1 << 6),
  EMBER_AF_PRICE_SERVER_ACTIVATED_PRICE_MATRIX_EVENT_MASK    = (1 << 7),
  EMBER_AF_PRICE_SERVER_ACTIVATED_BLOCK_THRESHOLD_EVENT_MASK = (1 << 8),
} EmberAfPriceServerPendingEvents;

#define PRICE_EVENT_TIME_NO_PENDING_EVENTS  0xFFFFFFFFU

/**
 * @brief Initializes data used by the price server tick.
 *
 **/
void emberAfPriceClusterServerInitTick(void);

/**
 * @brief Sets the event flag and schedules the price tick based on the next expiring pending event.
 * @param endpoint The endpoint of the device that supports the price server.
 * @event The event bit that should be set when scheduling the next tick.
 *
 **/
void emberAfPriceClusterScheduleTickCallback(uint8_t endpoint, EmberAfPriceServerPendingEvents event);

/**
 * @brief This function clears an event flag from the pending events bitmask.
 * @event The event bit that should be cleared.
 *
 **/
void emberAfPriceClusterClearPendingEvent(EmberAfPriceServerPendingEvents event);

#endif  // #ifndef _PRICE_SERVER_TICK_H_
