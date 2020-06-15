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
 * @brief Routines for the Events Server plugin, which implements the server
 *        side of the Events cluster.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "app/framework/util/common.h"
#include "events-server.h"

#if (EMBER_AF_PLUGIN_EVENTS_SERVER_TAMPER_LOG_SIZE != 0)
#define EMBER_AF_PLUGIN_EVENTS_SERVER_TAMPER_LOG_ENABLE
#endif // EMBER_AF_PLUGIN_EVENTS_SERVER_TAMPER_LOG_SIZE != 0

#if (EMBER_AF_PLUGIN_EVENTS_SERVER_FAULT_LOG_SIZE != 0)
#define EMBER_AF_PLUGIN_EVENTS_SERVER_FAULT_LOG_ENABLE
#endif // EMBER_AF_PLUGIN_EVENTS_SERVER_FAULT_LOG_SIZE != 0

#if (EMBER_AF_PLUGIN_EVENTS_SERVER_GENERAL_LOG_SIZE != 0)
#define EMBER_AF_PLUGIN_EVENTS_SERVER_GENERAL_LOG_ENABLE
#endif // EMBER_AF_PLUGIN_EVENTS_SERVER_GENERAL_LOG_SIZE != 0

#if (EMBER_AF_PLUGIN_EVENTS_SERVER_SECURITY_LOG_SIZE != 0)
#define EMBER_AF_PLUGIN_EVENTS_SERVER_SECURITY_LOG_ENABLE
#endif // EMBER_AF_PLUGIN_EVENTS_SERVER_SECURITY_LOG_SIZE != 0

#if (EMBER_AF_PLUGIN_EVENTS_SERVER_NETWORK_LOG_SIZE != 0)
#define EMBER_AF_PLUGIN_EVENTS_SERVER_NETWORK_LOG_ENABLE
#endif // EMBER_AF_PLUGIN_EVENTS_SERVER_NETWORK_LOG_SIZE != 0

// If additional log types are defined in the future this is what will need to
// be modified:
//   update the ami.xml file appropriately
//   add the definition of the new log(s). i.e. static EmberAfEventLog fooLog...
//   add the new enum(s) to the allLogIds array
//   allocate the space for the new events. i.e. static EmberAfEvent fooEvents...
//   add the new log size enum(s) to the EMBER_AF_PLUGIN_EVENTS_SERVER_EVENT_LOG_SIZE
//   update the emberAfEventsClusterServerInitCallback and getEventLog functions

typedef struct {
  EmberAfEvent event;
  uint8_t flags;
} EmberAfEventMetadata;

typedef struct {
  uint8_t nextEntry;
  uint8_t maxEntries;
  EmberAfEventMetadata *entries;
} EmberAfEventLog;

#ifdef EMBER_AF_PLUGIN_EVENTS_SERVER_TAMPER_LOG_ENABLE
static EmberAfEventLog tamperLog[EMBER_AF_EVENTS_CLUSTER_SERVER_ENDPOINT_COUNT];
#endif
#ifdef EMBER_AF_PLUGIN_EVENTS_SERVER_FAULT_LOG_ENABLE
static EmberAfEventLog faultLog[EMBER_AF_EVENTS_CLUSTER_SERVER_ENDPOINT_COUNT];
#endif
#ifdef EMBER_AF_PLUGIN_EVENTS_SERVER_GENERAL_LOG_ENABLE
static EmberAfEventLog generalLog[EMBER_AF_EVENTS_CLUSTER_SERVER_ENDPOINT_COUNT];
#endif
#ifdef EMBER_AF_PLUGIN_EVENTS_SERVER_SECURITY_LOG_ENABLE
static EmberAfEventLog securityLog[EMBER_AF_EVENTS_CLUSTER_SERVER_ENDPOINT_COUNT];
#endif
#ifdef EMBER_AF_PLUGIN_EVENTS_SERVER_NETWORK_LOG_ENABLE
static EmberAfEventLog networkLog[EMBER_AF_EVENTS_CLUSTER_SERVER_ENDPOINT_COUNT];
#endif

static EmberAfEventLogId allLogIds[] = {
  EMBER_ZCL_EVENT_LOG_ID_TAMPER_LOG,
  EMBER_ZCL_EVENT_LOG_ID_FAULT_LOG,
  EMBER_ZCL_EVENT_LOG_ID_GENERAL_EVENT_LOG,
  EMBER_ZCL_EVENT_LOG_ID_SECURITY_EVENT_LOG,
  EMBER_ZCL_EVENT_LOG_ID_NETWORK_EVENT_LOG,
#if (defined(EMBER_AF_GBCS_COMPATIBLE) && defined(EMBER_AF_PLUGIN_GAS_PROXY_FUNCTION))
  EMBER_ZCL_EVENT_LOG_ID_GBCS_GENERAL_EVENT_LOG,
  EMBER_ZCL_EVENT_LOG_ID_GBCS_SECURITY_EVENT_LOG,
#endif
};
#define NUM_EVENT_LOGS (sizeof(allLogIds) / sizeof(allLogIds[0]))

// The following arrays are defined to allocate the space the various event logs.
// The only time they are referenced is in the init function where the address
// of each log is stored in the entries field of the event logs.
#ifdef EMBER_AF_PLUGIN_EVENTS_SERVER_TAMPER_LOG_ENABLE
static EmberAfEventMetadata tamperEvents[EMBER_AF_EVENTS_CLUSTER_SERVER_ENDPOINT_COUNT]
[EMBER_AF_PLUGIN_EVENTS_SERVER_TAMPER_LOG_SIZE];
#endif
#ifdef EMBER_AF_PLUGIN_EVENTS_SERVER_FAULT_LOG_ENABLE
static EmberAfEventMetadata faultEvents[EMBER_AF_EVENTS_CLUSTER_SERVER_ENDPOINT_COUNT]
[EMBER_AF_PLUGIN_EVENTS_SERVER_FAULT_LOG_SIZE];
#endif
#ifdef EMBER_AF_PLUGIN_EVENTS_SERVER_GENERAL_LOG_ENABLE
static EmberAfEventMetadata generalEvents[EMBER_AF_EVENTS_CLUSTER_SERVER_ENDPOINT_COUNT]
[EMBER_AF_PLUGIN_EVENTS_SERVER_GENERAL_LOG_SIZE];
#endif
#ifdef EMBER_AF_PLUGIN_EVENTS_SERVER_SECURITY_LOG_ENABLE
static EmberAfEventMetadata securityEvents[EMBER_AF_EVENTS_CLUSTER_SERVER_ENDPOINT_COUNT]
[EMBER_AF_PLUGIN_EVENTS_SERVER_SECURITY_LOG_SIZE];
#endif
#ifdef EMBER_AF_PLUGIN_EVENTS_SERVER_NETWORK_LOG_ENABLE
static EmberAfEventMetadata networkEvents[EMBER_AF_EVENTS_CLUSTER_SERVER_ENDPOINT_COUNT]
[EMBER_AF_PLUGIN_EVENTS_SERVER_NETWORK_LOG_SIZE];
#endif

