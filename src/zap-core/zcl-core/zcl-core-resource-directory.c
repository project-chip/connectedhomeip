/***************************************************************************//**
 * @file
 * @brief
 ******************************************************************************/

#include PLATFORM_HEADER
#include CONFIGURATION_HEADER
#include EMBER_AF_API_STACK
#include EMBER_AF_API_BUFFER_MANAGEMENT
#include EMBER_AF_API_BUFFER_QUEUE
#include EMBER_AF_API_ZCL_CORE
#include EMBER_AF_API_ZCL_CORE_RESOURCE_DIRECTORY
#include EMBER_AF_API_ZCL_CORE_DTLS_MANAGER
#include EMBER_AF_API_ZCL_CORE_WELL_KNOWN

#define MAX_PATH_WITH_QUERY (100) // This needs to be more sophisticated. Picking a number for now.
#define MAX_REGISTRATION_PAYLOAD (2500)

#define M(x) EM_ZCL_URI_FLAG_METHOD_ ## x
#define GXXX (M(GET))
#define XPXX (M(POST))
#define GPXX (M(GET) | M(POST))
#define GXPX (M(GET) | M(PUT))
#define GXPD (M(GET) | M(PUT) | M(DELETE))
#define GPPD (M(GET) | M(POST) | M(PUT) | M(DELETE))

#define F(x) EM_ZCL_URI_FLAG_FORMAT_ ## x
#define FCBOR (F(CBOR))
#define FLINK (F(LINK))

static bool uriConfigurationMatch(EmZclContext_t *context, void *data, uint8_t depth);

static void rdConfUriHandler(EmZclContext_t *context);
static void rdConfIdUriHandler(EmZclContext_t *context);

static void registrationResponseHandler(EmberCoapStatus status,
                                        EmberCoapCode code,
                                        EmberCoapReadOptions *options,
                                        uint8_t *payload,
                                        uint16_t payloadLength,
                                        EmberCoapResponseInfo *info);
static void sendRegistration(EmberZclCoapEndpoint_t *destination);
void emZclResourceDirectoryRegisterCommand(void);

static uint8_t constructRegistrationUri(uint8_t *uri);
static int16_t constructRegistrationPayload(uint8_t *payload, uint16_t payloadLength);

typedef enum {
  EMBER_RD_SERVER_STATE_NOT_REGISTERED = 0,
  EMBER_RD_SERVER_STATE_REGISTERED = 1,
  EMBER_RD_SERVER_STATE_PERFORMING_REGISTRATION = 2,
  EMBER_RD_SERVER_STATE_REMOVING_REGISTRATION = 3,
} rdConfigState_t;

typedef struct {
  EmberZclCoapEndpoint_t server;
  uint16_t serverConfId; // configuration's ID on the RD server
  rdConfigState_t state;
  uint8_t id; // configuration's unique ID
} rdConfiguration_t;

typedef struct {
  uint16_t stateField;
  uint8_t rdField[EMBER_ZCL_URI_MAX_LENGTH];
} configStruct_t;
#define EMBER_ZCLIP_STRUCT configStruct_t
static const ZclipStructSpec configSpec[] = {
  EMBER_ZCLIP_OBJECT(sizeof(EMBER_ZCLIP_STRUCT),
                     2,         // fieldCount
                     NULL),     // names
  EMBER_ZCLIP_FIELD_NAMED(EMBER_ZCLIP_TYPE_UNSIGNED_INTEGER, stateField, "r"),
  EMBER_ZCLIP_FIELD_NAMED(EMBER_ZCLIP_TYPE_STRING, rdField, "rd"),
};
#undef EMBER_ZCLIP_STRUCT

//----------------------------------------------------------------
// A queue of buffers holding ResponseHandlerToCall_t structs.
static Buffer configurationList = NULL_BUFFER;

//----------------------------------------------------------------
void emZclResourceDirectoryClientMarkBuffers(void)
{
  emMarkBuffer(&configurationList);
}

EmZclUriPath emZclRdUriPaths[] = {
  // rd
  {   1, 255, GPPD | FCBOR, emZclUriPathStringMatch, "rd", NULL, rdConfUriHandler },

  // rd/conf
  {   1, 255, GPPD | FCBOR, emZclUriPathStringMatch, "conf", NULL, rdConfUriHandler },

  // rd/conf/XXXX
  { 255, 255, GPPD | FCBOR, uriConfigurationMatch, NULL, NULL, rdConfIdUriHandler },
};

