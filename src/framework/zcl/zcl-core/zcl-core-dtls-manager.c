/***************************************************************************//**
 * @file
 * @brief
 ******************************************************************************/

#include PLATFORM_HEADER
#include CONFIGURATION_HEADER
#ifdef MBEDTLS_CONFIG_FILE
  #include MBEDTLS_CONFIG_FILE
#endif
#include CHIP_AF_API_STACK
#include CHIP_AF_API_BUFFER_MANAGEMENT
#include CHIP_AF_API_BUFFER_QUEUE
#include CHIP_AF_API_ZCL_CORE
#include CHIP_AF_API_ZCL_CORE_DTLS_MANAGER

#include "app/coap/coap.h"
#include "stack/ip/tls/tls-sha256.h"

//----------------------------------------------------------------
// Struct holding information about a DTLS session.
typedef struct {
  // This is used only for matching up with the correct
  // chipOpenDtlsConnectionReturn() and
  // chipDtlsSecureSessionEstablished() callbacks.
  ChipIpv6Address remoteAddress;
  uint16_t remotePort;

  // Notify the app that a connection has been established. This may need to
  // turn into a linked list if we have multiple plugins trying to connect to the
  // same host at the same time.
  void (*connectionResponseHandler)(uint8_t);
} ResponseHandlerToCall_t;

//----------------------------------------------------------------
// A queue of buffers holding ResponseHandlerToCall_t structs.
static Buffer responseHandlersList = NULL_BUFFER;

//----------------------------------------------------------------
void chZclDtlsManagerMarkBuffers(void)
{
  emMarkBuffer(&responseHandlersList);
}

// Called to get the session ID when sending a message.
uint8_t chipZclDtlsManagerGetSessionIdByAddress(const ChipIpv6Address *remoteAddress,
                                                 uint16_t remotePort)
{
  return chipGetSecureDtlsSessionId(remoteAddress, CHIP_COAP_SECURE_PORT, remotePort);
}

ChipStatus chipZclDtlsManagerGetAddressBySessionId(const uint8_t sessionId,
                                                     ChipIpv6Address *remoteAddress)
{
  return chipGetDtlsConnectionPeerAddressBySessionId(sessionId, remoteAddress);
}

ChipStatus chipZclDtlsManagerGetPortBySessionId(const uint8_t sessionId,
                                                  uint16_t *remotePort)
{
  return chipGetDtlsConnectionPeerPortBySessionId(sessionId, remotePort);
}

#define PUBLIC_KEY_MAX_SIZE 65 // Can be expanded if necessary
ChipStatus chipZclDtlsManagerGetUidBySessionId(const uint8_t sessionId,
                                                 ChipZclUid_t *remoteUid)
{
  uint8_t publicKey[PUBLIC_KEY_MAX_SIZE];
  uint16_t publicKeySize;

  ChipStatus status = chipGetDtlsConnectionPeerPublicKeyBySessionId(sessionId,
                                                                      publicKey,
                                                                      PUBLIC_KEY_MAX_SIZE,
                                                                      &publicKeySize);

  if (status != CHIP_SUCCESS) {
    return status;
  }

  Sha256State state;
  emSha256Start(&state);
  emSha256HashBytes(&state, (const uint8_t *)"zcl.uid", 7);
  emSha256HashBytes(&state, publicKey, publicKeySize);
  emSha256Finish(&state, remoteUid->bytes);

  return CHIP_SUCCESS;
}

uint8_t chipZclDtlsManagerGetSessionIdByUid(const ChipZclUid_t *remoteUid, uint16_t remotePort)
{
  uint8_t curSessionId = CHIP_NULL_SESSION_ID;
  uint8_t firstSessionId = CHIP_NULL_SESSION_ID;
  while ((curSessionId = chipGetDtlsConnectionNextSessionId(curSessionId))
         != CHIP_NULL_SESSION_ID) {
    if (firstSessionId == CHIP_NULL_SESSION_ID) {
      firstSessionId = curSessionId;
    } else {
      if (firstSessionId == curSessionId) { // wrapped around
        return CHIP_NULL_SESSION_ID;
      }
    }
    ChipZclUid_t curUid;
    uint16_t curPort;
    if ((chipZclDtlsManagerGetUidBySessionId(curSessionId, &curUid) == CHIP_SUCCESS) && (chipZclDtlsManagerGetPortBySessionId(curSessionId, &curPort) == CHIP_SUCCESS)) {
      if (((remotePort == 0) || (curPort == remotePort)) && (memcmp(&curUid.bytes, &remoteUid->bytes, sizeof(curUid.bytes)) == 0)) {
        return curSessionId;
      }
    }
  }
  return CHIP_NULL_SESSION_ID;
}

static Buffer findSessionBuffer(const ChipIpv6Address *remoteAddress,
                                uint16_t remotePort)
{
  for (Buffer buffer = emBufferQueueHead(&responseHandlersList);
       buffer != NULL_BUFFER;
       buffer = emBufferQueueNext(&responseHandlersList, buffer)) {
    ResponseHandlerToCall_t *entry = (ResponseHandlerToCall_t *)emGetBufferPointer(buffer);
    if (entry->remotePort != remotePort) {
      continue;
    }
    if (memcmp(&(entry->remoteAddress), remoteAddress, sizeof(entry->remoteAddress)) != 0) {
      continue;
    }
    // chipAfPluginZclCorePrint("Found Session. Address: ");
    // chipAfPluginZclCoreDebugExec(chipAfPrintIpv6Address(&entry->remoteAddress));
    // chipAfPluginZclCorePrintln(" Port: %d", entry->remotePort);
    return buffer;
  }
  return NULL_BUFFER;
}

