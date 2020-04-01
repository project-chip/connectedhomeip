/***************************************************************************//**
 * @file
 * @brief
 ******************************************************************************/

#include PLATFORM_HEADER
#include CONFIGURATION_HEADER
#include CHIP_AF_API_STACK
#include CHIP_AF_API_BUFFER_MANAGEMENT
#include CHIP_AF_API_BUFFER_QUEUE
#include CHIP_AF_API_ZCL_CORE
#include CHIP_AF_API_ZCL_CORE_RESOURCE_DIRECTORY
#include CHIP_AF_API_ZCL_CORE_DTLS_MANAGER
#include CHIP_AF_API_ZCL_CORE_WELL_KNOWN

#define MAX_PATH_WITH_QUERY (100) // This needs to be more sophisticated. Picking a number for now.
#define MAX_REGISTRATION_PAYLOAD (2500)

#define M(x) CH_ZCL_URI_FLAG_METHOD_ ## x
#define GXXX (M(GET))
#define XPXX (M(POST))
#define GPXX (M(GET) | M(POST))
#define GXPX (M(GET) | M(PUT))
#define GXPD (M(GET) | M(PUT) | M(DELETE))
#define GPPD (M(GET) | M(POST) | M(PUT) | M(DELETE))

#define F(x) CH_ZCL_URI_FLAG_FORMAT_ ## x
#define FCBOR (F(CBOR))
#define FLINK (F(LINK))

static bool uriConfigurationMatch(ChZclContext_t *context, void *data, uint8_t depth);

static void rdConfUriHandler(ChZclContext_t *context);
static void rdConfIdUriHandler(ChZclContext_t *context);

static void registrationResponseHandler(ChipCoapStatus status,
                                        ChipCoapCode code,
                                        ChipCoapReadOptions *options,
                                        uint8_t *payload,
                                        uint16_t payloadLength,
                                        ChipCoapResponseInfo *info);
static void sendRegistration(ChipZclCoapEndpoint_t *destination);
void chZclResourceDirectoryRegisterCommand(void);

static uint8_t constructRegistrationUri(uint8_t *uri);
static int16_t constructRegistrationPayload(uint8_t *payload, uint16_t payloadLength);

typedef enum {
  CHIP_RD_SERVER_STATE_NOT_REGISTERED = 0,
  CHIP_RD_SERVER_STATE_REGISTERED = 1,
  CHIP_RD_SERVER_STATE_PERFORMING_REGISTRATION = 2,
  CHIP_RD_SERVER_STATE_REMOVING_REGISTRATION = 3,
} rdConfigState_t;

typedef struct {
  ChipZclCoapEndpoint_t server;
  uint16_t serverConfId; // configuration's ID on the RD server
  rdConfigState_t state;
  uint8_t id; // configuration's unique ID
} rdConfiguration_t;

typedef struct {
  uint16_t stateField;
  uint8_t rdField[CHIP_ZCL_URI_MAX_LENGTH];
} configStruct_t;
#define CHIP_ZCLIP_STRUCT configStruct_t
static const ZclipStructSpec configSpec[] = {
  CHIP_ZCLIP_OBJECT(sizeof(CHIP_ZCLIP_STRUCT),
                     2,         // fieldCount
                     NULL),     // names
  CHIP_ZCLIP_FIELD_NAMED(CHIP_ZCLIP_TYPE_UNSIGNED_INTEGER, stateField, "r"),
  CHIP_ZCLIP_FIELD_NAMED(CHIP_ZCLIP_TYPE_STRING, rdField, "rd"),
};
#undef CHIP_ZCLIP_STRUCT

//----------------------------------------------------------------
// A queue of buffers holding ResponseHandlerToCall_t structs.
static Buffer configurationList = NULL_BUFFER;

//----------------------------------------------------------------
void chZclResourceDirectoryClientMarkBuffers(void)
{
  emMarkBuffer(&configurationList);
}

ChZclUriPath chZclRdUriPaths[] = {
  // rd
  {   1, 255, GPPD | FCBOR, chZclUriPathStringMatch, "rd", NULL, rdConfUriHandler },

  // rd/conf
  {   1, 255, GPPD | FCBOR, chZclUriPathStringMatch, "conf", NULL, rdConfUriHandler },

  // rd/conf/XXXX
  { 255, 255, GPPD | FCBOR, uriConfigurationMatch, NULL, NULL, rdConfIdUriHandler },
};

