/***************************************************************************//**
 * @file
 * @brief
 ******************************************************************************/

#include PLATFORM_HEADER
#include CONFIGURATION_HEADER
#include EMBER_AF_API_ZCL_CORE
#include EMBER_AF_API_COMMAND_INTERPRETER2
#include EMBER_AF_API_ZCL_CORE_DTLS_MANAGER
#include EMBER_AF_API_DEBUG_PRINT

// ----------------------------------------------------------------------------
// ZCL DTLS Session Manager CLI

// zcl dtls-manager certs [no arguments]
void emZclCliDtlsManagerCertsCommand(void)
{
  emberSetDtlsDeviceCertificate(NULL, NULL); // Use compiled certificates
}

// zcl dtls-manager psk <ip> <psk> <identity>
void emZclCliDtlsSetPskCommand(void)
{
  EmberIpv6Address address;
  if (emberGetIpArgument(0, (uint8_t *) &address.bytes)) {
    uint8_t keyLength;
    uint8_t *key = emberStringCommandArgument(1, &keyLength);
    uint8_t identityLength;
    uint8_t *identity = emberStringCommandArgument(2, &identityLength);
    emberSetDtlsPresharedKey((const uint8_t *) key,
                             keyLength,
                             (const uint8_t *) identity,
                             identityLength,
                             &address);
  }
}

static void dtlsSessionOpen(uint8_t sessionId)
{
  emberAfAppPrintln("DTLS session opened, session id:0x%x", sessionId);
}

// zcl dtls-manager open <mode> <ip> <port>
void emZclCliDtlsManagerOpenCommand(void)
{
  uint8_t mode = emberUnsignedCommandArgument(0);
  EmberIpv6Address address;
  if (emberGetIpArgument(1, (uint8_t *) &address.bytes)) {
    uint16_t port = emberUnsignedCommandArgument(2);
    EmberStatus status =
      emberZclDtlsManagerGetConnection(&address, port, mode, &dtlsSessionOpen);
    if (status == EMBER_SUCCESS) {
      emberAfAppPrintln("DTLS session opening.");
    } else {
      emberAfAppPrintln("DTLS session open failed, status:0x%x", status);
    }
  }
}

// zcl dtls-manager close-all
void emZclCliDtlsManagerCloseAllCommand(void)
{
  emberAfAppPrintln("Close all DTLS sessions.");
  emberZclDtlsManagerCloseAllConnections();
}

// zcl dtls-manager list
void emZclCliDtlsManagerListCommand(void)
{
  emberAfAppPrintln("DTLS Sessions:");
  uint8_t firstSessionId = emberGetDtlsConnectionNextSessionId(EMBER_NULL_SESSION_ID);
  if (firstSessionId == EMBER_NULL_SESSION_ID) {
    return;
  }

  uint8_t curSessionId = firstSessionId;
  do {
    emberAfAppPrint("%d:", curSessionId);
    EmberIpv6Address address;
    if (emberZclDtlsManagerGetAddressBySessionId(curSessionId, &address) == EMBER_SUCCESS) {
      emberAfAppPrint(" a=[");
      emberAfAppDebugExec(emberAfPrintIpv6Address(&address));
      emberAfAppPrint("]");
      uint16_t port;
      if (emberZclDtlsManagerGetPortBySessionId(curSessionId, &port) == EMBER_SUCCESS) {
        emberAfAppPrint(":%u", port);
      }
    }
    EmberZclUid_t uid;
    if (emberZclDtlsManagerGetUidBySessionId(curSessionId, &uid) == EMBER_SUCCESS) {
      emberAfAppPrint(" uid=");
      emberAfAppPrintBuffer(uid.bytes, sizeof(uid), false);
    }
    emberAfAppPrintln("");
  } while (((curSessionId = emberGetDtlsConnectionNextSessionId(curSessionId))
            != EMBER_NULL_SESSION_ID) && (curSessionId != firstSessionId));
}
