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
 * @brief Host specific code related to the event table.
 *******************************************************************************
   ******************************************************************************/

// clusters specific header
#include "app/framework/include/af.h"
#include "../../util/common.h"
#include "load-control-event-table.h"

#include "app/framework/plugin/esi-management/esi-management.h"

static void emAfCallEventAction(EmberAfLoadControlEvent *event,
                                uint8_t eventStatus,
                                uint8_t sequenceNumber,
                                bool replyToSingleEsi,
                                uint8_t esiIndex);

// This assumes that the Option Flag Enum uses only two bits
const uint8_t controlValueToStatusEnum[] = {
  EMBER_ZCL_AMI_EVENT_STATUS_EVENT_COMPLETED_NO_USER_PARTICIPATION_PREVIOUS_OPT_OUT, // ! EVENT_OPT_FLAG_PARTIAL
  // && ! EVENT_OPT_FLAG_OPT_IN
  EMBER_ZCL_AMI_EVENT_STATUS_EVENT_COMPLETED,                                        // ! EVENT_OPT_FLAG_PARTIAL
  // && EVENT_OPT_FLAG_OPT_IN
  EMBER_ZCL_AMI_EVENT_STATUS_EVENT_PARTIALLY_COMPLETED_WITH_USER_OPT_OUT,   // EVENT_OPT_FLAG_PARTIAL
  // && ! EVENT_OPT_FLAG_OPT_IN
  EMBER_ZCL_AMI_EVENT_STATUS_EVENT_PARTIALLY_COMPLETED_DUE_TO_USER_OPT_IN   // EVENT_OPT_FLAG_PARTIAL
  // && EVENT_OPT_FLAG_OPT_IN
};

typedef struct {
  EmberAfLoadControlEvent event;
  uint8_t entryStatus;
} LoadControlEventTableEntry;

static LoadControlEventTableEntry loadControlEventTable[EMBER_AF_DEMAND_RESPONSE_LOAD_CONTROL_CLUSTER_CLIENT_ENDPOINT_COUNT][EMBER_AF_PLUGIN_DRLC_EVENT_TABLE_SIZE];

static bool overlapFound(EmberAfLoadControlEvent *newEvent,
                         EmberAfLoadControlEvent *existingEvent)
{
  // CCB 1291: overlap exists if deviceClass or UEG overlaps (in addition to
  // event time check)
  bool deviceClassOverlaps = (newEvent->deviceClass
                              & existingEvent->deviceClass);
  bool uegOverlaps = ((0 == newEvent->utilityEnrollmentGroup)
                      || (0 == existingEvent->utilityEnrollmentGroup)
                      || (newEvent->utilityEnrollmentGroup
                          & existingEvent->utilityEnrollmentGroup));

  if (deviceClassOverlaps || uegOverlaps) {
    if (newEvent->startTime < (existingEvent->startTime + ((uint32_t)existingEvent->duration * 60))
        && existingEvent->startTime < (newEvent->startTime + ((uint32_t)newEvent->duration * 60))) {
      return true;
    }
  }

  return false;
}

static bool entryIsScheduledOrStarted(uint8_t entryStatus)
{
  if (entryStatus == ENTRY_SCHEDULED || entryStatus == ENTRY_STARTED) {
    return true;
  }

  return false;
}

static void initEventData(EmberAfLoadControlEvent *event)
{
  MEMSET(event, 0, sizeof(EmberAfLoadControlEvent));
}

/**
 * Clears the table of any entries which pertain to a
 * specific eventId.
 **/
static void voidAllEntriesWithEventId(uint8_t endpoint,
                                      uint32_t eventId)
{
  uint8_t i;
  uint8_t ep = emberAfFindClusterClientEndpointIndex(endpoint, ZCL_DEMAND_RESPONSE_LOAD_CONTROL_CLUSTER_ID);
  LoadControlEventTableEntry *e;

  if (ep == 0xFF) {
    return;
  }

  for (i = 0; i < EMBER_AF_PLUGIN_DRLC_EVENT_TABLE_SIZE; i++) {
    e = &loadControlEventTable[ep][i];
    if (e->event.eventId == eventId) {
      e->entryStatus = ENTRY_VOID;
    }
  }
}

