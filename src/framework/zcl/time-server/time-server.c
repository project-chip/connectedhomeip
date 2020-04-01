/***************************************************************************//**
 * @file
 * @brief
 ******************************************************************************/

#include PLATFORM_HEADER
#include CONFIGURATION_HEADER
#include CHIP_AF_API_STACK
#ifdef CHIP_AF_API_DEBUG_PRINT
  #include CHIP_AF_API_DEBUG_PRINT
#endif
#include CHIP_AF_API_ZCL_CORE
#include CHIP_AF_API_HAL

#include "thread-callbacks.h"
#include "time-server.h"

// Events used in the time server.
ChipEventControl chZclTimeServerTickEventControl;

// Static variables.
static ChipZclEndpointId_t singletonEpId = CHIP_ZCL_ENDPOINT_NULL;

// Static function definitions.
static void timeServerInitEndpoint(ChipZclEndpointId_t endpointId);
static ChipZclStatus_t readTime(ChipZclEndpointId_t endpointId, uint32_t *time);
static ChipZclStatus_t writeTime(ChipZclEndpointId_t endpointId, uint32_t time);

// Static functions.
static void timeServerInitEndpoint(ChipZclEndpointId_t endpointId)
{
  ChipZclStatus_t status;
  uint32_t currentTime;
  uint8_t timeStatus = 0;

  if (chZclEndpointHasCluster(endpointId,
                              &chipZclClusterTimeServerSpec)) {
    const ChZclAttributeEntry_t *attribute
      = chZclFindAttribute(&chipZclClusterTimeServerSpec,
                           CHIP_ZCL_CLUSTER_TIME_SERVER_ATTRIBUTE_TIME,
                           false); // exclude remote
    if (!attribute) {
      return;
    }
    if (chZclIsAttributeSingleton(attribute)) {
      if (singletonEpId != CHIP_ZCL_ENDPOINT_NULL) {
        return;
      }
      singletonEpId = endpointId;
    }

    // Initialize the attribute with the real time, if it's available.
    currentTime = chipZclClusterTimeServerGetCurrentTimeCallback();
    if (currentTime != 0) {
      writeTime(endpointId, currentTime);
    }

#ifdef CHIP_AF_PLUGIN_TIME_SERVER_MASTER
    // The first bit of TimeStatus indicates whether the real time clock
    // corresponding to the Time attribute is internally set to the time
    // standard.
    timeStatus |= BIT(0);
#elif defined(CHIP_AF_PLUGIN_TIME_SERVER_SYNCHRONIZED)
    // The Synchronized bit specifies whether Time has been set over the ZigBee
    // network to synchronize it (as close as may be practical) to the time standard
    // bit must be explicitly written to indicate this - i.e. it is not set
    // automatically on writing to the Time attribute. If the Master bit is 1, the value of
    // this bit is 0.
    timeStatus |= BIT(1);
#endif

#ifdef CHIP_AF_PLUGIN_TIME_SERVER_MASTER_ZONE_DST
    // The third bit of TimeStatus indicates whether the TimeZone, DstStart,
    // DstEnd, and DstShift attributes are set internally to correct values for
    // the location of the clock.
    if (emAfContainsTimeServerAttribute(endpointId,
                                        CHIP_ZCL_CLUSTER_TIME_SERVER_ATTRIBUTE_TIME_ZONE)
        && emAfContainsTimeServerAttribute(endpointId,
                                           CHIP_ZCL_CLUSTER_TIME_SERVER_ATTRIBUTE_DST_START)
        && emAfContainsTimeServerAttribute(endpointId,
                                           CHIP_ZCL_CLUSTER_TIME_SERVER_ATTRIBUTE_DST_END)
        && emAfContainsTimeServerAttribute(endpointId,
                                           CHIP_ZCL_CLUSTER_TIME_SERVER_ATTRIBUTE_DST_SHIFT)) {
      timeStatus |= BIT(2);
    }
#endif //CHIP_AF_PLUGIN_TIME_SERVER_MASTER_ZONE_DST

#ifdef CHIP_AF_PLUGIN_TIME_SERVER_SUPERSEDING
    // Indicates that the time server should be considered as a more authoritative
    // time server.
    timeStatus |= BIT(3);
#endif //CHIP_AF_PLUGIN_TIME_SERVER_SUPERSEDING

    status = chipZclWriteAttribute(endpointId,
                                    &chipZclClusterTimeServerSpec,
                                    CHIP_ZCL_CLUSTER_TIME_SERVER_ATTRIBUTE_TIME_STATUS,
                                    (uint8_t *)&timeStatus,
                                    sizeof(timeStatus));
    if (status != CHIP_ZCL_STATUS_SUCCESS) {
      chipAfPluginTimeServerPrintln("ERR: writing time status %x", status);
    }

    // Start the time update scheduler.
    chipEventControlSetDelayMS(chZclTimeServerTickEventControl,
                                MILLISECOND_TICKS_PER_SECOND);
  }
}

