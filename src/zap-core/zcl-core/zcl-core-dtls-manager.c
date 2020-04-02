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
#ifdef MBEDTLS_CONFIG_FILE
  #include MBEDTLS_CONFIG_FILE
#endif
#include EMBER_AF_API_STACK
#include EMBER_AF_API_BUFFER_MANAGEMENT
#include EMBER_AF_API_BUFFER_QUEUE
#include EMBER_AF_API_ZCL_CORE
#include EMBER_AF_API_ZCL_CORE_DTLS_MANAGER

#include "app/coap/coap.h"
#include "stack/ip/tls/tls-sha256.h"

//----------------------------------------------------------------
// Struct holding information about a DTLS session.
typedef struct {
  // This is used only for matching up with the correct
  // emberOpenDtlsConnectionReturn() and
  // emberDtlsSecureSessionEstablished() callbacks.
  EmberIpv6Address remoteAddress;
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
void emZclDtlsManagerMarkBuffers(void)
{
  emMarkBuffer(&responseHandlersList);
}

// Called to get the session ID when sending a message.
uint8_t emberZclDtlsManagerGetSessionIdByAddress(const EmberIpv6Address *remoteAddress,
                                                 uint16_t remotePort)
{
  return emberGetSecureDtlsSessionId(remoteAddress, EMBER_COAP_SECURE_PORT, remotePort);
}

EmberStatus emberZclDtlsManagerGetAddressBySessionId(const uint8_t sessionId,
                                                     EmberIpv6Address *remoteAddress)
{
  return emberGetDtlsConnectionPeerAddressBySessionId(sessionId, remoteAddress);
}

EmberStatus emberZclDtlsManagerGetPortBySessionId(const uint8_t sessionId,
                                                  uint16_t *remotePort)
{
  return emberGetDtlsConnectionPeerPortBySessionId(sessionId, remotePort);
}

#define PUBLIC_KEY_MAX_SIZE 65 // Can be expanded if necessary
EmberStatus emberZclDtlsManagerGetUidBySessionId(const uint8_t sessionId,
                                                 EmberZclUid_t *remoteUid)
{
  uint8_t publicKey[PUBLIC_KEY_MAX_SIZE];
  uint16_t publicKeySize;

  EmberStatus status = emberGetDtlsConnectionPeerPublicKeyBySessionId(sessionId,
                                                                      publicKey,
                                                                      PUBLIC_KEY_MAX_SIZE,
                                                                      &publicKeySize);

  if (status != EMBER_SUCCESS) {
    return status;
  }

  Sha256State state;
  emSha256Start(&state);
  emSha256HashBytes(&state, (const uint8_t *)"zcl.uid", 7);
  emSha256HashBytes(&state, publicKey, publicKeySize);
  emSha256Finish(&state, remoteUid->bytes);

  return EMBER_SUCCESS;
}

uint8_t emberZclDtlsManagerGetSessionIdByUid(const EmberZclUid_t *remoteUid, uint16_t remotePort)
{
  uint8_t curSessionId = EMBER_NULL_SESSION_ID;
  uint8_t firstSessionId = EMBER_NULL_SESSION_ID;
  while ((curSessionId = emberGetDtlsConnectionNextSessionId(curSessionId))
         != EMBER_NULL_SESSION_ID) {
    if (firstSessionId == EMBER_NULL_SESSION_ID) {
      firstSessionId = curSessionId;
    } else {
      if (firstSessionId == curSessionId) { // wrapped around
        return EMBER_NULL_SESSION_ID;
      }
    }
    EmberZclUid_t curUid;
    uint16_t curPort;
    if ((emberZclDtlsManagerGetUidBySessionId(curSessionId, &curUid) == EMBER_SUCCESS) && (emberZclDtlsManagerGetPortBySessionId(curSessionId, &curPort) == EMBER_SUCCESS)) {
      if (((remotePort == 0) || (curPort == remotePort)) && (memcmp(&curUid.bytes, &remoteUid->bytes, sizeof(curUid.bytes)) == 0)) {
        return curSessionId;
      }
    }
  }
  return EMBER_NULL_SESSION_ID;
}

static Buffer findSessionBuffer(const EmberIpv6Address *remoteAddress,
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
    // emberAfPluginZclCorePrint("Found Session. Address: ");
    // emberAfPluginZclCoreDebugExec(emberAfPrintIpv6Address(&entry->remoteAddress));
    // emberAfPluginZclCorePrintln(" Port: %d", entry->remotePort);
    return buffer;
  }
  return NULL_BUFFER;
}