static bool hexSegmentToInt(const ChZclContext_t *context,
                            uint8_t depth,
                            size_t size,
                            uintmax_t *result)
{
  return (context->uriPathLength[depth] <= size * 2 // bytes to nibbles
          && chZclHexStringToInt(context->uriPath[depth],
                                 context->uriPathLength[depth],
                                 result));
}

static bool uriConfigurationMatch(ChZclContext_t *context, void *data, uint8_t depth)
{
  uintmax_t retVal;
  if (hexSegmentToInt(context, depth, sizeof(context->rdConfId), &retVal)) {
    assert(retVal < 8 * sizeof(context->rdConfId)); // make sure value fits
    context->rdConfId = retVal;
    return true;
  } else {
    return false;
  }
}

static uint8_t getNextConfigurationId()
{
  uint8_t curId = 0;
  Buffer buffer = emBufferQueueHead(&configurationList);
  while (buffer != NULL_BUFFER) {
    rdConfiguration_t *entry = (rdConfiguration_t *)emGetBufferPointer(buffer);
    if (entry->id == curId) { // start again
      curId++;
      buffer = emBufferQueueHead(&configurationList);
    } else {
      buffer = emBufferQueueNext(&configurationList, buffer);
    }
  }
  return curId;
}

bool chipAfPluginResourceDirectoryHaveRegistered()
{
  Buffer finger = emBufferQueueHead(&configurationList);
  while (finger != NULL_BUFFER) {
    rdConfiguration_t *tmp = (rdConfiguration_t *)emGetBufferPointer(finger);
    if (tmp->state == CHIP_RD_SERVER_STATE_REGISTERED) {
      return true;
    }
    finger = emBufferQueueNext(&configurationList, finger);
  }

  return false;
}

static rdConfiguration_t *findConfigByDestination(ChipZclUid_t *uid, ChipIpv6Address *address, uint16_t port)
{
  rdConfiguration_t *configuration = NULL;
  Buffer finger = emBufferQueueHead(&configurationList);
  while (finger != NULL_BUFFER) {
    rdConfiguration_t *tmp = (rdConfiguration_t *)emGetBufferPointer(finger);

    if (uid != NULL) {   // lookup by UID (if available)
      if ((tmp->server.flags & CHIP_ZCL_HAVE_UID_FLAG)
          && (MEMCOMPARE(&tmp->server.uid.bytes, uid->bytes, sizeof(tmp->server.uid.bytes)) == 0)) {
        configuration = tmp;
        break;
      }
    }

    if (address != NULL) { // try by address and port (if available)
      if ((tmp->server.port == port)
          && (MEMCOMPARE(&tmp->server.address, address, sizeof(tmp->server.address)) == 0)) {
        configuration = tmp;
        break;
      }
    }
    finger = emBufferQueueNext(&configurationList, finger);
  }
  return configuration;
}

static bool addRdConfiguration(CborState *state, const rdConfiguration_t *configuration)
{
  configStruct_t configurationValue;
  chZclCoapEndpointToUri(&configuration->server, configurationValue.rdField);
  configurationValue.stateField = configuration->state;
  return emCborEncodeStruct(state, configSpec, &configurationValue);
}

static void printDestination(char *prefix, ChipZclCoapEndpoint_t *destination)
{
  chipAfPluginZclCorePrint("%s flags=0x%u, a=[", prefix, destination->flags);
  chipAfPluginZclCoreDebugExec(chipAfPrintIpv6Address(&destination->address));
  chipAfPluginZclCorePrint("]:%u, uid=", destination->port);
  chipAfPluginZclCorePrintBuffer(destination->uid.bytes, sizeof(destination->uid), false);
  chipAfPluginZclCorePrintln("");
}