static ChipZclStatus_t readTime(ChipZclEndpointId_t endpointId, uint32_t *time)
{
  chipAfPluginTimeServerPrintln("Server Time Read ep=%d", endpointId);

  ChipZclStatus_t status =
    chipZclReadAttribute(endpointId,
                          &chipZclClusterTimeServerSpec,
                          CHIP_ZCL_CLUSTER_TIME_SERVER_ATTRIBUTE_TIME,
                          (uint8_t *)time,
                          sizeof(*time));
  if (status != CHIP_ZCL_STATUS_SUCCESS) {
    chipAfPluginTimeServerPrintln("ERR: reading time %x", status);
  }
  return status;
}

static ChipZclStatus_t writeTime(ChipZclEndpointId_t endpointId, uint32_t time)
{
  chipAfPluginTimeServerPrintln("Server Time Write ep=%d time=%d", endpoint, time);

  ChipZclStatus_t status =
    chipZclWriteAttribute(endpointId,
                           &chipZclClusterTimeServerSpec,
                           CHIP_ZCL_CLUSTER_TIME_SERVER_ATTRIBUTE_TIME,
                           (uint8_t *)&time,
                           sizeof(time));
  if (status != CHIP_ZCL_STATUS_SUCCESS) {
    chipAfPluginTimeServerPrintln("ERR: writing time %x", status);
  }

  //TODO- Refresh and calculate all the other optional attributes here...

  return status;
}

void chZclTimeServerInit(void)
{
  for (uint8_t i = 0; i < chZclEndpointCount; i++) {
    const ChZclEndpointEntry_t *epEntry = &chZclEndpointTable[i];
    if (epEntry) {
      ChipZclEndpointId_t endpointId = epEntry->endpointId;
      if (chZclEndpointHasCluster(endpointId,
                                  &chipZclClusterTimeServerSpec)) {
        chipAfPluginTimeServerPrintln("Server Init.. ZCL Has Time Cluster on ep=%d", endpointId);
        if (!chipZclClusterTimeServerInitCallback(endpointId)) {
          // User did not handle the server init, so we do it.
          timeServerInitEndpoint(endpointId);
        }
      }
    }
  }
}

void chZclTimeServerTickEventHandler(void)
{
  // The time server event handler ticks every second once intialised.
  chipEventControlSetInactive(chZclTimeServerTickEventControl);

  // Update TimeServer time on all endpoints.
  for (uint8_t i = 0; i < chZclEndpointCount; i++) {
    const ChZclEndpointEntry_t *epEntry = &chZclEndpointTable[i];
    if (epEntry) {
      ChipZclEndpointId_t endpointId = epEntry->endpointId;
      if (chZclEndpointHasCluster(endpointId,
                                  &chipZclClusterTimeServerSpec)) {
        if (!chipZclClusterTimeServerTickCallback(endpointId)) {
          // User did not handle the time server tick, so we do it-
          // Update the currentTime attribute with the real time if we have it.
          // Otherwise, just increment the currentTime attribute value.
          uint32_t currentTime = chipZclClusterTimeServerGetCurrentTimeCallback();
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
  chipEventControlSetDelayMS(chZclTimeServerTickEventControl,
                              MILLISECOND_TICKS_PER_SECOND);
}

void emAfTimeClusterServerSetCurrentTime(uint32_t utcTime)
{
  // Set the time on all endpoints that do not have a singleton time attribute
  // as well as on one of the endpoints with a singleton attribute.
  for (uint8_t i = 0; i < chZclEndpointCount; i++) {
    const ChZclEndpointEntry_t *epEntry = &chZclEndpointTable[i];
    if (epEntry) {
      ChipZclEndpointId_t endpointId = epEntry->endpointId;
      if (chZclEndpointHasCluster(endpointId,
                                  &chipZclClusterTimeServerSpec)) {
        const ChZclAttributeEntry_t *attribute
          = chZclFindAttribute(&chipZclClusterTimeServerSpec,
                               CHIP_ZCL_CLUSTER_TIME_SERVER_ATTRIBUTE_TIME,
                               false); // exclude remote
        if (attribute) {
          if (!chZclIsAttributeSingleton(attribute)
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
  uint32_t currentTime = chipZclClusterTimeServerGetCurrentTimeCallback();

  // If we don't have the current time, we have to try to get it from an
  // endpoint by rolling through all of them until one returns a time.
  if (currentTime == 0) {
    for (uint8_t i = 0; i < chZclEndpointCount; i++) {
      const ChZclEndpointEntry_t *epEntry = &chZclEndpointTable[i];
      if (epEntry) {
        ChipZclEndpointId_t endpointId = epEntry->endpointId;
        if (chZclEndpointHasCluster(endpointId,
                                    &chipZclClusterTimeServerSpec)) {
          if (readTime(endpointId, &currentTime) == CHIP_ZCL_STATUS_SUCCESS) {
            break;
          }
        }
      }
    }
  }

  return currentTime;
}
