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
 * @brief Test code for the Connection Manager plugin.
 *******************************************************************************
   ******************************************************************************/

#define EMBER_AF_PLUGIN_CONNECTION_MANAGER_REJOIN_TIME_M    15
#define EMBER_AF_PLUGIN_CONNECTION_MANAGER_RETRY_TIME_S     5
#define EMBER_AF_PLUGIN_CONNECTION_MANAGER_REJOIN_ATTEMPTS  20

#define TOKEN_FORCE_SHORT_POLL      0
#define TOKEN_FORCE_SHORT_POLL_SIZE 1