/**
 * The callback function passed to the ESI management plugin. It handles
 * ESI entry deletions.
 */
static void esiDeletionCallback(uint8_t esiIndex)
{
  uint8_t i, j;
  for (i = 0; i < EMBER_AF_DEMAND_RESPONSE_LOAD_CONTROL_CLUSTER_CLIENT_ENDPOINT_COUNT; i++) {
    for (j = 0; j < EMBER_AF_PLUGIN_DRLC_EVENT_TABLE_SIZE; j++) {
      loadControlEventTable[i][j].event.esiBitmask &= ~BIT(esiIndex);
    }
  }
}

/**
 * The tick function simply checks entries in the table
 * and sends informational messages about event start
 * and event complete.
 */
void emAfLoadControlEventTableTick(uint8_t endpoint)
{
  uint32_t ct = emberAfGetCurrentTime();
  uint8_t ep = emberAfFindClusterClientEndpointIndex(endpoint, ZCL_DEMAND_RESPONSE_LOAD_CONTROL_CLUSTER_ID);
  uint8_t i;
  LoadControlEventTableEntry *e;
  bool eventCurrentlyRunning = false;

  if (ep == 0xFF) {
    return;
  }

  // First, go through any running events and end the ones that are complete
  // An event that is extended by a random delay (start or duration) must keep
  // other events from starting; we can't just check a scheduled event's
  // start time and start delay against the current time because a previously
  // running event may be delayed
  for ( i = 0; i < EMBER_AF_PLUGIN_DRLC_EVENT_TABLE_SIZE; i++ ) {
    e = &loadControlEventTable[ep][i];
    if (e->entryStatus == ENTRY_STARTED) {
      if ((e->event.startTime
           + e->event.startRand   // CCB 1513: startRand does affect end time
           + ((uint32_t)e->event.duration * 60)
           + e->event.durationRand) <= ct) {
        emAfCallEventAction(&(e->event),
                            controlValueToStatusEnum[e->event.optionControl],
                            emberAfNextSequence(),
                            false,
                            0);
        voidAllEntriesWithEventId(endpoint,
                                  e->event.eventId);
        return;
      } else {
        eventCurrentlyRunning = true;
      }
    }
  }

  // Now check for scheduled, superseded, and canceled events
  for ( i = 0; i < EMBER_AF_PLUGIN_DRLC_EVENT_TABLE_SIZE; i++ ) {
    e = &loadControlEventTable[ep][i];
    if (e->entryStatus == ENTRY_SCHEDULED) {
      if ((e->event.startTime + e->event.startRand <= ct)
          && (false == eventCurrentlyRunning)) { // 15-0132-03 test 10.38
        // Bug: 13546
        // When the event starts always send a Report Event status message.
        // If user opted-out, then send that status instead of event started.
        emAfCallEventAction(&(e->event),
                            ((e->event.optionControl & EVENT_OPT_FLAG_OPT_IN)
                             ? EMBER_ZCL_AMI_EVENT_STATUS_EVENT_STARTED
                             : EMBER_ZCL_AMI_EVENT_STATUS_USER_HAS_CHOOSE_TO_OPT_OUT),
                            emberAfNextSequence(),
                            false,
                            0);
        e->entryStatus = ENTRY_STARTED;
        // We may have waited for a previous event to finish due to a delay
        // In that case, we need to keep track of our current start time so that
        // we end on time and not sooner
        // We account for this by incrementing the startRand by the difference
        // in scheduled start versus actual start. Do not increment startTime
        // as the events' order are determined by that value
        e->event.startRand = ct - e->event.startTime;
        return;
      }
    } else if (e->entryStatus == ENTRY_IS_SUPERSEDED_EVENT) {
      if (e->event.startTime <= ct) {
        emAfCallEventAction(&(e->event),
                            EMBER_ZCL_AMI_EVENT_STATUS_THE_EVENT_HAS_BEEN_SUPERSEDED,
                            emberAfNextSequence(),
                            false,
                            0);
        voidAllEntriesWithEventId(endpoint,
                                  e->event.eventId);
        return;
      }
    } else if (e->entryStatus == ENTRY_IS_CANCELLED_EVENT) {
      if (e->event.startTime <= ct) {
        emAfCallEventAction(&(e->event),
                            EMBER_ZCL_AMI_EVENT_STATUS_THE_EVENT_HAS_BEEN_CANCELED,
                            emberAfNextSequence(),
                            false,
                            0);
        voidAllEntriesWithEventId(endpoint,
                                  e->event.eventId);
        return;
      }
    }
  }
}