static void rdConfUriHandler(ChZclContext_t *context)
{
  chipAfPluginZclCorePrintln("Handling URI w/depth=%u", context->uriPathSegments);
  switch (context->code) {
    case CHIP_COAP_CODE_GET: {
      CborState state;
      uint8_t buffer[CH_ZCL_MAX_PAYLOAD_SIZE];
      emCborEncodeStart(&state, buffer, sizeof(buffer));
      if (!emCborEncodeIndefiniteMap(&state)) {
        chipAfPluginZclCorePrintln("Unable to encode map");
        chZclRespond500InternalServerError(context->info);
        return;
      }
      Buffer finger = emBufferQueueHead(&configurationList);
      while (finger != NULL_BUFFER) {
        const rdConfiguration_t *configuration
          = (const rdConfiguration_t *)emGetBufferPointer(finger);
        if (!emCborEncodeValue(&state,
                               CHIP_ZCLIP_TYPE_UNSIGNED_INTEGER,
                               sizeof(configuration->id),
                               (const uint8_t *)&configuration->id)) {
          chipAfPluginZclCorePrintln("Unable to encode value %u", configuration->id);
          chZclRespond500InternalServerError(context->info);
          return;
        }

        if (!addRdConfiguration(&state, configuration)) {
          chipAfPluginZclCorePrintln("Unable to write structure");
          chZclRespond500InternalServerError(context->info);
          return;
        }

        finger = emBufferQueueNext(&configurationList, finger);
      }

      if (!emCborEncodeBreak(&state)) {
        chipAfPluginZclCorePrintln("Unable to break out of map");
        chZclRespond500InternalServerError(context->info);
        return;
      }
      chZclRespond205ContentCborState(context->info, &state);
      break;
    }
    case CHIP_COAP_CODE_POST: {
      CborState state;
      emCborDecodeStart(&state, context->payload, context->payloadLength);
      configStruct_t configurationValue = {
        .stateField = CHIP_RD_SERVER_STATE_NOT_REGISTERED
      };
      if (!emCborDecodeStruct(&state, configSpec, &configurationValue)) {
        chipAfPluginZclCorePrintln("Unable to read structure");
        chZclRespond500InternalServerError(context->info);
        return;
      }

      if (configurationValue.stateField != CHIP_RD_SERVER_STATE_NOT_REGISTERED) {
        chipAfPluginZclCorePrintln("Bad state was requested");
        chZclRespond400BadRequest(context->info);
        return;
      }

      ChipZclCoapEndpoint_t destination;
      if (chZclUriToCoapEndpoint(configurationValue.rdField, &destination) == 0) {
        chipAfPluginZclCorePrintln("Unable to parse destination");
        chZclRespond500InternalServerError(context->info);
        return;
      }

      rdConfiguration_t config = {
        .server = destination,
        .state = CHIP_RD_SERVER_STATE_PERFORMING_REGISTRATION,
        .id = 0, // set below
      };
      config.id = getNextConfigurationId();
      Buffer buffer = emFillBuffer((uint8_t *)&config, sizeof(config));
      if (buffer == NULL_BUFFER) {
        chZclRespond500InternalServerError(context->info);
        return;
      }
      emBufferQueueAdd(&configurationList, buffer);

      uint8_t path[11]; // "rd/conf/<2-digit-hex>" = 11 chars
      sprintf((char *)path, "rd/conf/%x", config.id);
      chipAfPluginZclCorePrintln("Creating registration w/ID=%u", config.id);
      chipAfPluginResourceDirectoryClientRegister(&config.server.address, config.server.port);
      chZclRespond201Created(context->info, path);
      break;
    }
    case CHIP_COAP_CODE_PUT:
      chZclRespond405MethodNotAllowed(context->info);
      break;
    case CHIP_COAP_CODE_DELETE:
      chZclRespond405MethodNotAllowed(context->info);
      break;
    default:
      assert(0);
  }
}

