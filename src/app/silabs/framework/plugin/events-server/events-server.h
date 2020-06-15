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
 * @brief APIs and defines for the Events Server plugin, which implements the
 *        server side of the Events cluster.
 *******************************************************************************
   ******************************************************************************/

typedef struct {
  uint16_t eventId;
  uint32_t eventTime;
  uint8_t eventData[EMBER_AF_PLUGIN_EVENTS_SERVER_EVENT_DATA_LENGTH + 1];
} EmberAfEvent;

#define ZCL_EVENTS_INVALID_INDEX 0xFF

#if defined(EMBER_AF_PLUGIN_GAS_PROXY_FUNCTION)
// GBCS Alert Codes
#define GBCS_ALERT_BILLING_DATA_LOG_UPDATED       (0x800A)
#define GBCS_EVENT_ID_UNAUTHD_COMM_ACC_ATT        (0x803E)
#define GBCS_EVENT_ID_EVENT_LOG_CLEARED           (0x8052)
#define GBCS_EVENT_ID_FAILED_AUTH                 (0x8053)
#define GBCS_EVENT_ID_IMM_HAN_CMD_RXED_ACTED      (0x8054)
#define GBCS_EVENT_ID_IMM_HAN_CMD_RXED_NOT_ACTED  (0x8055)
#define GBCS_EVENT_ID_FUT_HAN_CMD_ACTED           (0x8066)
#define GBCS_EVENT_ID_FUT_HAN_CMD_NOT_ACTED       (0x8067)
#define GBCS_EVENT_ID_GPF_DEVICE_LOG_CHGD         (0x8071)
#define GBCS_EVENT_ID_GSME_CMD_NOT_RETRVD         (0x809D)
#define GBCS_EVENT_LOG_ID_GSME_EVENT_LOG          (0x6)
#define GBCS_EVENT_LOG_ID_GSME_SECURITY_EVENT_LOG (0x7)
#endif

/**
 * @brief Clears all events in the specified event log.
 *
 * @param endpoint The endpoint for which the event log will be cleared.
 * @param logId The log to be cleared.
 * @return True if the log was successfully cleared or false if logId is invalid.
 **/
bool emberAfEventsServerClearEventLog(uint8_t endpoint,
                                      EmberAfEventLogId logId);

/**
 * @brief Prints all events in the specified event log.
 *
 * @param endpoint The endpoint for which the event log will be printed.
 * @param logId The log to be printed.
 **/
void emberAfEventsServerPrintEventLog(uint8_t endpoint,
                                      EmberAfEventLogId logId);

/**
 * @brief Prints an event.
 *
 * @param event The event to print.
 **/
void emberAfEventsServerPrintEvent(const EmberAfEvent *event);

/**
 * @brief Gets an event from the specified event log.
 *
 * This function can be used to get an event at a specific location in
 * the specified log.
 *
 * @param endpoint The relevant endpoint.
 * @param logId The relevant log.
 * @param index The index in the event log.
 * @param event The ::EmberAfEvent structure describing the event.
 * @return True if the event was found or false if the index is invalid.
 */
bool emberAfEventsServerGetEvent(uint8_t endpoint,
                                 EmberAfEventLogId logId,
                                 uint8_t index,
                                 EmberAfEvent *event);

/**
 * @brief Stores an event in the specified event log.
 *
 * This function is used to set an event at a specific location in
 * the specified log.
 *
 * @param endpoint The relevant endpoint.
 * @param logId The relevant log.
 * @param index The index in the event log.
 * @param event The ::EmberAfEvent structure describing the event.
 * If NULL, the event is removed from the server.
 * @return True if the event was set or removed or false if the index is invalid.
 */
bool emberAfEventsServerSetEvent(uint8_t endpoint,
                                 EmberAfEventLogId logId,
                                 uint8_t index,
                                 const EmberAfEvent *event);

/**
 * @brief Adds an event to the specified event log.
 *
 * This function is used to add an event at the next available location in
 * the specified log. Once the event log is full, new events will start
 * overwriting old events at the beginning of the table.
 *
 * @param endpoint The relevant endpoint.
 * @param logId The relevant log.
 * @param event The ::EmberAfEvent structure describing the event.
 * @return the index of the location in the log where the event was added or
 * ZCL_EVENTS_INVALID_INDEX if the specified event log is full.
 */
uint8_t emberAfEventsServerAddEvent(uint8_t endpoint,
                                    EmberAfEventLogId logId,
                                    const EmberAfEvent *event);

/**
 * @brief Publishes an event.
 *
 * This function will locate the event in the specified log at the specified
 * location and using the information from the event build and send a
 * PublishEvent command.
 *
 * @param nodeId The destination nodeId.
 * @param srcEndpoint The source endpoint.
 * @param dstEndpoint The destination endpoint.
 * @param logId The relevant log.
 * @param index The index in the event log.
 * @param eventControl Actions to be taken regarding this event. For example,
 * Report event to HAN and/or Report event to WAN.
 **/
void emberAfEventsServerPublishEventMessage(EmberNodeId nodeId,
                                            uint8_t srcEndpoint,
                                            uint8_t dstEndpoint,
                                            EmberAfEventLogId logId,
                                            uint8_t index,
                                            uint8_t eventControl);
