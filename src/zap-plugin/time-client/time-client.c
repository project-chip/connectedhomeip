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
#include EMBER_AF_API_ZCL_CORE_WELL_KNOWN
#include "thread-callbacks.h"
#ifdef EMBER_AF_PLUGIN_TIME_SERVER
  #include "../time-server/time-server.h"
#endif

typedef struct {
  EmberZclDestination_t destination;
  uint8_t timeStatus;
} TimeServer_t;

// Events used in the time client.
EmberEventControl emZclTimeClientDiscoveryEventControl;

// Static variables.
static uint8_t discoveryRetryCount = 0;
static uint8_t discoveredServerCount = 0;
static TimeServer_t networkTimeServerList[EMBER_AF_PLUGIN_TIME_CLIENT_SERVER_DISCOVERY_TABLE_SIZE] = {};
static EmberZclDestination_t candidateServerDestination = {
  .network = {
    .address = { { 0, } },     // filled in after time server discovery.
    .flags = EMBER_ZCL_HAVE_IPV6_ADDRESS_FLAG,
    .port = EMBER_COAP_PORT,
  },
  .application = {
    .data = {
      .endpointId = EMBER_ZCL_ENDPOINT_NULL,     // filled in after time server discovery.
    },
    .type = EMBER_ZCL_APPLICATION_DESTINATION_TYPE_ENDPOINT,
  },
};

// Static function definitions.
static void addNetworkTimeServerList(EmberZclDestination_t *destination,
                                     uint8_t timeStatus);
static TimeServer_t *getServerBasedOnBits(uint8_t bitMask);
static void discoveryResponseHandler(EmberCoapStatus status,
                                     EmberCoapCode code,
                                     EmberCoapReadOptions *options,
                                     uint8_t *payload,
                                     uint16_t payloadLength,
                                     EmberCoapResponseInfo *info);
static void attributeReadResponseHandler(EmberZclMessageStatus_t status,
                                         const EmberZclAttributeContext_t *context,
                                         const void *buffer,
                                         size_t bufferLength);
static bool sendTimeServerDiscovery(void);
static TimeServer_t *sortDiscoveredTimeServers(void);

// Static functions.
static void addNetworkTimeServerList(EmberZclDestination_t *destination,
                                     uint8_t timeStatus)
{
  if (discoveredServerCount < EMBER_AF_PLUGIN_TIME_CLIENT_SERVER_DISCOVERY_TABLE_SIZE) {
    // Check if the discovered time server already exists in our server table.
    for (uint8_t i = 0; i < EMBER_AF_PLUGIN_TIME_CLIENT_SERVER_DISCOVERY_TABLE_SIZE; ++i ) {
      TimeServer_t *server = &(networkTimeServerList[i]);
      if ((!MEMCOMPARE(&server->destination, destination, sizeof(EmberZclDestination_t)))
          && (server->timeStatus == timeStatus)) {
        return; // no need to add a duplicate entry.
      }
    }

    TimeServer_t *currentInList = &(networkTimeServerList[discoveredServerCount]);
    currentInList->destination = *destination;
    currentInList->timeStatus = timeStatus;
    discoveredServerCount++;

    emberAfPluginTimeClientPrint("added time svr: node=");
    emberAfPluginTimeClientPrintBuffer(destination->network.data.address.bytes, 16, 0);
    emberAfPluginTimeClientPrintln(" ep=%d timeSts=%d",
                                   destination->application.data.endpointId,
                                   timeStatus);
  }
}

static TimeServer_t *getServerBasedOnBits(uint8_t bitMask)
{
  for (uint8_t i = 0; i < EMBER_AF_PLUGIN_TIME_CLIENT_SERVER_DISCOVERY_TABLE_SIZE; i++ ) {
    TimeServer_t *listEntry = &networkTimeServerList[i];
    if (listEntry->timeStatus & bitMask) {
      // superceeding bit is set.
      return listEntry;
    }
  }
  return NULL;
}

static void discoveryResponseHandler(EmberCoapStatus status,
                                     EmberCoapCode code,
                                     EmberCoapReadOptions *options,
                                     uint8_t *payload,
                                     uint16_t payloadLength,
                                     EmberCoapResponseInfo *info)
{
  // Handles time server zcl (cluster) discovery responses.

  if (status == EMBER_COAP_MESSAGE_RESPONSE) {
    // Get content format from response coap options.
    EmberCoapContentFormatType cf = EMBER_COAP_CONTENT_FORMAT_NONE;
    if (options != NULL) {
      uint32_t valueLoc;
      cf = (emberReadIntegerOption(options,
                                   EMBER_COAP_OPTION_CONTENT_FORMAT,
                                   &valueLoc))
           ? (EmberCoapContentFormatType)valueLoc
           : EMBER_COAP_CONTENT_FORMAT_NONE;
    }

    uint8_t *incoming = (uint8_t *)payload;
    EmZclUriContext_t uri;
    EmberZclClusterSpec_t spec;
    uri.clusterSpec = &spec;

    // Decode the first "/zcl/e/" tag in the response.
    if (emZclParseUri(payload,
                      payloadLength,
                      &incoming,
                      cf,
                      &uri)) {
      emberAfPluginTimeClientPrintln("time svr discovery rsp");

      // Update global candidate server network address and endpoint.
      candidateServerDestination.network.address = info->remoteAddress;
      candidateServerDestination.application.data.endpointId = uri.endpointId;

      const EmberZclAttributeId_t attributeIds[] = {
        EMBER_ZCL_CLUSTER_TIME_SERVER_ATTRIBUTE_TIME_STATUS,
      };
      size_t attributeIdsCount =
        sizeof(attributeIds) / sizeof(EmberZclAttributeId_t);

      EmberStatus status =
        emberZclSendAttributeRead(&candidateServerDestination,
                                  &emberZclClusterTimeServerSpec,
                                  attributeIds,
                                  attributeIdsCount,
                                  attributeReadResponseHandler);
      if (status == EMBER_SUCCESS) {
        emberAfPluginTimeClientPrintln("sent read time svr attribute 0x%02X req",
                                       attributeIds[0]);
      }
    } else {
      emberAfPluginTimeClientPrintln("payload Invalid");
    }
  }
}

