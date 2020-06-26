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
 * @brief Routines for the Critical Message Queue plugin, which stores and
 *        transmits (with retries) messages enqueued by the user.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "af-types.h"

#ifdef EMBER_TEST
  #define debugPrintln(...) emberAfCorePrintln(__VA_ARGS__)
#else
  #define debugPrintln(...)
#endif

typedef struct {
  EmberOutgoingMessageType type;
  uint16_t indexOrDestination;
  EmberApsFrame apsStruct;
  uint8_t zclTransactionIndex;
  uint8_t retries;
  uint8_t length;
  uint8_t data[128];  // variable length based on the previous element
} CriticalMessageQueueEntry;

#define INFINITE_REDELIVERY_ATTEMPTS 0xFF
#if defined(EMBER_AF_PLUGIN_WWAH_APP_EVENT_RETRY_MANAGER)
  #define CMQ_SIZE EMBER_AF_PLUGIN_WWAH_APP_EVENT_RETRY_MANAGER_QUEUE_SIZE
#else
  #define CMQ_SIZE 10
#endif
#define MAX_ATTEMPTS_DEFAULT INFINITE_REDELIVERY_ATTEMPTS
#define FIRST_BACKOFF_DEFAULT 5
#define BACKOFF_RATIO_DEFAULT 2
#define MAX_BACKOFF_DEFAULT 300

#define ABS_MAX_BACKOFF_SEC \
  (24   /* hours */         \
   * 60 /* minutes */       \
   * 60) /* seconds */

static void enqueueCallback(EmberOutgoingMessageType type,
                            uint16_t indexOrDestination,
                            EmberApsFrame *apsFrame,
                            uint16_t msgLen,
                            uint8_t *message,
                            EmberStatus status);

static void retryCallback(EmberOutgoingMessageType type,
                          uint16_t indexOrDestination,
                          EmberApsFrame *apsFrame,
                          uint16_t msgLen,
                          uint8_t *message,
                          EmberStatus status);

const EmberAfMessageSentFunction emberAfPluginCriticalMessageQueueEnqueueCallback = &enqueueCallback;
const EmberAfMessageSentFunction emAfPluginCriticalMessageQueueRetryCallback = &retryCallback;

static CriticalMessageQueueEntry messageQueue[CMQ_SIZE];
static uint8_t messageQueueCount;

static uint8_t firstBackoffTimeSeconds = FIRST_BACKOFF_DEFAULT;
static uint8_t backoffSequenceRatio = BACKOFF_RATIO_DEFAULT;
static uint32_t maxBackoffTimeSeconds = MAX_BACKOFF_DEFAULT;
static uint8_t maxRedeliveryAttempts = MAX_ATTEMPTS_DEFAULT;

EmberEventControl emberAfPluginCriticalMessageQueueRetryEventControl;

static uint32_t currentBackoff = FIRST_BACKOFF_DEFAULT;
static bool retryInProgress = false;

static void resetBackoffs(void)
{
  emberEventControlSetInactive(emberAfPluginCriticalMessageQueueRetryEventControl);
  currentBackoff = firstBackoffTimeSeconds;
}
static void increaseBackoffs(void)
{
  currentBackoff *= backoffSequenceRatio;
  if (currentBackoff > maxBackoffTimeSeconds ) {
    currentBackoff = maxBackoffTimeSeconds;
  }
  if (messageQueueCount) {
    emberEventControlSetDelayMS(emberAfPluginCriticalMessageQueueRetryEventControl, currentBackoff * MILLISECOND_TICKS_PER_SECOND);
  }
}

void emberAfPluginCriticalMessageQueueInit(void)
{
  messageQueueCount = 0;
}

static void createQueueEntry(EmberOutgoingMessageType type,
                             uint16_t indexOrDestination,
                             EmberApsFrame *apsFrame,
                             uint16_t msgLen,
                             uint8_t *message)
{
  uint8_t index;
  if (messageQueueCount < CMQ_SIZE) {
    index = messageQueueCount;
    messageQueueCount++;
  } else {
    index = CMQ_SIZE - 1;
  }
  messageQueue[index].type = type;
  messageQueue[index].indexOrDestination = indexOrDestination;
  MEMMOVE(&(messageQueue[index].apsStruct), apsFrame, sizeof(EmberApsFrame));
  messageQueue[index].retries = 0;
  messageQueue[index].length = msgLen;
  MEMMOVE(messageQueue[index].data, message, msgLen);
}

static uint8_t findQueueEntry(EmberOutgoingMessageType type,
                              uint16_t indexOrDestination,
                              EmberApsFrame *apsFrame,
                              uint16_t msgLen,
                              uint8_t *message)
{
  for (int i = 0; i <= messageQueueCount; i++) {
    if (messageQueue[i].type == type
        && messageQueue[i].indexOrDestination == indexOrDestination
        && messageQueue[i].length == msgLen
        && messageQueue[i].apsStruct.profileId == apsFrame->profileId
        && messageQueue[i].apsStruct.clusterId == apsFrame->clusterId
        && messageQueue[i].apsStruct.sourceEndpoint == apsFrame->sourceEndpoint
        && messageQueue[i].apsStruct.destinationEndpoint == apsFrame->destinationEndpoint
        //&& !MEMCOMPARE(&(messageQueue[i].apsStruct), apsFrame, sizeof(EmberApsFrame))
        // previously , I ran in to an issue with the aps radius not matching, so I only compare the ones that seem important
        && !MEMCOMPARE(messageQueue[i].data, message, msgLen)) {
      return i;
    }
  }
  return 0xFF;
}

