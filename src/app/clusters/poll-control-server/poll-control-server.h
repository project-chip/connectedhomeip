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
 * @brief APIs and defines for the Poll Control Server plugin.
 *******************************************************************************
   ******************************************************************************/

// Callback triggered after multiple failed trust center poll control checkins
void emberAfPluginPollControlServerCheckInTimeoutCallback(void);

/**
 * @brief Sets whether or not the Poll Control server uses non TC clients
 *
 * sets the behavior of the Poll Control server when determining whether or not
 * to use non Trust Center Poll Control clients.
 *
 * @param IgnoreNonTc a boolean determining whether the server should ignore any
 *        non Trust Center Poll Control clients
 */
void emberAfPluginPollControlServerSetIgnoreNonTrustCenter(bool ignoreNonTc);

/**
 * @brief Returns the current value of ignoreNonTrustCenter for the Poll Control server
 */
bool emberAfPluginPollControlServerGetIgnoreNonTrustCenter(void);
