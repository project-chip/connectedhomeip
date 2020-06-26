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
 * @brief For more information on the usage of this plugin see drlc-server.h
 *******************************************************************************
   ******************************************************************************/

#include "../../include/af.h"
#include "../../util/common.h"
#include "drlc-server.h"

static EmberAfLoadControlEvent scheduledLoadControlEventTable[EMBER_AF_DEMAND_RESPONSE_LOAD_CONTROL_CLUSTER_SERVER_ENDPOINT_COUNT][EMBER_AF_PLUGIN_DRLC_SERVER_SCHEDULED_EVENT_TABLE_SIZE];
// The following matrix is a map of event order that is sorted by startTime and issuerEventId
// e.g. [ep][0] -> 2 (index of scheduledLoadControlEventTable)
//      [ep][1] -> 0
static uint8_t eventOrderToLoadControlIndex[EMBER_AF_DEMAND_RESPONSE_LOAD_CONTROL_CLUSTER_SERVER_ENDPOINT_COUNT][EMBER_AF_PLUGIN_DRLC_SERVER_SCHEDULED_EVENT_TABLE_SIZE];
#define ORDER_NOT_SET     0xFF

void emberAfDemandResponseLoadControlClusterServerInitCallback(uint8_t endpoint)
{
  emAfClearScheduledLoadControlEvents(endpoint); //clear all events at init
}

bool emberAfDemandResponseLoadControlClusterGetScheduledEventsCallback(uint32_t startTime,
                                                                       uint8_t  numberOfEvents,
                                                                       uint32_t issuerEventId)
{
  uint8_t i, sent = 0;
  uint8_t ep = emberAfFindClusterServerEndpointIndex(emberAfCurrentEndpoint(),
                                                     ZCL_DEMAND_RESPONSE_LOAD_CONTROL_CLUSTER_ID);
  EmberAfClusterCommand *currentCommand = emberAfCurrentCommand();
  uint8_t orderedIndex;
  EmberAfLoadControlEvent event;

  if (ep == 0xFF) {
    return false;
  }

  // CCB 1297
  // startTime = 0 does NOT mean get all events from "now"

  // Go through our sorted table (by following the eventOrderToLoadControlIndex
  // map) and send out the scheduled events
  for (i = 0; i < EMBER_AF_PLUGIN_DRLC_SERVER_SCHEDULED_EVENT_TABLE_SIZE; i++) {
    orderedIndex = eventOrderToLoadControlIndex[ep][i];
    // If no more events have been set (and thus, ordered), break
    if (ORDER_NOT_SET == orderedIndex) {
      break;
    }

    // CCB 1297: events with startTime = "now" are sent with startTime = 0, not
    // the current time that we internally set it to at the time of creation
    // We do, however, update the duration based on the elapsed time
    // emAfGetScheduledLoadControlEvent() normalizes the duration time
    if (EMBER_SUCCESS != emAfGetScheduledLoadControlEvent(
          emberAfCurrentEndpoint(),
          orderedIndex,
          &event)) {
      continue;
    }

    // check how many we have sent, if they have a positive number of events
    // they want returned and we have hit it we should exit.
    if (numberOfEvents != 0 && sent >= numberOfEvents) {
      break;
    }

    // If the event is inactive or its start time is before the requested start
    // time we ignore it.
    if (event.source[0] == 0xFF || event.startTime < startTime) {
      continue;
    }

    // CCB 1297: filter on issuerEventId if it was sent in command
    if ((issuerEventId != 0xFFFFFFFF)
        && (event.startTime == startTime)
        && (event.eventId < issuerEventId)) {
      continue;
    }

    // send the event
    emberAfFillCommandDemandResponseLoadControlClusterLoadControlEvent(event.eventId,
                                                                       event.deviceClass,
                                                                       event.utilityEnrollmentGroup,
                                                                       event.startTime,
                                                                       event.duration,
                                                                       event.criticalityLevel,
                                                                       event.coolingTempOffset,
                                                                       event.heatingTempOffset,
                                                                       event.coolingTempSetPoint,
                                                                       event.heatingTempSetPoint,
                                                                       event.avgLoadPercentage,
                                                                       event.dutyCycle,
                                                                       event.eventControl);
    emberAfSetCommandEndpoints(currentCommand->apsFrame->destinationEndpoint,
                               currentCommand->apsFrame->sourceEndpoint);
    emberAfGetCommandApsFrame()->options |= EMBER_APS_OPTION_SOURCE_EUI64;
    emberAfSendCommandUnicast(EMBER_OUTGOING_DIRECT, currentCommand->source);
    sent++;  //record that we sent it. and continue
  }

  // Bug 13547:
  //   Only send a Default response if there were no events returned.
  //   The LCE messages are the "next" messages in the sequence so no
  //   default response is needed for that successful case.
  if (sent == 0) {
    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_NOT_FOUND);
  }

  return true;
}