void unregistrationResponseHandler(ChipCoapStatus status,
                                   ChipCoapCode code,
                                   ChipCoapReadOptions *options,
                                   uint8_t *payload,
                                   uint16_t payloadLength,
                                   ChipCoapResponseInfo *info)
{
  switch (status) {
    case CHIP_COAP_MESSAGE_TIMED_OUT:
    case CHIP_COAP_MESSAGE_RESET:
      // Since our discovery is multicast, we get the TIMED_OUT status when we
      // are done receiving responses (note that we could have received no
      // responses). If we found a server, we should have already moved on to the
      // next state. Otherwise, then we keep trying to discover a server.
      chipAfPluginZclCorePrintln("Timed out while registering with server");
      break;
    case CHIP_COAP_MESSAGE_ACKED:
      // This means that our request was ACK'd, but the actual response is coming.
      break;
    case CHIP_COAP_MESSAGE_RESPONSE:
      if (!chipCoapIsSuccessResponse(code)) {
        chipAfPluginZclCorePrintln("Unegistration failure: code 0x%X", code);
      } else {
        rdConfiguration_t *configuration = findConfigByDestination(NULL, &info->remoteAddress, info->remotePort);
        if (configuration == NULL) {
          chipAfPluginZclCorePrintln("Unegistration failure: No local configuration ID");
          return;
        }

        configuration->state = CHIP_RD_SERVER_STATE_NOT_REGISTERED;
        chipAfPluginZclCorePrintln("Unegistration success (local: %x, remote: %x)", configuration->id, configuration->serverConfId);
      }
      break;
    default:
      chipAfPluginZclCorePrintln("Received unexpected status: %u", status);
      assert(false);
  }
}

static void rdConfIdUriHandler(ChZclContext_t *context)
{
  chipAfPluginZclCorePrintln("Handling URI w/Id=%u", context->rdConfId);
  Buffer finger = emBufferQueueHead(&configurationList);
  rdConfiguration_t *configuration = NULL;
  Buffer configFinger = NULL_BUFFER;
  while (finger != NULL_BUFFER) {
    rdConfiguration_t *tmp = (rdConfiguration_t *)emGetBufferPointer(finger);

    if (tmp->id == context->rdConfId) {
      configuration = tmp;
      configFinger = finger;
      break;
    }
    finger = emBufferQueueNext(&configurationList, finger);
  }
  if (configuration == NULL) {
    chZclRespond404NotFound(context->info);
    return;
  }

  switch (context->code) {
    case CHIP_COAP_CODE_GET: {
      CborState state;
      uint8_t buffer[CH_ZCL_MAX_PAYLOAD_SIZE];
      emCborEncodeStart(&state, buffer, sizeof(buffer));

      if (!addRdConfiguration(&state, configuration)) {
        chipAfPluginZclCorePrintln("Unable to write structure");
        chZclRespond500InternalServerError(context->info);
        return;
      }

      chZclRespond205ContentCborState(context->info, &state);
      break;
    }
    case CHIP_COAP_CODE_POST:
      chZclRespond405MethodNotAllowed(context->info);
      break;
    case CHIP_COAP_CODE_PUT: {
      CborState state;
      emCborDecodeStart(&state, context->payload, context->payloadLength);
      configStruct_t configurationValue;
      if (!emCborDecodeStruct(&state, configSpec, &configurationValue)) {
        chipAfPluginZclCorePrintln("Unable to read structure");
        chZclRespond500InternalServerError(context->info);
        return;
      }
      ChipZclCoapEndpoint_t destination;
      if (chZclUriToCoapEndpoint(configurationValue.rdField, &destination) == 0) {
        chipAfPluginZclCorePrintln("Unable to parse destination");
        chZclRespond500InternalServerError(context->info);
        return;
      }

      // Validate request
      if (destination.port == configuration->server.port && (MEMCOMPARE(&destination.address, &configuration->server.address, sizeof(destination.address)) != 0)) {
        chipAfPluginZclCorePrintln("Destination does not match configID's destination");
        printDestination("1->", &destination);
        printDestination("2->", &configuration->server);
        chZclRespond400BadRequest(context->info);
        return;
      }

      switch (configurationValue.stateField) {
        case CHIP_RD_SERVER_STATE_NOT_REGISTERED:
        case CHIP_RD_SERVER_STATE_REGISTERED:
          if (configurationValue.stateField != configuration->state) {
            chipAfPluginZclCorePrintln("Illegal state transistion");
            chZclRespond400BadRequest(context->info);
            return;
          }
          break;
        case CHIP_RD_SERVER_STATE_PERFORMING_REGISTRATION:
          if (configuration->state == CHIP_RD_SERVER_STATE_REMOVING_REGISTRATION) {
            chipAfPluginZclCorePrintln("Illegal state transistion");
            chZclRespond400BadRequest(context->info);
            return;
          }
          break;
        case CHIP_RD_SERVER_STATE_REMOVING_REGISTRATION:
          break;
      }

      // Perform action
      switch (configurationValue.stateField) {
        case CHIP_RD_SERVER_STATE_PERFORMING_REGISTRATION:
          chipAfPluginZclCorePrintln("Performing registration %u", configuration->id);
          configuration->state = CHIP_RD_SERVER_STATE_PERFORMING_REGISTRATION;
          chipAfPluginResourceDirectoryClientRegister(&configuration->server.address, configuration->server.port);
          break;
        case CHIP_RD_SERVER_STATE_REMOVING_REGISTRATION: {
          if (configuration->state == CHIP_RD_SERVER_STATE_REGISTERED) {
            chipAfPluginZclCorePrintln("Removing registration %u", configuration->id);
            uint8_t uri[MAX_PATH_WITH_QUERY];
            uint8_t uriLength = sprintf((char *)uri, "rd/%x", configuration->serverConfId);
            assert(uriLength < MAX_PATH_WITH_QUERY);

            chipAfPluginZclCorePrintln("Unregistering with resource directory (uri: %s)", uri);
            ChipStatus status = chZclSend(&configuration->server,
                                           CHIP_COAP_CODE_DELETE,
                                           uri,
                                           NULL,
                                           0,
                                           unregistrationResponseHandler,
                                           NULL,
                                           0,
                                           false);
            if (status != CHIP_SUCCESS) {
              chipAfPluginZclCorePrintln("Unable to send unregistration");
              chZclRespond500InternalServerError(context->info);
              return;
            }
          } else {
            configuration->state = CHIP_RD_SERVER_STATE_NOT_REGISTERED;
          }
        }
        break;
      }
      uint8_t path[12]; // "/rd/conf/<2-digit-hex>" = 12 chars
      sprintf((char *)path, "/rd/conf/%X", configuration->id);
      chZclRespond204ChangedWithPath(context->info, path);
      break;
    }
    case CHIP_COAP_CODE_DELETE: {
      if (configuration->state != CHIP_RD_SERVER_STATE_NOT_REGISTERED) {
        chZclRespond400BadRequest(context->info);
        return;
      }

      emBufferQueueRemove(&configurationList, configFinger);
      chZclRespond202Deleted(context->info);
      break;
    }
    default:
      assert(0);
  }
}

