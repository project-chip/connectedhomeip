/***************************************************************************//**
 *
 *    <COPYRIGHT>
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
 *
 ******************************************************************************
 * @file
 * @brief
 ******************************************************************************/

#include PLATFORM_HEADER
#include CONFIGURATION_HEADER
#include EMBER_AF_API_STACK
#include EMBER_AF_API_BUFFER_MANAGEMENT
#include EMBER_AF_API_EVENT_QUEUE
#include EMBER_AF_API_HAL
#ifdef EMBER_AF_API_DEBUG_PRINT
  #include EMBER_AF_API_DEBUG_PRINT
#endif
#include EMBER_AF_API_ZCL_CORE
#include EMBER_AF_API_ZCL_CORE_WELL_KNOWN
#include EMBER_AF_API_ZCL_CORE_DTLS_MANAGER

// TODO: Use an appropriate timeout.
#define DISCOVERY_TIMEOUT_MS   1500
#define OPEN_DTLS_SESSION_TIMEOUT_MS   3000

#define UID_RESOLUTION_TIMEOUT_MS 1500

// When memory for this structure is allocated, enough space is reserved at the
// end to also hold the URI (with NUL terminator), payload, and application
// data.
typedef struct {
  Event event;
  EmberZclCoapEndpoint_t destination;
  EmberIpv6Address remoteAddress;
  EmberCoapCode code;
  EmberCoapResponseHandler handler;
  size_t uriPathLength; // includes NUL terminator
  uint16_t payloadLength;
  uint16_t applicationDataLength;
  bool skipCoapTxRetry : 1;
  //uint8_t uriPath[uriPathLength];
  //uint8_t payload[payloadLength];
  //uint8_t applicationData[applicationDataLength];
} MessageEvent;

static bool discoverAddress(const EmberZclCoapEndpoint_t *destination);
static void uidDiscoveryResponseHandler(EmberCoapStatus status,
                                        EmberCoapCode code,
                                        EmberCoapReadOptions *options,
                                        uint8_t *payload,
                                        uint16_t payloadLength,
                                        EmberCoapResponseInfo *info);
static bool validateUidDiscoveryResponse(const uint8_t *payload,
                                         uint16_t payloadLength,
                                         EmberZclUid_t *uid);
static void defaultCoapResponseHandler(EmberCoapStatus status,
                                       EmberCoapCode code,
                                       EmberCoapReadOptions *options,
                                       uint8_t *payload,
                                       uint16_t payloadLength,
                                       EmberCoapResponseInfo *info);
static void eventHandler(MessageEvent *event);
static void eventMarker(MessageEvent *event);
static bool destinationPredicate(MessageEvent *event,
                                 const EmberZclUid_t *uid);
static void uidResolutionHandler(EmberCoapStatus status,
                                 EmberCoapCode code,
                                 EmberCoapReadOptions *options,
                                 uint8_t *payload,
                                 uint16_t payloadLength,
                                 EmberCoapResponseInfo *info);

void coapSendDtlsConnectionReturnHandle(uint8_t sessionId);

extern EventQueue emAppEventQueue;
static EventActions actions = {
  .queue = &emAppEventQueue,
  .handler = (void (*)(struct Event_s *))eventHandler,
  .marker = (void (*)(struct Event_s *))eventMarker,
  .name = "zcl core messaging"
};

EmberStatus emZclSend(const EmberZclCoapEndpoint_t *destination,
                      EmberCoapCode code,
                      const uint8_t *uriPath,
                      const uint8_t *payload,
                      uint16_t payloadLength,
                      EmberCoapResponseHandler handler,
                      void *applicationData,
                      uint16_t applicationDataLength,
                      bool skipCoapTxRetry)
{
  EmberCoapOption emZclSendOptions[] = {
    { EMBER_COAP_OPTION_CONTENT_FORMAT, NULL, 1, EMBER_COAP_CONTENT_FORMAT_CBOR, },
  };
  return emZclSendWithOptions(destination,
                              code,
                              uriPath,
                              emZclSendOptions,
                              COUNTOF(emZclSendOptions),
                              payload,
                              payloadLength,
                              handler,
                              applicationData,
                              applicationDataLength,
                              skipCoapTxRetry);
}

