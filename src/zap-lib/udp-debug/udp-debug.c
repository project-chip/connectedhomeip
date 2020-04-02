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
