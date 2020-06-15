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
 * @brief CLI for the Network Creator Security plugin.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"

#include "network-creator-security.h"

extern EmberKeyData distributedKey;

void emAfPluginNetworkCreatorSecuritySetJoiningLinkKeyCommand(void)
{
  EmberEUI64 eui64;
  EmberKeyData keyData;
  EmberStatus status;

  emberCopyBigEndianEui64Argument(0, eui64);
  emberCopyKeyArgument(1, &keyData);

  status = emberAddTransientLinkKey(eui64, &keyData);

  emberAfCorePrintln("%p: %p: 0x%X",
                     EMBER_AF_PLUGIN_NETWORK_CREATOR_SECURITY_PLUGIN_NAME,
                     "Set joining link key",
                     status);
}

void emAfPluginNetworkCreatorSecurityClearJoiningLinkKeyCommand(void)
{
  emberClearTransientLinkKeys();

  emberAfCorePrintln("%p: %p: 0x%X",
                     EMBER_AF_PLUGIN_NETWORK_CREATOR_SECURITY_PLUGIN_NAME,
                     "Clear joining link keys",
                     EMBER_SUCCESS);
}

void emAfPluginNetworkCreatorSecurityOpenOrCloseNetworkCommand(void)
{
  EmberStatus status;
  bool open = (emberStringCommandArgument(-1, NULL)[0] == 'o');

  status = (open
            ? emberAfPluginNetworkCreatorSecurityOpenNetwork()
            : emberAfPluginNetworkCreatorSecurityCloseNetwork());

  emberAfCorePrintln("%p: %p network: 0x%X",
                     EMBER_AF_PLUGIN_NETWORK_CREATOR_SECURITY_PLUGIN_NAME,
                     (open ? "Open" : "Close"),
                     status);
}

void emAfPluginNetworkCreatorSecurityOpenNetworkWithKeyCommand(void)
{
  EmberEUI64 eui64;
  EmberKeyData keyData;
  EmberStatus status;

  emberCopyBigEndianEui64Argument(0, eui64);
  emberCopyKeyArgument(1, &keyData);
  status = emberAfPluginNetworkCreatorSecurityOpenNetworkWithKeyPair(eui64, keyData);

  emberAfCorePrintln("%p: Open network: 0x%X",
                     EMBER_AF_PLUGIN_NETWORK_CREATOR_SECURITY_PLUGIN_NAME,
                     status);
}

void emAfPluginNetworkCreatorSecurityConfigureDistributedKey(void)
{
  emberCopyKeyArgument(0, &distributedKey);
}