EmberStatus emberZclRequestBlock(const EmberZclCoapEndpoint_t *destination,
                                 const uint8_t *uriPath,
                                 EmberCoapBlockOption *block2Option,
                                 EmberCoapResponseHandler responseHandler)
{
  EmberCoapOption sendOptions[1];

  emberInitCoapOption(sendOptions,
                      EMBER_COAP_OPTION_BLOCK2,
                      emberBlockOptionValue(false,
                                            block2Option->logSize,
                                            block2Option->number));

  return emZclSendWithOptions(destination,
                              EMBER_COAP_CODE_GET,
                              uriPath,
                              sendOptions,
                              COUNTOF(sendOptions),
                              NULL,
                              0,
                              responseHandler,
                              NULL,
                              0,
                              false);
}

EmberStatus emZclSendWithOptions(const EmberZclCoapEndpoint_t *destination,
                                 EmberCoapCode code,
                                 const uint8_t *uriPath,
                                 const EmberCoapOption options[],
                                 uint16_t optionsLength,
                                 const uint8_t *payload,
                                 uint16_t payloadLength,
                                 EmberCoapResponseHandler handler,
                                 void *applicationData,
                                 uint16_t applicationDataLength,
                                 bool skipCoapTxRetry)
{
  EmberIpv6Address remoteAddressBuf;
  const EmberIpv6Address *remoteAddress = NULL;
  bool createEvent = false;
  bool haveCachedUid = false;
  bool openDtls = false;
  bool discoveryNeeded = false;

  EmberCoapSendInfo info = {
    .nonConfirmed = false,
    .skipCoapTxRetry = false, // We only want this set to true when UID needs resolution

    .localAddress = { { 0 } }, // use default
    .localPort = 0,        // use default
    .remotePort = destination->port,

    .options = options,
    .numberOfOptions = optionsLength,

    .responseTimeoutMs = 0, // use default

    .responseAppData = applicationData,
    .responseAppDataLength = applicationDataLength,

    .transmitHandler = NULL // unused
  };

  uint8_t sessionId = EMBER_NULL_SESSION_ID;
  if (destination->flags & EMBER_ZCL_HAVE_UID_FLAG) {
    sessionId = emberZclDtlsManagerGetSessionIdByUid(&destination->uid, destination->port);
  } else if (destination->flags & EMBER_ZCL_HAVE_IPV6_ADDRESS_FLAG) {
    sessionId = emberZclDtlsManagerGetSessionIdByAddress(&destination->address, destination->port);
  }

  if (sessionId != EMBER_NULL_SESSION_ID) { // use secure session if available
    info.localPort = EMBER_COAP_SECURE_PORT;
    info.transmitHandlerData = (void *) (uint32_t) sessionId;
    info.transmitHandler = &emberDtlsTransmitHandler;
  }

  // The following logic works in one of the three following ways:
  //
  // 1. If we currently have an IPv6 address then we will send to that, while first making sure to
  // create a DTLS session if needed. If DTLS is needed an event will be created for this, and messages sent after.
  //
  // 2. If we have UID and cached IPv6, we will create an event to rerun UID resolution and do COAP TX, unless
  // DTLS is needed. In this case, we will create a DTLS session first and then send the packet and create a
  // resolution event after.
  //
  // 3. If we have not gotten an IPv6 address or UID we will run address discovery, and create an event
  // to make sure this occurs. Then we will send to the new address once it has been discovered.
  if (destination->flags & EMBER_ZCL_HAVE_IPV6_ADDRESS_FLAG) {
    // use the IPv6 address
    remoteAddress = &destination->address;
  } else if (emZclCacheGet(&destination->uid, &remoteAddressBuf)) {
    remoteAddress = &remoteAddressBuf;
    if (skipCoapTxRetry) {
      info.skipCoapTxRetry = true;
      createEvent = true;
      haveCachedUid = true;
    }
  } else if (discoverAddress(destination)) {
    createEvent = true;
    discoveryNeeded = true;
  } else {
    emberAfPluginZclCorePrintln("Unable to lookup remoteAddress");
    return EMBER_ERR_FATAL;
  }

  // If DTLS session is not found try to open one
  if ((destination->flags & EMBER_ZCL_USE_COAPS_FLAG)
      && sessionId == EMBER_NULL_SESSION_ID
      && !discoveryNeeded) {
    // If IPv6 address was not resolved the execution shouldn't get here
    if (((destination->flags & EMBER_ZCL_HAVE_IPV6_ADDRESS_FLAG) == 0) && (remoteAddress == NULL)) {
      assert(0);
    }

    // If we got here as a result of a callback from "open DTLS connection" smth went wrong.
    if (destination->flags & EMBER_ZCL_AVOID_NEW_DTLS_SESSION) {
      return EMBER_ERR_FATAL;
    }

    emberAfPluginZclCorePrintln("Opening a DTLS session ");
    openDtls = true;
    createEvent = true;
  }

  if (createEvent) {
    size_t uriPathLength = strlen((const char *)uriPath) + 1; // include the NUL
    Buffer buffer = emAllocateBuffer(sizeof(MessageEvent)
                                     + uriPathLength
                                     + payloadLength
                                     + applicationDataLength);
    if (buffer == NULL_BUFFER) {
      emberAfPluginZclCorePrintln("Need more RAM (have: %u, need: %u)", emBufferBytesRemaining(), sizeof(MessageEvent) + uriPathLength + payloadLength + applicationDataLength);
      return EMBER_NO_BUFFERS;
    }
    uint8_t *finger = emGetBufferPointer(buffer);
    MessageEvent *event = (MessageEvent *)(void *)finger;
    event->event.actions = &actions;
    event->event.next = NULL;
    event->event.timeToExecute = 0;
    event->destination = *destination;
    MEMSET(&event->remoteAddress, 0, sizeof(EmberIpv6Address)); // filled in later
    event->code = code;
    event->handler = handler;
    event->uriPathLength = uriPathLength;
    event->payloadLength = payloadLength;
    event->applicationDataLength = applicationDataLength;
    finger += sizeof(MessageEvent);
    MEMCOPY(finger, uriPath, event->uriPathLength);
    finger += event->uriPathLength;
    MEMCOPY(finger, payload, event->payloadLength);
    finger += event->payloadLength;
    MEMCOPY(finger, applicationData, event->applicationDataLength);
    event->skipCoapTxRetry = skipCoapTxRetry;

    if (openDtls) {
      event->destination.flags = destination->flags | EMBER_ZCL_HAVE_IPV6_ADDRESS_FLAG;
      emberEventSetDelayMs((Event *)event, OPEN_DTLS_SESSION_TIMEOUT_MS);
      emberZclDtlsManagerGetConnection(&destination->address,
                                       destination->port,
                                       EMBER_DTLS_MODE_CERT,
                                       coapSendDtlsConnectionReturnHandle);
      return EMBER_SUCCESS;
    }

    if (haveCachedUid) {
      MEMCOPY(&event->remoteAddress, remoteAddress, sizeof(EmberIpv6Address));
      emberEventSetDelayMs((Event *)event, UID_RESOLUTION_TIMEOUT_MS); // Set UID Resolution timeout.
      if (event->skipCoapTxRetry && event->handler == NULL) {
        event->handler = uidResolutionHandler;
      }
      return emberCoapSend(remoteAddress,
                           code,
                           uriPath,
                           payload,
                           payloadLength,
                           uidResolutionHandler,
                           &info);
    }
    // If we arent doing DTLS or UID Resolution, run UID discovery
    emberEventSetDelayMs((Event *)event, DISCOVERY_TIMEOUT_MS); // Set address discovery timeout.
    return EMBER_SUCCESS;
  }

  return emberCoapSend(remoteAddress,
                       code,
                       uriPath,
                       payload,
                       payloadLength,
                       (handler == NULL
                        ? defaultCoapResponseHandler
                        : handler),
                       &info);
}

