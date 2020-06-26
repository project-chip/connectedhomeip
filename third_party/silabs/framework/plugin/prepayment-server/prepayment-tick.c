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
 * @brief Implemented routines for managing prepayment tick.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "prepayment-tick.h"
#include "prepayment-debt-schedule.h"
#include "prepayment-modes-table.h"

static uint16_t PendingPrepaymentEvents;

void emberAfPrepaymentClusterClearPendingEvent(uint16_t event)
{
  PendingPrepaymentEvents &= ~(event);
}

void emberAfPrepaymentClusterScheduleTickCallback(uint8_t endpoint, uint16_t event)
{
  PendingPrepaymentEvents |= event;

  // This call will execute any ready events and schedule the tick for the nearest future event.
  emberAfPrepaymentClusterServerTickCallback(endpoint);
}

void emberAfPrepaymentClusterServerTickCallback(uint8_t endpoint)
{
  uint32_t timeNowUtc;
  uint32_t minEventDelaySec = 0xFFFFFFFF;
  uint32_t eventDelaySec;
  uint16_t minTimeEvent = 0;

  uint8_t ep = emberAfFindClusterServerEndpointIndex(endpoint, ZCL_PREPAYMENT_CLUSTER_ID);
  emberAfPrepaymentClusterPrintln("Prepayment Tick Callback, epx=%d", ep);
  if ( ep == 0xFF ) {
    return;
  }

  timeNowUtc = emberAfGetCurrentTime();

  // Look at all currently pending events and determine the min delay time for each.
  // If any are less than nextEventTimeSec, update nextEventTimeSec.

  if ( PendingPrepaymentEvents & PREPAYMENT_TICK_CHANGE_DEBT_EVENT ) {
    eventDelaySec = emberAfPrepaymentServerSecondsUntilDebtCollectionEvent(timeNowUtc);
    if ( eventDelaySec == 0 ) {
      // Execute now
      emberAfPrepaymentClusterPrintln("Prepayment Tick:  Handle Debt Collection");
      emberAfPrepaymentServerSetDebtMode(endpoint, timeNowUtc);
      // Recalculate next delay time
      eventDelaySec = emberAfPrepaymentServerSecondsUntilDebtCollectionEvent(timeNowUtc);
    }
    if ( eventDelaySec < minEventDelaySec ) {
      minEventDelaySec = eventDelaySec;
      minTimeEvent = PREPAYMENT_TICK_CHANGE_PAYMENT_MODE_EVENT;
    }
  }

  if ( PendingPrepaymentEvents & PREPAYMENT_TICK_CHANGE_PAYMENT_MODE_EVENT ) {
    eventDelaySec = emberAfPrepaymentServerSecondsUntilPaymentModeEvent(timeNowUtc);
    if ( eventDelaySec == 0 ) {
      // Execute now
      emberAfPrepaymentClusterPrintln("Prepayment Tick:  Handle Payment Mode");
      emberAfPrepaymentServerSetPaymentMode(endpoint);
      // Recalculate next delay time
      eventDelaySec = emberAfPrepaymentServerSecondsUntilPaymentModeEvent(timeNowUtc);
    }
    if ( eventDelaySec < minEventDelaySec ) {
      minEventDelaySec = eventDelaySec;
      minTimeEvent = PREPAYMENT_TICK_CHANGE_PAYMENT_MODE_EVENT;
    }
  }
  emberAfPrepaymentClusterPrintln("Scheduling Tick Callback in %d sec, eventBitFlag=%d, pendingEvents=%d", minEventDelaySec, minTimeEvent, PendingPrepaymentEvents);
  emberAfScheduleServerTick(endpoint, ZCL_PREPAYMENT_CLUSTER_ID, (minEventDelaySec * MILLISECOND_TICKS_PER_SECOND) );
}