static ChipStatus sendRegistrationBlock(ChipZclCoapEndpoint_t *destination, uint8_t *payload, int16_t payloadLength, uint8_t blockLogSize, uint8_t blockNum)
{
  uint8_t uri[MAX_PATH_WITH_QUERY];
  uint8_t uriLength = constructRegistrationUri(uri);
  assert(uriLength < MAX_PATH_WITH_QUERY);

  ChipCoapOption sendOptions[2];

  // Options must be added in the increasing order of Option Number
  chipInitCoapOption(&(sendOptions[0]),
                      CHIP_COAP_OPTION_CONTENT_FORMAT,
                      CHIP_COAP_CONTENT_FORMAT_LINK_FORMAT_PLUS_CBOR);
  int16_t offset = blockNum * (1 << blockLogSize);
  bool haveMore = payloadLength > ((blockNum + 1) * (1 << blockLogSize));
  chipInitCoapOption(&(sendOptions[1]),
                      CHIP_COAP_OPTION_BLOCK1,
                      chipBlockOptionValue(haveMore,
                                            blockLogSize,
                                            blockNum));
  chipAfPluginZclCorePrintln("Sending registration blocksize=%u, blockNum=%u", blockLogSize, blockNum);
  return chZclSendWithOptions(destination,
                              CHIP_COAP_CODE_POST,
                              uri,
                              sendOptions,
                              COUNTOF(sendOptions),
                              payload + offset,
                              haveMore ? 1 << blockLogSize : (payloadLength - offset),
                              registrationResponseHandler,
                              payload,
                              payloadLength,
                              false);
}