static bool coapSendDtlsEventPredicate(MessageEvent *event, const EmberZclCoapEndpoint_t *destination)
{
  if (event->destination.flags & EMBER_ZCL_HAVE_IPV6_ADDRESS_FLAG) {
    return ((MEMCOMPARE(&destination->address, &event->destination.address, sizeof(EmberIpv6Address)) == 0)
            && (MEMCOMPARE(&destination->port, &event->destination.port, sizeof(uint16_t)) == 0));
  } else {
    return false;
  }
}

// Callback from DTLS connection creation triggered by emZclSendWithOptions()
void coapSendDtlsConnectionReturnHandle(uint8_t sessionId)
{
  EmberZclCoapEndpoint_t destination;

  if (emberZclDtlsManagerGetAddressBySessionId(sessionId, &destination.address) != EMBER_SUCCESS) {
    emberAfPluginZclCorePrintln("Remote address retrieval failure");
    return;
  }

  if (emberZclDtlsManagerGetPortBySessionId(sessionId, &destination.port) != EMBER_SUCCESS) {
    emberAfPluginZclCorePrintln("Remote port retrieval failure");
    return;
  }

  MessageEvent *event = (MessageEvent *)emberFindAllEvents(&emAppEventQueue, &actions, (EventPredicate)coapSendDtlsEventPredicate, (void *)&destination);

  while (event != NULL) {
    uint8_t *finger = (uint8_t *) event;
    finger += sizeof(MessageEvent);
    const uint8_t *uriPath = finger;
    finger += event->uriPathLength;
    const uint8_t *payload = finger;
    finger += event->payloadLength;
    uint8_t *applicationData = finger;
    //finger += event->applicationDataLength;

    // As an extra insurance against an infinite loop EMBER_ZCL_AVOID_NEW_DTLS_SESSION flag
    // will prevent emZclSendWithOptions() from openining a new DTLS session for this packet
    destination.flags = EMBER_ZCL_USE_COAPS_FLAG
                        | EMBER_ZCL_HAVE_IPV6_ADDRESS_FLAG | EMBER_ZCL_AVOID_NEW_DTLS_SESSION;

    emZclSend(&destination,
              event->code,
              uriPath,
              payload,
              event->payloadLength,
              event->handler,
              applicationData,
              event->applicationDataLength,
              event->skipCoapTxRetry);

    MessageEvent *next = (MessageEvent *)event->event.next;
    event->event.next = NULL;
    event = next;
  }
}

