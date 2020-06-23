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
 * @brief Implemented routines for managing price tick.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "price-server-tick.h"
#include "price-server.h"

bool emAfPluginPriceServerBillingPeriodRepeat = true;

static EmberAfPriceServerPendingEvents PendingPriceEvents;

void emberAfPriceClusterServerInitTick()
{
  PendingPriceEvents = EMBER_AF_PRICE_SERVER_NO_PRICES_EVENT_MASK;
}

void emberAfPriceClusterClearPendingEvent(EmberAfPriceServerPendingEvents event)
{
  PendingPriceEvents &= ~(event);
}

void emberAfPriceClusterScheduleTickCallback(uint8_t endpoint, EmberAfPriceServerPendingEvents event)
{
  PendingPriceEvents |= event;

  // This call will execute any ready events and schedule the tick for the nearest future event.
  emberAfPriceClusterServerTickCallback(endpoint);
}

void emberAfPriceClusterServerTickCallback(uint8_t endpoint)
{
  uint32_t timeNowUtc;
  uint32_t minEventDelaySec = ZCL_PRICE_CLUSTER_END_TIME_NEVER;
  uint32_t eventDelaySec;
  uint16_t minTimeEventMask = 0;
  uint32_t delayMSec;

  uint8_t ep = emberAfFindClusterServerEndpointIndex(endpoint, ZCL_PRICE_CLUSTER_ID);
  emberAfPriceClusterPrintln("Price Tick Callback, ep=%d", ep);
  if ( ep == 0xFF ) {
    return;
  }

  timeNowUtc = emberAfGetCurrentTime();
  UNUSED_VAR(timeNowUtc);
  UNUSED_VAR(minTimeEventMask);

  // Look at all currently pending events and determine the min delay time for each.
  if ((PendingPriceEvents & EMBER_AF_PRICE_SERVER_GET_SCHEDULED_PRICES_EVENT_MASK) != 0U) {
    eventDelaySec = emberAfPriceServerSecondsUntilGetScheduledPricesEvent();
    if ( eventDelaySec == 0 ) {
      // Execute now
      emberAfPriceClusterPrintln("Price Tick:  Get Scheduled Prices");
      emberAfPriceServerSendGetScheduledPrices(endpoint);
      // Recalculate next delay time
      eventDelaySec = emberAfPriceServerSecondsUntilGetScheduledPricesEvent();
    }
    if ( eventDelaySec <= minEventDelaySec ) { // This should preempt any other events.
      minEventDelaySec = eventDelaySec;
      minTimeEventMask = EMBER_AF_PRICE_SERVER_GET_SCHEDULED_PRICES_EVENT_MASK;
    }
    if ( eventDelaySec == PRICE_EVENT_TIME_NO_PENDING_EVENTS ) {
      emberAfPriceClusterClearPendingEvent(EMBER_AF_PRICE_SERVER_GET_SCHEDULED_PRICES_EVENT_MASK);
    }
  }

  if ((PendingPriceEvents & EMBER_AF_PRICE_SERVER_CHANGE_BILLING_PERIOD_EVENT_MASK) != 0U) {
    eventDelaySec = emberAfPriceServerSecondsUntilBillingPeriodEvent(endpoint);
    if ( eventDelaySec == 0 ) {
      // Execute now
      emberAfPriceClusterPrintln("Price Tick:  Billing Period");
      emberAfPriceServerRefreshBillingPeriod(endpoint, false);   // force?
      // Recalculate next delay time
      eventDelaySec = emberAfPriceServerSecondsUntilBillingPeriodEvent(endpoint);
    }
    if (eventDelaySec == PRICE_EVENT_TIME_NO_PENDING_EVENTS) {
      // There is no next billing event, so we need to schedule one with the
      // same duration as the current event. See Billing Period Duration
      // description in D.4.2.4.10.2 of the SE spec.
      if (emAfPluginPriceServerBillingPeriodRepeat) {
        emberAfPriceServerRefreshBillingPeriod(endpoint, true); // force?
        // Recalculate next delay time
        eventDelaySec = emberAfPriceServerSecondsUntilBillingPeriodEvent(endpoint);
      } else {
        emberAfPriceClusterClearPendingEvent(EMBER_AF_PRICE_SERVER_CHANGE_BILLING_PERIOD_EVENT_MASK);
      }
    }
    if ( eventDelaySec < minEventDelaySec ) {
      minEventDelaySec = eventDelaySec;
      minTimeEventMask = EMBER_AF_PRICE_SERVER_CHANGE_BILLING_PERIOD_EVENT_MASK;
    }
  }

  if ((PendingPriceEvents & EMBER_AF_PRICE_SERVER_CHANGE_BLOCK_PERIOD_EVENT_MASK) != 0U) {
    eventDelaySec = emberAfPriceServerSecondsUntilBlockPeriodEvent(endpoint);
    if ( eventDelaySec == 0 ) {
      // Execute now
      emberAfPriceClusterPrintln("Price Tick:  Block Period");
      emberAfPriceServerRefreshBlockPeriod(endpoint, false);   // repeat?

      // Per SE1.2 Spec D4.4.3,
      // PublishPrice is required at the start of a Block Period
      emberAfPluginPriceServerPriceUpdateBindings();

      // Recalculate next delay time
      eventDelaySec = emberAfPriceServerSecondsUntilBlockPeriodEvent(endpoint);
    }
    if (eventDelaySec == PRICE_EVENT_TIME_NO_PENDING_EVENTS) {
      // If there are no future block periods scheduled, that means we may
      // need to schedule a future event for when the current one expires.
      // See D.4.2.4.2 of the SE spec.
      emberAfPriceServerRefreshBlockPeriod(endpoint, true); // repeat?
      // Recalculate next delay time
      eventDelaySec = emberAfPriceServerSecondsUntilBlockPeriodEvent(endpoint);
    }
    if ( eventDelaySec < minEventDelaySec ) {
      minEventDelaySec = eventDelaySec;
      minTimeEventMask = EMBER_AF_PRICE_SERVER_CHANGE_BLOCK_PERIOD_EVENT_MASK;
    }
    if ( eventDelaySec == PRICE_EVENT_TIME_NO_PENDING_EVENTS ) {
      emberAfPriceClusterClearPendingEvent(EMBER_AF_PRICE_SERVER_CHANGE_BLOCK_PERIOD_EVENT_MASK);
    }
  }

  if ((PendingPriceEvents & EMBER_AF_PRICE_SERVER_CHANGE_CALORIFIC_VALUE_EVENT_MASK) != 0U) {
    eventDelaySec = emberAfPriceServerSecondsUntilCalorificValueEvent(endpoint);
    if ( eventDelaySec == 0 ) {
      // Execute now
      emberAfPriceClusterPrintln("Price Tick:  Calorific Value");
      emberAfPriceServerRefreshCalorificValue(endpoint);
      // Recalculate next delay time
      eventDelaySec = emberAfPriceServerSecondsUntilCalorificValueEvent(endpoint);
    }
    if ( eventDelaySec < minEventDelaySec ) {
      minEventDelaySec = eventDelaySec;
      minTimeEventMask = EMBER_AF_PRICE_SERVER_CHANGE_CALORIFIC_VALUE_EVENT_MASK;
    }
    if ( eventDelaySec == PRICE_EVENT_TIME_NO_PENDING_EVENTS ) {
      emberAfPriceClusterClearPendingEvent(EMBER_AF_PRICE_SERVER_CHANGE_CALORIFIC_VALUE_EVENT_MASK);
    }
  }

  if ((PendingPriceEvents & EMBER_AF_PRICE_SERVER_CHANGE_CO2_VALUE_EVENT_MASK) != 0U) {
    eventDelaySec = emberAfPriceServerSecondsUntilCO2ValueEvent(endpoint);
    if ( eventDelaySec == 0 ) {
      // Execute now
      emberAfPriceClusterPrintln("Price Tick:  CO2 Value");
      emberAfPriceServerRefreshCO2Value(endpoint);
      // Recalculate next delay time
      eventDelaySec = emberAfPriceServerSecondsUntilCO2ValueEvent(endpoint);
    }
    if ( eventDelaySec < minEventDelaySec ) {
      minEventDelaySec = eventDelaySec;
      minTimeEventMask = EMBER_AF_PRICE_SERVER_CHANGE_CO2_VALUE_EVENT_MASK;
    }
    if ( eventDelaySec == PRICE_EVENT_TIME_NO_PENDING_EVENTS ) {
      emberAfPriceClusterClearPendingEvent(EMBER_AF_PRICE_SERVER_CHANGE_CO2_VALUE_EVENT_MASK);
    }
  }

  if ((PendingPriceEvents & EMBER_AF_PRICE_SERVER_CHANGE_CONVERSION_FACTOR_EVENT_MASK) != 0U) {
    eventDelaySec = emberAfPriceServerSecondsUntilConversionFactorEvent(endpoint);
    if ( eventDelaySec == 0 ) {
      // Execute now
      emberAfPriceClusterPrintln("Price Tick:  Conversion Factor");
      emberAfPriceServerRefreshConversionFactor(endpoint);
      // Recalculate next delay time
      eventDelaySec = emberAfPriceServerSecondsUntilConversionFactorEvent(endpoint);
    }
    if ( eventDelaySec < minEventDelaySec ) {
      minEventDelaySec = eventDelaySec;
      minTimeEventMask = EMBER_AF_PRICE_SERVER_CHANGE_CONVERSION_FACTOR_EVENT_MASK;
    }
    if ( eventDelaySec == PRICE_EVENT_TIME_NO_PENDING_EVENTS ) {
      emberAfPriceClusterClearPendingEvent(EMBER_AF_PRICE_SERVER_CHANGE_CONVERSION_FACTOR_EVENT_MASK);
    }
  }

  if ((PendingPriceEvents & EMBER_AF_PRICE_SERVER_CHANGE_TARIFF_INFORMATION_EVENT_MASK) != 0U) {
    eventDelaySec = emberAfPriceServerSecondsUntilTariffInfoEvent(endpoint);
    if ( eventDelaySec == 0 ) {
      // Execute now
      emberAfPriceClusterPrintln("Price Tick: Tariff Information");
      emberAfPriceServerRefreshTariffInformation(endpoint);
      // Recalculate next delay time
      eventDelaySec = emberAfPriceServerSecondsUntilTariffInfoEvent(endpoint);
    }
    if ( eventDelaySec < minEventDelaySec ) {
      minEventDelaySec = eventDelaySec;
      minTimeEventMask = EMBER_AF_PRICE_SERVER_CHANGE_TARIFF_INFORMATION_EVENT_MASK;
    }
    if ( eventDelaySec == PRICE_EVENT_TIME_NO_PENDING_EVENTS ) {
      emberAfPriceClusterClearPendingEvent(EMBER_AF_PRICE_SERVER_CHANGE_TARIFF_INFORMATION_EVENT_MASK);
    }
  }

  if ((PendingPriceEvents & EMBER_AF_PRICE_SERVER_ACTIVATED_PRICE_MATRIX_EVENT_MASK) != 0U) {
    eventDelaySec = emberAfPriceServerSecondsUntilActivePriceMatrixEvent(endpoint);
    if ( eventDelaySec == 0 ) {
      // Execute now
      emberAfPriceClusterPrintln("Price Tick: PriceMatrix Information activated");
      emberAfPriceServerRefreshPriceMatrixInformation(endpoint);
      // Recalculate next delay time
      eventDelaySec = emberAfPriceServerSecondsUntilActivePriceMatrixEvent(endpoint);
    }
    if ( eventDelaySec < minEventDelaySec ) {
      minEventDelaySec = eventDelaySec;
      minTimeEventMask = EMBER_AF_PRICE_SERVER_ACTIVATED_PRICE_MATRIX_EVENT_MASK;
    }
    if ( eventDelaySec == PRICE_EVENT_TIME_NO_PENDING_EVENTS ) {
      emberAfPriceClusterClearPendingEvent(EMBER_AF_PRICE_SERVER_ACTIVATED_PRICE_MATRIX_EVENT_MASK);
    }
  }

  if ((PendingPriceEvents & EMBER_AF_PRICE_SERVER_ACTIVATED_BLOCK_THRESHOLD_EVENT_MASK) != 0U) {
    eventDelaySec = emberAfPriceServerSecondsUntilActiveBlockThresholdsEvent(endpoint);
    if ( eventDelaySec == 0 ) {
      // Execute now
      emberAfPriceClusterPrintln("Price Tick: BlockThresholds activated");
      emberAfPriceServerRefreshBlockThresholdsInformation(endpoint);
      // Recalculate next delay time
      eventDelaySec = emberAfPriceServerSecondsUntilActiveBlockThresholdsEvent(endpoint);
    }
    if ( eventDelaySec < minEventDelaySec ) {
      minEventDelaySec = eventDelaySec;
      minTimeEventMask = EMBER_AF_PRICE_SERVER_ACTIVATED_BLOCK_THRESHOLD_EVENT_MASK;
    }
    if ( eventDelaySec == PRICE_EVENT_TIME_NO_PENDING_EVENTS ) {
      emberAfPriceClusterClearPendingEvent(EMBER_AF_PRICE_SERVER_ACTIVATED_BLOCK_THRESHOLD_EVENT_MASK);
    }
  }

  if ( minEventDelaySec == 0 ) {
    // Get Scheduled Prices wants to send each price with a 250ms delay between them (< 1 second).
    // To accommodate this behavior, any event that returns 0 for its delay time after execution
    // will be padded with a 250ms delay.
    delayMSec = MILLISECOND_TICKS_PER_QUARTERSECOND;
  } else if ( minEventDelaySec != 0xFFFFFFFFU ) {
    delayMSec = (minEventDelaySec * MILLISECOND_TICKS_PER_SECOND);
  } else {
    //delayMSec = minEventDelaySec;
    return;   // Nothing left to do.
  }

  if (PendingPriceEvents != 0x00) {
    emberAfPriceClusterPrintln("Scheduling Tick Callback in %d sec, eventBitFlag=%d, pendingEvents=%d", minEventDelaySec, minTimeEventMask, PendingPriceEvents);
  } else {
    emberAfPriceClusterPrintln("Scheduling Tick Callback in %d msec", delayMSec);
  }
  emberAfScheduleServerTick(endpoint, ZCL_PRICE_CLUSTER_ID, delayMSec);
}