/**
 * This function is used to schedule events in the
 * load control event table.
 */
void emAfScheduleLoadControlEvent(uint8_t endpoint,
                                  EmberAfLoadControlEvent *newEvent)
{
  uint8_t i;
  uint8_t ep = emberAfFindClusterClientEndpointIndex(endpoint, ZCL_DEMAND_RESPONSE_LOAD_CONTROL_CLUSTER_ID);
  LoadControlEventTableEntry *e;
  uint32_t ct = emberAfGetCurrentTime();
  EmberAfClusterCommand *curCommand = emberAfCurrentCommand();
  uint8_t esiIndex =
    emberAfPluginEsiManagementUpdateEsiAndGetIndex(curCommand);
  //emberAfPluginEsiManagementUpdateEsiAndGetIndex(emberAfCurrentCommand());

  if (ep == 0xFF) {
    return;
  }

  //validate starttime + duration
  if (newEvent->startTime == 0xffffffffUL
      || newEvent->duration > 0x5a0) {
    emAfCallEventAction(newEvent,
                        EMBER_ZCL_AMI_EVENT_STATUS_LOAD_CONTROL_EVENT_COMMAND_REJECTED,
                        emberAfCurrentCommand()->seqNum,
                        true,
                        esiIndex);
    return;
  }

  //validate expiration
  if (ct > (newEvent->startTime + ((uint32_t)newEvent->duration * 60))) {
    emAfCallEventAction(newEvent,
                        EMBER_ZCL_AMI_EVENT_STATUS_REJECTED_EVENT_EXPIRED,
                        emberAfCurrentCommand()->seqNum,
                        true,
                        esiIndex);
    return;
  }

  //validate event id
  for (i = 0; i < EMBER_AF_PLUGIN_DRLC_EVENT_TABLE_SIZE; i++) {
    e = &loadControlEventTable[ep][i];
    if (entryIsScheduledOrStarted(e->entryStatus)
        && (e->event.eventId == newEvent->eventId)) {
      // Bug 13805: from multi-ESI specs (5.7.3.5): When a device receives
      // duplicate events (same event ID) from multiple ESIs, it shall send an
      // event response to each ESI. Future duplicate events from the same
      // ESI(s) shall be either ignored by sending no response at all or with a
      // default response containing a success status code.
      //emberAfSendDefaultResponse(emberAfCurrentCommand(),
      //                           EMBER_ZCL_STATUS_DUPLICATE_EXISTS);

      // First time hearing this event from this ESI. If the ESI is present in
      // the table add the ESI to the event ESI bitmask and respond. If it is
      // a duplicate from the same ESI, we just ingore it.
      if ((e->event.esiBitmask & BIT(esiIndex)) == 0
          && esiIndex < EMBER_AF_PLUGIN_ESI_MANAGEMENT_ESI_TABLE_SIZE) {
        e->event.esiBitmask |= BIT(esiIndex);
        emAfCallEventAction(&(e->event),
                            EMBER_ZCL_AMI_EVENT_STATUS_LOAD_CONTROL_EVENT_COMMAND_RX,
                            emberAfCurrentCommand()->seqNum,
                            true,
                            esiIndex);
      }

      return;
    }
  }

  //locate empty table entry
  for (i = 0; i < EMBER_AF_PLUGIN_DRLC_EVENT_TABLE_SIZE; i++) {
    e = &loadControlEventTable[ep][i];
    if (e->entryStatus == ENTRY_VOID) {
      MEMMOVE(&(e->event), newEvent, sizeof(EmberAfLoadControlEvent));

      //check for supercession
      for (i = 0; i < EMBER_AF_PLUGIN_DRLC_EVENT_TABLE_SIZE; i++) {
        LoadControlEventTableEntry *currentEntry = &loadControlEventTable[ep][i];
        if (currentEntry->entryStatus == ENTRY_SCHEDULED
            || currentEntry->entryStatus == ENTRY_STARTED) {
          // If the event is superseded we need to let the application know
          // according to the following conditions interpreted from 075356r15
          // with help from NTS.
          //    1. If superseded event has not started, send superseded
          //       notification to application immediately.
          //    2. If superseded event HAS started, allow to run and send
          //       superseded message 1 second before new event starts.
          //       (to do this we subtract 1 from new event start time to know
          //        when to notify the application that the current running
          //        event has been superseded.)
          if (overlapFound(newEvent, &(currentEntry->event))) {
            if (currentEntry->entryStatus != ENTRY_STARTED) {
              currentEntry->event.startTime = ct;
            } else {
              currentEntry->event.startTime = (newEvent->startTime + newEvent->startRand - 1);
            }
            currentEntry->entryStatus = ENTRY_IS_SUPERSEDED_EVENT;
          }
        }
      }

      e->entryStatus = ENTRY_SCHEDULED;

      // If the ESI is in the table, we add it to the ESI bitmask of this event
      // and we respond.
      if (esiIndex < EMBER_AF_PLUGIN_ESI_MANAGEMENT_ESI_TABLE_SIZE) {
        e->event.esiBitmask = BIT(esiIndex);
        emAfCallEventAction(&(e->event),
                            EMBER_ZCL_AMI_EVENT_STATUS_LOAD_CONTROL_EVENT_COMMAND_RX,
                            emberAfCurrentCommand()->seqNum,
                            true,
                            esiIndex);
      }

      return;
    }
  }

  // If we get here we have failed to schedule the event because we probably
  // don't have any room in the table and must reject scheduling. We reject but
  // others have chosen to bump the earliest event. There is an ongoing
  // discussion on this issue will be discussed for possible CCB.
  emAfCallEventAction(newEvent,
                      EMBER_ZCL_AMI_EVENT_STATUS_LOAD_CONTROL_EVENT_COMMAND_REJECTED,
                      emberAfCurrentCommand()->seqNum,
                      true,
                      esiIndex);
}