static bool discoverAddress(const EmberZclCoapEndpoint_t *destination)
{
  if (destination->flags & EMBER_ZCL_HAVE_UID_FLAG) {
    return (emberZclDiscByUid(&destination->uid,
                              EMBER_ZCL_UID_BITS, // discovery matches all uid bits.
                              uidDiscoveryResponseHandler));
  }
  //TODO Other discovery types here?
  return false;
}

static void uidDiscoveryResponseHandler(EmberCoapStatus status,
                                        EmberCoapCode code,
                                        EmberCoapReadOptions *options,
                                        uint8_t *payload,
                                        uint16_t payloadLength,
                                        EmberCoapResponseInfo *info)
{
  if (status == EMBER_COAP_MESSAGE_RESPONSE) {
    EmberZclUid_t uid;
    if (validateUidDiscoveryResponse(payload,
                                     payloadLength,
                                     &uid)) {
      emZclCacheAdd(&uid, &info->remoteAddress, NULL);
      // Find and send any buffered messages with a matching uid destination.
      MessageEvent *event
        = (MessageEvent *)emberFindAllEvents(&emAppEventQueue,
                                             &actions,
                                             (EventPredicate)destinationPredicate,
                                             (void *)&uid);
      while (event != NULL) {
        uint8_t *finger = (uint8_t *) event;
        finger += sizeof(MessageEvent);
        const uint8_t *uriPath = finger;
        finger += event->uriPathLength;
        const uint8_t *payload = finger;
        finger += event->payloadLength;
        uint8_t *applicationData = finger;
        //finger += event->applicationDataLength;

        EmberZclCoapEndpoint_t destination;
        // Use secure session if that's what the original packet required.
        destination.flags = (event->destination.flags & EMBER_ZCL_USE_COAPS_FLAG) | EMBER_ZCL_HAVE_IPV6_ADDRESS_FLAG;
        MEMCOPY(&destination.address,
                &info->remoteAddress,
                sizeof(EmberIpv6Address));
        destination.port = event->destination.port;

        emZclSend(&destination,
                  event->code,
                  uriPath,
                  payload,
                  event->payloadLength,
                  event->handler,
                  applicationData,
                  event->applicationDataLength,
                  false);

        MessageEvent *next = (MessageEvent *)event->event.next;
        event->event.next = NULL;
        event = next;
      }
    }
  }
}