void deleteQueueEntry(uint8_t index)
{
  for (; index < messageQueueCount; index++) {
    MEMCOPY(&(messageQueue[index]), &messageQueue[index + 1], sizeof(CriticalMessageQueueEntry));
  }
  messageQueueCount--;
}

void emberAfPluginCriticalMessageQueuePrintQueue(void)
{
  emberAfCorePrintln("%d/%d entries", messageQueueCount, CMQ_SIZE);
  for (int i = 0; i < messageQueueCount; i++) {
    emberAfCorePrint("Entry %d: Destination: %s %x Len: %d [ ",
                     i,
                     (messageQueue[i].type == EMBER_OUTGOING_DIRECT
                      ? "NODE"
                      : ((messageQueue[i].type == EMBER_OUTGOING_VIA_ADDRESS_TABLE)
                         ? "ADDR" : "BIND")),
                     messageQueue[i].indexOrDestination,
                     messageQueue[i].length
                     );
    emberAfCorePrintBuffer(messageQueue[i].data, messageQueue[i].length, true);
    emberAfCorePrintln(" ]");
  }
}

void emberAfPluginCriticalMessageQueuePrintConfig(void)
{
  emberAfCorePrintln("First backoff time (sec): %d", firstBackoffTimeSeconds);
  emberAfCorePrintln("Backoff sequence common ratio: %d", backoffSequenceRatio);
  emberAfCorePrintln("Max backoff time (sec): %d", maxBackoffTimeSeconds);
  emberAfCorePrintln("Max redelivery attempts: %d", maxRedeliveryAttempts);
}

static void retryCMQHead(void)
{
  assert(messageQueueCount > 0);
  retryInProgress = true;
  emberAfSendUnicastWithCallback(messageQueue[0].type,
                                 messageQueue[0].indexOrDestination,
                                 &(messageQueue[0].apsStruct),
                                 messageQueue[0].length,
                                 messageQueue[0].data,
                                 &retryCallback);
}

static void enqueueCallback(EmberOutgoingMessageType type,
                            uint16_t indexOrDestination,
                            EmberApsFrame *apsFrame,
                            uint16_t msgLen,
                            uint8_t *message,
                            EmberStatus status)
{
  resetBackoffs();
  if (status != EMBER_SUCCESS) {
    debugPrintln("Error: CMQ message enqueue error 0x%X", status);
    createQueueEntry(type, indexOrDestination, apsFrame, msgLen, message);
  }
  if (messageQueueCount
      && !retryInProgress) {
    retryCMQHead();
  }
}

static void retryCallback(EmberOutgoingMessageType type,
                          uint16_t indexOrDestination,
                          EmberApsFrame *apsFrame,
                          uint16_t msgLen,
                          uint8_t *message,
                          EmberStatus status)
{
  retryInProgress = false;
  uint8_t index = findQueueEntry(type, indexOrDestination, apsFrame, msgLen, message);
  if (index == 0xFF) {
    emberAfCorePrintln("ERROR: Retry Callback for unknown message");
    return;
  }
  if (status == EMBER_SUCCESS) {
    deleteQueueEntry(index);
    resetBackoffs();
    if (messageQueueCount) {
      retryCMQHead();
    }
  } else {
    debugPrintln("Error: CMQ message retry error 0x%X", status);
    messageQueue[index].retries++;
    if (maxRedeliveryAttempts != INFINITE_REDELIVERY_ATTEMPTS
        && messageQueue[index].retries >= maxRedeliveryAttempts) {
      deleteQueueEntry(index);
    }
    increaseBackoffs();
  }
}

void emberAfPluginCriticalMessageQueueRetryEventHandler(void)
{
  emberEventControlSetInactive(emberAfPluginCriticalMessageQueueRetryEventControl);
  retryCMQHead();
}

#if defined(EMBER_AF_PLUGIN_WWAH_APP_EVENT_RETRY_MANAGER)
EmberStatus emberAfWwahAppEventRetryManagerConfigBackoffParamsCallback(uint8_t inFirstBackoffTimeSeconds,
                                                                       uint8_t inBackoffSeqCommonRatio,
                                                                       uint32_t inMaxBackoffTimeSeconds,
                                                                       uint8_t inMaxRedeliveryAttempts)
{
  if (!(inFirstBackoffTimeSeconds)
      || (!inBackoffSeqCommonRatio)
      || (inMaxBackoffTimeSeconds < inFirstBackoffTimeSeconds)
      || (inMaxBackoffTimeSeconds > ABS_MAX_BACKOFF_SEC)
      || (!inMaxRedeliveryAttempts) ) {
    return EMBER_BAD_ARGUMENT;
  }

  firstBackoffTimeSeconds = inFirstBackoffTimeSeconds;
  backoffSequenceRatio = inBackoffSeqCommonRatio;
  maxBackoffTimeSeconds = inMaxBackoffTimeSeconds;
  maxRedeliveryAttempts = inMaxRedeliveryAttempts;

  return EMBER_SUCCESS;
}

void emberAfWwahAppEventRetryManagerSetBackoffParamsToDefault(void)
{
  emberAfPluginCriticalMessageQueueInit();
  firstBackoffTimeSeconds = FIRST_BACKOFF_DEFAULT;
  backoffSequenceRatio = BACKOFF_RATIO_DEFAULT;
  maxBackoffTimeSeconds = MAX_BACKOFF_DEFAULT;
  maxRedeliveryAttempts = MAX_ATTEMPTS_DEFAULT;
  currentBackoff = FIRST_BACKOFF_DEFAULT;
  retryInProgress = false;
}
#endif // defined(EMBER_AF_PLUGIN_WWAH_APP_EVENT_RETRY_MANAGER)