#define EMBER_AF_PLUGIN_EVENTS_SERVER_EVENT_LOG_SIZE \
  (EMBER_AF_PLUGIN_EVENTS_SERVER_TAMPER_LOG_SIZE     \
   + EMBER_AF_PLUGIN_EVENTS_SERVER_FAULT_LOG_SIZE    \
   + EMBER_AF_PLUGIN_EVENTS_SERVER_GENERAL_LOG_SIZE  \
   + EMBER_AF_PLUGIN_EVENTS_SERVER_SECURITY_LOG_SIZE \
   + EMBER_AF_PLUGIN_EVENTS_SERVER_NETWORK_LOG_SIZE)
#if (EMBER_AF_PLUGIN_EVENTS_SERVER_EVENT_LOG_SIZE == 0)
#error The Events Server plugin requires at least one log be enabled (i.e. log size plugin option > 0).
#endif

#define ZCL_EVENTS_VALID                      BIT(1)
#define eventIsValid(emd)                     ((emd)->flags & ZCL_EVENTS_VALID)

#define ZCL_EVENTS_SEARCH_CRITERIA_ALL_EVENT_IDS        0x0000
#define ZCL_EVENTS_SEARCH_CRITERIA_PUBLISH_ALL_EVENTS   0x00

#define GET_EVENT_LOG_LOGID_MASK          0x0F
#define GET_EVENT_LOG_EVENT_CONTROL_MASK  0xF0
#define requestedLogId(ecli)              ((ecli) & GET_EVENT_LOG_LOGID_MASK)
#define retrieveFullInformation(ecli)     ((ecli) & EMBER_ZCL_EVENT_CONTROL_RETRIEVE_FULL_INFORMATION)

#define PUBLISH_EVENT_LOG_PAYLOAD_CONTROL_MASK   0x0F
#define PUBLISH_EVENT_LOG_NUMBER_OF_EVENTS_MASK  0xF0
#define eventCrossesFrameBoundary(lpc)           ((lpc) & EventCrossesFrameBoundary)
#define numberOfEvents(lpc)                      (((lpc) & PUBLISH_EVENT_LOG_NUMBER_OF_EVENTS_MASK) >> 4)

// Publish Event Log: ZCL Header
//   Frame Control (1 byte)
//   Sequence Number (1 byte)
//   Command ID (1 byte)
#define PUBLISH_EVENT_ZCL_HEADER_LENGTH               3

// Publish Event Log: Payload Header
//   Total Number of Matching Events (2 byte)
//   Command Index (1 byte)
//   Total Commands (1 byte)
//   Log Payload Control (1 byte)
#define PUBLISH_EVENT_LOG_PAYLOAD_HEADER_LENGTH       5

// Publish Event Log: Event Info
//   Log ID (1 byte)
//   Event ID (2 bytes)
//   Event Time (4 bytes)
//   Event Data Length (1 byte)
#define PUBLISH_EVENT_LOG_EVENT_INFO_LENGTH           8

// The PublishEventLogState and PublishEventLogPartner structures are used to
// manage the multiple PublishEventLog responses that may be required when a
// GetEventLog command is received.
typedef struct {
  uint8_t eventsToPublishIndex;
  uint8_t *remainingEventData;
  uint8_t remainingEventDataLen;
} PublishEventLogState;

typedef struct {
  bool isIntraPan;
  union {
    struct {
      EmberNodeId nodeId;
      uint8_t       clientEndpoint;
      uint8_t       serverEndpoint;
    } intra;
    struct {
      EmberEUI64 eui64;
      EmberPanId panId;
    } inter;
  } pan;
  uint8_t sequence;
  bool publishFullInformation;
  uint16_t maxPayloadLength;
  uint16_t totalMatchingEvents;
  uint16_t eventsToPublishCount;
  uint16_t eventsToPublish[EMBER_AF_PLUGIN_EVENTS_SERVER_EVENT_LOG_SIZE];
  uint8_t totalCommands;
  uint8_t commandIndex;
  PublishEventLogState state;
} PublishEventLogPartner;
static PublishEventLogPartner partner;

//------------------------------------------------------------------------------
// Forward Declarations

static EmberAfEventLog *getEventLog(uint8_t endpoint, EmberAfEventLogId logId);
static void clearEventLog(EmberAfEventLog *eventLog);
#if defined(EMBER_AF_PRINT_ENABLE) && defined(EMBER_AF_PRINT_EVENTS_CLUSTER)
static void printEventLog(uint8_t endpoint, EmberAfEventLogId logId);
#endif // defined(EMBER_AF_PRINT_ENABLE) && defined(EMBER_AF_PRINT_EVENTS_CLUSTER)

static uint16_t findMatchingEvents(uint8_t endpoint,
                                   uint8_t logId,
                                   uint16_t eventId,
                                   uint32_t startTime,
                                   uint32_t endTime,
                                   uint8_t maxEvents,
                                   uint16_t eventOffset,
                                   uint16_t *eventToPublishCount,
                                   uint16_t *eventsToPublish);
static uint8_t getPublishEventLogTotalCommands(uint8_t endpoint,
                                               uint16_t payloadSize,
                                               bool publishFullInformation,
                                               uint16_t eventToPublishCount,
                                               uint16_t *eventsToPublish);
static uint8_t fillPublishEventLogSubPayload(uint8_t endpoint,
                                             PublishEventLogState *state,
                                             bool publishFullInformation,
                                             uint8_t *subPayload,
                                             uint16_t subPayloadMaxSpace,
                                             uint16_t *subPayloadLen,
                                             uint16_t eventToPublishCount,
                                             uint16_t *eventsToPublish,
                                             uint8_t *logPayloadControl);
static uint8_t findNextEvent(EmberAfEventLog *eventLog,
                             uint8_t prevIndex,
                             uint16_t eventId,
                             uint32_t startTime,
                             uint32_t endTime);

static void mangleCommandForGBCSNonTOMCmd(uint8_t * ep,
                                          uint8_t * logId);
static void initEventLog();

//------------------------------------------------------------------------------
// Callbacks

// Initialize the various event logs.
static bool clusterInitialized = false;
void emberAfEventsClusterServerInitCallback(uint8_t endpoint)
{
  initEventLog();
}

