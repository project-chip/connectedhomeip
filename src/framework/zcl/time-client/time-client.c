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
#include CHIP_AF_API_ZCL_CORE_WELL_KNOWN
#include "thread-callbacks.h"
#ifdef CHIP_AF_PLUGIN_TIME_SERVER
  #include "../time-server/time-server.h"
#endif

typedef struct {
  ChipZclDestination_t destination;
  uint8_t timeStatus;
} TimeServer_t;

// Events used in the time client.
ChipEventControl chZclTimeClientDiscoveryEventControl;

// Static variables.
static uint8_t discoveryRetryCount = 0;
static uint8_t discoveredServerCount = 0;
static TimeServer_t networkTimeServerList[CHIP_AF_PLUGIN_TIME_CLIENT_SERVER_DISCOVERY_TABLE_SIZE] = {};
static ChipZclDestination_t candidateServerDestination = {
  .network = {
    .address = { { 0, } },     // filled in after time server discovery.
    .flags = CHIP_ZCL_HAVE_IPV6_ADDRESS_FLAG,
    .port = CHIP_COAP_PORT,
  },
  .application = {
    .data = {
      .endpointId = CHIP_ZCL_ENDPOINT_NULL,     // filled in after time server discovery.
    },
    .type = CHIP_ZCL_APPLICATION_DESTINATION_TYPE_ENDPOINT,
  },
};

// Static function definitions.
static void addNetworkTimeServerList(ChipZclDestination_t *destination,
                                     uint8_t timeStatus);
static TimeServer_t *getServerBasedOnBits(uint8_t bitMask);
static void discoveryResponseHandler(ChipCoapStatus status,
                                     ChipCoapCode code,
                                     ChipCoapReadOptions *options,
                                     uint8_t *payload,
                                     uint16_t payloadLength,
                                     ChipCoapResponseInfo *info);
static void attributeReadResponseHandler(ChipZclMessageStatus_t status,
                                         const ChipZclAttributeContext_t *context,
                                         const void *buffer,
                                         size_t bufferLength);
static bool sendTimeServerDiscovery(void);
static TimeServer_t *sortDiscoveredTimeServers(void);

// Static functions.
static void addNetworkTimeServerList(ChipZclDestination_t *destination,
                                     uint8_t timeStatus)
{
  if (discoveredServerCount < CHIP_AF_PLUGIN_TIME_CLIENT_SERVER_DISCOVERY_TABLE_SIZE) {
    // Check if the discovered time server already exists in our server table.
    for (uint8_t i = 0; i < CHIP_AF_PLUGIN_TIME_CLIENT_SERVER_DISCOVERY_TABLE_SIZE; ++i ) {
      TimeServer_t *server = &(networkTimeServerList[i]);
      if ((!MEMCOMPARE(&server->destination, destination, sizeof(ChipZclDestination_t)))
          && (server->timeStatus == timeStatus)) {
        return; // no need to add a duplicate entry.
      }
    }

    TimeServer_t *currentInList = &(networkTimeServerList[discoveredServerCount]);
    currentInList->destination = *destination;
    currentInList->timeStatus = timeStatus;
    discoveredServerCount++;

    chipAfPluginTimeClientPrint("added time svr: node=");
    chipAfPluginTimeClientPrintBuffer(destination->network.data.address.bytes, 16, 0);
    chipAfPluginTimeClientPrintln(" ep=%d timeSts=%d",
                                   destination->application.data.endpointId,
                                   timeStatus);
  }
}

static TimeServer_t *getServerBasedOnBits(uint8_t bitMask)
{
  for (uint8_t i = 0; i < CHIP_AF_PLUGIN_TIME_CLIENT_SERVER_DISCOVERY_TABLE_SIZE; i++ ) {
    TimeServer_t *listEntry = &networkTimeServerList[i];
    if (listEntry->timeStatus & bitMask) {
      // superceeding bit is set.
      return listEntry;
    }
  }
  return NULL;
}

