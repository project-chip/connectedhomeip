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