static void registrationResponseHandler(ChipCoapStatus status,
                                        ChipCoapCode code,
                                        ChipCoapReadOptions *options,
                                        uint8_t *payload,
                                        uint16_t payloadLength,
                                        ChipCoapResponseInfo *info)
{
  switch (status) {
    case CHIP_COAP_MESSAGE_TIMED_OUT:
    case CHIP_COAP_MESSAGE_RESET:
      // Since our discovery is multicast, we get the TIMED_OUT status when we
      // are done receiving responses (note that we could have received no
      // responses). If we found a server, we should have already moved on to the
      // next state. Otherwise, then we keep trying to discover a server.
      chipAfPluginZclCorePrintln("Timed out while registering with server");
      break;
    case CHIP_COAP_MESSAGE_ACKED:
      // This means that our request was ACK'd, but the actual response is coming.
      break;
    case CHIP_COAP_MESSAGE_RESPONSE: {
      rdConfiguration_t *configuration = findConfigByDestination(NULL, &info->remoteAddress, info->remotePort);
      if (configuration == NULL) {
        chipAfPluginZclCorePrintln("Registration failure: No local configuration ID");
        return;
      }
      if (!chipCoapIsSuccessResponse(code)) {
        chipAfPluginZclCorePrintln("Registration failure: code 0x%X", code);
        configuration->state = CHIP_RD_SERVER_STATE_NOT_REGISTERED;
        return;
      }
      if (code == CHIP_COAP_CODE_231_CONTINUE) {
        ChipCoapBlockOption blkOptions;
        if (!chipReadBlockOption(options, CHIP_COAP_OPTION_BLOCK1, &blkOptions)) {
          chipAfPluginZclCorePrintln("Registration failure: Unable to read block1 options");
          configuration->state = CHIP_RD_SERVER_STATE_NOT_REGISTERED;
          return;
        }
        sendRegistrationBlock(&configuration->server, info->applicationData, info->applicationDataLength, blkOptions.logSize, blkOptions.number + 1);
      } else {
        uint8_t path[MAX_PATH_WITH_QUERY];
        int16_t pathLen = chipReadLocationPath(options, path, sizeof(path));
        path[pathLen] = 0;   // null terminator
        uint8_t *finger = &path[3];   // path = "rd/####"
        uintmax_t confId;
        emHexStringToInt(finger, pathLen - 3, &confId);
        assert(confId < (1 << (8 * sizeof(configuration->serverConfId))));   // make sure value fits
        configuration->serverConfId = confId;
        chipAfPluginZclCorePrintln("Registration success (local: %x, remote: %x)", configuration->id, configuration->serverConfId);
        configuration->state = CHIP_RD_SERVER_STATE_REGISTERED;
      }
    }
    break;
    default:
      assert(false);
  }
}

static void sendRegistration(ChipZclCoapEndpoint_t *destination)
{
  Buffer payloadBuffer = emAllocateBuffer(MAX_REGISTRATION_PAYLOAD);
  uint8_t *payload = (uint8_t *)emGetBufferPointer(payloadBuffer);
  int16_t payloadLength = constructRegistrationPayload(payload, emGetBufferLength(payloadBuffer));
  assert(payloadLength <= MAX_REGISTRATION_PAYLOAD);

  if (payloadLength == -1) {
    return;
  }

  emSetBufferLength(payloadBuffer, payloadLength); // shrink buffer to size

  chipAfPluginZclCorePrintln("Registering with resource directory");
  printDestination("destination: ", destination);

  #define REGISTRATION_BLOCK_LOGSIZE 9
  ChipStatus status = sendRegistrationBlock(destination, payload, payloadLength, REGISTRATION_BLOCK_LOGSIZE, 0);

  if (status != CHIP_SUCCESS) {
    chipAfPluginZclCorePrintln("ERR: Registration failed: 0x%x", status);
  } else {
    chipAfPluginZclCorePrintln("Registration sent");
  }
}