void emAfLoadControlEventOptInOrOut(uint8_t endpoint,
                                    uint32_t eventId,
                                    bool optIn)
{
  uint8_t i;
  uint8_t ep = emberAfFindClusterClientEndpointIndex(endpoint, ZCL_DEMAND_RESPONSE_LOAD_CONTROL_CLUSTER_ID);
  LoadControlEventTableEntry *e;

  if (ep == 0xFF) {
    return;
  }

  for (i = 0; i < EMBER_AF_PLUGIN_DRLC_EVENT_TABLE_SIZE; i++) {
    e = &loadControlEventTable[ep][i];
    if (entryIsScheduledOrStarted(e->entryStatus)
        && e->event.eventId == eventId) {
      // used to find out if we have opted in our out of a running event
      bool previousEventOption = (e->event.optionControl & EVENT_OPT_FLAG_OPT_IN);

      // set the event opt in flag
      e->event.optionControl =
        (optIn
         ? (e->event.optionControl | EVENT_OPT_FLAG_OPT_IN)
         : (e->event.optionControl & ~EVENT_OPT_FLAG_OPT_IN));

      // if we have opted in or out of a running event we need to set the
      // partial flag.
      if ((previousEventOption != optIn)
          && e->entryStatus == ENTRY_STARTED) {
        e->event.optionControl |= EVENT_OPT_FLAG_PARTIAL;
      }

      // Bug: 13546
      // SE 1.0 and 1.1 dictate that if the event has not yet started,
      // and the user opts-out then don't send a status message.
      // Effectively the event is not changing so don't bother
      // notifying the ESI.  When the event would normally start,
      // the opt-out takes effect and that is when we send the opt-out
      // message.
      if (!(e->event.optionControl & ~EVENT_OPT_FLAG_OPT_IN
            && e->entryStatus == ENTRY_SCHEDULED)) {
        emAfCallEventAction(
          &(e->event),
          (optIn
           ? EMBER_ZCL_AMI_EVENT_STATUS_USER_HAS_CHOOSE_TO_OPT_IN
           : EMBER_ZCL_AMI_EVENT_STATUS_USER_HAS_CHOOSE_TO_OPT_OUT),
          emberAfNextSequence(),
          false,
          0);
      }
      return;
    }
  }
}