static void initEventLog()
{
  if (!clusterInitialized) {
    uint8_t i;
    for (i = 0; i < EMBER_AF_EVENTS_CLUSTER_SERVER_ENDPOINT_COUNT; i++) {
#ifdef EMBER_AF_PLUGIN_EVENTS_SERVER_TAMPER_LOG_ENABLE
      tamperLog[i].maxEntries = EMBER_AF_PLUGIN_EVENTS_SERVER_TAMPER_LOG_SIZE;
      tamperLog[i].entries = &tamperEvents[i][0];
      clearEventLog(&tamperLog[i]);
#endif

#ifdef EMBER_AF_PLUGIN_EVENTS_SERVER_FAULT_LOG_ENABLE
      faultLog[i].maxEntries = EMBER_AF_PLUGIN_EVENTS_SERVER_FAULT_LOG_SIZE;
      faultLog[i].entries = &faultEvents[i][0];
      clearEventLog(&faultLog[i]);
#endif

#ifdef EMBER_AF_PLUGIN_EVENTS_SERVER_GENERAL_LOG_ENABLE
      generalLog[i].maxEntries = EMBER_AF_PLUGIN_EVENTS_SERVER_GENERAL_LOG_SIZE;
      generalLog[i].entries = &generalEvents[i][0];
      clearEventLog(&generalLog[i]);
#endif

#ifdef EMBER_AF_PLUGIN_EVENTS_SERVER_SECURITY_LOG_ENABLE
      securityLog[i].maxEntries = EMBER_AF_PLUGIN_EVENTS_SERVER_SECURITY_LOG_SIZE;
      securityLog[i].entries = &securityEvents[i][0];
      clearEventLog(&securityLog[i]);
#endif

#ifdef EMBER_AF_PLUGIN_EVENTS_SERVER_NETWORK_LOG_ENABLE
      networkLog[i].maxEntries = EMBER_AF_PLUGIN_EVENTS_SERVER_NETWORK_LOG_SIZE;
      networkLog[i].entries = &networkEvents[i][0];
      clearEventLog(&networkLog[i]);
#endif
    }
    partner.commandIndex = ZCL_EVENTS_INVALID_INDEX;
    clusterInitialized = true;
  }
}

// This function is scheduled upon receipt of a GetEventLog command. Every tick
// we'll send a PublishEventLog command back to the requestor until all responses
// are sent.
void emberAfEventsClusterServerTickCallback(uint8_t endpoint)
{
  // Allocate for the largest possible size, unfortunately
  uint8_t subPayload[EMBER_AF_MAXIMUM_SEND_PAYLOAD_LENGTH];
  uint16_t subPayloadLen = 0;
  uint8_t logPayloadControl;
  uint16_t maxSubPayloadLen = partner.maxPayloadLength - PUBLISH_EVENT_ZCL_HEADER_LENGTH - PUBLISH_EVENT_LOG_PAYLOAD_HEADER_LENGTH;
  uint8_t numberOfEvents = 0;

  numberOfEvents = fillPublishEventLogSubPayload(endpoint,
                                                 &partner.state,
                                                 partner.publishFullInformation,
                                                 subPayload,
                                                 maxSubPayloadLen,
                                                 &subPayloadLen,
                                                 partner.eventsToPublishCount,
                                                 partner.eventsToPublish,
                                                 &logPayloadControl);
  if (numberOfEvents > 0) {
    emberAfEventsClusterPrintln("TX PublishEventLog at command index 0x%x of 0x%x, number of events 0x%x",
                                partner.commandIndex,
                                partner.totalCommands,
                                numberOfEvents);
//IRP282 & Jira EMAPPFWKV2-1376: clear most significant nibble of logPayloadControl when the PublishEventLog
//is part of a GBZ message i.e. WAN only commands and when the command is meant for a device on the HAN, logPayloadControl
//field should be as defined per ZSE specs.
#if defined(EMBER_AF_PLUGIN_GAS_PROXY_FUNCTION)
    if (partner.pan.intra.clientEndpoint == EMBER_AF_PLUGIN_GAS_PROXY_FUNCTION_HIDDEN_CLIENT_SIDE_ENDPOINT) {
      CLEARBITS(logPayloadControl, PUBLISH_EVENT_LOG_NUMBER_OF_EVENTS_MASK);
    }
#endif

#ifdef EMBER_AF_GBCS_COMPATIBLE
    // GBCS explicitly lists some commands that need to be sent with "disable
    // default response" flag set. This is one of them.
    // We make it conditional on GBCS so it does not affect standard SE apps.
    emberAfSetDisableDefaultResponse(EMBER_AF_DISABLE_DEFAULT_RESPONSE_ONE_SHOT);
#endif

    emberAfFillCommandEventsClusterPublishEventLog(partner.totalMatchingEvents,
                                                   partner.commandIndex,
                                                   partner.totalCommands,
                                                   logPayloadControl,
                                                   subPayload,
                                                   subPayloadLen);
    // Rewrite the sequence number of the response so it matches the request.
    appResponseData[1] = partner.sequence;
    if (partner.isIntraPan) {
      emberAfSetCommandEndpoints(partner.pan.intra.serverEndpoint,
                                 partner.pan.intra.clientEndpoint);
      emberAfSendCommandUnicast(EMBER_OUTGOING_DIRECT, partner.pan.intra.nodeId);
    } else {
      emberAfSendCommandInterPan(partner.pan.inter.panId,
                                 partner.pan.inter.eui64,
                                 EMBER_NULL_NODE_ID,
                                 0, // multicast id - unused
                                 SE_PROFILE_ID);
    }
    partner.commandIndex++;
  }

  if (numberOfEvents != 0
      && partner.commandIndex < partner.totalCommands) {
    emberAfScheduleServerTick(endpoint,
                              ZCL_EVENTS_CLUSTER_ID,
                              MILLISECOND_TICKS_PER_QUARTERSECOND);
  } else {
    partner.commandIndex = ZCL_EVENTS_INVALID_INDEX;
  }
}

