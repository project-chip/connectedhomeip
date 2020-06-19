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
 * @brief Definitions for the Trust Center Network Key Update Broadcast plugin.
 *******************************************************************************
   ******************************************************************************/

extern EmberEventControl emberAfPluginTrustCenterNwkKeyUpdateBroadcastMyEventControl;
void emberAfPluginTrustCenterNwkKeyUpdateBroadcastMyEventHandler(void);

#define TC_KEY_UPDATE_BROADCAST_EVENT \
  { &emAfTcKeyUpdateBroadcastEvent, emAfTcKeyUpdateBroadcastEventHandler },

#if defined(EMBER_AF_PLUGIN_TEST_HARNESS) || defined(EMBER_SCRIPTED_TEST)
// For testing, we need to support a single application that can do
// unicast AND broadcast key updates.  This function is actually
// emberAfTrustCenterStartNetworkKeyUpdate() but is renamed.
EmberStatus emberAfTrustCenterStartBroadcastNetworkKeyUpdate(void);
#endif

// Because both the unicast and broadcast plugins for Trust Center NWK Key update
// define this function, we must protect it to eliminate the redudandant
// function declaration.  Unicast and broadcast headers may be included together
// since the code then doesn't need to determine which plugin (unicast or
// broadcast) is being used and thus which header it should inclued.
#if !defined(EM_AF_TC_START_NETWORK_KEY_UPDATE_DECLARATION)
  #define EM_AF_TC_START_NETWORK_KEY_UPDATE_DECLARATION
EmberStatus emberAfTrustCenterStartNetworkKeyUpdate(void);
#endif