void emAfCancelLoadControlEvent(uint8_t endpoint,
                                uint32_t eventId,
                                uint8_t cancelControl,
                                uint32_t effectiveTime)
{
  uint8_t i;
  uint8_t ep = emberAfFindClusterClientEndpointIndex(endpoint, ZCL_DEMAND_RESPONSE_LOAD_CONTROL_CLUSTER_ID);
  LoadControlEventTableEntry *e;
  EmberAfLoadControlEvent undefEvent;
  uint32_t cancelTime = 0;
  uint8_t esiIndex =
    emberAfPluginEsiManagementUpdateEsiAndGetIndex(emberAfCurrentCommand());

  if (ep == 0xFF) {
    return;
  }

  for (i = 0; i < EMBER_AF_PLUGIN_DRLC_EVENT_TABLE_SIZE; i++) {
    e = &loadControlEventTable[ep][i];
    if (e->event.eventId == eventId
        && e->entryStatus != ENTRY_VOID) {
      // Found the event, validate effective time
      if ((effectiveTime == 0xffffffffUL)
          || (effectiveTime > (e->event.startTime
                               + (((uint32_t) e->event.duration) * 60)))) {
        emAfCallEventAction(&(e->event),
                            EMBER_ZCL_AMI_EVENT_STATUS_REJECTED_INVALID_CANCEL_COMMAND_INVALID_EFFECTIVE_TIME,
                            emberAfCurrentCommand()->seqNum,
                            true,
                            esiIndex);
        return;
      }

      // We're good, Run the cancel
      if (cancelControl & CANCEL_WITH_RANDOMIZATION) {
        if (effectiveTime == 0) {
          cancelTime = emberAfGetCurrentTime();
        }
        // CCB 1513: delay by durationRand if instructed, else by startRand
        cancelTime += (e->event.durationRand ? e->event.durationRand
                       : e->event.startRand);
      } else {
        cancelTime = effectiveTime;
      }
      e->entryStatus = ENTRY_IS_CANCELLED_EVENT; //will generate message on next tick
      e->event.startTime = cancelTime;
      return;
    }
  }

  // If we get here, we have failed to find the event
  // requested to cancel, send a fail message.
  initEventData(&undefEvent);
  undefEvent.destinationEndpoint =
    emberAfCurrentCommand()->apsFrame->destinationEndpoint;
  undefEvent.eventId = eventId;
  emAfCallEventAction(&undefEvent,
                      EMBER_ZCL_AMI_EVENT_STATUS_REJECTED_INVALID_CANCEL_UNDEFINED_EVENT,
                      emberAfCurrentCommand()->seqNum,
                      true,
                      esiIndex);
}