// The GetEventLog command allows a client to request events from a server's
// event logs. One or more PublishEventLog commands are returned on receipt
// of this command. A ZCL Default Response with status NOT_FOUND shall be
// returned if no events match the given search criteria.
bool emberAfEventsClusterGetEventLogCallback(uint8_t eventControlLogId,
                                             uint16_t eventId,
                                             uint32_t startTime,
                                             uint32_t endTime,
                                             uint8_t numberOfEvents,
                                             uint16_t eventOffset)
{
  EmberAfClusterCommand *cmd = emberAfCurrentCommand();
  uint8_t endpoint = emberAfCurrentEndpoint();

  emberAfEventsClusterPrintln("RX: GetEventLog 0x%x, 0x%2x, 0x%4x, 0x%4x, 0x%x, 0x%2x",
                              eventControlLogId,
                              eventId,
                              startTime,
                              endTime,
                              numberOfEvents,
                              eventOffset);

  mangleCommandForGBCSNonTOMCmd(&endpoint,
                                &eventControlLogId);

  // Only one GetEventLog can be processed at a time.
  if (partner.commandIndex != ZCL_EVENTS_INVALID_INDEX) {
    emberAfEventsClusterPrintln("%p%p%p",
                                "Error: ",
                                "Cannot get event log: ",
                                "only one GetEventLog command can be processed at a time");
    emberAfSendDefaultResponse(cmd, EMBER_ZCL_STATUS_FAILURE);
    return true;
  }

  partner.totalMatchingEvents = findMatchingEvents(endpoint,
                                                   requestedLogId(eventControlLogId),
                                                   eventId,
                                                   startTime,
                                                   endTime,
                                                   numberOfEvents,
                                                   eventOffset,
                                                   &partner.eventsToPublishCount,
                                                   partner.eventsToPublish);
  if (partner.totalMatchingEvents == 0 || partner.totalMatchingEvents <= eventOffset) {
    emberAfEventsClusterPrintln("No matching events to return!");
    emberAfSendDefaultResponse(cmd, EMBER_ZCL_STATUS_NOT_FOUND);
  } else {
    partner.isIntraPan = (cmd->interPanHeader == NULL);
    if (partner.isIntraPan) {
      partner.pan.intra.nodeId = cmd->source;
      partner.pan.intra.clientEndpoint = cmd->apsFrame->sourceEndpoint;
      partner.pan.intra.serverEndpoint = cmd->apsFrame->destinationEndpoint;
    } else {
      partner.pan.inter.panId = cmd->interPanHeader->panId;
      MEMCOPY(partner.pan.inter.eui64, cmd->interPanHeader->longAddress, EUI64_SIZE);
    }
    partner.sequence = cmd->seqNum;
    partner.maxPayloadLength = EMBER_AF_MAXIMUM_SEND_PAYLOAD_LENGTH;
    partner.publishFullInformation = retrieveFullInformation(eventControlLogId);
    partner.totalCommands = getPublishEventLogTotalCommands(endpoint,
                                                            partner.maxPayloadLength,
                                                            partner.publishFullInformation,
                                                            partner.eventsToPublishCount,
                                                            partner.eventsToPublish);
    partner.commandIndex = 0;
    partner.state.eventsToPublishIndex = 0;
    partner.state.remainingEventData = NULL;
    partner.state.remainingEventDataLen = 0;
    emberAfScheduleServerTick(endpoint,
                              ZCL_EVENTS_CLUSTER_ID,
                              MILLISECOND_TICKS_PER_QUARTERSECOND);
  }
  return true;
}

// The ClearEventLog command requests that an Events server device clear the
// specified event log(s). The Events server device SHOULD clear the requested
// events logs, however it is understood that market specific restrictions may
// be applied to prevent this.
//
// To determine whether or not we should clear the requested log we will
// callback to the application with the requested logId and let the application
// tell us which logs are OK to be cleared.
//
// Note: that when setting the clearedEventLogs bitmap included within the
// ClearEventLogResponse we take advantage of the fact that the logId value
// currently indicates the the bit number in the bitmap to be set.  If this ever
// changes we'll need to modify the way we set the clearedEventLogs.
bool emberAfEventsClusterClearEventLogRequestCallback(uint8_t logId)
{
  uint8_t i, clearedEventLogs = 0;
  uint8_t endpoint = emberAfCurrentEndpoint();

  emberAfEventsClusterPrintln("RX: ClearEventLogRequest 0x%X", logId);

  mangleCommandForGBCSNonTOMCmd(&endpoint,
                                &logId);

#if defined(EMBER_AF_PLUGIN_GAS_PROXY_FUNCTION)
  if ((endpoint == EMBER_AF_PLUGIN_GAS_PROXY_FUNCTION_ESI_ENDPOINT)
      && (logId == EMBER_ZCL_EVENT_LOG_ID_SECURITY_EVENT_LOG)) {
    emberAfEventsClusterPrintln("ERR: Modifying or deleting entries from the GPF Security Log is not allowed.");
    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_FAILURE);
    return true;
  }
#endif

  if (emberAfPluginEventsServerOkToClearLogCallback((EmberAfEventLogId)logId)) {
    if (!emberAfEventsServerClearEventLog(endpoint, (EmberAfEventLogId)logId)) {
      emberAfEventsClusterPrintln("%p%p%p",
                                  "Error: ",
                                  "Cannot clear event log: ",
                                  "invalid endpoint and/or logId");
      emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_FAILURE);
      return true;
    }
    clearedEventLogs |= BIT(logId);
  } else if (EMBER_ZCL_EVENT_LOG_ID_ALL_LOGS == logId) {
    for (i = 0; i < NUM_EVENT_LOGS; i++) {
      if (emberAfPluginEventsServerOkToClearLogCallback(allLogIds[i])) {
        if (!emberAfEventsServerClearEventLog(endpoint, allLogIds[i])) {
          emberAfEventsClusterPrintln("%p%p%p",
                                      "Error: ",
                                      "Cannot clear event log: ",
                                      "invalid endpoint");
          emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_FAILURE);
          return true;
        }
        clearedEventLogs |= BIT(allLogIds[i]);
      }
    }
  }

  emberAfFillCommandEventsClusterClearEventLogResponse(clearedEventLogs);
  emberAfSendResponse();
  return true;
}

//------------------------------------------------------------------------------
// Other Miscellaneous functions

// Clear the specified event log by marking all entries invalid. Returns false
// if endpoint or logId is invalid
bool emberAfEventsServerClearEventLog(uint8_t endpoint, EmberAfEventLogId logId)
{
  uint8_t i;
  EmberAfEventLog *eventLog;
  bool status = false;

  if (EMBER_ZCL_EVENT_LOG_ID_ALL_LOGS == logId) {
    for (i = 0; i < NUM_EVENT_LOGS; i++) {
      eventLog = getEventLog(endpoint, allLogIds[i]);
      // we know that logId is valid so if getEventLog returns NULL it must
      // mean that endpoint is invalid so we'll break out of the loop and
      // return false immediately.
      if (NULL == eventLog) {
        break;
      }
      clearEventLog(eventLog);
    }

    status = true;
    goto kickout;
  } else {
    eventLog = getEventLog(endpoint, logId);
    if (NULL != eventLog) {
      clearEventLog(eventLog);
      status = true;
      goto kickout;
    }
  }

  kickout:

#if (defined(EMBER_AF_GBCS_COMPATIBLE) && defined(EMBER_AF_PLUGIN_GAS_PROXY_FUNCTION))
  /* Per GBCS req via CHTS v1.46, Section 4.5.13. We should log an event in
   * the security log when a command to clear all event log entries in the GPF ESI / CHF Event Log
   * is executed.*/
  if (status
      && ((EMBER_AF_PLUGIN_GAS_PROXY_FUNCTION_MIRROR_ENDPOINT == endpoint)
          || (EMBER_AF_PLUGIN_GAS_PROXY_FUNCTION_ESI_ENDPOINT == endpoint))
      && ((EMBER_ZCL_EVENT_LOG_ID_GENERAL_EVENT_LOG == logId)
          || (EMBER_ZCL_EVENT_LOG_ID_GBCS_GENERAL_EVENT_LOG == logId))) {
    uint8_t index;
    EmberAfEvent event;

    event.eventId = GBCS_EVENT_ID_EVENT_LOG_CLEARED;
    event.eventTime = emberAfGetCurrentTime();
    event.eventData[0] = 0x00;

    index = emberAfEventsServerAddEvent(endpoint,
                                        EMBER_ZCL_EVENT_LOG_ID_SECURITY_EVENT_LOG,
                                        &event);
    if (index == ZCL_EVENTS_INVALID_INDEX) {
      emberAfEventsClusterPrintln("ERR: Failed to log event (Event Log Cleared) to Security Event Log on ep(0x%X)",
                                  endpoint);
    } else {
      emberAfEventsClusterPrintln("INFO: Clear Event Log event added to Security Event Log on ep(0x%X) at index(0x%x)",
                                  endpoint,
                                  index);
    }
  }
#endif

  return status;
}