bool emberAfDemandResponseLoadControlClusterReportEventStatusCallback(uint32_t issuerEventId,
                                                                      uint8_t eventStatus,
                                                                      uint32_t eventStatusTime,
                                                                      uint8_t criticalityLevelApplied,
                                                                      uint16_t coolingTemperatureSetPointApplied,
                                                                      uint16_t heatingTemperatureSetPointApplied,
                                                                      int8_t averageLoadAdjustmentPercentageApplied,
                                                                      uint8_t dutyCycleApplied,
                                                                      uint8_t eventControl,
                                                                      uint8_t signatureType,
                                                                      uint8_t* signature)
{
  emberAfDemandResponseLoadControlClusterPrintln("= RX Event Status =");
  emberAfDemandResponseLoadControlClusterPrintln("  eid: %4x", issuerEventId);
  emberAfDemandResponseLoadControlClusterPrintln("   es: %x", eventStatus);
  emberAfDemandResponseLoadControlClusterPrintln("  est: T%4x", eventStatusTime);
  emberAfDemandResponseLoadControlClusterFlush();
  emberAfDemandResponseLoadControlClusterPrintln("  cla: %x", criticalityLevelApplied);
  emberAfDemandResponseLoadControlClusterFlush();
  emberAfDemandResponseLoadControlClusterPrintln("ctspa: %2x (%d)",
                                                 coolingTemperatureSetPointApplied,
                                                 coolingTemperatureSetPointApplied);
  emberAfDemandResponseLoadControlClusterPrintln("htspa: %2x (%d)",
                                                 heatingTemperatureSetPointApplied,
                                                 heatingTemperatureSetPointApplied);
  emberAfDemandResponseLoadControlClusterFlush();
  emberAfDemandResponseLoadControlClusterPrintln("  avg: %x",
                                                 averageLoadAdjustmentPercentageApplied);
  emberAfDemandResponseLoadControlClusterPrintln("   dc: %x", dutyCycleApplied);
  emberAfDemandResponseLoadControlClusterPrintln("   ec: %x", eventControl);
  emberAfDemandResponseLoadControlClusterFlush();

  emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
  return true;
}

void emAfClearScheduledLoadControlEvents(uint8_t endpoint)
{
  uint8_t i;
  uint8_t ep = emberAfFindClusterServerEndpointIndex(endpoint, ZCL_DEMAND_RESPONSE_LOAD_CONTROL_CLUSTER_ID);

  if (ep == 0xFF) {
    return;
  }

  for (i = 0; i < EMBER_AF_PLUGIN_DRLC_SERVER_SCHEDULED_EVENT_TABLE_SIZE; i++) {
    EmberAfLoadControlEvent *event = &scheduledLoadControlEventTable[ep][i];
    event->source[0] = 0xFF; // event inactive if first byte of source is 0xFF
  }

  clearEventOrderToLoadControlIndex(ep);
}

EmberStatus emAfGetScheduledLoadControlEvent(uint8_t endpoint, uint8_t index, EmberAfLoadControlEvent *event)
{
  uint8_t ep = emberAfFindClusterServerEndpointIndex(endpoint, ZCL_DEMAND_RESPONSE_LOAD_CONTROL_CLUSTER_ID);

  if (ep == 0xFF) {
    return EMBER_INVALID_ENDPOINT;
  }

  if (index < EMBER_AF_PLUGIN_DRLC_SERVER_SCHEDULED_EVENT_TABLE_SIZE) {
    MEMMOVE(event,
            &scheduledLoadControlEventTable[ep][index],
            sizeof(EmberAfLoadControlEvent));
    if (event->source[1] == 0x01 // start now?
        && (emberAfGetCurrentTime()
            < event->startTime + (uint32_t)event->duration * 60)) {
      event->duration = (event->duration
                         - (emberAfGetCurrentTime() - event->startTime) / 60);
      event->startTime = 0x0000;
    }
    return EMBER_SUCCESS;
  }
  emberAfDemandResponseLoadControlClusterPrintln("slce index out of range");
  return EMBER_INDEX_OUT_OF_RANGE;
}