static void callResponseHandler(const EmberIpv6Address *remoteAddress,
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

void emberOpenDtlsConnectionReturn(uint32_t result,
                                   const EmberIpv6Address *remoteAddress,
                                   uint16_t localPort,
                                   uint16_t remotePort)
{
  if (result != 0) {
    #ifdef MBEDTLS_ERROR_C
    char error_buf[100];
    mbedtls_strerror(result, error_buf, 100);
    emberAfPluginZclCorePrintln("something went wrong in creating a dtls session, mbedtls err: %s",
                                error_buf);
    #else
    emberAfPluginZclCorePrintln("something went wrong in creating a dtls session, mbedtls err: %d",
                                result);
    #endif
    callResponseHandler(remoteAddress, remotePort, EMBER_NULL_SESSION_ID);
  } else {
    emberAfPluginZclCorePrint("Opening DTLS connection with Port=%d, address=", remotePort);
    emberAfPluginZclCoreDebugExec(emberAfPrintIpv6Address(remoteAddress));
    emberAfPluginZclCorePrintln("");
  }
}

void emberDtlsSecureSessionEstablished(uint8_t flags,
                                       uint8_t sessionId,
                                       const EmberIpv6Address *localAddress,
                                       const EmberIpv6Address *remoteAddress,
                                       uint16_t localPort,
                                       uint16_t remotePort)
{
  emberAfPluginZclCorePrint("secure session available: %d (%s). Port=%d, address=",
                            sessionId,
                            flags ? "server" : "client",
                            remotePort);
  emberAfPluginZclCoreDebugExec(emberAfPrintIpv6Address(remoteAddress));
  emberAfPluginZclCorePrint(", uid=");
  EmberZclUid_t uid;
  if (emberZclDtlsManagerGetUidBySessionId(sessionId, &uid) != EMBER_SUCCESS) {
    emberAfPluginZclCorePrintln("UID retrieval failure");
  } else {
    emberAfPluginZclCorePrintBuffer(uid.bytes, sizeof(uid.bytes), false);
    emberAfPluginZclCorePrintln("");
  }
  callResponseHandler(remoteAddress, remotePort, sessionId);
}

EmberStatus emberZclDtlsManagerGetConnection(const EmberIpv6Address *remoteAddress,
                                             uint16_t remotePort,
                                             EmberDtlsMode mode,
                                             void (*returnHandle)(uint8_t))
{
  emberAfPluginZclCorePrint("Looking up DTLS connection with Port=%d, address=", remotePort);
  emberAfPluginZclCoreDebugExec(emberAfPrintIpv6Address(remoteAddress));
  emberAfPluginZclCorePrintln("");
  uint8_t sessionId = emberZclDtlsManagerGetSessionIdByAddress(remoteAddress, remotePort);

  if (sessionId != EMBER_NULL_SESSION_ID) {
    emberAfPluginZclCorePrintln("Found existing session: %d", sessionId);
    if (returnHandle != NULL) {
      returnHandle(sessionId);
    }
    return EMBER_SUCCESS;
  }

  emberAfPluginZclCorePrintln("Opening new DTLS connection");
  if (findSessionBuffer(remoteAddress, remotePort) == NULL_BUFFER) {
    ResponseHandlerToCall_t session;
    session.remoteAddress = *remoteAddress;
    session.remotePort = remotePort;
    session.connectionResponseHandler = returnHandle;
    Buffer buffer = emFillBuffer((uint8_t *)&session, sizeof(session));
    if (buffer == NULL_BUFFER) {
      return EMBER_ERR_FATAL;
    }
    emBufferQueueAdd(&responseHandlersList, buffer);
  }
  emberOpenDtlsConnection(mode, remoteAddress, EMBER_COAP_SECURE_PORT, remotePort);
  return EMBER_SUCCESS;
}

void emberProcessCoap(const uint8_t *message,
                      uint16_t messageLength,
                      EmberCoapRequestInfo *info)
{
  if (messageLength > 0) {
    emberAfPluginZclCorePrintln("received secure data.");
    emProcessCoapMessage(NULL_BUFFER,
                         message,
                         messageLength,
                         (CoapRequestHandler) & emberCoapRequestHandler,
                         info);
  } else {
    emberAfPluginZclCorePrintln("failed to parse secure data.");
  }
}

void emberZclDtlsManagerCloseAllConnections(void)
{
  uint8_t curSessionId;
  while ((curSessionId = emberGetDtlsConnectionNextSessionId(EMBER_NULL_SESSION_ID))
         != EMBER_NULL_SESSION_ID) {
    emberAfPluginZclCorePrintln("Closing session: %d", curSessionId);
    emberCloseDtlsConnection(curSessionId);
  }
}

void emberCloseDtlsConnectionReturn(uint8_t sessionId, EmberStatus status)
{
  if (status == EMBER_SUCCESS) {
    emberAfPluginZclCorePrintln("closed session %d successfully.", sessionId);
  } else {
    emberAfPluginZclCorePrintln("failure to close session status: %d", status);
  }
}