// Print the specified event log.
void emberAfEventsServerPrintEventLog(uint8_t endpoint, EmberAfEventLogId logId)
{
#if defined(EMBER_AF_PRINT_ENABLE) && defined(EMBER_AF_PRINT_EVENTS_CLUSTER)
  uint8_t i;

  if (EMBER_ZCL_EVENT_LOG_ID_ALL_LOGS == logId) {
    for (i = 0; i < NUM_EVENT_LOGS; i++) {
      printEventLog(endpoint, allLogIds[i]);
    }
  } else {
    printEventLog(endpoint, logId);
  }
#endif // defined(EMBER_AF_PRINT_ENABLE) && defined(EMBER_AF_PRINT_EVENTS_CLUSTER)
}

// Print an event
void emberAfEventsServerPrintEvent(const EmberAfEvent *event)
{
#if defined(EMBER_AF_PRINT_ENABLE) && defined(EMBER_AF_PRINT_EVENTS_CLUSTER)
  emberAfEventsClusterPrintln("       eventId: 0x%2x", event->eventId);
  emberAfEventsClusterPrintln("     eventTime: 0x%4x", event->eventTime);
  emberAfEventsClusterPrintln("  eventDataLen: 0x%x", emberAfStringLength(event->eventData));
  emberAfEventsClusterPrint("     eventData: ");
  emberAfEventsClusterPrintString(event->eventData);
  emberAfEventsClusterPrintln("");
#endif // defined(EMBER_AF_PRINT_ENABLE) && defined(EMBER_AF_PRINT_EVENTS_CLUSTER)
}

// Retrieves the event at the index.  Returns false if logId or index is invalid.
bool emberAfEventsServerGetEvent(uint8_t endpoint,
                                 EmberAfEventLogId logId,
                                 uint8_t index,
                                 EmberAfEvent *event)
{
  EmberAfEventLog *eventLog = getEventLog(endpoint, logId);
  if (NULL != eventLog  && index < eventLog->maxEntries) {
    MEMCOPY(event, &eventLog->entries[index].event, sizeof(EmberAfEvent));
    return true;
  }
  return false;
}

// Sets the event at the index.  Returns false if logId or index is invalid.
bool emberAfEventsServerSetEvent(uint8_t endpoint,
                                 EmberAfEventLogId logId,
                                 uint8_t index,
                                 const EmberAfEvent *event)
{
  bool status = false;
  EmberAfEventLog *eventLog = getEventLog(endpoint, logId);
  if (NULL != eventLog  && index < eventLog->maxEntries) {
    if (NULL == event) {
      eventLog->entries[index].flags &= ~ZCL_EVENTS_VALID;
      status = true;
      goto kickout;
    }

    MEMCOPY(&eventLog->entries[index].event, event, sizeof(EmberAfEvent));
    eventLog->entries[index].flags |= ZCL_EVENTS_VALID;
    // ensure we don't overwrite this entry
    if (index > eventLog->nextEntry) {
      eventLog->nextEntry  = index + 1;
    }
    status = true;
    goto kickout;
  }
  kickout:

  if (status) {
    emberAfPluginEventsServerLogDataUpdatedCallback(emberAfCurrentCommand());
  }
  return status;
}

// Add an event to the specified event log.
uint8_t emberAfEventsServerAddEvent(uint8_t endpoint,
                                    EmberAfEventLogId logId,
                                    const EmberAfEvent *event)
{
  EmberAfEventLog *eventLog = getEventLog(endpoint, logId);
  uint8_t index;
  if (NULL != eventLog) {
    uint8_t nextEntry = (eventLog->nextEntry)++;
    if (eventLog->nextEntry >= eventLog->maxEntries) {
      eventLog->nextEntry = 0;
    }
    MEMCOPY(&eventLog->entries[nextEntry].event, event, sizeof(EmberAfEvent));
    eventLog->entries[nextEntry].flags |= ZCL_EVENTS_VALID;
    index = nextEntry;
    goto kickout;
  }

  index = ZCL_EVENTS_INVALID_INDEX;
  kickout:

  if (index == ZCL_EVENTS_INVALID_INDEX) {
    emberAfEventsClusterPrintln("Cannot add Event to log 0x%x", logId);
  } else {
    emberAfEventsClusterPrintln("Event added to log 0x%x at index 0x%x", logId, index);
    emberAfPluginEventsServerLogDataUpdatedCallback(emberAfCurrentCommand());
  }

  return index;
}

// Publish an event.
void emberAfEventsServerPublishEventMessage(EmberNodeId nodeId,
                                            uint8_t srcEndpoint,
                                            uint8_t dstEndpoint,
                                            EmberAfEventLogId logId,
                                            uint8_t index,
                                            uint8_t eventControl)
{
  EmberStatus status;
  EmberAfEvent event;

  if (!emberAfEventsServerGetEvent(srcEndpoint, logId, index, &event)) {
    emberAfEventsClusterPrintln("%p%p%p",
                                "Error: ",
                                "Cannot publish event: ",
                                "invalid logId and/or index");
    return;
  }

  emberAfFillCommandEventsClusterPublishEvent(logId,
                                              event.eventId,
                                              event.eventTime,
                                              eventControl,
                                              event.eventData);
  emberAfSetCommandEndpoints(srcEndpoint, dstEndpoint);
  emberAfGetCommandApsFrame()->options |= EMBER_APS_OPTION_SOURCE_EUI64;
  status = emberAfSendCommandUnicast(EMBER_OUTGOING_DIRECT, nodeId);
  if (status != EMBER_SUCCESS) {
    emberAfEventsClusterPrintln("%p%p0x%x",
                                "Error: ",
                                "Cannot publish event: ",
                                status);
  }
}

