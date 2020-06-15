/**
 *
 *    Copyright (c) 2020 Silicon Labs
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
 */
/***************************************************************************//**
 * @file
 * @brief CLI for the Key Establishment plugin.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "app/util/serial/command-interpreter2.h"

//-----------------------------------------------------------------------------
// Forward Declarations

void emAfKeyEstablishmentStartCommand(void);
void emAfKeyEstablishmentInterpanCommand(void);

//-----------------------------------------------------------------------------
// Globals Declarations

#if !defined(EMBER_AF_GENERATE_CLI)

static const char * cbkeStartCommandArguments[] = {
  "Target node ID",
  "Target node endpoint",
  NULL,
};

static const char * cbkeInterPanCommandArguments[] = {
  "Target PAN ID",
  "Target EUI64 (big endian)",
  NULL,
};

EmberCommandEntry emberAfPluginKeyEstablishmentCommands[] = {
  emberCommandEntryActionWithDetails("start",
                                     emAfKeyEstablishmentStartCommand,
                                     "vu",
                                     "Initiate key establishment with the target.",
                                     cbkeStartCommandArguments),
  emberCommandEntryActionWithDetails("interpan",
                                     emAfKeyEstablishmentInterpanCommand,
                                     "vb",
                                     "Initiate interpan key establishment with the target",
                                     cbkeInterPanCommandArguments),
  emberCommandEntryTerminator(),
};

#endif // EMBER_AF_GENERATE_CLI

//-----------------------------------------------------------------------------
// Functions

void emAfKeyEstablishmentStartCommand(void)
{
  EmberStatus status;
  EmberNodeId newPartnerId = (EmberNodeId)emberUnsignedCommandArgument(0);
  uint8_t endpoint = (uint8_t)emberUnsignedCommandArgument(1);
  emberAfCorePrintln("Starting %pment w/ 0x%2x, EP: 0x%x", \
                     "Key Establish",
                     newPartnerId,
                     endpoint);
  emberAfCoreFlush();
  emberSerialBufferTick();

  status = emberAfInitiateKeyEstablishment(newPartnerId, endpoint);
  emberAfCorePrintln("%p", (status == EMBER_SUCCESS ? "Success" : "Error"));
}

void emAfKeyEstablishmentInterpanCommand(void)
{
  EmberEUI64 eui64;
  EmberPanId panId = (EmberPanId)emberUnsignedCommandArgument(0);
  EmberStatus status;
  emberCopyBigEndianEui64Argument(1, eui64);

  emberAfCorePrint("Starting %pment w/ ", "Key Establish");
  emberAfCoreDebugExec(emberAfPrintBigEndianEui64(eui64));
  emberAfCorePrintln("");
  emberAfCoreFlush();
  emberSerialBufferTick();

  status = emberAfInitiateInterPanKeyEstablishment(panId, eui64);
  emberAfCorePrintln("%p", (status == EMBER_SUCCESS ? "Success" : "Error"));
}