EmberStatus emAfSetScheduledLoadControlEvent(uint8_t endpoint, uint8_t index, const EmberAfLoadControlEvent *event)
{
  uint8_t ep = emberAfFindClusterServerEndpointIndex(endpoint, ZCL_DEMAND_RESPONSE_LOAD_CONTROL_CLUSTER_ID);

  if (ep == 0xFF) {
    return EMBER_INVALID_ENDPOINT;
  }

  if (index < EMBER_AF_PLUGIN_DRLC_SERVER_SCHEDULED_EVENT_TABLE_SIZE) {
    bool newEvent = (scheduledLoadControlEventTable[ep][index].source[0] == 0xFF);
    MEMMOVE(&scheduledLoadControlEventTable[ep][index],
            event,
            sizeof(EmberAfLoadControlEvent));
    if (scheduledLoadControlEventTable[ep][index].startTime == 0x0000) {
      scheduledLoadControlEventTable[ep][index].startTime = emberAfGetCurrentTime();
      scheduledLoadControlEventTable[ep][index].source[1] = 0x01;
    } else {
      scheduledLoadControlEventTable[ep][index].source[1] = 0x00;
    }
    updateEventOrderToLoadControlIndex(ep, index, true, newEvent);
    return EMBER_SUCCESS;
  }
  emberAfDemandResponseLoadControlClusterPrintln("slce index out of range");
  return EMBER_INDEX_OUT_OF_RANGE;
}

void emAfPluginDrlcServerPrintInfo(uint8_t endpoint)
{
#if defined(EMBER_AF_PRINT_ENABLE) && defined(EMBER_AF_PRINT_DEMAND_RESPONSE_LOAD_CONTROL_CLUSTER)
  uint8_t i;
  uint8_t ep = emberAfFindClusterServerEndpointIndex(endpoint, ZCL_DEMAND_RESPONSE_LOAD_CONTROL_CLUSTER_ID);

  if (ep == 0xFF) {
    return;
  }

  for (i = 0; i < EMBER_AF_PLUGIN_DRLC_SERVER_SCHEDULED_EVENT_TABLE_SIZE; i++) {
    EmberAfLoadControlEvent *lce = &scheduledLoadControlEventTable[ep][i];
    emberAfDemandResponseLoadControlClusterFlush();
    emberAfDemandResponseLoadControlClusterPrintln("= LCE %d =", i);
    emberAfDemandResponseLoadControlClusterPrintln("eid: 0x%4x", lce->eventId);
    emberAfDemandResponseLoadControlClusterPrintln("src: 0x%x%x", lce->source[1], lce->source[0]);
    emberAfDemandResponseLoadControlClusterPrintln("sep: 0x%x", lce->sourceEndpoint);
    emberAfDemandResponseLoadControlClusterFlush();
    emberAfDemandResponseLoadControlClusterPrintln("dep: 0x%x", lce->destinationEndpoint);
    emberAfDemandResponseLoadControlClusterPrintln("dev: 0x%2x", lce->deviceClass);
    emberAfDemandResponseLoadControlClusterPrintln("ueg: 0x%x", lce->utilityEnrollmentGroup);
    emberAfDemandResponseLoadControlClusterPrintln(" st: 0x%4x", lce->startTime);
    emberAfDemandResponseLoadControlClusterFlush();
    emberAfDemandResponseLoadControlClusterPrintln("dur: 0x%2x", lce->duration);
    emberAfDemandResponseLoadControlClusterPrintln(" cl: 0x%x", lce->criticalityLevel);
    emberAfDemandResponseLoadControlClusterPrintln("cto: 0x%x", lce->coolingTempOffset);
    emberAfDemandResponseLoadControlClusterPrintln("hto: 0x%x", lce->heatingTempOffset);
    emberAfDemandResponseLoadControlClusterFlush();
    emberAfDemandResponseLoadControlClusterPrintln("cts: 0x%2x", lce->coolingTempSetPoint);
    emberAfDemandResponseLoadControlClusterPrintln("hts: 0x%2x", lce->heatingTempSetPoint);
    emberAfDemandResponseLoadControlClusterPrintln("alp: 0x%x", lce->avgLoadPercentage);
    emberAfDemandResponseLoadControlClusterPrintln(" dc: 0x%x", lce->dutyCycle);
    emberAfDemandResponseLoadControlClusterFlush();
    emberAfDemandResponseLoadControlClusterPrintln(" ev: 0x%x", lce->eventControl);
    emberAfDemandResponseLoadControlClusterPrintln(" sr: 0x%2x", lce->startRand);
    emberAfDemandResponseLoadControlClusterPrintln(" dr: 0x%2x", lce->durationRand);
    emberAfDemandResponseLoadControlClusterPrintln(" oc: 0x%x", lce->optionControl);
    emberAfDemandResponseLoadControlClusterFlush();
  }
  emberAfDemandResponseLoadControlClusterPrintln("Table size: %d",
                                                 EMBER_AF_PLUGIN_DRLC_SERVER_SCHEDULED_EVENT_TABLE_SIZE);
#endif //defined(EMBER_AF_PRINT_ENABLE) && defined(EMBER_AF_PRINT_DEMAND_RESPONSE_LOAD_CONTROL_CLUSTER)
}