//------------------------------------------------------------------------------
// Internal Functions

// Given an endpoint and a logId return the pointer to the corresponding event log.
static EmberAfEventLog *getEventLog(uint8_t endpoint, EmberAfEventLogId logId)
{
  EmberAfEventLog *eventLog = NULL;
  uint8_t ep = emberAfFindClusterServerEndpointIndex(endpoint, ZCL_EVENTS_CLUSTER_ID);

  if (ep == 0xFF) {
    return NULL;
  }

  switch (logId) {
    case EMBER_ZCL_EVENT_LOG_ID_TAMPER_LOG:
#ifdef EMBER_AF_PLUGIN_EVENTS_SERVER_TAMPER_LOG_ENABLE
      eventLog = &tamperLog[ep];
#else
      emberAfEventsClusterPrintln("%p%p",
                                  "Error: ",
                                  "Tamper log disabled");
#endif
      break;

    case EMBER_ZCL_EVENT_LOG_ID_FAULT_LOG:
#ifdef EMBER_AF_PLUGIN_EVENTS_SERVER_FAULT_LOG_ENABLE
      eventLog = &faultLog[ep];
#else
      emberAfEventsClusterPrintln("%p%p",
                                  "Error: ",
                                  "Fault log disabled");
#endif
      break;

    case EMBER_ZCL_EVENT_LOG_ID_GENERAL_EVENT_LOG:
#if (defined(EMBER_AF_GBCS_COMPATIBLE) && defined(EMBER_AF_PLUGIN_GAS_PROXY_FUNCTION))
    case EMBER_ZCL_EVENT_LOG_ID_GBCS_GENERAL_EVENT_LOG:
#endif
#ifdef EMBER_AF_PLUGIN_EVENTS_SERVER_GENERAL_LOG_ENABLE
      eventLog = &generalLog[ep];
#else
      emberAfEventsClusterPrintln("%p%p",
                                  "Error: ",
                                  "General log disabled");
#endif
      break;

    case EMBER_ZCL_EVENT_LOG_ID_SECURITY_EVENT_LOG:
#if (defined(EMBER_AF_GBCS_COMPATIBLE) && defined(EMBER_AF_PLUGIN_GAS_PROXY_FUNCTION))
    case EMBER_ZCL_EVENT_LOG_ID_GBCS_SECURITY_EVENT_LOG:
#endif
#ifdef EMBER_AF_PLUGIN_EVENTS_SERVER_SECURITY_LOG_ENABLE
      eventLog = &securityLog[ep];
#else
      emberAfEventsClusterPrintln("%p%p",
                                  "Error: ",
                                  "Security log disabled");
#endif
      break;

    case EMBER_ZCL_EVENT_LOG_ID_NETWORK_EVENT_LOG:
#ifdef EMBER_AF_PLUGIN_EVENTS_SERVER_NETWORK_LOG_ENABLE
      eventLog = &networkLog[ep];
#else
      emberAfEventsClusterPrintln("%p%p",
                                  "Error: ",
                                  "Network log disabled");
#endif
      break;

    default:
      emberAfEventsClusterPrintln("%p%p0x%x",
                                  "Error: ",
                                  "Unknown log: ",
                                  logId);
      break;
  }

  return eventLog;
}

// Clear the given event log by marking all entries invalid
static void clearEventLog(EmberAfEventLog *eventLog)
{
  uint8_t i;

  for (i = 0; i < eventLog->maxEntries; i++) {
    eventLog->entries[i].flags &= ~ZCL_EVENTS_VALID;
  }
  eventLog->nextEntry = 0;
  emberAfPluginEventsServerLogDataUpdatedCallback(emberAfCurrentCommand());
}

#if defined(EMBER_AF_PRINT_ENABLE) && defined(EMBER_AF_PRINT_EVENTS_CLUSTER)
// Print all valid entries within the given event log
static void printEventLog(uint8_t endpoint, EmberAfEventLogId logId)
{
  const EmberAfEventLog *eventLog;
  uint8_t i, count = 0;

  eventLog = getEventLog(endpoint, logId);
  if (NULL == eventLog) {
    return;
  }

  // Count the number of valid entries in the given event log. We could use
  // the nextEntry value within the eventLog for this if we never call
  // emberAfEventsServerSetEvent will a NULL event pointer thus potentially creating
  // holes in the log
  for (i = 0; i < eventLog->maxEntries; i++) {
    if (eventIsValid(&eventLog->entries[i])) {
      count++;
    }
  }

  emberAfEventsClusterPrintln("Log: 0x%x Total 0x%x", logId, count);
  emberAfEventsClusterPrintln("");
  if (count > 0) {
    for (i = 0; i < eventLog->maxEntries; i++) {
      if (!eventIsValid(&eventLog->entries[i])) {
        continue;
      }
      emberAfEventsClusterPrintln("Index: 0x%x", i);
      emberAfEventsServerPrintEvent(&eventLog->entries[i].event);
    }
  }
}
#endif // defined(EMBER_AF_PRINT_ENABLE) && defined(EMBER_AF_PRINT_EVENTS_CLUSTER)