static bool validateUidDiscoveryResponse(const uint8_t *payload,
                                         uint16_t payloadLength,
                                         EmberZclUid_t *uid)
{
  // Checks that uid discovery response payload contains a valid UID uri.

  uint16_t uidPrefixLen = strlen(EM_ZCL_URI_QUERY_UID_SHA_256);

  if (payloadLength >= uidPrefixLen + EMBER_ZCL_UID_BASE64URL_LENGTH) {
    for (uint16_t i = 0; i < (payloadLength - uidPrefixLen); ++i) {
      if (MEMCOMPARE(&payload[i], EM_ZCL_URI_QUERY_UID_SHA_256, uidPrefixLen) == 0) {
        // Found the uid prefix, set ptr to start of the uidB64u string.
        uint8_t *uidB64u = (uint8_t *)&payload[i] + uidPrefixLen;

        uint16_t uidBits;
        if (!emZclBase64UrlToUid(uidB64u,
                                 EMBER_ZCL_UID_BASE64URL_LENGTH,
                                 uid,
                                 &uidBits)
            || (uidBits != EMBER_ZCL_UID_BITS)) {
          return false;
        }
        return true;
      }
    }
  }
  return false;
}

static void uidResolutionHandler(EmberCoapStatus status,
                                 EmberCoapCode code,
                                 EmberCoapReadOptions *options,
                                 uint8_t *payload,
                                 uint16_t payloadLength,
                                 EmberCoapResponseInfo *info)
{
  // We want to either clear the address and send the discovery again or
  // cancel the event
  if (status == EMBER_COAP_MESSAGE_TIMED_OUT) {
    emberAfPluginZclCorePrintln("The UID message has timed out. Remove and send again");
    emZclCacheRemoveAllByIpv6Prefix(&info->remoteAddress, EMBER_IPV6_BITS);
  } else if (status == EMBER_COAP_MESSAGE_RESPONSE) {
    emberAfPluginZclCorePrintln("UID Resolved sucessfully");
    // In this case, we just allow the cancelled events to die
    // since we do not need to redo UID resolution.
  }
}

// When a message times out, we assume the remote address has changed and
// needs to be rediscovered, so we remove any key pointing to that address in
// our cache.  The next attempt to send to one of those keys will result in a
// rediscovery.
void emZclCoapStatusHandler(EmberCoapStatus status, EmberCoapResponseInfo *info)
{
  if (status == EMBER_COAP_MESSAGE_TIMED_OUT) {
    emZclCacheRemoveAllByIpv6Prefix(&info->remoteAddress, EMBER_IPV6_BITS);
  }
}

