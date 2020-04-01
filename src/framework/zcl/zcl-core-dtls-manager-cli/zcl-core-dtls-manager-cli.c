/***************************************************************************//**
 * @file
 * @brief
 ******************************************************************************/

#include PLATFORM_HEADER
#include CONFIGURATION_HEADER
#include CHIP_AF_API_ZCL_CORE
#include CHIP_AF_API_COMMAND_INTERPRETER2
#include CHIP_AF_API_ZCL_CORE_DTLS_MANAGER
#include CHIP_AF_API_DEBUG_PRINT

// ----------------------------------------------------------------------------
// ZCL DTLS Session Manager CLI

// zcl dtls-manager certs [no arguments]
void chZclCliDtlsManagerCertsCommand(void)
{
  chipSetDtlsDeviceCertificate(NULL, NULL); // Use compiled certificates
}

// zcl dtls-manager psk <ip> <psk> <identity>
void chZclCliDtlsSetPskCommand(void)
{
  ChipIpv6Address address;
  if (chipGetIpArgument(0, (uint8_t *) &address.bytes)) {
    uint8_t keyLength;
    uint8_t *key = chipStringCommandArgument(1, &keyLength);
    uint8_t identityLength;
    uint8_t *identity = chipStringCommandArgument(2, &identityLength);
    chipSetDtlsPresharedKey((const uint8_t *) key,
                             keyLength,
                             (const uint8_t *) identity,
                             identityLength,
                             &address);
  }
}

static void dtlsSessionOpen(uint8_t sessionId)
{
  chipAfAppPrintln("DTLS session opened, session id:0x%x", sessionId);
}

// zcl dtls-manager open <mode> <ip> <port>
void chZclCliDtlsManagerOpenCommand(void)
{
  uint8_t mode = chipUnsignedCommandArgument(0);
  ChipIpv6Address address;
  if (chipGetIpArgument(1, (uint8_t *) &address.bytes)) {
    uint16_t port = chipUnsignedCommandArgument(2);
    ChipStatus status =
      chipZclDtlsManagerGetConnection(&address, port, mode, &dtlsSessionOpen);
    if (status == CHIP_SUCCESS) {
      chipAfAppPrintln("DTLS session opening.");
    } else {
      chipAfAppPrintln("DTLS session open failed, status:0x%x", status);
    }
  }
}

// zcl dtls-manager close-all
void chZclCliDtlsManagerCloseAllCommand(void)
{
  chipAfAppPrintln("Close all DTLS sessions.");
  chipZclDtlsManagerCloseAllConnections();
}

// zcl dtls-manager list
void chZclCliDtlsManagerListCommand(void)
{
  chipAfAppPrintln("DTLS Sessions:");
  uint8_t firstSessionId = chipGetDtlsConnectionNextSessionId(CHIP_NULL_SESSION_ID);
  if (firstSessionId == CHIP_NULL_SESSION_ID) {
    return;
  }

  uint8_t curSessionId = firstSessionId;
  do {
    chipAfAppPrint("%d:", curSessionId);
    ChipIpv6Address address;
    if (chipZclDtlsManagerGetAddressBySessionId(curSessionId, &address) == CHIP_SUCCESS) {
      chipAfAppPrint(" a=[");
      chipAfAppDebugExec(chipAfPrintIpv6Address(&address));
      chipAfAppPrint("]");
      uint16_t port;
      if (chipZclDtlsManagerGetPortBySessionId(curSessionId, &port) == CHIP_SUCCESS) {
        chipAfAppPrint(":%u", port);
      }
    }
    ChipZclUid_t uid;
    if (chipZclDtlsManagerGetUidBySessionId(curSessionId, &uid) == CHIP_SUCCESS) {
      chipAfAppPrint(" uid=");
      chipAfAppPrintBuffer(uid.bytes, sizeof(uid), false);
    }
    chipAfAppPrintln("");
  } while (((curSessionId = chipGetDtlsConnectionNextSessionId(curSessionId))
            != CHIP_NULL_SESSION_ID) && (curSessionId != firstSessionId));
}
