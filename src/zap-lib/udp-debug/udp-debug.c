/***************************************************************************//**
 * @file
 * @brief
 ******************************************************************************/

#include PLATFORM_HEADER
#include CONFIGURATION_HEADER
#include EMBER_AF_API_STACK
#ifdef EMBER_AF_API_DEBUG_PRINT
  #include EMBER_AF_API_DEBUG_PRINT
#endif

#ifndef ALIAS
  #define ALIAS(x) x
#endif

void ALIAS(emberUdpHandler)(const uint8_t * destination,
                            const uint8_t * source,
                            uint16_t localPort,
                            uint16_t remotePort,
                            const uint8_t * payload,
                            uint16_t payloadLength)
{
  emberAfCorePrint("UDP RX:");

  emberAfCorePrint(" s=");
  emberAfCoreDebugExec(
    emberAfPrintIpv6Address((const EmberIpv6Address *)source)
    );

  emberAfCorePrint(" d=");
  emberAfCoreDebugExec(
    emberAfPrintIpv6Address((const EmberIpv6Address *)destination)
    );

  emberAfCorePrint(" l=%u", localPort);

  emberAfCorePrint(" r=%u", remotePort);

  emberAfCorePrint(" [");
  emberAfCorePrintBuffer(payload, payloadLength, false);
  emberAfCorePrintln("]");
}