bool emAfCancelAllLoadControlEvents(uint8_t endpoint,
                                    uint8_t cancelControl)
{
  uint8_t i;
  uint8_t ep = emberAfFindClusterClientEndpointIndex(endpoint, ZCL_DEMAND_RESPONSE_LOAD_CONTROL_CLUSTER_ID);
  LoadControlEventTableEntry *e;
  bool generatedResponse = false;

  if (ep == 0xFF) {
    return false;
  }

  for (i = 0; i < EMBER_AF_PLUGIN_DRLC_EVENT_TABLE_SIZE; i++) {
    e = &loadControlEventTable[ep][i];
    if (e->entryStatus != ENTRY_VOID) {
      emAfCancelLoadControlEvent(endpoint, e->event.eventId, cancelControl, 0);
      generatedResponse = true;
    }
  }
  return generatedResponse;
}

static void emAfCallEventAction(EmberAfLoadControlEvent *event,
                                uint8_t eventStatus,
                                uint8_t sequenceNumber,
                                bool replyToSingleEsi,
                                uint8_t esiIndex)
{
  if (replyToSingleEsi) {
    EmberAfPluginEsiManagementEsiEntry* esiEntry =
      emberAfPluginEsiManagementEsiLookUpByIndex(esiIndex);
    if (esiEntry != NULL) {
      emberAfEventAction(event,
                         eventStatus,
                         sequenceNumber,
                         esiIndex); // send it to a specific ESI.
    }
    // Response intended for a single ESI. If it does not appear in the table,
    // nothing to do.
  } else {
    emberAfEventAction(event,
                       eventStatus,
                       sequenceNumber,
                       0xFF); // send it to all ESIs in the event bitmask.
  }
}

void emAfNoteSignatureFailure(void)
{
  emberAfDemandResponseLoadControlClusterPrintln("Failed to append signature to message.");
}

void emAfLoadControlEventTablePrint(uint8_t endpoint)
{
#if defined(EMBER_AF_PRINT_ENABLE) && defined(EMBER_AF_PRINT_DEMAND_RESPONSE_LOAD_CONTROL_CLUSTER)
  uint8_t ep = emberAfFindClusterClientEndpointIndex(endpoint, ZCL_DEMAND_RESPONSE_LOAD_CONTROL_CLUSTER_ID);
  LoadControlEventTableEntry *e;
  uint8_t i;

  if (ep == 0xFF) {
    return;
  }

  emberAfDemandResponseLoadControlClusterPrintln("ind  st id       sta      dur  ec oc");
  emberAfDemandResponseLoadControlClusterFlush();

  for (i = 0; i < EMBER_AF_PLUGIN_DRLC_EVENT_TABLE_SIZE; i++) {
    e = &loadControlEventTable[ep][i];
    emberAfDemandResponseLoadControlClusterPrintln("[%x] %x %4x %4x %2x %x %x",
                                                   i,
                                                   e->entryStatus,
                                                   e->event.eventId,
                                                   e->event.startTime,
                                                   e->event.duration,
                                                   e->event.eventControl,
                                                   e->event.optionControl);
    emberAfDemandResponseLoadControlClusterFlush();
  }
#endif //defined(EMBER_AF_PRINT_ENABLE) && defined(EMBER_AF_PRINT_DEMAND_RESPONSE_LOAD_CONTROL_CLUSTER)
}

void emAfLoadControlEventTableInit(uint8_t endpoint)
{
  // Subscribe receive deletion announcements.
  emberAfPluginEsiManagementSubscribeToDeletionAnnouncements(esiDeletionCallback);

  emAfLoadControlEventTableClear(endpoint);
}

void emAfLoadControlEventTableClear(uint8_t endpoint)
{
  uint8_t ep = emberAfFindClusterClientEndpointIndex(endpoint, ZCL_DEMAND_RESPONSE_LOAD_CONTROL_CLUSTER_ID);
  uint8_t i;

  if (ep == 0xFF) {
    return;
  }

  for (i = 0; i < EMBER_AF_PLUGIN_DRLC_EVENT_TABLE_SIZE; i++) {
    MEMSET(&loadControlEventTable[ep][i], 0, sizeof(LoadControlEventTableEntry));
  }
}
