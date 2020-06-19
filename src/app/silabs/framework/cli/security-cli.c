/**
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
 * @brief Routines to initiate key establishment, handle key establishment callbacks,
 * and print info about the security keys on the device.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/util/common.h"

#include "app/framework/util/service-discovery.h"

#include "app/util/serial/command-interpreter2.h"
#include "app/framework/cli/security-cli.h"
#include "app/framework/util/af-main.h"

#include "app/framework/security/af-security.h"
#include "stack/include/packet-buffer.h"

#ifdef EZSP_HOST
 #define setTcRejoinsUsingWellKnownKeyAllowed(allow) \
  (void)ezspSetPolicy(EZSP_TC_REJOINS_USING_WELL_KNOWN_KEY_POLICY, (allow))
 #define setTcRejoinsUsingWellKnownKeyTimeout(timeout) \
  (void)ezspSetConfigurationValue(EZSP_CONFIG_TC_REJOINS_USING_WELL_KNOWN_KEY_TIMEOUT_S, (timeout))
 #define getTransientKeyTableEntry ezspGetTransientKeyTableEntry
#else  // SoC
extern uint16_t emAllowTcRejoinsUsingWellKnownKeyTimeoutSec;
 #define setTcRejoinsUsingWellKnownKeyAllowed(allow) \
  emberSetTcRejoinsUsingWellKnownKeyAllowed((allow))
 #define setTcRejoinsUsingWellKnownKeyTimeout(timeout) \
  emAllowTcRejoinsUsingWellKnownKeyTimeoutSec = (timeout)
 #define getTransientKeyTableEntry emberGetTransientKeyTableEntry
#endif

//------------------------------------------------------------------------------
// Globals

EmberKeyData cliPreconfiguredLinkKey = DUMMY_KEY;
EmberKeyData cliNetworkKey           = DUMMY_KEY;

/**
 * @addtogroup cli
 * @{
 */
/**
 * @brief Commands used to change the default link or network key
 *        that will be used when forming or joining a network.
 *
 *        <b>changekey link &lt;key&gt;</b>
 *        - key - 16 byte array. The Link Key provided as a
 *          16 byte array.
 *
 *        <b>changekey network &lt;key&gt;</b>
 *        - key - 16 byte array. The Network Key provided
 *          as a 16 byte array.
 *
 */
#define EMBER_AF_DOXYGEN_CLI__SECURITY_CHANGEKEY_COMMANDS
/** @} END addtogroup */

/**
 * @addtogroup cli
 * @{
 */
/**
 * @brief Command used for key establishment.
 *
 *        <b>cbke start &lt;new partner id&gt; &lt;destination endpoint&gt;</b>
 *        - new partner ID - uint16_t. The 2 byte node ID of the partner
 *          with whom to start cbke.
 *
 *        <b>cbke interpan &lt;pan id&gt; &lt;eui64&gt;</b>
 *        - PAN ID - uint16_t. The 2 byte PAN ID that the target is located on.
 *        - eui64 - EmberEUI64. The 8 byte EUI64 of the target (big endian)
 *
 *        <b>cbke partner &lt;node id&gt; &lt;endpoint&gt;</b>
 *        - node ID - uint16_t. The two byte node ID of the device
 *          with whom to initiate key establishment.
 *        - endpoint - uint8_t. The endpoint on which to begin
 *          key establishment.
 *
 */
#define EMBER_AF_DOXYGEN_CLI__SECURITY_CBKE_COMMANDS
/** @} END addtogroup */

#ifndef EMBER_AF_GENERATE_CLI

EmberCommandEntry changeKeyCommands[] = {
  { "link", changeKeyCommand, "b" },
  { "network", changeKeyCommand, "b" },
  { NULL }
};

#endif // EMBER_AF_GENERATE_CLI

/**
 * @addtogroup security
 * @{
 */
/** @brief Commands used for security.
 *
 * <b>security mfg-token get</b>
 * <b>security mfg-token set &lt;Magic number&gt; &lt;EmberKeySettings value&gt;</b>
 *   - Magic number - uint32_t.  The 4-byte magic number
 *     EMBER_MFG_SECURITY_CONFIG_MAGIC_NUMBER to prevent accidental execution.
 *   - EmberKeySettings value - uint16_t. The 2-byte value for EmberKeySettings
 *     indicating key permissions.
 *
 */
#define EMBER_AF_DOXYGEN_CLI__SECURITY_MFG_TOKEN_COMMANDS
/** @} END addtogroup */

#ifndef EMBER_AF_HAS_SECURITY_PROFILE_NONE
void getSetMfgToken(void);

EmberCommandEntry mfgToken[] = {
  emberCommandEntryAction("get", getSetMfgToken, "",
                          "Print the MFG security config token"),
  emberCommandEntryAction("set", getSetMfgToken, "wv",
                          "Set the MFG Security config token.  CANNOT BE UNSET BY CLI."),
  emberCommandEntryTerminator(),
};