void emAfPluginDrlcServerSlceMessage(EmberNodeId nodeId,
                                     uint8_t srcEndpoint,
                                     uint8_t dstEndpoint,
                                     uint8_t index)
{
  EmberAfLoadControlEvent event;
  EmberStatus status;
  status = emAfGetScheduledLoadControlEvent(srcEndpoint, index, &event);

  if (status != EMBER_SUCCESS) {
    emberAfDemandResponseLoadControlClusterPrintln("send slce fail: 0x%x", status);
    return;
  }

  emberAfFillCommandDemandResponseLoadControlClusterLoadControlEvent(event.eventId,
                                                                     event.deviceClass,
                                                                     event.utilityEnrollmentGroup,
                                                                     event.startTime,
                                                                     event.duration,
                                                                     event.criticalityLevel,
                                                                     event.coolingTempOffset,
                                                                     event.heatingTempOffset,
                                                                     event.coolingTempSetPoint,
                                                                     event.heatingTempSetPoint,
                                                                     event.avgLoadPercentage,
                                                                     event.dutyCycle,
                                                                     event.eventControl);

  emberAfSetCommandEndpoints(srcEndpoint, dstEndpoint);
  emberAfSendCommandUnicast(EMBER_OUTGOING_DIRECT, nodeId);
}

// This internal function checks where the load event control should be placed
// in a startTime- and issuerEventId-sorted array
// This function does not check if the load control event is already in the
// ordered array, so calling it with an existing entry will return a new index
// position, for which the caller must handle appropriately
// E.g. a LCE has start time 5 and issuer event ID 2 and lies at index 4 in the
// eventOrderToLoadControlIndex array. The user changes the start time to 9 and
// issuer event ID to 1, calls this function, and receives index 10. It is up to
// the caller to move the values at indices 5-10 to indices 4-9 and then set
// eventOrderToLoadControlIndex[10] to the LCE's index in the
// scheduledLoadControlEventTable array
uint8_t findPlacementInEventOrderToLoadControlIndex(uint8_t ep, uint8_t index)
{
  uint8_t i;
  uint8_t currentIndex;

  for (i = 0; (i < EMBER_AF_PLUGIN_DRLC_SERVER_SCHEDULED_EVENT_TABLE_SIZE) && (ORDER_NOT_SET != eventOrderToLoadControlIndex[ep][i]); i++) {
    currentIndex = eventOrderToLoadControlIndex[ep][i];
    if ((scheduledLoadControlEventTable[ep][index].startTime < scheduledLoadControlEventTable[ep][currentIndex].startTime)
        || ((scheduledLoadControlEventTable[ep][index].startTime == scheduledLoadControlEventTable[ep][currentIndex].startTime)
            && (scheduledLoadControlEventTable[ep][index].eventId < scheduledLoadControlEventTable[ep][currentIndex].eventId))) {
      break;
    }
  }

  return i;
}