static void discoveryResponseHandler(ChipCoapStatus status,
                                     ChipCoapCode code,
                                     ChipCoapReadOptions *options,
                                     uint8_t *payload,
                                     uint16_t payloadLength,
                                     ChipCoapResponseInfo *info)
{
  // Handles time server zcl (cluster) discovery responses.

  if (status == CHIP_COAP_MESSAGE_RESPONSE) {
    // Get content format from response coap options.
    ChipCoapContentFormatType cf = CHIP_COAP_CONTENT_FORMAT_NONE;
    if (options != NULL) {
      uint32_t valueLoc;
      cf = (chipReadIntegerOption(options,
                                   CHIP_COAP_OPTION_CONTENT_FORMAT,
                                   &valueLoc))
           ? (ChipCoapContentFormatType)valueLoc
           : CHIP_COAP_CONTENT_FORMAT_NONE;
    }

    uint8_t *incoming = (uint8_t *)payload;
    ChZclUriContext_t uri;
    ChipZclClusterSpec_t spec;
    uri.clusterSpec = &spec;

    // Decode the first "/zcl/e/" tag in the response.
    if (chZclParseUri(payload,
                      payloadLength,
                      &incoming,
                      cf,
                      &uri)) {
      chipAfPluginTimeClientPrintln("time svr discovery rsp");

      // Update global candidate server network address and endpoint.
      candidateServerDestination.network.address = info->remoteAddress;
      candidateServerDestination.application.data.endpointId = uri.endpointId;

      const ChipZclAttributeId_t attributeIds[] = {
        CHIP_ZCL_CLUSTER_TIME_SERVER_ATTRIBUTE_TIME_STATUS,
      };
      size_t attributeIdsCount =
        sizeof(attributeIds) / sizeof(ChipZclAttributeId_t);

      ChipStatus status =
        chipZclSendAttributeRead(&candidateServerDestination,
                                  &chipZclClusterTimeServerSpec,
                                  attributeIds,
                                  attributeIdsCount,
                                  attributeReadResponseHandler);
      if (status == CHIP_SUCCESS) {
        chipAfPluginTimeClientPrintln("sent read time svr attribute 0x%02X req",
                                       attributeIds[0]);
      }
    } else {
      chipAfPluginTimeClientPrintln("payload Invalid");
    }
  }
}

static void attributeReadResponseHandler(ChipZclMessageStatus_t status,
                                         const ChipZclAttributeContext_t *context,
                                         const void *buffer,
                                         size_t bufferLength)
{
  // Handles read attribute responses.

  if (!chipZclAreClusterSpecsEqual(&chipZclClusterTimeServerSpec,
                                    context->clusterSpec)
      || (buffer == NULL)
      || (bufferLength == 0)) {
    return;
  }

  if (context->status == CHIP_ZCL_STATUS_SUCCESS) {
    if (context->groupId != CHIP_ZCL_GROUP_NULL) {
      return; // A group response is not valid in this context.
    } else {
      candidateServerDestination.application.data.endpointId = context->endpointId;
      candidateServerDestination.application.type = CHIP_ZCL_APPLICATION_DESTINATION_TYPE_ENDPOINT;
    }

    chipAfPluginTimeClientPrintln("time svr read attribute rsp: ep=%d, attr=0x%02X",
                                   context->endpointId,
                                   context->attributeId);

    switch (context->attributeId) {
      case CHIP_ZCL_CLUSTER_TIME_SERVER_ATTRIBUTE_TIME_STATUS: {
        uint8_t timeStatus = (buffer) ? *((uint8_t*)buffer) : 0;

        // We are only interested in the time server if TimeStatus attribute
        // bit0(=Master) or bit1(=Syncronized) is set.
        if (timeStatus & 0x03) {
          addNetworkTimeServerList(&candidateServerDestination, timeStatus);
        }
        break;
      }
      case CHIP_ZCL_CLUSTER_TIME_SERVER_ATTRIBUTE_TIME: {
        chipAfCorePrint("assigned Nwk TimeServer: node=");
        chipAfCorePrintBuffer(candidateServerDestination.network.address.bytes, 16, 0);
        chipAfCorePrintln(", ep=%d", candidateServerDestination.application.data.endpointId);

        if (!chipZclClusterTimeClientTimeSyncCallback(&candidateServerDestination)) {
          // User is not handling the update, so we update all the instances
          // of time server on this node.
        #ifdef CHIP_AF_PLUGIN_TIME_SERVER
          uint32_t utcTime = (buffer) ? *((uint32_t*)buffer) : 0;
          emAfTimeClusterServerSetCurrentTime(utcTime);
        #endif
        }
        break;
      }
      default: {
        break;
      }
    }
  }
}

