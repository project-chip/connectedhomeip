/***************************************************************************//**
 * @file
 * @brief
 ******************************************************************************/

#include PLATFORM_HEADER
#include CONFIGURATION_HEADER
#include CHIP_AF_API_STACK
#include CHIP_AF_API_BUFFER_MANAGEMENT
#include CHIP_AF_API_EVENT_QUEUE
#include CHIP_AF_API_HAL
#ifdef CHIP_AF_API_DEBUG_PRINT
  #include CHIP_AF_API_DEBUG_PRINT
#endif
#include CHIP_AF_API_ZCL_CORE
#include CHIP_AF_API_ZCL_CORE_WELL_KNOWN
#include CHIP_AF_API_ZCL_CORE_DTLS_MANAGER

// TODO: Use an appropriate timeout.
#define DISCOVERY_TIMEOUT_MS   1500
#define OPEN_DTLS_SESSION_TIMEOUT_MS   3000

#define UID_RESOLUTION_TIMEOUT_MS 1500

// When memory for this structure is allocated, enough space is reserved at the
// end to also hold the URI (with NUL terminator), payload, and application
// data.
typedef struct {
  Event event;
  ChipZclCoapEndpoint_t destination;
  ChipIpv6Address remoteAddress;
  ChipCoapCode code;
  ChipCoapResponseHandler handler;
  size_t uriPathLength; // includes NUL terminator
  uint16_t payloadLength;
  uint16_t applicationDataLength;
  bool skipCoapTxRetry : 1;
  //uint8_t uriPath[uriPathLength];
  //uint8_t payload[payloadLength];
  //uint8_t applicationData[applicationDataLength];
} MessageEvent;

static bool discoverAddress(const ChipZclCoapEndpoint_t *destination);
static void uidDiscoveryResponseHandler(ChipCoapStatus status,
                                        ChipCoapCode code,
                                        ChipCoapReadOptions *options,
                                        uint8_t *payload,
                                        uint16_t payloadLength,
                                        ChipCoapResponseInfo *info);
static bool validateUidDiscoveryResponse(const uint8_t *payload,
                                         uint16_t payloadLength,
                                         ChipZclUid_t *uid);
static void defaultCoapResponseHandler(ChipCoapStatus status,
                                       ChipCoapCode code,
                                       ChipCoapReadOptions *options,
                                       uint8_t *payload,
                                       uint16_t payloadLength,
                                       ChipCoapResponseInfo *info);
static void eventHandler(MessageEvent *event);
static void eventMarker(MessageEvent *event);
static bool destinationPredicate(MessageEvent *event,
                                 const ChipZclUid_t *uid);
static void uidResolutionHandler(ChipCoapStatus status,
                                 ChipCoapCode code,
                                 ChipCoapReadOptions *options,
                                 uint8_t *payload,
                                 uint16_t payloadLength,
                                 ChipCoapResponseInfo *info);

void coapSendDtlsConnectionReturnHandle(uint8_t sessionId);

extern EventQueue emAppEventQueue;
static EventActions actions = {
  .queue = &emAppEventQueue,
  .handler = (void (*)(struct Event_s *))eventHandler,
  .marker = (void (*)(struct Event_s *))eventMarker,
  .name = "zcl core messaging"
};

ChipStatus chZclSend(const ChipZclCoapEndpoint_t *destination,
                      ChipCoapCode code,
                      const uint8_t *uriPath,
                      const uint8_t *payload,
                      uint16_t payloadLength,
                      ChipCoapResponseHandler handler,
                      void *applicationData,
                      uint16_t applicationDataLength,
                      bool skipCoapTxRetry)
{
  ChipCoapOption chZclSendOptions[] = {
    { CHIP_COAP_OPTION_CONTENT_FORMAT, NULL, 1, CHIP_COAP_CONTENT_FORMAT_CBOR, },
  };
  return chZclSendWithOptions(destination,
                              code,
                              uriPath,
                              chZclSendOptions,
                              COUNTOF(chZclSendOptions),
                              payload,
                              payloadLength,
                              handler,
                              applicationData,
                              applicationDataLength,
                              skipCoapTxRetry);
}