static void defaultCoapResponseHandler(EmberCoapStatus status,
                                       EmberCoapCode code,
                                       EmberCoapReadOptions *options,
                                       uint8_t *payload,
                                       uint16_t payloadLength,
                                       EmberCoapResponseInfo *info)
{
  emZclCoapStatusHandler(status, info);
}

static void eventHandler(MessageEvent *event)
{
  // Called on address discovery response timeout.
  if (event->handler != NULL) {
    // Inform response handler that the uid discovery timed out.
    const uint8_t *finger = (const uint8_t *)event;
    finger += sizeof(MessageEvent);
    //const uint8_t *uriPath = finger;
    finger += event->uriPathLength;
    //const uint8_t *payload = (event->payloadLength == 0 ? NULL : finger);
    finger += event->payloadLength;
    //finger += event->applicationDataLength;
    EmberCoapResponseInfo info;
    info.applicationData = (uint8_t *) finger;
    info.applicationDataLength = event->applicationDataLength;
    emZclCacheRemoveAllByIpv6Prefix(&event->remoteAddress, EMBER_IPV6_BITS);
    (*event->handler)(EMBER_ZCL_MESSAGE_STATUS_DISCOVERY_TIMEOUT,
                      EMBER_COAP_CODE_EMPTY,
                      NULL, // EmberCoapReadOptions
                      NULL, // payload
                      0,    // payload length
                      &info);
    if (event->skipCoapTxRetry) {
      uint8_t *finger = (uint8_t *) event;
      finger += sizeof(MessageEvent);
      const uint8_t *uriPath = finger;
      finger += event->uriPathLength;
      const uint8_t *payload = finger;
      finger += event->payloadLength;
      uint8_t *applicationData = finger;

      EmberZclCoapEndpoint_t destination;
      destination.flags = EMBER_ZCL_HAVE_IPV6_ADDRESS_FLAG;
      MEMCOPY(&destination.address,
              &info.remoteAddress,
              sizeof(EmberIpv6Address));
      destination.port = event->destination.port;
      emZclSend(&destination,
                event->code,
                uriPath,
                payload,
                event->payloadLength,
                event->handler,
                applicationData,
                event->applicationDataLength,
                true);
    }
  }
}

static void eventMarker(MessageEvent *event)
{
}

static bool destinationPredicate(MessageEvent *event,
                                 const EmberZclUid_t *uid)
{
  if (event->destination.flags & EMBER_ZCL_HAVE_UID_FLAG) {
    return (MEMCOMPARE(uid,
                       &event->destination.uid,
                       sizeof(EmberZclUid_t)) == 0);
  }
  return false;
}

typedef struct {
  EmberZclMessageStatus_t status;
  const uint8_t * const name;
} ZclMessageStatusEntry;
static const ZclMessageStatusEntry statusTable[] = {
  { EMBER_ZCL_MESSAGE_STATUS_DISCOVERY_TIMEOUT, (const uint8_t *)"DISCOVERY TIMEOUT", },
  { EMBER_ZCL_MESSAGE_STATUS_COAP_TIMEOUT, (const uint8_t *)"COAP TIMEOUT", },
  { EMBER_ZCL_MESSAGE_STATUS_COAP_ACK, (const uint8_t *)"COAP ACK", },
  { EMBER_ZCL_MESSAGE_STATUS_COAP_RESET, (const uint8_t *)"COAP RESET", },
  { EMBER_ZCL_MESSAGE_STATUS_COAP_RESPONSE, (const uint8_t *)"COAP RESPONSE", },
};
const uint8_t *emZclGetMessageStatusName(EmberZclMessageStatus_t status)
{
  for (size_t i = 0; i < COUNTOF(statusTable); i++) {
    if (status == statusTable[i].status) {
      return statusTable[i].name;
    }
  }
  return (const uint8_t *)"?????";
}
