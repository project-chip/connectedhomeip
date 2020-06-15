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
 * @brief CLI for APS Link Key Authorization feature.
 *******************************************************************************
   ******************************************************************************/

#include "af.h"
#include "app/util/serial/command-interpreter2.h"
#include "aps-link-key-authorization.h"

#ifdef EMBER_SCRIPTED_TEST
#include "app/framework/util/af-main.h" // emberAfGetBindingTableSize
#include "app/framework/plugin/aps-link-key-authorization/aps-link-key-authorization-test.h"
#endif

static void printEnabledState(void)
{
  emberAfAppPrintln(" APS link key authorization is %s.",
                    (emberAfApsLinkKeyAuthorizationIsEnabled()) ? "enabled" : "disabled");
}

// plugin aps-link-key-authorization info
void emberAfPluginApsLinkKeyAuthorizationCliInfo(void)
{
  uint8_t exemptListCount;
  uint16_t exemptList[EMBER_AF_PLUGIN_APS_LINK_KEY_AUTHORIZATION_MAX_EXEMPT_CLUSTERS];

  printEnabledState();

  emberAfApsLinkKeyAuthorizationGetExemptClusterList(&exemptListCount, exemptList);

  emberAfAppPrintln(" Exempt list:\n   max length: %d\n   actual length: %d",
                    EMBER_AF_PLUGIN_APS_LINK_KEY_AUTHORIZATION_MAX_EXEMPT_CLUSTERS, exemptListCount);

  if (exemptListCount) {
    uint16_t i;
    emberAfAppPrint("   clusters: ");

    for (i = 0; i < exemptListCount; i++) {
      emberAfAppPrint("0x%2X ", exemptList[i]);
    }
    emberAfAppPrintln("");
  }
}

// plugin aps-link-key-authorization enable <enable>
void emberAfPluginApsLinkKeyAuthorizationCliEnable(void)
{
  emberAfApsLinkKeyAuthorizationEnable(
    (bool)emberUnsignedCommandArgument(0));
  printEnabledState();
}

// plugin aps-link-key-authorization enable-with-exempt-list <enable> <exemptList>
// <exemptList> is expected in 16-bit hex format, for example: 0x0001 0xACDC 0xBEEF
void emberAfPluginApsLinkKeyAuthorizationCliEnableWithExemptList(void)
{
  EmberStatus retVal;
  uint16_t cliExemptList[EMBER_AF_PLUGIN_APS_LINK_KEY_AUTHORIZATION_MAX_EXEMPT_CLUSTERS];
  uint8_t cliExemptListCount = emberCommandArgumentCount() - 1;
  uint8_t i;

  // Store the list elements
  for (i = 1; i <= cliExemptListCount; i++) {
    cliExemptList[i - 1] = emberUnsignedCommandArgument(i);
  }

  if (emberUnsignedCommandArgument(0)) {
    retVal = emberAfApsLinkKeyAuthorizationEnableWithExemptClusterList(cliExemptListCount, (uint8_t *)cliExemptList);
  } else {
    retVal = emberAfApsLinkKeyAuthorizationDisableWithExemptClusterList(cliExemptListCount, (uint8_t *)cliExemptList);
  }

  printEnabledState();

  if (retVal != EMBER_SUCCESS) {
    emberAfAppPrintln("Failed to store all clusters to exempt list, list is full.");
  } else {
    emberAfAppPrintln("All clusters are stored to exempt list successfully.");
  }
}

// plugin aps-link-key-authorization clear-exempt-list
void emberAfPluginApsLinkKeyAuthorizationCliClear(void)
{
  emberAfApsLinkKeyAuthorizationClearExemptClusterList();
  emberAfAppPrintln(" Exempt list is cleared.");
}

// plugin aps-link-key-authorization add-cluster-exempted <cluster>
void emberAfPluginApsLinkKeyAuthorizationCliAddClusterExempted(void)
{
  emberAfApsLinkKeyAuthorizationAddClusterExempted(emberUnsignedCommandArgument(0));
}