static void attributeReadResponseHandler(EmberZclMessageStatus_t status,
                                         const EmberZclAttributeContext_t *context,
                                         const void *buffer,
                                         size_t bufferLength)
{
  // Handles read attribute responses.

  if (!emberZclAreClusterSpecsEqual(&emberZclClusterTimeServerSpec,
                                    context->clusterSpec)
      || (buffer == NULL)
      || (bufferLength == 0)) {
    return;
  }

  if (context->status == EMBER_ZCL_STATUS_SUCCESS) {
    if (context->groupId != EMBER_ZCL_GROUP_NULL) {
      return; // A group response is not valid in this context.
    } else {
      candidateServerDestination.application.data.endpointId = context->endpointId;
      candidateServerDestination.application.type = EMBER_ZCL_APPLICATION_DESTINATION_TYPE_ENDPOINT;
    }

    emberAfPluginTimeClientPrintln("time svr read attribute rsp: ep=%d, attr=0x%02X",
                                   context->endpointId,
                                   context->attributeId);

    switch (context->attributeId) {
      case EMBER_ZCL_CLUSTER_TIME_SERVER_ATTRIBUTE_TIME_STATUS: {
        uint8_t timeStatus = (buffer) ? *((uint8_t*)buffer) : 0;

        // We are only interested in the time server if TimeStatus attribute
        // bit0(=Master) or bit1(=Syncronized) is set.
        if (timeStatus & 0x03) {
          addNetworkTimeServerList(&candidateServerDestination, timeStatus);
        }
        break;
      }
      case EMBER_ZCL_CLUSTER_TIME_SERVER_ATTRIBUTE_TIME: {
        emberAfCorePrint("assigned Nwk TimeServer: node=");
        emberAfCorePrintBuffer(candidateServerDestination.network.address.bytes, 16, 0);
        emberAfCorePrintln(", ep=%d", candidateServerDestination.application.data.endpointId);

        if (!emberZclClusterTimeClientTimeSyncCallback(&candidateServerDestination)) {
          // User is not handling the update, so we update all the instances
          // of time server on this node.
        #ifdef EMBER_AF_PLUGIN_TIME_SERVER
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
  bool sent = emberZclDiscByClusterId(&emberZclClusterTimeServerSpec,
                                      discoveryResponseHandler);

  if (sent) {
    emberAfPluginTimeClientPrintln("time svr discovery command sent");
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

void emZclTimeClientDiscoveryEventHandler(void)
{
  emberEventControlSetInactive(emZclTimeClientDiscoveryEventControl);

  // Send the TimeServer discovery request (multicast).
  if (discoveryRetryCount < EMBER_AF_PLUGIN_TIME_CLIENT_SERVER_DISCOVERY_RETRY_COUNT) {
    discoveryRetryCount++;
    sendTimeServerDiscovery();
    emberEventControlSetDelayMS(emZclTimeClientDiscoveryEventControl,
                                (EMBER_AF_PLUGIN_TIME_CLIENT_SERVER_DISCOVERY_RETRY_INTERVAL_IN_SECONDS * 1000));
  } else {
    TimeServer_t *timeServer = NULL;
    discoveredServerCount = 0;
    discoveryRetryCount = 0;

    // Sort the time server table to find the best Time Server.
    timeServer = sortDiscoveredTimeServers();
    if (timeServer) {
      // Read the Time(UTC) attribute from the sorted Time Server
      // (attribute response will confirm time server is operational).
      const EmberZclAttributeId_t attributeIds[] = {
        EMBER_ZCL_CLUSTER_TIME_SERVER_ATTRIBUTE_TIME,
      };
      size_t attributeIdsCount = sizeof(attributeIds) / sizeof(EmberZclAttributeId_t);
      EmberStatus status = emberZclSendAttributeRead(&timeServer->destination,
                                                     &emberZclClusterTimeServerSpec,
                                                     attributeIds,
                                                     attributeIdsCount,
                                                     attributeReadResponseHandler);
      if (status == EMBER_SUCCESS) {
        emberAfPluginTimeClientPrintln("sent read time svr attribute 0x%02X req",
                                       attributeIds[0]);
      } else {
        // Schedule retry
        emberEventControlSetDelayMS(emZclTimeClientDiscoveryEventControl,
                                    (EMBER_AF_PLUGIN_TIME_CLIENT_SERVER_DISCOVERY_RETRY_INTERVAL_IN_SECONDS * 1000));
        return;
      }
    }
    // Schedule re-discovery (long) interval time.
    emberEventControlSetDelayMS(emZclTimeClientDiscoveryEventControl,
                                (EMBER_AF_PLUGIN_TIME_CLIENT_SERVER_DISCOVERY_INTERVAL_IN_MINUTES * 60 * 1000));
  }
}

void emZclTimeClientInit(void)
{
  // Start the Discovery State machine by scheduling the event
  // with startup delay of 2 seconds.
  emberEventControlSetDelayMS(emZclTimeClientDiscoveryEventControl, 2000);
}