static void callResponseHandler(const ChipIpv6Address *remoteAddress,
                                uint16_t remotePort,
                                uint8_t sessionId)
{
  Buffer buffer;
  while ((buffer = findSessionBuffer(remoteAddress, remotePort)) != NULL_BUFFER) {
    ResponseHandlerToCall_t *entry = (ResponseHandlerToCall_t *)emGetBufferPointer(buffer);
    if (entry->connectionResponseHandler != NULL) {
      entry->connectionResponseHandler(sessionId);
    }
    emBufferQueueRemove(&responseHandlersList, buffer);
  }
}

void chipOpenDtlsConnectionReturn(uint32_t result,
                                   const ChipIpv6Address *remoteAddress,
                                   uint16_t localPort,
                                   uint16_t remotePort)
{
  if (result != 0) {
    #ifdef MBEDTLS_ERROR_C
    char error_buf[100];
    mbedtls_strerror(result, error_buf, 100);
    chipAfPluginZclCorePrintln("something went wrong in creating a dtls session, mbedtls err: %s",
                                error_buf);
    #else
    chipAfPluginZclCorePrintln("something went wrong in creating a dtls session, mbedtls err: %d",
                                result);
    #endif
    callResponseHandler(remoteAddress, remotePort, CHIP_NULL_SESSION_ID);
  } else {
    chipAfPluginZclCorePrint("Opening DTLS connection with Port=%d, address=", remotePort);
    chipAfPluginZclCoreDebugExec(chipAfPrintIpv6Address(remoteAddress));
    chipAfPluginZclCorePrintln("");
  }
}

void chipDtlsSecureSessionEstablished(uint8_t flags,
                                       uint8_t sessionId,
                                       const ChipIpv6Address *localAddress,
                                       const ChipIpv6Address *remoteAddress,
                                       uint16_t localPort,
                                       uint16_t remotePort)
{
  chipAfPluginZclCorePrint("secure session available: %d (%s). Port=%d, address=",
                            sessionId,
                            flags ? "server" : "client",
                            remotePort);
  chipAfPluginZclCoreDebugExec(chipAfPrintIpv6Address(remoteAddress));
  chipAfPluginZclCorePrint(", uid=");
  ChipZclUid_t uid;
  if (chipZclDtlsManagerGetUidBySessionId(sessionId, &uid) != CHIP_SUCCESS) {
    chipAfPluginZclCorePrintln("UID retrieval failure");
  } else {
    chipAfPluginZclCorePrintBuffer(uid.bytes, sizeof(uid.bytes), false);
    chipAfPluginZclCorePrintln("");
  }
  callResponseHandler(remoteAddress, remotePort, sessionId);
}

ChipStatus chipZclDtlsManagerGetConnection(const ChipIpv6Address *remoteAddress,
                                             uint16_t remotePort,
                                             ChipDtlsMode mode,
                                             void (*returnHandle)(uint8_t))
{
  chipAfPluginZclCorePrint("Looking up DTLS connection with Port=%d, address=", remotePort);
  chipAfPluginZclCoreDebugExec(chipAfPrintIpv6Address(remoteAddress));
  chipAfPluginZclCorePrintln("");
  uint8_t sessionId = chipZclDtlsManagerGetSessionIdByAddress(remoteAddress, remotePort);

  if (sessionId != CHIP_NULL_SESSION_ID) {
    chipAfPluginZclCorePrintln("Found existing session: %d", sessionId);
    if (returnHandle != NULL) {
      returnHandle(sessionId);
    }
    return CHIP_SUCCESS;
  }

  chipAfPluginZclCorePrintln("Opening new DTLS connection");
  if (findSessionBuffer(remoteAddress, remotePort) == NULL_BUFFER) {
    ResponseHandlerToCall_t session;
    session.remoteAddress = *remoteAddress;
    session.remotePort = remotePort;
    session.connectionResponseHandler = returnHandle;
    Buffer buffer = emFillBuffer((uint8_t *)&session, sizeof(session));
    if (buffer == NULL_BUFFER) {
      return CHIP_ERR_FATAL;
    }
    emBufferQueueAdd(&responseHandlersList, buffer);
  }
  chipOpenDtlsConnection(mode, remoteAddress, CHIP_COAP_SECURE_PORT, remotePort);
  return CHIP_SUCCESS;
}

void chipProcessCoap(const uint8_t *message,
                      uint16_t messageLength,
                      ChipCoapRequestInfo *info)
{
  if (messageLength > 0) {
    chipAfPluginZclCorePrintln("received secure data.");
    emProcessCoapMessage(NULL_BUFFER,
                         message,
                         messageLength,
                         (CoapRequestHandler) & chipCoapRequestHandler,
                         info);
  } else {
    chipAfPluginZclCorePrintln("failed to parse secure data.");
  }
}

void chipZclDtlsManagerCloseAllConnections(void)
{
  uint8_t curSessionId;
  while ((curSessionId = chipGetDtlsConnectionNextSessionId(CHIP_NULL_SESSION_ID))
         != CHIP_NULL_SESSION_ID) {
    chipAfPluginZclCorePrintln("Closing session: %d", curSessionId);
    chipCloseDtlsConnection(curSessionId);
  }
}

void chipCloseDtlsConnectionReturn(uint8_t sessionId, ChipStatus status)
{
  if (status == CHIP_SUCCESS) {
    chipAfPluginZclCorePrintln("closed session %d successfully.", sessionId);
  } else {
    chipAfPluginZclCorePrintln("failure to close session status: %d", status);
  }
}