static bool sendTimeServerDiscovery(void)
{
  bool sent = chipZclDiscByClusterId(&chipZclClusterTimeServerSpec,
                                      discoveryResponseHandler);

  if (sent) {
    chipAfPluginTimeClientPrintln("time svr discovery command sent");
  }

  return sent;
}

static TimeServer_t *sortDiscoveredTimeServers(void)
{
  // Devices SHALL synchronize to a Time server with the highest rank according
  // to the following rules, listed in order of precedence:
  // A server with the Superseding bit set SHALL be chosen over a server without
  //   the bit set.
  // A server with the Master bit SHALL be chosen over a server without the bit
  //   set.
  // TODO- The server with the lower short address SHALL be chosen (note that this
  //   means a coordinator with the Superseding and Master bit set will always
  //   be chosen as the network time server). -- Can't infer from IP address.
  // A Time server with neither the Master nor Synchronized bits set SHOULD not
  //   be chosen as the network time server.

  // Check for Superceeding bit from the list of discovered servers, if this is
  // not found check for a server with Master bit set.
  TimeServer_t *timeServer = NULL;
  timeServer = getServerBasedOnBits(8); // Test the Superceeding bit.
  if (!timeServer) {
    timeServer = getServerBasedOnBits(1); // Test the Master bit.
  }
  return timeServer;
}

void chZclTimeClientDiscoveryEventHandler(void)
{
  chipEventControlSetInactive(chZclTimeClientDiscoveryEventControl);

  // Send the TimeServer discovery request (multicast).
  if (discoveryRetryCount < CHIP_AF_PLUGIN_TIME_CLIENT_SERVER_DISCOVERY_RETRY_COUNT) {
    discoveryRetryCount++;
    sendTimeServerDiscovery();
    chipEventControlSetDelayMS(chZclTimeClientDiscoveryEventControl,
                                (CHIP_AF_PLUGIN_TIME_CLIENT_SERVER_DISCOVERY_RETRY_INTERVAL_IN_SECONDS * 1000));
  } else {
    TimeServer_t *timeServer = NULL;
    discoveredServerCount = 0;
    discoveryRetryCount = 0;

    // Sort the time server table to find the best Time Server.
    timeServer = sortDiscoveredTimeServers();
    if (timeServer) {
      // Read the Time(UTC) attribute from the sorted Time Server
      // (attribute response will confirm time server is operational).
      const ChipZclAttributeId_t attributeIds[] = {
        CHIP_ZCL_CLUSTER_TIME_SERVER_ATTRIBUTE_TIME,
      };
      size_t attributeIdsCount = sizeof(attributeIds) / sizeof(ChipZclAttributeId_t);
      ChipStatus status = chipZclSendAttributeRead(&timeServer->destination,
                                                     &chipZclClusterTimeServerSpec,
                                                     attributeIds,
                                                     attributeIdsCount,
                                                     attributeReadResponseHandler);
      if (status == CHIP_SUCCESS) {
        chipAfPluginTimeClientPrintln("sent read time svr attribute 0x%02X req",
                                       attributeIds[0]);
      } else {
        // Schedule retry
        chipEventControlSetDelayMS(chZclTimeClientDiscoveryEventControl,
                                    (CHIP_AF_PLUGIN_TIME_CLIENT_SERVER_DISCOVERY_RETRY_INTERVAL_IN_SECONDS * 1000));
        return;
      }
    }
    // Schedule re-discovery (long) interval time.
    chipEventControlSetDelayMS(chZclTimeClientDiscoveryEventControl,
                                (CHIP_AF_PLUGIN_TIME_CLIENT_SERVER_DISCOVERY_INTERVAL_IN_MINUTES * 60 * 1000));
  }
}

void chZclTimeClientInit(void)
{
  // Start the Discovery State machine by scheduling the event
  // with startup delay of 2 seconds.
  chipEventControlSetDelayMS(chZclTimeClientDiscoveryEventControl, 2000);
}