static bool hexSegmentToInt(const EmZclContext_t *context,
                            uint8_t depth,
                            size_t size,
                            uintmax_t *result)
{
  return (context->uriPathLength[depth] <= size * 2 // bytes to nibbles
          && emZclHexStringToInt(context->uriPath[depth],
                                 context->uriPathLength[depth],
                                 result));
}

static bool uriConfigurationMatch(EmZclContext_t *context, void *data, uint8_t depth)
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

bool emberAfPluginResourceDirectoryHaveRegistered()
{
  Buffer finger = emBufferQueueHead(&configurationList);
  while (finger != NULL_BUFFER) {
    rdConfiguration_t *tmp = (rdConfiguration_t *)emGetBufferPointer(finger);
    if (tmp->state == EMBER_RD_SERVER_STATE_REGISTERED) {
      return true;
    }
    finger = emBufferQueueNext(&configurationList, finger);
  }

  return false;
}

static rdConfiguration_t *findConfigByDestination(EmberZclUid_t *uid, EmberIpv6Address *address, uint16_t port)
{
  rdConfiguration_t *configuration = NULL;
  Buffer finger = emBufferQueueHead(&configurationList);
  while (finger != NULL_BUFFER) {
    rdConfiguration_t *tmp = (rdConfiguration_t *)emGetBufferPointer(finger);

    if (uid != NULL) {   // lookup by UID (if available)
      if ((tmp->server.flags & EMBER_ZCL_HAVE_UID_FLAG)
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
  emZclCoapEndpointToUri(&configuration->server, configurationValue.rdField);
  configurationValue.stateField = configuration->state;
  return emCborEncodeStruct(state, configSpec, &configurationValue);
}

static void printDestination(char *prefix, EmberZclCoapEndpoint_t *destination)
{
  emberAfPluginZclCorePrint("%s flags=0x%u, a=[", prefix, destination->flags);
  emberAfPluginZclCoreDebugExec(emberAfPrintIpv6Address(&destination->address));
  emberAfPluginZclCorePrint("]:%u, uid=", destination->port);
  emberAfPluginZclCorePrintBuffer(destination->uid.bytes, sizeof(destination->uid), false);
  emberAfPluginZclCorePrintln("");
}

static void rdConfUriHandler(EmZclContext_t *context)
{
  emberAfPluginZclCorePrintln("Handling URI w/depth=%u", context->uriPathSegments);
  switch (context->code) {
    case EMBER_COAP_CODE_GET: {
      CborState state;
      uint8_t buffer[EM_ZCL_MAX_PAYLOAD_SIZE];
      emCborEncodeStart(&state, buffer, sizeof(buffer));
      if (!emCborEncodeIndefiniteMap(&state)) {
        emberAfPluginZclCorePrintln("Unable to encode map");
        emZclRespond500InternalServerError(context->info);
        return;
      }
      Buffer finger = emBufferQueueHead(&configurationList);
      while (finger != NULL_BUFFER) {
        const rdConfiguration_t *configuration
          = (const rdConfiguration_t *)emGetBufferPointer(finger);
        if (!emCborEncodeValue(&state,
                               EMBER_ZCLIP_TYPE_UNSIGNED_INTEGER,
                               sizeof(configuration->id),
                               (const uint8_t *)&configuration->id)) {
          emberAfPluginZclCorePrintln("Unable to encode value %u", configuration->id);
          emZclRespond500InternalServerError(context->info);
          return;
        }

        if (!addRdConfiguration(&state, configuration)) {
          emberAfPluginZclCorePrintln("Unable to write structure");
          emZclRespond500InternalServerError(context->info);
          return;
        }

        finger = emBufferQueueNext(&configurationList, finger);
      }

      if (!emCborEncodeBreak(&state)) {
        emberAfPluginZclCorePrintln("Unable to break out of map");
        emZclRespond500InternalServerError(context->info);
        return;
      }
      emZclRespond205ContentCborState(context->info, &state);
      break;
    }
    case EMBER_COAP_CODE_POST: {
      CborState state;
      emCborDecodeStart(&state, context->payload, context->payloadLength);
      configStruct_t configurationValue = {
        .stateField = EMBER_RD_SERVER_STATE_NOT_REGISTERED
      };
      if (!emCborDecodeStruct(&state, configSpec, &configurationValue)) {
        emberAfPluginZclCorePrintln("Unable to read structure");
        emZclRespond500InternalServerError(context->info);
        return;
      }

      if (configurationValue.stateField != EMBER_RD_SERVER_STATE_NOT_REGISTERED) {
        emberAfPluginZclCorePrintln("Bad state was requested");
        emZclRespond400BadRequest(context->info);
        return;
      }

      EmberZclCoapEndpoint_t destination;
      if (emZclUriToCoapEndpoint(configurationValue.rdField, &destination) == 0) {
        emberAfPluginZclCorePrintln("Unable to parse destination");
        emZclRespond500InternalServerError(context->info);
        return;
      }

      rdConfiguration_t config = {
        .server = destination,
        .state = EMBER_RD_SERVER_STATE_PERFORMING_REGISTRATION,
        .id = 0, // set below
      };
      config.id = getNextConfigurationId();
      Buffer buffer = emFillBuffer((uint8_t *)&config, sizeof(config));
      if (buffer == NULL_BUFFER) {
        emZclRespond500InternalServerError(context->info);
        return;
      }
      emBufferQueueAdd(&configurationList, buffer);

      uint8_t path[11]; // "rd/conf/<2-digit-hex>" = 11 chars
      sprintf((char *)path, "rd/conf/%x", config.id);
      emberAfPluginZclCorePrintln("Creating registration w/ID=%u", config.id);
      emberAfPluginResourceDirectoryClientRegister(&config.server.address, config.server.port);
      emZclRespond201Created(context->info, path);
      break;
    }
    case EMBER_COAP_CODE_PUT:
      emZclRespond405MethodNotAllowed(context->info);
      break;
    case EMBER_COAP_CODE_DELETE:
      emZclRespond405MethodNotAllowed(context->info);
      break;
    default:
      assert(0);
  }
}

void unregistrationResponseHandler(EmberCoapStatus status,
                                   EmberCoapCode code,
                                   EmberCoapReadOptions *options,
                                   uint8_t *payload,
                                   uint16_t payloadLength,
                                   EmberCoapResponseInfo *info)
{
  switch (status) {
    case EMBER_COAP_MESSAGE_TIMED_OUT:
    case EMBER_COAP_MESSAGE_RESET:
      // Since our discovery is multicast, we get the TIMED_OUT status when we
      // are done receiving responses (note that we could have received no
      // responses). If we found a server, we should have already moved on to the
      // next state. Otherwise, then we keep trying to discover a server.
      emberAfPluginZclCorePrintln("Timed out while registering with server");
      break;
    case EMBER_COAP_MESSAGE_ACKED:
      // This means that our request was ACK'd, but the actual response is coming.
      break;
    case EMBER_COAP_MESSAGE_RESPONSE:
      if (!emberCoapIsSuccessResponse(code)) {
        emberAfPluginZclCorePrintln("Unegistration failure: code 0x%X", code);
      } else {
        rdConfiguration_t *configuration = findConfigByDestination(NULL, &info->remoteAddress, info->remotePort);
        if (configuration == NULL) {
          emberAfPluginZclCorePrintln("Unegistration failure: No local configuration ID");
          return;
        }

        configuration->state = EMBER_RD_SERVER_STATE_NOT_REGISTERED;
        emberAfPluginZclCorePrintln("Unegistration success (local: %x, remote: %x)", configuration->id, configuration->serverConfId);
      }
      break;
    default:
      emberAfPluginZclCorePrintln("Received unexpected status: %u", status);
      assert(false);
  }
}

static void rdConfIdUriHandler(EmZclContext_t *context)
{
  emberAfPluginZclCorePrintln("Handling URI w/Id=%u", context->rdConfId);
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
    emZclRespond404NotFound(context->info);
    return;
  }

  switch (context->code) {
    case EMBER_COAP_CODE_GET: {
      CborState state;
      uint8_t buffer[EM_ZCL_MAX_PAYLOAD_SIZE];
      emCborEncodeStart(&state, buffer, sizeof(buffer));

      if (!addRdConfiguration(&state, configuration)) {
        emberAfPluginZclCorePrintln("Unable to write structure");
        emZclRespond500InternalServerError(context->info);
        return;
      }

      emZclRespond205ContentCborState(context->info, &state);
      break;
    }
    case EMBER_COAP_CODE_POST:
      emZclRespond405MethodNotAllowed(context->info);
      break;
    case EMBER_COAP_CODE_PUT: {
      CborState state;
      emCborDecodeStart(&state, context->payload, context->payloadLength);
      configStruct_t configurationValue;
      if (!emCborDecodeStruct(&state, configSpec, &configurationValue)) {
        emberAfPluginZclCorePrintln("Unable to read structure");
        emZclRespond500InternalServerError(context->info);
        return;
      }
      EmberZclCoapEndpoint_t destination;
      if (emZclUriToCoapEndpoint(configurationValue.rdField, &destination) == 0) {
        emberAfPluginZclCorePrintln("Unable to parse destination");
        emZclRespond500InternalServerError(context->info);
        return;
      }

      // Validate request
      if (destination.port == configuration->server.port && (MEMCOMPARE(&destination.address, &configuration->server.address, sizeof(destination.address)) != 0)) {
        emberAfPluginZclCorePrintln("Destination does not match configID's destination");
        printDestination("1->", &destination);
        printDestination("2->", &configuration->server);
        emZclRespond400BadRequest(context->info);
        return;
      }

      switch (configurationValue.stateField) {
        case EMBER_RD_SERVER_STATE_NOT_REGISTERED:
        case EMBER_RD_SERVER_STATE_REGISTERED:
          if (configurationValue.stateField != configuration->state) {
            emberAfPluginZclCorePrintln("Illegal state transistion");
            emZclRespond400BadRequest(context->info);
            return;
          }
          break;
        case EMBER_RD_SERVER_STATE_PERFORMING_REGISTRATION:
          if (configuration->state == EMBER_RD_SERVER_STATE_REMOVING_REGISTRATION) {
            emberAfPluginZclCorePrintln("Illegal state transistion");
            emZclRespond400BadRequest(context->info);
            return;
          }
          break;
        case EMBER_RD_SERVER_STATE_REMOVING_REGISTRATION:
          break;
      }

      // Perform action
      switch (configurationValue.stateField) {
        case EMBER_RD_SERVER_STATE_PERFORMING_REGISTRATION:
          emberAfPluginZclCorePrintln("Performing registration %u", configuration->id);
          configuration->state = EMBER_RD_SERVER_STATE_PERFORMING_REGISTRATION;
          emberAfPluginResourceDirectoryClientRegister(&configuration->server.address, configuration->server.port);
          break;
        case EMBER_RD_SERVER_STATE_REMOVING_REGISTRATION: {
          if (configuration->state == EMBER_RD_SERVER_STATE_REGISTERED) {
            emberAfPluginZclCorePrintln("Removing registration %u", configuration->id);
            uint8_t uri[MAX_PATH_WITH_QUERY];
            uint8_t uriLength = sprintf((char *)uri, "rd/%x", configuration->serverConfId);
            assert(uriLength < MAX_PATH_WITH_QUERY);

            emberAfPluginZclCorePrintln("Unregistering with resource directory (uri: %s)", uri);
            EmberStatus status = emZclSend(&configuration->server,
                                           EMBER_COAP_CODE_DELETE,
                                           uri,
                                           NULL,
                                           0,
                                           unregistrationResponseHandler,
                                           NULL,
                                           0,
                                           false);
            if (status != EMBER_SUCCESS) {
              emberAfPluginZclCorePrintln("Unable to send unregistration");
              emZclRespond500InternalServerError(context->info);
              return;
            }
          } else {
            configuration->state = EMBER_RD_SERVER_STATE_NOT_REGISTERED;
          }
        }
        break;
      }
      uint8_t path[12]; // "/rd/conf/<2-digit-hex>" = 12 chars
      sprintf((char *)path, "/rd/conf/%X", configuration->id);
      emZclRespond204ChangedWithPath(context->info, path);
      break;
    }
    case EMBER_COAP_CODE_DELETE: {
      if (configuration->state != EMBER_RD_SERVER_STATE_NOT_REGISTERED) {
        emZclRespond400BadRequest(context->info);
        return;
      }

      emBufferQueueRemove(&configurationList, configFinger);
      emZclRespond202Deleted(context->info);
      break;
    }
    default:
      assert(0);
  }
}

static EmberStatus sendRegistrationBlock(EmberZclCoapEndpoint_t *destination, uint8_t *payload, int16_t payloadLength, uint8_t blockLogSize, uint8_t blockNum)
{
  uint8_t uri[MAX_PATH_WITH_QUERY];
  uint8_t uriLength = constructRegistrationUri(uri);
  assert(uriLength < MAX_PATH_WITH_QUERY);

  EmberCoapOption sendOptions[2];

  // Options must be added in the increasing order of Option Number
  emberInitCoapOption(&(sendOptions[0]),
                      EMBER_COAP_OPTION_CONTENT_FORMAT,
                      EMBER_COAP_CONTENT_FORMAT_LINK_FORMAT_PLUS_CBOR);
  int16_t offset = blockNum * (1 << blockLogSize);
  bool haveMore = payloadLength > ((blockNum + 1) * (1 << blockLogSize));
  emberInitCoapOption(&(sendOptions[1]),
                      EMBER_COAP_OPTION_BLOCK1,
                      emberBlockOptionValue(haveMore,
                                            blockLogSize,
                                            blockNum));
  emberAfPluginZclCorePrintln("Sending registration blocksize=%u, blockNum=%u", blockLogSize, blockNum);
  return emZclSendWithOptions(destination,
                              EMBER_COAP_CODE_POST,
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

static void registrationResponseHandler(EmberCoapStatus status,
                                        EmberCoapCode code,
                                        EmberCoapReadOptions *options,
                                        uint8_t *payload,
                                        uint16_t payloadLength,
                                        EmberCoapResponseInfo *info)
{
  switch (status) {
    case EMBER_COAP_MESSAGE_TIMED_OUT:
    case EMBER_COAP_MESSAGE_RESET:
      // Since our discovery is multicast, we get the TIMED_OUT status when we
      // are done receiving responses (note that we could have received no
      // responses). If we found a server, we should have already moved on to the
      // next state. Otherwise, then we keep trying to discover a server.
      emberAfPluginZclCorePrintln("Timed out while registering with server");
      break;
    case EMBER_COAP_MESSAGE_ACKED:
      // This means that our request was ACK'd, but the actual response is coming.
      break;
    case EMBER_COAP_MESSAGE_RESPONSE: {
      rdConfiguration_t *configuration = findConfigByDestination(NULL, &info->remoteAddress, info->remotePort);
      if (configuration == NULL) {
        emberAfPluginZclCorePrintln("Registration failure: No local configuration ID");
        return;
      }
      if (!emberCoapIsSuccessResponse(code)) {
        emberAfPluginZclCorePrintln("Registration failure: code 0x%X", code);
        configuration->state = EMBER_RD_SERVER_STATE_NOT_REGISTERED;
        return;
      }
      if (code == EMBER_COAP_CODE_231_CONTINUE) {
        EmberCoapBlockOption blkOptions;
        if (!emberReadBlockOption(options, EMBER_COAP_OPTION_BLOCK1, &blkOptions)) {
          emberAfPluginZclCorePrintln("Registration failure: Unable to read block1 options");
          configuration->state = EMBER_RD_SERVER_STATE_NOT_REGISTERED;
          return;
        }
        sendRegistrationBlock(&configuration->server, info->applicationData, info->applicationDataLength, blkOptions.logSize, blkOptions.number + 1);
      } else {
        uint8_t path[MAX_PATH_WITH_QUERY];
        int16_t pathLen = emberReadLocationPath(options, path, sizeof(path));
        path[pathLen] = 0;   // null terminator
        uint8_t *finger = &path[3];   // path = "rd/####"
        uintmax_t confId;
        emHexStringToInt(finger, pathLen - 3, &confId);
        assert(confId < (1 << (8 * sizeof(configuration->serverConfId))));   // make sure value fits
        configuration->serverConfId = confId;
        emberAfPluginZclCorePrintln("Registration success (local: %x, remote: %x)", configuration->id, configuration->serverConfId);
        configuration->state = EMBER_RD_SERVER_STATE_REGISTERED;
      }
    }
    break;
    default:
      assert(false);
  }
}

static void sendRegistration(EmberZclCoapEndpoint_t *destination)
{
  Buffer payloadBuffer = emAllocateBuffer(MAX_REGISTRATION_PAYLOAD);
  uint8_t *payload = (uint8_t *)emGetBufferPointer(payloadBuffer);
  int16_t payloadLength = constructRegistrationPayload(payload, emGetBufferLength(payloadBuffer));
  assert(payloadLength <= MAX_REGISTRATION_PAYLOAD);

  if (payloadLength == -1) {
    return;
  }

  emSetBufferLength(payloadBuffer, payloadLength); // shrink buffer to size

  emberAfPluginZclCorePrintln("Registering with resource directory");
  printDestination("destination: ", destination);

  #define REGISTRATION_BLOCK_LOGSIZE 9
  EmberStatus status = sendRegistrationBlock(destination, payload, payloadLength, REGISTRATION_BLOCK_LOGSIZE, 0);

  if (status != EMBER_SUCCESS) {
    emberAfPluginZclCorePrintln("ERR: Registration failed: 0x%x", status);
  } else {
    emberAfPluginZclCorePrintln("Registration sent");
  }
}

void resourceDirectoryDtlsSessionIdReturn(uint8_t sessionId)
{
  if (sessionId == EMBER_NULL_SESSION_ID) {
    emberAfPluginZclCorePrintln("No DTLS session was established to resource directory");
    return;
  }

  emberAfPluginZclCorePrintln("DTLS Session ID was returned: %u", sessionId);
  EmberIpv6Address address;
  if (emberZclDtlsManagerGetAddressBySessionId(sessionId, &address) != EMBER_SUCCESS) {
    emberAfPluginZclCorePrintln("Remote address retrieval failure");
    return;
  }
  uint16_t port;
  if (emberZclDtlsManagerGetPortBySessionId(sessionId, &port) != EMBER_SUCCESS) {
    emberAfPluginZclCorePrintln("Remote port retrieval failure");
    return;
  }
  EmberZclUid_t uid;
  if (emberZclDtlsManagerGetUidBySessionId(sessionId, &uid) != EMBER_SUCCESS) {
    emberAfPluginZclCorePrintln("UID retrieval failure");
    return;
  }
  rdConfiguration_t *configuration = findConfigByDestination(&uid, &address, port);
  if (configuration == NULL) {
    emberAfPluginZclCorePrint("Configuration retrieval failure a=[");
    emberAfPluginZclCoreDebugExec(emberAfPrintIpv6Address(&address));
    emberAfPluginZclCorePrint("]:%u || uid=", port);
    emberAfPluginZclCorePrintBuffer(uid.bytes, sizeof(uid), false);
    emberAfPluginZclCorePrintln("");
    return;
  }

  if (MEMCOMPARE(configuration->server.uid.bytes, uid.bytes, sizeof(uid.bytes)) != 0) {
    configuration->server.flags |= EMBER_ZCL_HAVE_UID_FLAG;
    MEMCOPY(configuration->server.uid.bytes, uid.bytes, sizeof(uid.bytes));
  }

  sendRegistration(&configuration->server);
}

void emberAfPluginResourceDirectoryClientRegister(EmberIpv6Address *resourceDirectoryIp, uint16_t resourceDirectoryPort)
{
  emberAfPluginZclCorePrint("Registering to server ip=");
  emberAfPluginZclCoreDebugExec(emberAfPrintIpv6Address(resourceDirectoryIp));
  emberAfPluginZclCorePrintln(" p=%d", resourceDirectoryPort);

  emberZclDtlsManagerGetConnection(resourceDirectoryIp, resourceDirectoryPort, EMBER_DTLS_MODE_CERT, resourceDirectoryDtlsSessionIdReturn);
}

static uint8_t constructRegistrationUri(uint8_t *uri)
{
  uint8_t *finger = uri;
  finger += sprintf((char *)finger, "/rd?");
  finger += sprintf((char *)finger, "ep=ni:///sha-256;");
  uint16_t uidLength = emZclUidToBase64Url(&emZclUid, EMBER_ZCL_UID_BITS, finger);
  assert(uidLength != 0);
  finger += uidLength;

  return finger - uri;
}

static int16_t constructRegistrationPayload(uint8_t *payload, uint16_t payloadLength)
{
  EmZclDiscPayloadContext_t dpc;
  emZclInitDiscPayloadContext(&dpc, EMBER_COAP_CONTENT_FORMAT_LINK_FORMAT_PLUS_CBOR, payload, payloadLength);
  dpc.startPayload(&dpc);

  uint8_t i;
  emberAfPluginZclCorePrintln("Constructing registration payload of %u elements.", emZclEndpointCount);
  for (i = 0; i < emZclEndpointCount; i++) {
    const EmZclEndpointEntry_t *epEntry = &emZclEndpointTable[i];
    const EmberZclClusterSpec_t **clusterSpecs = epEntry->clusterSpecs;
    const EmberZclClusterSpec_t *spec = NULL;
    spec = *clusterSpecs;

    emberAfPluginZclCorePrintln("  - Adding endpointId: %u", epEntry->endpointId);
    while (spec != NULL) {
      emberAfPluginZclCorePrintln("    - Adding clusterSpec: 0x%x%c", spec->id, spec->role ? 's' : 'c');
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
    emberAfPluginZclCorePrintln("Payload length: %u", dpc.payloadLength(&dpc));
    return dpc.payloadLength(&dpc);
  } else {
    emberAfPluginZclCorePrintln("ERR: Failed to construct registration payload 0x%x", dpc.status);
    return -1;
  }
}
