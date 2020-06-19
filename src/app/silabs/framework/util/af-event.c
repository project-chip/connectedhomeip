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
 * @brief Application event code that is common to both the SOC and EZSP platforms.
 *******************************************************************************
   ******************************************************************************/

#include PLATFORM_HEADER     // Micro and compiler specific typedefs and macros
#include "../include/af.h"
#include "callback.h"
#include "af-event.h"
#include "stack/include/error.h"
#include "../security/crypto-state.h"
#include "app/framework/util/service-discovery.h"
#ifdef EMBER_AF_PLUGIN_FRAGMENTATION
#include "app/framework/plugin/fragmentation/fragmentation.h"
#endif

#include "app/framework/plugin/test-harness/test-harness.h"

#include "app/framework/util/attribute-storage.h"

// *****************************************************************************
// Globals

// Task ids used to run events through idling
EmberTaskId emAfTaskId;

#ifdef EMBER_AF_GENERATED_EVENT_CODE
EMBER_AF_GENERATED_EVENT_CODE
#endif //EMBER_AF_GENERATED_EVENT_CODE

#if defined(EMBER_AF_GENERATED_EVENT_CONTEXT)
uint16_t emAfAppEventContextLength = EMBER_AF_EVENT_CONTEXT_LENGTH;
EmberAfEventContext emAfAppEventContext[] = {
  EMBER_AF_GENERATED_EVENT_CONTEXT
};
#endif //EMBER_AF_GENERATED_EVENT_CONTEXT

const char * emAfEventStrings[] = {
  EM_AF_SERVICE_DISCOVERY_EVENT_STRINGS

#ifdef EMBER_AF_GENERATED_EVENTS
  EMBER_AF_GENERATED_EVENT_STRINGS
#endif

#ifdef EMBER_AF_PLUGIN_FRAGMENTATION
  EMBER_AF_FRAGMENTATION_EVENT_STRINGS
#endif

  EMBER_AF_TEST_HARNESS_EVENT_STRINGS
};

EmberEventData emAfEvents[] = {
  EM_AF_SERVICE_DISCOVERY_EVENTS

#ifdef EMBER_AF_GENERATED_EVENTS
  EMBER_AF_GENERATED_EVENTS
#endif

#ifdef EMBER_AF_PLUGIN_FRAGMENTATION
  EMBER_AF_FRAGMENTATION_EVENTS
#endif

  EMBER_KEY_ESTABLISHMENT_TEST_HARNESS_EVENT

  { NULL, NULL }
};

const char emAfStackEventString[] = "Stack";

// *****************************************************************************
// Functions

// A function used to initialize events for idling
void emAfInitEvents(void)
{
  emberTaskEnableIdling(true);
  emAfTaskId = emberTaskInit(emAfEvents);
}

void emberAfRunEvents(void)
{
  // Don't run events while crypto operation is in progress
  // (BUGZID: 12127)
  if (emAfIsCryptoOperationInProgress()) {
    // DEBUG Bugzid: 11944
    emberAfCoreFlush();
    return;
  }
  emberRunTask(emAfTaskId);
}

const char * emberAfGetEventString(uint8_t index)
{
  return (index == 0XFF
          ? emAfStackEventString
          : emAfEventStrings[index]);
}

static EmberAfEventContext *findEventContext(uint8_t endpoint,
                                             EmberAfClusterId clusterId,
                                             bool isClient)
{
#if defined(EMBER_AF_GENERATED_EVENT_CONTEXT)
  uint8_t i;
  for (i = 0; i < emAfAppEventContextLength; i++) {
    EmberAfEventContext *context = &(emAfAppEventContext[i]);
    if (context->endpoint == endpoint
        && context->clusterId == clusterId
        && context->isClient == isClient) {
      return context;
    }
  }
#endif //EMBER_AF_GENERATED_EVENT_CONTEXT
  return NULL;
}