EmberCommandEntry emAfSecurityCommands[] = {
  emberCommandEntrySubMenu("mfg-token", mfgToken,
                           "Get/set the MFG Security config token"),
  emberCommandEntryTerminator(),
};

#endif

//------------------------------------------------------------------------------
// Forward Declarations

uint8_t printKeyTable(bool preconfiguredKey);
uint8_t printTransientKeyTable(void);
uint32_t getOutgoingApsFrameCounter(void);

//------------------------------------------------------------------------------

// ******************************************************
// changekey link    <16 byte key>
// changekey network <16 byte key>
// ******************************************************

// Changes the default link or network key that will be used when
// forming or joining a network.
void changeKeyCommand(void)
{
  if (EMBER_NO_NETWORK != emberNetworkState()) {
    emberAfCorePrintln("%pstack must be down.", "ERROR: ");
  } else {
    emberCopyKeyArgument(0,
                         emberCurrentCommand->name[0] == 'l'
                         ? &cliPreconfiguredLinkKey
                         : &cliNetworkKey);
    emberAfDebugPrintln("set key");
  }
}

// Key Establishment commands (cbke <command>) moved to
//   app/framework/plugin/key-establishment/key-establishment-cli.c

void printKeyInfo(void)
{
  uint8_t entriesUsed;
  EmberKeyStruct nwkKey;

#ifndef EMBER_AF_HAS_SECURITY_PROFILE_NONE
  {
    EmberMfgSecurityStruct config;
    if (EMBER_SUCCESS == emberGetMfgSecurityConfig(&config)) {
      if (!(config.keySettings & EMBER_KEY_PERMISSIONS_READING_ALLOWED)) {
        emberAfCorePrintln("\nWARNING:  READING OF KEY VALUES DISABLED BY MFG TOKEN\n");
        emberAfCoreFlush();
      }
    }
  }
#endif

  if ( EMBER_SUCCESS != emberGetKey(EMBER_CURRENT_NETWORK_KEY,
                                    &nwkKey)) {
    MEMSET((uint8_t*)&nwkKey, 0xFF, sizeof(EmberKeyStruct));
  }
  emberAfCorePrintln("%p out FC: %4x",
                     "NWK Key",
                     nwkKey.outgoingFrameCounter);
  emberAfCorePrintln("%p seq num: 0x%x",
                     "NWK Key",
                     nwkKey.sequenceNumber);
  emberAfCorePrint("%p: ",
                   "NWK Key");
  emberAfPrintZigbeeKey(emberKeyContents(&nwkKey.key));

  emberAfCorePrintln("%p out FC: %4x",
                     "Link Key",
                     getOutgoingApsFrameCounter());

  emberAfCorePrintln("TC %p", "Link Key");
  emberAfCoreFlush();
  printKeyTable(true);

  emberAfCorePrintln("%p Table", "Link Key");
  emberAfCoreFlush();
  entriesUsed = printKeyTable(false);
  UNUSED_VAR(entriesUsed);

  emberAfCorePrintln("%d/%d entries used.",
                     entriesUsed,
                     emberAfGetKeyTableSize());
  emberAfCoreFlush();

  emberAfCorePrintln("%p Table", "Transient Key");
  emberAfCoreFlush();

  entriesUsed = printTransientKeyTable();
  UNUSED_VAR(entriesUsed);

  emberAfCorePrintln("%d entr%s consuming %d packet buffer%s.",
                     entriesUsed,
                     entriesUsed > 1 ? "ies" : "y",
                     entriesUsed,
                     entriesUsed > 1 ? "s" : "");
  emberAfCoreFlush();
}

uint8_t printKeyTable(bool preconfiguredKey)
{
  uint8_t i;
  uint8_t entriesUsed = 0;
  uint8_t loopCount = (preconfiguredKey ? 1 : emberAfGetKeyTableSize());

  emberAfCorePrintln("Index IEEE Address         In FC     Type  Auth  Key");

  for (i = 0; i < loopCount; i++) {
    EmberKeyStruct entry;

    if (preconfiguredKey) {
      i = 0xFE; // last

      // Try to get whatever key type is stored in the pre-configured key slot.
      if (EMBER_SUCCESS
          != emberGetKey(EMBER_TRUST_CENTER_LINK_KEY, &entry) ) {
        continue;
      }
    } else if ( EMBER_SUCCESS != emberGetKeyTableEntry(i, &entry) ) {
      continue;
    } else {
      // MISRA requires ..else if.. to have a terminating else.
    }

    if (!preconfiguredKey) {
      emberAfCorePrint("%d     ", i);
    } else {
      emberAfCorePrint("-     ");
    }
    emberAfCoreDebugExec(emberAfPrintBigEndianEui64(entry.partnerEUI64));
    emberAfCorePrint("  %4x  ", entry.incomingFrameCounter);
    emberAfCorePrint("L     %c     ",
                     (entry.bitmask & EMBER_KEY_IS_AUTHORIZED
                      ? 'y'
                      : 'n'));

    emberAfPrintZigbeeKey(emberKeyContents(&(entry.key)));

    emberAfCoreFlush();
    entriesUsed++;
  }

  return entriesUsed;
}

