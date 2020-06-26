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
 * @brief The Time server plugin is responsible for keeping track of the current
 * time.  All endpoints that implement the Time cluster server should use a
 * singleton time attribute.  Sleepy devices should not use this plugin as it
 * will prevent the device from sleeping for longer than one second.
 *******************************************************************************
   ******************************************************************************/

#include "../../include/af.h"
#include "../../util/common.h"
#include "time-server.h"

#define INVALID_ENDPOINT 0xFF

static EmberAfStatus readTime(uint8_t endpoint, uint32_t *time);
static EmberAfStatus writeTime(uint8_t endpoint, uint32_t time);

static uint8_t singleton = INVALID_ENDPOINT;

#define emAfContainsTimeServerAttribute(endpoint, attribute) \
  emberAfContainsAttribute((endpoint), ZCL_TIME_CLUSTER_ID, (attribute), CLUSTER_MASK_SERVER, EMBER_AF_NULL_MANUFACTURER_CODE)

void emberAfTimeClusterServerInitCallback(uint8_t endpoint)
{
  EmberAfAttributeMetadata *metadata;
  EmberAfStatus status;
  uint32_t currentTime;
  uint8_t timeStatus = 0;

  metadata = emberAfLocateAttributeMetadata(endpoint,
                                            ZCL_TIME_CLUSTER_ID,
                                            ZCL_TIME_ATTRIBUTE_ID,
                                            CLUSTER_MASK_SERVER,
                                            EMBER_AF_NULL_MANUFACTURER_CODE);
  if (emberAfAttributeIsSingleton(metadata)) {
    if (singleton != INVALID_ENDPOINT) {
      return;
    }
    singleton = endpoint;
  }

  // Initialize the attribute with the real time, if it's available.
  currentTime = emberAfGetCurrentTimeCallback();
  if (currentTime != 0) {
    writeTime(endpoint, currentTime);
  }

#ifdef EMBER_AF_PLUGIN_TIME_SERVER_MASTER
  // The first bit of TimeStatus indicates whether the real time clock
  // corresponding to the Time attribute is internally set to the time
  // standard.
  timeStatus |= BIT(0);
#elif defined(EMBER_AF_PLUGIN_TIME_SERVER_SYNCHRONIZED)
  // The Synchronized bit specifies whether Time has been set over the ZigBee
  // network to synchronize it (as close as may be practical) to the time standard
  // bit must be explicitly written to indicate this - i.e. it is not set
  // automatically on writing to the Time attribute. If the Master bit is 1, the value of
  // this bit is 0.
  timeStatus |= BIT(1);
#endif

#ifdef EMBER_AF_PLUGIN_TIME_SERVER_MASTER_ZONE_DST
  // The third bit of TimeStatus indicates whether the TimeZone, DstStart,
  // DstEnd, and DstShift attributes are set internally to correct values for
  // the location of the clock.
  if (emAfContainsTimeServerAttribute(endpoint, ZCL_TIME_ZONE_ATTRIBUTE_ID)
      && emAfContainsTimeServerAttribute(endpoint, ZCL_DST_START_ATTRIBUTE_ID)
      && emAfContainsTimeServerAttribute(endpoint, ZCL_DST_END_ATTRIBUTE_ID)
      && emAfContainsTimeServerAttribute(endpoint, ZCL_DST_SHIFT_ATTRIBUTE_ID)) {
    timeStatus |= BIT(2);
  }
#endif //EMBER_AF_PLUGIN_TIME_SERVER_MASTER_ZONE_DST

#ifdef EMBER_AF_PLUGIN_TIME_SERVER_SUPERSEDING
  // Indicates that the time server should be considered as a more authoritative
  // time server.
  timeStatus |= BIT(3);
#endif //EMBER_AF_PLUGIN_TIME_SERVER_SUPERSEDING

  status = emberAfWriteAttribute(endpoint,
                                 ZCL_TIME_CLUSTER_ID,
                                 ZCL_TIME_STATUS_ATTRIBUTE_ID,
                                 CLUSTER_MASK_SERVER,
                                 (uint8_t *)&timeStatus,
                                 ZCL_BITMAP8_ATTRIBUTE_TYPE);
  if (status != EMBER_ZCL_STATUS_SUCCESS) {
    emberAfTimeClusterPrintln("ERR: writing time status %x", status);
  }

  // Ticks are scheduled for all endpoints that do not have a singleton time
  // attribute and for one of the endpoints with a singleton attribute.
  emberAfScheduleServerTick(endpoint,
                            ZCL_TIME_CLUSTER_ID,
                            MILLISECOND_TICKS_PER_SECOND);
}