// find events needing to be published that match the given criteria
static uint16_t findMatchingEvents(uint8_t endpoint,
                                   uint8_t logId,
                                   uint16_t eventId,
                                   uint32_t startTime,
                                   uint32_t endTime,
                                   uint8_t maxEvents,
                                   uint16_t eventOffset,
                                   uint16_t *eventsToPublishCount,
                                   uint16_t *eventsToPublish)
{
  uint8_t i;
  EmberAfEventLog *eventLogs[NUM_EVENT_LOGS];
  uint8_t candidateEventIndex[NUM_EVENT_LOGS];
  bool eventsFound = false;
  EmberAfEvent *nextEvent, *eventCandidate;
  uint8_t nextEventLogIndex = 0;
  uint16_t totalMatchingEvents = 0, skippedEvents = 0;

  *eventsToPublishCount = 0;

  // Find all the potential first events. These are the most recent events
  // matching the given criteria in each of the event logs.
  for (i = 0; i < NUM_EVENT_LOGS; i++) {
    eventLogs[i] = NULL;
    candidateEventIndex[i] = ZCL_EVENTS_INVALID_INDEX;

    if (logId != EMBER_ZCL_EVENT_LOG_ID_ALL_LOGS
        && logId != allLogIds[i]) {
      continue;
    }

    eventLogs[i] = getEventLog(endpoint, allLogIds[i]);
    if (eventLogs[i] == NULL) {
      continue;
    }

    candidateEventIndex[i] = findNextEvent(eventLogs[i],
                                           ZCL_EVENTS_INVALID_INDEX,
                                           eventId,
                                           startTime,
                                           endTime);
    if (!eventsFound && candidateEventIndex[i] != ZCL_EVENTS_INVALID_INDEX) {
      eventsFound = true;
    }
  }

  // Now compare each potential next event to each other and choose
  // the appropriate next event.  Once chosen, process it, and find
  // the next potential event from the log from which the chosen event
  // is located. Continue this process until all events are processed.
  while (eventsFound) {
    nextEvent = NULL;
    for (i = 0; i < NUM_EVENT_LOGS; i++) {
      if (candidateEventIndex[i] == ZCL_EVENTS_INVALID_INDEX) {
        continue;
      }
      eventCandidate = &(eventLogs[i]->entries[candidateEventIndex[i]].event);
      if (nextEvent == NULL
          || eventCandidate->eventTime > nextEvent->eventTime) {
        nextEvent = eventCandidate;
        nextEventLogIndex = i;
      }
    }
    if (nextEvent != NULL) {
      // If we get here then we found the next most recent matching event.
      // We need to find the next event in the log from which this event is
      // located then bump the total number of matching events but then decide
      // if we should add the event to the given eventsToPublish array.  This
      // will be determined based on the number of events requested and the
      // offset given.
      uint8_t nextEventIndex = candidateEventIndex[nextEventLogIndex];
      candidateEventIndex[nextEventLogIndex] = findNextEvent(eventLogs[nextEventLogIndex],
                                                             candidateEventIndex[nextEventLogIndex],
                                                             eventId,
                                                             startTime,
                                                             endTime);
      totalMatchingEvents++;

      if (skippedEvents < eventOffset) {
        skippedEvents++;
        continue;
      }

      if (maxEvents == ZCL_EVENTS_SEARCH_CRITERIA_PUBLISH_ALL_EVENTS
          || *eventsToPublishCount < maxEvents) {
        *eventsToPublish++ = ((uint16_t)allLogIds[nextEventLogIndex]) << 8 | (uint16_t)nextEventIndex;
        (*eventsToPublishCount)++;
      }
    } else {
      eventsFound = false;
    }
  }

  return totalMatchingEvents;
}

// calculate how many Publish Event Log commands it will take to publish all
// the events in the given eventsToPublish array..
static uint8_t getPublishEventLogTotalCommands(uint8_t endpoint,
                                               uint16_t maxPayloadLength,
                                               bool publishFullInformation,
                                               uint16_t eventsToPublishCount,
                                               uint16_t *eventsToPublish)
{
  uint8_t totalCommands = 0;
  uint8_t numberOfEvents;
  PublishEventLogState state;
  uint16_t subPayloadLen = 0;
  uint16_t maxSubPayloadLen = maxPayloadLength - PUBLISH_EVENT_ZCL_HEADER_LENGTH - PUBLISH_EVENT_LOG_PAYLOAD_HEADER_LENGTH;

  state.eventsToPublishIndex = 0;
  state.remainingEventData = NULL;
  state.remainingEventDataLen = 0;
  numberOfEvents = fillPublishEventLogSubPayload(endpoint,
                                                 &state,
                                                 publishFullInformation,
                                                 NULL,
                                                 maxSubPayloadLen,
                                                 &subPayloadLen,
                                                 eventsToPublishCount,
                                                 eventsToPublish,
                                                 NULL); // logPayloadControl
  while (numberOfEvents > 0) {
    totalCommands++;
    numberOfEvents = fillPublishEventLogSubPayload(endpoint,
                                                   &state,
                                                   publishFullInformation,
                                                   NULL,
                                                   maxSubPayloadLen,
                                                   &subPayloadLen,
                                                   eventsToPublishCount,
                                                   eventsToPublish,
                                                   NULL); // logPayloadControl
  }
  return totalCommands;
}