uint8_t printTransientKeyTable(void)
{
  EmberStatus status;
  EmberTransientKeyData transientKeyData;
  uint8_t index = 0;

  emberAfCorePrintln("Index IEEE Address         In FC     TTL(s) Flag    Key    ");

  status = getTransientKeyTableEntry(index, &transientKeyData);

  while (status == EMBER_SUCCESS) {
    emberAfCorePrint("%d     ", index);
    emberAfCoreDebugExec(emberAfPrintBigEndianEui64(transientKeyData.eui64));
    emberAfCorePrint("  %4x  ", transientKeyData.incomingFrameCounter);
    emberAfCorePrint("0x%02X", transientKeyData.remainingTimeSeconds);
    emberAfCorePrint(" 0x%02X  ", transientKeyData.bitmask);
    emberAfPrintZigbeeKey(emberKeyContents(&(transientKeyData.keyData)));
    emberAfCorePrintln("");
    emberAfCoreFlush();

    index += 1;
    status = getTransientKeyTableEntry(index, &transientKeyData);
  }

  return index;
}

uint32_t getOutgoingApsFrameCounter(void)
{
  EmberKeyStruct entry;
  if (emberGetKey(EMBER_TRUST_CENTER_LINK_KEY, &entry) != EMBER_SUCCESS) {
    return 0xFFFFFFFFUL;
  }
  return entry.outgoingFrameCounter;
}

#ifndef EMBER_AF_HAS_SECURITY_PROFILE_NONE
void getSetMfgToken(void)
{
  EmberMfgSecurityStruct config;
  EmberStatus status;

  if (emberCurrentCommand->name[0] == 'g') {
    status = emberGetMfgSecurityConfig(&config);
    if (status == EMBER_SUCCESS) {
      emberAfSecurityPrintln("EmberKeySettings: 0x%2X", config.keySettings);
      emberAfSecurityPrintln("  Permissions: %p",
                             ((config.keySettings & EMBER_KEY_PERMISSIONS_READING_ALLOWED)
                              ? "Reading Allowed"
                              : ((config.keySettings & EMBER_KEY_PERMISSIONS_HASHING_ALLOWED)
                                 ? "Hashing only"
                                 : "NONE")));
    } else {
      emberAfSecurityPrintln("Error: Failed to get config, status: 0x%X", status);
    }
  } else {
    uint32_t magicNumber = emberUnsignedCommandArgument(0);
    config.keySettings = (EmberKeySettings)emberUnsignedCommandArgument(1);
    status = emberSetMfgSecurityConfig(magicNumber, &config);
  }

  if (status != EMBER_SUCCESS) {
    emberAfSecurityPrintln("Failed: 0x%X", status);
  }
}
#endif

void optionSecurityAllowTrustCenterRejoinUsingWellKnownKey(void)
{
  bool allow = (bool)emberUnsignedCommandArgument(0);
  setTcRejoinsUsingWellKnownKeyAllowed(allow);
  emberAfCorePrintln("Trust Center %p allow trust center rejoins using "
                     "well-known key.",
                     allow
                     ? "WILL"
                     : "WILL NOT");
}

void optionSecurityAllowTrustCenterRejoinUsingWellKnownKeyTimeout(void)
{
  uint16_t timeout = (uint16_t)emberUnsignedCommandArgument(0);
  setTcRejoinsUsingWellKnownKeyTimeout(timeout);
  emberAfCorePrintln("Set timeout for Trust Center rejoins using well-known "
                     "link key to %d seconds.",
                     timeout);
}

void optionSecuritySetKeyRequestPolicy(void)
{
#ifdef EZSP_HOST
  uint8_t tcPolicy = (uint8_t)emberUnsignedCommandArgument(0);
  uint8_t appPolicy = (uint8_t)emberUnsignedCommandArgument(1);

  // The EZSP policies are offset by EZSP_DENY_TC_KEY_REQUESTS and
  // EZSP_DENY_APP_KEY_REQUESTS, respectively
  tcPolicy += EZSP_DENY_TC_KEY_REQUESTS;
  appPolicy += EZSP_DENY_APP_KEY_REQUESTS;

  (void)ezspSetPolicy(EZSP_TC_KEY_REQUEST_POLICY, tcPolicy);
  (void)ezspSetPolicy(EZSP_APP_KEY_REQUEST_POLICY, tcPolicy);
#else // EZSP_HOST
  emberTrustCenterLinkKeyRequestPolicy = (uint8_t)emberUnsignedCommandArgument(0);
  emberAppLinkKeyRequestPolicy = (uint8_t)emberUnsignedCommandArgument(1);
#endif // EZSP_HOST
}