void updateEventOrderToLoadControlIndex(uint8_t ep,
                                        uint8_t index,
                                        bool eventAdded,
                                        bool isNewEvent)
{
  uint8_t i;
  uint8_t currentOrderIndex;
  uint8_t newOrderIndex;

  if (eventAdded) {
    // A brand new load control event was added to the table
    if (isNewEvent) {
      // First find the ordered placement of this new event
      newOrderIndex = findPlacementInEventOrderToLoadControlIndex(ep, index);
      if (EMBER_AF_PLUGIN_DRLC_SERVER_SCHEDULED_EVENT_TABLE_SIZE == newOrderIndex) {
        return;
      }

      // Now push everyone forward one spot from the found index
      for (i = EMBER_AF_PLUGIN_DRLC_SERVER_SCHEDULED_EVENT_TABLE_SIZE - 1; i > newOrderIndex; i--) {
        eventOrderToLoadControlIndex[ep][i] = eventOrderToLoadControlIndex[ep][i - 1];
      }

      // Finally, set the mapping for the newly added event
      eventOrderToLoadControlIndex[ep][newOrderIndex] = index;
    }
    // An existing load control event was modified
    else {
      // First record the current ordered index and calculate the new index
      for (currentOrderIndex = 0; currentOrderIndex < EMBER_AF_PLUGIN_DRLC_SERVER_SCHEDULED_EVENT_TABLE_SIZE; currentOrderIndex++) {
        if (eventOrderToLoadControlIndex[ep][currentOrderIndex] == index) {
          break;
        }
      }

      if (EMBER_AF_PLUGIN_DRLC_SERVER_SCHEDULED_EVENT_TABLE_SIZE == currentOrderIndex) {
        return;
      }

      newOrderIndex = findPlacementInEventOrderToLoadControlIndex(ep, index);
      // EMBER_AF_PLUGIN_DRLC_SERVER_SCHEDULED_EVENT_TABLE_SIZE is a perfectly
      // valid return. It means the new event must go last in the ordered array

      // If the event must go last and it already is last, done
      if ((EMBER_AF_PLUGIN_DRLC_SERVER_SCHEDULED_EVENT_TABLE_SIZE == newOrderIndex)
          && (EMBER_AF_PLUGIN_DRLC_SERVER_SCHEDULED_EVENT_TABLE_SIZE == currentOrderIndex + 1)) {
        return;
      }
      // else, if the new index is not the final index and it's calculated to be
      // be the next index, then this effectively is the same position
      // findPlacementInEventOrderToLoadControlIndex will never return the same
      // index, since the comparative operators won't reach that result
      else if (newOrderIndex == currentOrderIndex + 1) {
        return;
      }

      if (EMBER_AF_PLUGIN_DRLC_SERVER_SCHEDULED_EVENT_TABLE_SIZE == newOrderIndex) {
        newOrderIndex--;
      }

      // else, move the indices' values and set the new index
      if (currentOrderIndex < newOrderIndex) {
        // e.g. from 5 to 8, move 6-8 down one
        for (i = currentOrderIndex + 1; i <= newOrderIndex; i++) {
          eventOrderToLoadControlIndex[ep][i - 1] = eventOrderToLoadControlIndex[ep][i];
        }
      } else {
        // e.g. from 8 to 5, move 5-7 up one
        for (i = currentOrderIndex; i > newOrderIndex; i--) {
          eventOrderToLoadControlIndex[ep][i] = eventOrderToLoadControlIndex[ep][i - 1];
        }
      }

      eventOrderToLoadControlIndex[ep][newOrderIndex] = index;
    }
  } else {
    // An existing load control event cannot currently be removed
  }
}

void clearEventOrderToLoadControlIndex(uint8_t ep)
{
  uint8_t i;
  for (i = 0; i < EMBER_AF_PLUGIN_DRLC_SERVER_SCHEDULED_EVENT_TABLE_SIZE; i++) {
    eventOrderToLoadControlIndex[ep][i] = 0xFF;
  }
}