void resourceDirectoryDtlsSessionIdReturn(uint8_t sessionId)
{
  if (sessionId == CHIP_NULL_SESSION_ID) {
    chipAfPluginZclCorePrintln("No DTLS session was established to resource directory");
    return;
  }

  chipAfPluginZclCorePrintln("DTLS Session ID was returned: %u", sessionId);
  ChipIpv6Address address;
  if (chipZclDtlsManagerGetAddressBySessionId(sessionId, &address) != CHIP_SUCCESS) {
    chipAfPluginZclCorePrintln("Remote address retrieval failure");
    return;
  }
  uint16_t port;
  if (chipZclDtlsManagerGetPortBySessionId(sessionId, &port) != CHIP_SUCCESS) {
    chipAfPluginZclCorePrintln("Remote port retrieval failure");
    return;
  }
  ChipZclUid_t uid;
  if (chipZclDtlsManagerGetUidBySessionId(sessionId, &uid) != CHIP_SUCCESS) {
    chipAfPluginZclCorePrintln("UID retrieval failure");
    return;
  }
  rdConfiguration_t *configuration = findConfigByDestination(&uid, &address, port);
  if (configuration == NULL) {
    chipAfPluginZclCorePrint("Configuration retrieval failure a=[");
    chipAfPluginZclCoreDebugExec(chipAfPrintIpv6Address(&address));
    chipAfPluginZclCorePrint("]:%u || uid=", port);
    chipAfPluginZclCorePrintBuffer(uid.bytes, sizeof(uid), false);
    chipAfPluginZclCorePrintln("");
    return;
  }

  if (MEMCOMPARE(configuration->server.uid.bytes, uid.bytes, sizeof(uid.bytes)) != 0) {
    configuration->server.flags |= CHIP_ZCL_HAVE_UID_FLAG;
    MEMCOPY(configuration->server.uid.bytes, uid.bytes, sizeof(uid.bytes));
  }

  sendRegistration(&configuration->server);
}

void chipAfPluginResourceDirectoryClientRegister(ChipIpv6Address *resourceDirectoryIp, uint16_t resourceDirectoryPort)
{
  chipAfPluginZclCorePrint("Registering to server ip=");
  chipAfPluginZclCoreDebugExec(chipAfPrintIpv6Address(resourceDirectoryIp));
  chipAfPluginZclCorePrintln(" p=%d", resourceDirectoryPort);

  chipZclDtlsManagerGetConnection(resourceDirectoryIp, resourceDirectoryPort, CHIP_DTLS_MODE_CERT, resourceDirectoryDtlsSessionIdReturn);
}

static uint8_t constructRegistrationUri(uint8_t *uri)
{
  uint8_t *finger = uri;
  finger += sprintf((char *)finger, "/rd?");
  finger += sprintf((char *)finger, "ep=ni:///sha-256;");
  uint16_t uidLength = chZclUidToBase64Url(&chZclUid, CHIP_ZCL_UID_BITS, finger);
  assert(uidLength != 0);
  finger += uidLength;

  return finger - uri;
}

static int16_t constructRegistrationPayload(uint8_t *payload, uint16_t payloadLength)
{
  ChZclDiscPayloadContext_t dpc;
  chZclInitDiscPayloadContext(&dpc, CHIP_COAP_CONTENT_FORMAT_LINK_FORMAT_PLUS_CBOR, payload, payloadLength);
  dpc.startPayload(&dpc);

  uint8_t i;
  chipAfPluginZclCorePrintln("Constructing registration payload of %u elements.", chZclEndpointCount);
  for (i = 0; i < chZclEndpointCount; i++) {
    const ChZclEndpointEntry_t *epEntry = &chZclEndpointTable[i];
    const ChipZclClusterSpec_t **clusterSpecs = epEntry->clusterSpecs;
    const ChipZclClusterSpec_t *spec = NULL;
    spec = *clusterSpecs;

    chipAfPluginZclCorePrintln("  - Adding endpointId: %u", epEntry->endpointId);
    while (spec != NULL) {
      chipAfPluginZclCorePrintln("    - Adding clusterSpec: 0x%x%c", spec->id, spec->role ? 's' : 'c');
      dpc.startLink(&dpc);
      dpc.addResourceUri(&dpc, epEntry->endpointId, spec);
      dpc.addRt(&dpc, spec, true);
      dpc.addIf(&dpc, epEntry->endpointId, spec);
      dpc.addZe(&dpc, epEntry->endpointId, epEntry->deviceId);
      dpc.endLink(&dpc);

      clusterSpecs++;
      spec = *clusterSpecs;
    }
  }

  dpc.endPayload(&dpc);

  if (dpc.status == DISCOVERY_PAYLOAD_CONTEXT_STATUS_SUCCESS) {
    MEMCOPY(payload, dpc.payloadPointer(&dpc), dpc.payloadLength(&dpc));
    chipAfPluginZclCorePrintln("Payload length: %u", dpc.payloadLength(&dpc));
    return dpc.payloadLength(&dpc);
  } else {
    chipAfPluginZclCorePrintln("ERR: Failed to construct registration payload 0x%x", dpc.status);
    return -1;
  }
}