EmberStatus emberAfEventControlSetDelayMS(EmberEventControl *control,
                                          uint32_t delayMs)
{
  if (delayMs == 0) {
    emberEventControlSetActive(*control);
  } else if (delayMs <= EMBER_MAX_EVENT_CONTROL_DELAY_MS) {
    emberEventControlSetDelayMS(*control, delayMs);
  } else {
    return EMBER_BAD_ARGUMENT;
  }
  return EMBER_SUCCESS;
}

EmberStatus emberAfEventControlSetDelayQS(EmberEventControl *control,
                                          uint32_t delayQs)
{
  if (delayQs <= EMBER_MAX_EVENT_CONTROL_DELAY_QS) {
    return emberAfEventControlSetDelayMS(control, delayQs << 8);
  } else {
    return EMBER_BAD_ARGUMENT;
  }
}

EmberStatus emberAfEventControlSetDelayMinutes(EmberEventControl *control,
                                               uint16_t delayM)
{
  if (delayM <= EMBER_MAX_EVENT_CONTROL_DELAY_MINUTES) {
    return emberAfEventControlSetDelayMS(control, delayM << 16);
  } else {
    return EMBER_BAD_ARGUMENT;
  }
}

EmberStatus emberAfScheduleTickExtended(uint8_t endpoint,
                                        EmberAfClusterId clusterId,
                                        bool isClient,
                                        uint32_t delayMs,
                                        EmberAfEventPollControl pollControl,
                                        EmberAfEventSleepControl sleepControl)
{
  EmberAfEventContext *context = findEventContext(endpoint,
                                                  clusterId,
                                                  isClient);

  // Disabled endpoints cannot schedule events.  This will catch the problem in
  // simulation.
  EMBER_TEST_ASSERT(emberAfEndpointIsEnabled(endpoint));

  if (context != NULL
      && emberAfEndpointIsEnabled(endpoint)
      && (emberAfEventControlSetDelayMS(context->eventControl, delayMs)
          == EMBER_SUCCESS)) {
    context->pollControl = pollControl;
    context->sleepControl = sleepControl;
    return EMBER_SUCCESS;
  }
  return EMBER_BAD_ARGUMENT;
}

EmberStatus emberAfScheduleClusterTick(uint8_t endpoint,
                                       EmberAfClusterId clusterId,
                                       bool isClient,
                                       uint32_t delayMs,
                                       EmberAfEventSleepControl sleepControl)
{
  return emberAfScheduleTickExtended(endpoint,
                                     clusterId,
                                     isClient,
                                     delayMs,
                                     (sleepControl == EMBER_AF_OK_TO_HIBERNATE
                                      ? EMBER_AF_LONG_POLL
                                      : EMBER_AF_SHORT_POLL),
                                     (sleepControl == EMBER_AF_STAY_AWAKE
                                      ? EMBER_AF_STAY_AWAKE
                                      : EMBER_AF_OK_TO_SLEEP));
}

EmberStatus emberAfScheduleClientTickExtended(uint8_t endpoint,
                                              EmberAfClusterId clusterId,
                                              uint32_t delayMs,
                                              EmberAfEventPollControl pollControl,
                                              EmberAfEventSleepControl sleepControl)
{
  return emberAfScheduleTickExtended(endpoint,
                                     clusterId,
                                     EMBER_AF_CLIENT_CLUSTER_TICK,
                                     delayMs,
                                     pollControl,
                                     sleepControl);
}

EmberStatus emberAfScheduleClientTick(uint8_t endpoint,
                                      EmberAfClusterId clusterId,
                                      uint32_t delayMs)
{
  return emberAfScheduleClientTickExtended(endpoint,
                                           clusterId,
                                           delayMs,
                                           EMBER_AF_LONG_POLL,
                                           EMBER_AF_OK_TO_SLEEP);
}