void emberAfTimeClusterServerTickCallback(uint8_t endpoint)
{
  // Update the attribute with the real time if we have it; otherwise, just
  // increment the current the value.
  uint32_t currentTime = emberAfGetCurrentTimeCallback();
  if (currentTime == 0) {
    readTime(endpoint, &currentTime);
    currentTime++;
  }
  writeTime(endpoint, currentTime);

  // Reschedule the tick callback.
  emberAfScheduleServerTick(endpoint,
                            ZCL_TIME_CLUSTER_ID,
                            MILLISECOND_TICKS_PER_SECOND);
}

uint32_t emAfTimeClusterServerGetCurrentTime(void)
{
  EmberAfStatus status = EMBER_ZCL_STATUS_SUCCESS;
  uint32_t currentTime = emberAfGetCurrentTimeCallback();

  // If we don't have the current time, we have to try to get it from an
  // endpoint by rolling through all of them until one returns a time.
  if (currentTime == 0) {
    uint8_t i;
    for (i = 0; i < emberAfEndpointCount(); i++) {
      uint8_t endpoint = emberAfEndpointFromIndex(i);
      if (emAfContainsTimeServerAttribute(endpoint, ZCL_TIME_ATTRIBUTE_ID)) {
        status = readTime(endpoint, &currentTime);
        if (status == EMBER_ZCL_STATUS_SUCCESS) {
          break;
        }
      }
    }
  }

  return (status == EMBER_ZCL_STATUS_SUCCESS ? currentTime : 0);
}

void emAfTimeClusterServerSetCurrentTime(uint32_t utcTime)
{
  // Set the time on all endpoints that do not have a singleton time attribute
  // as well as on one of the endpoints with a singleton attribute.
  uint8_t i;
  for (i = 0; i < emberAfEndpointCount(); i++) {
    uint8_t endpoint = emberAfEndpointFromIndex(i);
    if (emAfContainsTimeServerAttribute(endpoint, ZCL_TIME_ATTRIBUTE_ID)) {
      EmberAfAttributeMetadata *metadata = emberAfLocateAttributeMetadata(endpoint,
                                                                          ZCL_TIME_CLUSTER_ID,
                                                                          ZCL_TIME_ATTRIBUTE_ID,
                                                                          CLUSTER_MASK_SERVER,
                                                                          EMBER_AF_NULL_MANUFACTURER_CODE);
      if (!emberAfAttributeIsSingleton(metadata) || singleton == endpoint) {
        writeTime(endpoint, utcTime);
      }
    }
  }
}

static EmberAfStatus readTime(uint8_t endpoint, uint32_t *time)
{
  EmberAfStatus status = emberAfReadAttribute(endpoint,
                                              ZCL_TIME_CLUSTER_ID,
                                              ZCL_TIME_ATTRIBUTE_ID,
                                              CLUSTER_MASK_SERVER,
                                              (uint8_t *)time,
                                              sizeof(*time),
                                              NULL); // data type
#if defined(EMBER_AF_PRINT_ENABLE) && defined(EMBER_AF_PRINT_TIME_CLUSTER)
  if (status != EMBER_ZCL_STATUS_SUCCESS) {
    emberAfTimeClusterPrintln("ERR: reading time %x", status);
  }
#endif //defined(EMBER_AF_PRINT_ENABLE) && defined(EMBER_AF_PRINT_TIME_CLUSTER)
  return status;
}

static EmberAfStatus writeTime(uint8_t endpoint, uint32_t time)
{
  EmberAfStatus status = emberAfWriteAttribute(endpoint,
                                               ZCL_TIME_CLUSTER_ID,
                                               ZCL_TIME_ATTRIBUTE_ID,
                                               CLUSTER_MASK_SERVER,
                                               (uint8_t *)&time,
                                               ZCL_UTC_TIME_ATTRIBUTE_TYPE);
#if defined(EMBER_AF_PRINT_ENABLE) && defined(EMBER_AF_PRINT_TIME_CLUSTER)
  if (status != EMBER_ZCL_STATUS_SUCCESS) {
    emberAfTimeClusterPrintln("ERR: writing time %x", status);
  }
#endif //defined(EMBER_AF_PRINT_ENABLE) && defined(EMBER_AF_PRINT_TIME_CLUSTER)
  return status;
}