ChipStatus chipZclRequestBlock(const ChipZclCoapEndpoint_t *destination,
                                 const uint8_t *uriPath,
                                 ChipCoapBlockOption *block2Option,
                                 ChipCoapResponseHandler responseHandler)
{
  ChipCoapOption sendOptions[1];

  chipInitCoapOption(sendOptions,
                      CHIP_COAP_OPTION_BLOCK2,
                      chipBlockOptionValue(false,
                                            block2Option->logSize,
                                            block2Option->number));

  return chZclSendWithOptions(destination,
                              CHIP_COAP_CODE_GET,
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

ChipStatus chZclSendWithOptions(const ChipZclCoapEndpoint_t *destination,
                                 ChipCoapCode code,
                                 const uint8_t *uriPath,
                                 const ChipCoapOption options[],
                                 uint16_t optionsLength,
                                 const uint8_t *payload,
                                 uint16_t payloadLength,
                                 ChipCoapResponseHandler handler,
                                 void *applicationData,
                                 uint16_t applicationDataLength,
                                 bool skipCoapTxRetry)
{
  ChipIpv6Address remoteAddressBuf;
  const ChipIpv6Address *remoteAddress = NULL;
  bool createEvent = false;
  bool haveCachedUid = false;
  bool openDtls = false;
  bool discoveryNeeded = false;

  ChipCoapSendInfo info = {
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

  uint8_t sessionId = CHIP_NULL_SESSION_ID;
  if (destination->flags & CHIP_ZCL_HAVE_UID_FLAG) {
    sessionId = chipZclDtlsManagerGetSessionIdByUid(&destination->uid, destination->port);
  } else if (destination->flags & CHIP_ZCL_HAVE_IPV6_ADDRESS_FLAG) {
    sessionId = chipZclDtlsManagerGetSessionIdByAddress(&destination->address, destination->port);
  }

  if (sessionId != CHIP_NULL_SESSION_ID) { // use secure session if available
    info.localPort = CHIP_COAP_SECURE_PORT;
    info.transmitHandlerData = (void *) (uint32_t) sessionId;
    info.transmitHandler = &chipDtlsTransmitHandler;
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
  if (destination->flags & CHIP_ZCL_HAVE_IPV6_ADDRESS_FLAG) {
    // use the IPv6 address
    remoteAddress = &destination->address;
  } else if (chZclCacheGet(&destination->uid, &remoteAddressBuf)) {
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
    chipAfPluginZclCorePrintln("Unable to lookup remoteAddress");
    return CHIP_ERR_FATAL;
  }

  // If DTLS session is not found try to open one
  if ((destination->flags & CHIP_ZCL_USE_COAPS_FLAG)
      && sessionId == CHIP_NULL_SESSION_ID
      && !discoveryNeeded) {
    // If IPv6 address was not resolved the execution shouldn't get here
    if (((destination->flags & CHIP_ZCL_HAVE_IPV6_ADDRESS_FLAG) == 0) && (remoteAddress == NULL)) {
      assert(0);
    }

    // If we got here as a result of a callback from "open DTLS connection" smth went wrong.
    if (destination->flags & CHIP_ZCL_AVOID_NEW_DTLS_SESSION) {
      return CHIP_ERR_FATAL;
    }

    chipAfPluginZclCorePrintln("Opening a DTLS session ");
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
      chipAfPluginZclCorePrintln("Need more RAM (have: %u, need: %u)", emBufferBytesRemaining(), sizeof(MessageEvent) + uriPathLength + payloadLength + applicationDataLength);
      return CHIP_NO_BUFFERS;
    }
    uint8_t *finger = emGetBufferPointer(buffer);
    MessageEvent *event = (MessageEvent *)(void *)finger;
    event->event.actions = &actions;
    event->event.next = NULL;
    event->event.timeToExecute = 0;
    event->destination = *destination;
    MEMSET(&event->remoteAddress, 0, sizeof(ChipIpv6Address)); // filled in later
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
      event->destination.flags = destination->flags | CHIP_ZCL_HAVE_IPV6_ADDRESS_FLAG;
      chipEventSetDelayMs((Event *)event, OPEN_DTLS_SESSION_TIMEOUT_MS);
      chipZclDtlsManagerGetConnection(&destination->address,
                                       destination->port,
                                       CHIP_DTLS_MODE_CERT,
                                       coapSendDtlsConnectionReturnHandle);
      return CHIP_SUCCESS;
    }

    if (haveCachedUid) {
      MEMCOPY(&event->remoteAddress, remoteAddress, sizeof(ChipIpv6Address));
      chipEventSetDelayMs((Event *)event, UID_RESOLUTION_TIMEOUT_MS); // Set UID Resolution timeout.
      if (event->skipCoapTxRetry && event->handler == NULL) {
        event->handler = uidResolutionHandler;
      }
      return chipCoapSend(remoteAddress,
                           code,
                           uriPath,
                           payload,
                           payloadLength,
                           uidResolutionHandler,
                           &info);
    }
    // If we arent doing DTLS or UID Resolution, run UID discovery
    chipEventSetDelayMs((Event *)event, DISCOVERY_TIMEOUT_MS); // Set address discovery timeout.
    return CHIP_SUCCESS;
  }

  return chipCoapSend(remoteAddress,
                       code,
                       uriPath,
                       payload,
                       payloadLength,
                       (handler == NULL
                        ? defaultCoapResponseHandler
                        : handler),
                       &info);
}

static bool coapSendDtlsEventPredicate(MessageEvent *event, const ChipZclCoapEndpoint_t *destination)
{
  if (event->destination.flags & CHIP_ZCL_HAVE_IPV6_ADDRESS_FLAG) {
    return ((MEMCOMPARE(&destination->address, &event->destination.address, sizeof(ChipIpv6Address)) == 0)
            && (MEMCOMPARE(&destination->port, &event->destination.port, sizeof(uint16_t)) == 0));
  } else {
    return false;
  }
}

// Callback from DTLS connection creation triggered by chZclSendWithOptions()
void coapSendDtlsConnectionReturnHandle(uint8_t sessionId)
{
  ChipZclCoapEndpoint_t destination;

  if (chipZclDtlsManagerGetAddressBySessionId(sessionId, &destination.address) != CHIP_SUCCESS) {
    chipAfPluginZclCorePrintln("Remote address retrieval failure");
    return;
  }

  if (chipZclDtlsManagerGetPortBySessionId(sessionId, &destination.port) != CHIP_SUCCESS) {
    chipAfPluginZclCorePrintln("Remote port retrieval failure");
    return;
  }

  MessageEvent *event = (MessageEvent *)chipFindAllEvents(&emAppEventQueue, &actions, (EventPredicate)coapSendDtlsEventPredicate, (void *)&destination);

  while (event != NULL) {
    uint8_t *finger = (uint8_t *) event;
    finger += sizeof(MessageEvent);
    const uint8_t *uriPath = finger;
    finger += event->uriPathLength;
    const uint8_t *payload = finger;
    finger += event->payloadLength;
    uint8_t *applicationData = finger;
    //finger += event->applicationDataLength;

    // As an extra insurance against an infinite loop CHIP_ZCL_AVOID_NEW_DTLS_SESSION flag
    // will prevent chZclSendWithOptions() from openining a new DTLS session for this packet
    destination.flags = CHIP_ZCL_USE_COAPS_FLAG
                        | CHIP_ZCL_HAVE_IPV6_ADDRESS_FLAG | CHIP_ZCL_AVOID_NEW_DTLS_SESSION;

    chZclSend(&destination,
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

static bool discoverAddress(const ChipZclCoapEndpoint_t *destination)
{
  if (destination->flags & CHIP_ZCL_HAVE_UID_FLAG) {
    return (chipZclDiscByUid(&destination->uid,
                              CHIP_ZCL_UID_BITS, // discovery matches all uid bits.
                              uidDiscoveryResponseHandler));
  }
  //TODO Other discovery types here?
  return false;
}

static void uidDiscoveryResponseHandler(ChipCoapStatus status,
                                        ChipCoapCode code,
                                        ChipCoapReadOptions *options,
                                        uint8_t *payload,
                                        uint16_t payloadLength,
                                        ChipCoapResponseInfo *info)
{
  if (status == CHIP_COAP_MESSAGE_RESPONSE) {
    ChipZclUid_t uid;
    if (validateUidDiscoveryResponse(payload,
                                     payloadLength,
                                     &uid)) {
      chZclCacheAdd(&uid, &info->remoteAddress, NULL);
      // Find and send any buffered messages with a matching uid destination.
      MessageEvent *event
        = (MessageEvent *)chipFindAllEvents(&emAppEventQueue,
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

        ChipZclCoapEndpoint_t destination;
        // Use secure session if that's what the original packet required.
        destination.flags = (event->destination.flags & CHIP_ZCL_USE_COAPS_FLAG) | CHIP_ZCL_HAVE_IPV6_ADDRESS_FLAG;
        MEMCOPY(&destination.address,
                &info->remoteAddress,
                sizeof(ChipIpv6Address));
        destination.port = event->destination.port;

        chZclSend(&destination,
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
                                         ChipZclUid_t *uid)
{
  // Checks that uid discovery response payload contains a valid UID uri.

  uint16_t uidPrefixLen = strlen(CH_ZCL_URI_QUERY_UID_SHA_256);

  if (payloadLength >= uidPrefixLen + CHIP_ZCL_UID_BASE64URL_LENGTH) {
    for (uint16_t i = 0; i < (payloadLength - uidPrefixLen); ++i) {
      if (MEMCOMPARE(&payload[i], CH_ZCL_URI_QUERY_UID_SHA_256, uidPrefixLen) == 0) {
        // Found the uid prefix, set ptr to start of the uidB64u string.
        uint8_t *uidB64u = (uint8_t *)&payload[i] + uidPrefixLen;

        uint16_t uidBits;
        if (!chZclBase64UrlToUid(uidB64u,
                                 CHIP_ZCL_UID_BASE64URL_LENGTH,
                                 uid,
                                 &uidBits)
            || (uidBits != CHIP_ZCL_UID_BITS)) {
          return false;
        }
        return true;
      }
    }
  }
  return false;
}

static void uidResolutionHandler(ChipCoapStatus status,
                                 ChipCoapCode code,
                                 ChipCoapReadOptions *options,
                                 uint8_t *payload,
                                 uint16_t payloadLength,
                                 ChipCoapResponseInfo *info)
{
  // We want to either clear the address and send the discovery again or
  // cancel the event
  if (status == CHIP_COAP_MESSAGE_TIMED_OUT) {
    chipAfPluginZclCorePrintln("The UID message has timed out. Remove and send again");
    chZclCacheRemoveAllByIpv6Prefix(&info->remoteAddress, CHIP_IPV6_BITS);
  } else if (status == CHIP_COAP_MESSAGE_RESPONSE) {
    chipAfPluginZclCorePrintln("UID Resolved sucessfully");
    // In this case, we just allow the cancelled events to die
    // since we do not need to redo UID resolution.
  }
}

// When a message times out, we assume the remote address has changed and
// needs to be rediscovered, so we remove any key pointing to that address in
// our cache.  The next attempt to send to one of those keys will result in a
// rediscovery.
void chZclCoapStatusHandler(ChipCoapStatus status, ChipCoapResponseInfo *info)
{
  if (status == CHIP_COAP_MESSAGE_TIMED_OUT) {
    chZclCacheRemoveAllByIpv6Prefix(&info->remoteAddress, CHIP_IPV6_BITS);
  }
}

static void defaultCoapResponseHandler(ChipCoapStatus status,
                                       ChipCoapCode code,
                                       ChipCoapReadOptions *options,
                                       uint8_t *payload,
                                       uint16_t payloadLength,
                                       ChipCoapResponseInfo *info)
{
  chZclCoapStatusHandler(status, info);
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
    ChipCoapResponseInfo info;
    info.applicationData = (uint8_t *) finger;
    info.applicationDataLength = event->applicationDataLength;
    chZclCacheRemoveAllByIpv6Prefix(&event->remoteAddress, CHIP_IPV6_BITS);
    (*event->handler)(CHIP_ZCL_MESSAGE_STATUS_DISCOVERY_TIMEOUT,
                      CHIP_COAP_CODE_EMPTY,
                      NULL, // ChipCoapReadOptions
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

      ChipZclCoapEndpoint_t destination;
      destination.flags = CHIP_ZCL_HAVE_IPV6_ADDRESS_FLAG;
      MEMCOPY(&destination.address,
              &info.remoteAddress,
              sizeof(ChipIpv6Address));
      destination.port = event->destination.port;
      chZclSend(&destination,
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
                                 const ChipZclUid_t *uid)
{
  if (event->destination.flags & CHIP_ZCL_HAVE_UID_FLAG) {
    return (MEMCOMPARE(uid,
                       &event->destination.uid,
                       sizeof(ChipZclUid_t)) == 0);
  }
  return false;
}

typedef struct {
  ChipZclMessageStatus_t status;
  const uint8_t * const name;
} ZclMessageStatusEntry;
static const ZclMessageStatusEntry statusTable[] = {
  { CHIP_ZCL_MESSAGE_STATUS_DISCOVERY_TIMEOUT, (const uint8_t *)"DISCOVERY TIMEOUT", },
  { CHIP_ZCL_MESSAGE_STATUS_COAP_TIMEOUT, (const uint8_t *)"COAP TIMEOUT", },
  { CHIP_ZCL_MESSAGE_STATUS_COAP_ACK, (const uint8_t *)"COAP ACK", },
  { CHIP_ZCL_MESSAGE_STATUS_COAP_RESET, (const uint8_t *)"COAP RESET", },
  { CHIP_ZCL_MESSAGE_STATUS_COAP_RESPONSE, (const uint8_t *)"COAP RESPONSE", },
};
const uint8_t *chZclGetMessageStatusName(ChipZclMessageStatus_t status)
{
  for (size_t i = 0; i < COUNTOF(statusTable); i++) {
    if (status == statusTable[i].status) {
      return statusTable[i].name;
    }
  }
  return (const uint8_t *)"?????";
}