// This function will loop through the events in the given eventsToPublish list
// packing as many events into the given Publish Event Log subPayload buffer
// as will fit. The format of the contents of the Publish Event Log subPayload
// is a repeated set of the following fields:
//
// Octets:         1    |      2    |      4     |    1..255   |
// Field Name:  Log ID  | Event ID  | Event Time | Event Data  |
//
// Because there are several logs and many events per log event traversal is
// maintained in the given PublishEventLogState structure.
//
// If the given subPayload pointer is NULL this means that the caller is not
// building the command to send at this time but rather just trying to
// calculate how many commands it will take to send all the events (see
// emberAfEventsClusterGetEventLogCallback).
//
// The return value is the number of events packed in the subPayload buffer.
// This function will also fill a reference to a passed byte with the
// log payload control. If the caller does not care about the logPayloadControl,
// they can set this value to NULL. This byte includes the number of events in
// the upper nibble and whether or not an event in the subPayload crosses over
// the frame boundary in the lower nibble. Remember, GBCS wants this upper
// nibble to be set to 0x0, since the number of events could total 100 which
// cannot fit in 4 bits.
static uint8_t fillPublishEventLogSubPayload(uint8_t endpoint,
                                             PublishEventLogState *state,
                                             bool publishFullInformation,
                                             uint8_t *subPayload,
                                             uint16_t subPayloadMaxSpace,
                                             uint16_t *subPayloadLen,
                                             uint16_t eventsToPublishCount,
                                             uint16_t *eventsToPublish,
                                             uint8_t *logPayloadControl)
{
  uint16_t subPayloadRemainingSpace = subPayloadMaxSpace;
  EmberAfEventLog *eventLog;
  EmberAfEvent *event;
  uint8_t eventDataLen;
  uint8_t *eventData;
  uint8_t numberOfEvents = 0;
  bool eventCrossesFrameBoundary = false;

  *subPayloadLen = 0;

  // Before finding the next event to publish first check to see if there is
  // remaining event data from a previously published event that needs to be
  // sent.
  if (state->remainingEventDataLen > 0) {
    numberOfEvents++;
    eventCrossesFrameBoundary = true;
    if (state->remainingEventDataLen > subPayloadRemainingSpace) {
      if (subPayload != NULL) {
        MEMCOPY(subPayload, state->remainingEventData, subPayloadRemainingSpace);
        *subPayloadLen += subPayloadRemainingSpace;
      }
      state->remainingEventData += subPayloadRemainingSpace;
      state->remainingEventDataLen -= subPayloadRemainingSpace;
      subPayloadRemainingSpace = 0;
    } else {
      if (subPayload != NULL) {
        MEMCOPY(subPayload, state->remainingEventData, state->remainingEventDataLen);
        *subPayloadLen += state->remainingEventDataLen;
      }
      state->remainingEventData = NULL;
      state->remainingEventDataLen = 0;
      subPayloadRemainingSpace -= state->remainingEventDataLen;
    }
  }

  // Loop through packing as many events as possible into the subPayload buffer.
  while (subPayloadRemainingSpace > PUBLISH_EVENT_LOG_EVENT_INFO_LENGTH
         && state->eventsToPublishIndex < eventsToPublishCount) {
    uint8_t logId = (eventsToPublish[state->eventsToPublishIndex] & 0xFF00) >> 8;
    uint8_t eventIndex = (eventsToPublish[state->eventsToPublishIndex] & 0x00FF);

    state->eventsToPublishIndex++;
    eventLog = getEventLog(endpoint, (EmberAfEventLogId)logId);
    if (eventLog == NULL) {
      continue;
    }

    event = &(eventLog->entries[eventIndex].event);
    numberOfEvents++;
    eventDataLen = (publishFullInformation) ? emberAfStringLength(event->eventData) : 0;
    if (subPayload != NULL) {
      emberAfCopyInt8u(subPayload, *subPayloadLen, logId);
      (*subPayloadLen)++;
      emberAfCopyInt16u(subPayload, *subPayloadLen, event->eventId);
      (*subPayloadLen) += 2;
      emberAfCopyInt32u(subPayload, *subPayloadLen, event->eventTime);
      (*subPayloadLen) += 4;
      emberAfCopyInt8u(subPayload, *subPayloadLen, eventDataLen);
      (*subPayloadLen)++;
    }
    subPayloadRemainingSpace -= PUBLISH_EVENT_LOG_EVENT_INFO_LENGTH;

    if (eventDataLen != 0) {
      eventData = &event->eventData[1];
      if (eventDataLen <= subPayloadRemainingSpace) {
        // The data associated with this event fits in the remaining space so
        // copy it in and move on to the next event.
        if (subPayload != NULL) {
          MEMCOPY(&subPayload[*subPayloadLen], eventData, eventDataLen);
          *subPayloadLen += eventDataLen;
        }
        subPayloadRemainingSpace -= eventDataLen;
      } else {
        // The data associated with this event won't fit in the remaining space.
        // We could split the event data so that part of the event is in this frame
        // and the remaining data is in the next but the spec says "Wherever
        // possible events SHOULD NOT be sent across payload boundaries". So the
        // only case where we should split the event is when the event is too big
        // for a single frame.
        if (eventDataLen > subPayloadMaxSpace) {
          // Event data is too big for a single frame so split it up.
          eventCrossesFrameBoundary = true;
          if (subPayload != NULL) {
            MEMCOPY(&subPayload[*subPayloadLen], eventData, subPayloadRemainingSpace);
            *subPayloadLen += subPayloadRemainingSpace;
          }
          state->remainingEventData = eventData + subPayloadRemainingSpace;;
          state->remainingEventDataLen = eventDataLen - subPayloadRemainingSpace;
          subPayloadRemainingSpace = 0;
        } else {
          // The event will fit into a single frame just not this one so
          // backout the event from this buffer by decrementing the subPayload
          // length and number of events. Also, decrement the eventsToPublishIndex in the
          // state struct as we want to start with this event in the next command.
          if (subPayload != NULL) {
            *subPayloadLen -= PUBLISH_EVENT_LOG_EVENT_INFO_LENGTH;
          }
          numberOfEvents--;
          state->eventsToPublishIndex--;
          break;
        }
      }
    }
  }

  if (logPayloadControl) {
    *logPayloadControl = (numberOfEvents << 4);
    if (eventCrossesFrameBoundary) {
      *logPayloadControl |= EMBER_ZCL_EVENT_LOG_PAYLOAD_CONTROL_EVENT_CROSSES_FRAME_BOUNDARY;
    }
  }

  return numberOfEvents;
}

// Find the next most recent event from the given log matching the given criteria.
// and starting from the given index.
static uint8_t findNextEvent(EmberAfEventLog *eventLog,
                             uint8_t prevIndex,
                             uint16_t eventId,
                             uint32_t startTime,
                             uint32_t endTime)
{
  uint8_t i, index = ZCL_EVENTS_INVALID_INDEX;
  EmberAfEventMetadata *eventMetadata;

  prevIndex = (prevIndex == ZCL_EVENTS_INVALID_INDEX) ? eventLog->maxEntries : prevIndex;

  // Need to send events in "most recent first" order.  We assume that events
  // are added as they occur so the first element is the oldest and the last
  // element is the newest.  As such we traverse the event log in reverse order.
  for (i = prevIndex - 1; i < eventLog->maxEntries; i--) {
    eventMetadata = &eventLog->entries[i];
    // emberAfEventsServerPrintEvent(&eventMetadata->event);
    if (!eventIsValid(eventMetadata)) {
      continue;
    }

    if (eventId != ZCL_EVENTS_SEARCH_CRITERIA_ALL_EVENT_IDS
        && eventId != eventMetadata->event.eventId) {
      continue;
    }

    if (eventMetadata->event.eventTime < startTime
        || eventMetadata->event.eventTime >= endTime) {
      continue;
    }

    index = i;
    break;
  }

  return index;
}

/*
 * @brief Redirecting GBCS Non-TOM messages
 *
 * Events cluster cmds are all redirected to the ESI endpoint for parsing.
 * Depending on the logId, we need to redirect them accordingly.
 *
 */
static void mangleCommandForGBCSNonTOMCmd(uint8_t * endpoint,
                                          uint8_t * logId)
{
#if (defined(EMBER_AF_GBCS_COMPATIBLE) && defined(EMBER_AF_PLUGIN_GAS_PROXY_FUNCTION))
  uint8_t newLogId = *logId & 0x0F;
  if (*endpoint == EMBER_AF_PLUGIN_GAS_PROXY_FUNCTION_ESI_ENDPOINT) {
    // When processing a ZSE Get Event Log command or a ZSE Clear Event Log
    // command with a Log ID nibble of 0x6 (GSME Event Log) or 0x7 (GSME
    // Security Log), a GPF shall process the command using the relevant GSME
    // Proxy Log copy of the GSME Event or Security Log. Other values of Log ID
    // shall refer to the GPF's own logs.
    if (newLogId == GBCS_EVENT_LOG_ID_GSME_EVENT_LOG
        || newLogId == GBCS_EVENT_LOG_ID_GSME_SECURITY_EVENT_LOG) {
      *endpoint = EMBER_AF_PLUGIN_GAS_PROXY_FUNCTION_MIRROR_ENDPOINT;
      emberAfEventsClusterPrintln("Redirecting to GPF meter mirror endpoint(0x%X) with logId(0x%X)",
                                  *endpoint, newLogId);
    } else {
      emberAfEventsClusterPrintln("Redirecting to GPF esi endpoint(0x%X) with logId(0x%X)",
                                  *endpoint, newLogId);
    }
  }

  *logId = (*logId & 0xF0) | (newLogId & 0x0F);
#endif
}