EmberStatus emberAfScheduleServerTickExtended(uint8_t endpoint,
                                              EmberAfClusterId clusterId,
                                              uint32_t delayMs,
                                              EmberAfEventPollControl pollControl,
                                              EmberAfEventSleepControl sleepControl)
{
  return emberAfScheduleTickExtended(endpoint,
                                     clusterId,
                                     EMBER_AF_SERVER_CLUSTER_TICK,
                                     delayMs,
                                     pollControl,
                                     sleepControl);
}

EmberStatus emberAfScheduleServerTick(uint8_t endpoint,
                                      EmberAfClusterId clusterId,
                                      uint32_t delayMs)
{
  return emberAfScheduleServerTickExtended(endpoint,
                                           clusterId,
                                           delayMs,
                                           EMBER_AF_LONG_POLL,
                                           EMBER_AF_OK_TO_SLEEP);
}

uint32_t emberAfMsToNextEventExtended(uint32_t maxMs, uint8_t* returnIndex)
{
  return emberMsToNextEventExtended(emAfEvents, maxMs, returnIndex);
}

uint32_t emberAfMsToNextEvent(uint32_t maxMs)
{
  return emberAfMsToNextEventExtended(maxMs, NULL);
}

EmberStatus emberAfDeactivateClusterTick(uint8_t endpoint,
                                         EmberAfClusterId clusterId,
                                         bool isClient)
{
  EmberAfEventContext *context = findEventContext(endpoint,
                                                  clusterId,
                                                  isClient);
  if (context != NULL) {
    emberEventControlSetInactive((*(context->eventControl)));
    return EMBER_SUCCESS;
  }
  return EMBER_BAD_ARGUMENT;
}

EmberStatus emberAfDeactivateClientTick(uint8_t endpoint,
                                        EmberAfClusterId clusterId)
{
  return emberAfDeactivateClusterTick(endpoint,
                                      clusterId,
                                      EMBER_AF_CLIENT_CLUSTER_TICK);
}

EmberStatus emberAfDeactivateServerTick(uint8_t endpoint,
                                        EmberAfClusterId clusterId)
{
  return emberAfDeactivateClusterTick(endpoint,
                                      clusterId,
                                      EMBER_AF_SERVER_CLUSTER_TICK);
}

#define MS_TO_QS(ms)    ((ms) >> 8)
#define MS_TO_MIN(ms)   ((ms) >> 16)
#define QS_TO_MS(qs)    ((qs) << 8)
#define MIN_TO_MS(min) ((min) << 16)

// Used to calculate the duration and unit used by the host to set the sleep timer
void emAfGetTimerDurationAndUnitFromMS(uint32_t durationMs,
                                       uint16_t *duration,
                                       EmberEventUnits *units)
{
  if (durationMs <= MAX_TIMER_UNITS_HOST) {
    *duration = (uint16_t)durationMs;
    *units = EMBER_EVENT_MS_TIME;
  } else if (MS_TO_QS(durationMs) <= MAX_TIMER_UNITS_HOST) {
    *duration = (uint16_t)(MS_TO_QS(durationMs));
    *units = EMBER_EVENT_QS_TIME;
  } else {
    *duration = (MS_TO_MIN(durationMs) <= MAX_TIMER_UNITS_HOST
                 ? (uint16_t)(MS_TO_MIN(durationMs))
                 : MAX_TIMER_UNITS_HOST);
    *units = EMBER_EVENT_MINUTE_TIME;
  }
}

uint32_t emAfGetMSFromTimerDurationAndUnit(uint16_t duration,
                                           EmberEventUnits units)
{
  uint32_t ms;
  if (units == EMBER_EVENT_MS_TIME) {
    ms = duration;
  } else if (units == EMBER_EVENT_QS_TIME) {
    ms = QS_TO_MS(duration);
  } else if (units == EMBER_EVENT_MINUTE_TIME) {
    ms = MIN_TO_MS(duration);
  } else if (units == EMBER_EVENT_ZERO_DELAY) {
    ms = 0;
  } else {
    ms = MAX_INT32U_VALUE;
  }
  return ms;
}
