/***************************************************************************//**
 * @file
 * @brief
 *******************************************************************************
 * # License
 * <b>Copyright 2018 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

#include PLATFORM_HEADER
#include CONFIGURATION_HEADER
#include EMBER_AF_API_STACK
#ifdef EMBER_AF_API_DEBUG_PRINT
  #include EMBER_AF_API_DEBUG_PRINT
#endif
#include EMBER_AF_API_ZCL_CORE
#include EMBER_AF_API_HAL

#include "thread-callbacks.h"
#include "time-server.h"

// Events used in the time server.
EmberEventControl emZclTimeServerTickEventControl;

// Static variables.
static EmberZclEndpointId_t singletonEpId = EMBER_ZCL_ENDPOINT_NULL;

// Static function definitions.
static void timeServerInitEndpoint(EmberZclEndpointId_t endpointId);
static EmberZclStatus_t readTime(EmberZclEndpointId_t endpointId, uint32_t *time);
static EmberZclStatus_t writeTime(EmberZclEndpointId_t endpointId, uint32_t time);

// Static functions.
static void timeServerInitEndpoint(EmberZclEndpointId_t endpointId)
{
  EmberZclStatus_t status;
  uint32_t currentTime;
  uint8_t timeStatus = 0;

  if (emZclEndpointHasCluster(endpointId,
                              &emberZclClusterTimeServerSpec)) {
    const EmZclAttributeEntry_t *attribute
      = emZclFindAttribute(&emberZclClusterTimeServerSpec,
                           EMBER_ZCL_CLUSTER_TIME_SERVER_ATTRIBUTE_TIME,
                           false); // exclude remote
    if (!attribute) {
      return;
    }
    if (emZclIsAttributeSingleton(attribute)) {
      if (singletonEpId != EMBER_ZCL_ENDPOINT_NULL) {
        return;
      }
      singletonEpId = endpointId;
    }

    // Initialize the attribute with the real time, if it's available.
    currentTime = emberZclClusterTimeServerGetCurrentTimeCallback();
    if (currentTime != 0) {
      writeTime(endpointId, currentTime);
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
    if (emAfContainsTimeServerAttribute(endpointId,
                                        EMBER_ZCL_CLUSTER_TIME_SERVER_ATTRIBUTE_TIME_ZONE)
        && emAfContainsTimeServerAttribute(endpointId,
                                           EMBER_ZCL_CLUSTER_TIME_SERVER_ATTRIBUTE_DST_START)
        && emAfContainsTimeServerAttribute(endpointId,
                                           EMBER_ZCL_CLUSTER_TIME_SERVER_ATTRIBUTE_DST_END)
        && emAfContainsTimeServerAttribute(endpointId,
                                           EMBER_ZCL_CLUSTER_TIME_SERVER_ATTRIBUTE_DST_SHIFT)) {
      timeStatus |= BIT(2);
    }
#endif //EMBER_AF_PLUGIN_TIME_SERVER_MASTER_ZONE_DST

#ifdef EMBER_AF_PLUGIN_TIME_SERVER_SUPERSEDING
    // Indicates that the time server should be considered as a more authoritative
    // time server.
    timeStatus |= BIT(3);
#endif //EMBER_AF_PLUGIN_TIME_SERVER_SUPERSEDING

    status = emberZclWriteAttribute(endpointId,
                                    &emberZclClusterTimeServerSpec,
                                    EMBER_ZCL_CLUSTER_TIME_SERVER_ATTRIBUTE_TIME_STATUS,
                                    (uint8_t *)&timeStatus,
                                    sizeof(timeStatus));
    if (status != EMBER_ZCL_STATUS_SUCCESS) {
      emberAfPluginTimeServerPrintln("ERR: writing time status %x", status);
    }

    // Start the time update scheduler.
    emberEventControlSetDelayMS(emZclTimeServerTickEventControl,
                                MILLISECOND_TICKS_PER_SECOND);
  }
}

static EmberZclStatus_t readTime(EmberZclEndpointId_t endpointId, uint32_t *time)
{
  emberAfPluginTimeServerPrintln("Server Time Read ep=%d", endpointId);

  EmberZclStatus_t status =
    emberZclReadAttribute(endpointId,
                          &emberZclClusterTimeServerSpec,
                          EMBER_ZCL_CLUSTER_TIME_SERVER_ATTRIBUTE_TIME,
                          (uint8_t *)time,
                          sizeof(*time));
  if (status != EMBER_ZCL_STATUS_SUCCESS) {
    emberAfPluginTimeServerPrintln("ERR: reading time %x", status);
  }
  return status;
}

static EmberZclStatus_t writeTime(EmberZclEndpointId_t endpointId, uint32_t time)
{
  emberAfPluginTimeServerPrintln("Server Time Write ep=%d time=%d", endpoint, time);

  EmberZclStatus_t status =
    emberZclWriteAttribute(endpointId,
                           &emberZclClusterTimeServerSpec,
                           EMBER_ZCL_CLUSTER_TIME_SERVER_ATTRIBUTE_TIME,
                           (uint8_t *)&time,
                           sizeof(time));
  if (status != EMBER_ZCL_STATUS_SUCCESS) {
    emberAfPluginTimeServerPrintln("ERR: writing time %x", status);
  }

  //TODO- Refresh and calculate all the other optional attributes here...

  return status;
}

void emZclTimeServerInit(void)
{
  for (uint8_t i = 0; i < emZclEndpointCount; i++) {
    const EmZclEndpointEntry_t *epEntry = &emZclEndpointTable[i];
    if (epEntry) {
      EmberZclEndpointId_t endpointId = epEntry->endpointId;
      if (emZclEndpointHasCluster(endpointId,
                                  &emberZclClusterTimeServerSpec)) {
        emberAfPluginTimeServerPrintln("Server Init.. ZCL Has Time Cluster on ep=%d", endpointId);
        if (!emberZclClusterTimeServerInitCallback(endpointId)) {
          // User did not handle the server init, so we do it.
          timeServerInitEndpoint(endpointId);
        }
      }
    }
  }
}

void emZclTimeServerTickEventHandler(void)
{
  // The time server event handler ticks every second once intialised.
  emberEventControlSetInactive(emZclTimeServerTickEventControl);

  // Update TimeServer time on all endpoints.
  for (uint8_t i = 0; i < emZclEndpointCount; i++) {
    const EmZclEndpointEntry_t *epEntry = &emZclEndpointTable[i];
    if (epEntry) {
      EmberZclEndpointId_t endpointId = epEntry->endpointId;
      if (emZclEndpointHasCluster(endpointId,
                                  &emberZclClusterTimeServerSpec)) {
        if (!emberZclClusterTimeServerTickCallback(endpointId)) {
          // User did not handle the time server tick, so we do it-
          // Update the currentTime attribute with the real time if we have it.
          // Otherwise, just increment the currentTime attribute value.
          uint32_t currentTime = emberZclClusterTimeServerGetCurrentTimeCallback();
          if (currentTime == 0) {
            readTime(endpointId, &currentTime);
            currentTime++;
          }
          writeTime(endpointId, currentTime);
        }
      }
    }
  }
  // Reschedule the next second.
  emberEventControlSetDelayMS(emZclTimeServerTickEventControl,
                              MILLISECOND_TICKS_PER_SECOND);
}

void emAfTimeClusterServerSetCurrentTime(uint32_t utcTime)
{
  // Set the time on all endpoints that do not have a singleton time attribute
  // as well as on one of the endpoints with a singleton attribute.
  for (uint8_t i = 0; i < emZclEndpointCount; i++) {
    const EmZclEndpointEntry_t *epEntry = &emZclEndpointTable[i];
    if (epEntry) {
      EmberZclEndpointId_t endpointId = epEntry->endpointId;
      if (emZclEndpointHasCluster(endpointId,
                                  &emberZclClusterTimeServerSpec)) {
        const EmZclAttributeEntry_t *attribute
          = emZclFindAttribute(&emberZclClusterTimeServerSpec,
                               EMBER_ZCL_CLUSTER_TIME_SERVER_ATTRIBUTE_TIME,
                               false); // exclude remote
        if (attribute) {
          if (!emZclIsAttributeSingleton(attribute)
              || (endpointId == singletonEpId)) {
            writeTime(endpointId, utcTime);
          }
        }
      }
    }
  }
}

uint32_t emAfTimeClusterServerGetCurrentTime(void)
{
  uint32_t currentTime = emberZclClusterTimeServerGetCurrentTimeCallback();

  // If we don't have the current time, we have to try to get it from an
  // endpoint by rolling through all of them until one returns a time.
  if (currentTime == 0) {
    for (uint8_t i = 0; i < emZclEndpointCount; i++) {
      const EmZclEndpointEntry_t *epEntry = &emZclEndpointTable[i];
      if (epEntry) {
        EmberZclEndpointId_t endpointId = epEntry->endpointId;
        if (emZclEndpointHasCluster(endpointId,
                                    &emberZclClusterTimeServerSpec)) {
          if (readTime(endpointId, &currentTime) == EMBER_ZCL_STATUS_SUCCESS) {
            break;
          }
        }
      }
    }
  }

  return currentTime;
}
