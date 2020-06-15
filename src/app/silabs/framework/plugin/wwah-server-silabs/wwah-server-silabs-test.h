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
 * @brief Include file for WWAH Server Silabs unit tests
 *******************************************************************************
   ******************************************************************************/

#ifndef SILABS_WWAH_SERVER_SILABS_TEST_H
#define SILABS_WWAH_SERVER_SILABS_TEST_H

#define EMBER_AF_PLUGIN_WWAH_SERVER_SILABS_MAX_EXEMPT_CLUSTERS 120

// TODO: Might be changed to critical message queue.
#define EMBER_AF_PLUGIN_IAS_ZONE_SERVER
#define EMBER_AF_PLUGIN_IAS_ZONE_SERVER_ENABLE_QUEUE
#define EMBER_AF_PLUGIN_IAS_ZONE_SERVER_QUEUE_DEPTH 10

#define TOKEN_PLUGIN_WWAH_CLIENT_ENDPOINT 0
#define TOKEN_PLUGIN_WWAH_CLIENT_ENDPOINT_SIZE  1
#define TOKEN_PLUGIN_WWAH_CONFIGURATION_MASK 1
#define TOKEN_PLUGIN_WWAH_CONFIGURATION_MASK_SIZE 1
#define TOKEN_WWAH_MAC_POLL_FAILURE_WAIT_TIME 2
#define TOKEN_WWAH_MAC_POLL_FAILURE_WAIT_TIME_SIZE 1
#define TOKEN_PLUGIN_WWAH_PENDING_NETWORK_UPDATE_CHANNEL  3
#define TOKEN_PLUGIN_WWAH_PENDING_NETWORK_UPDATE_CHANNEL_SIZE 1
#define TOKEN_PLUGIN_WWAH_PENDING_NETWORK_UPDATE_PANID  4
#define TOKEN_PLUGIN_WWAH_PENDING_NETWORK_UPDATE_PANID_SIZE 2
#define TOKEN_PLUGIN_WWAH_APP_EVENT_RETRY_ENABLED 5
#define TOKEN_PLUGIN_WWAH_APP_EVENT_RETRY_ENABLED_SIZE 1
#define TOKEN_PLUGIN_WWAH_APP_EVENT_RETRY_FIRST_BACKOFF_TIME_S 6
#define TOKEN_PLUGIN_WWAH_APP_EVENT_RETRY_FIRST_BACKOFF_TIME_S_SIZE 1
#define TOKEN_PLUGIN_WWAH_APP_EVENT_RETRY_BACKOFF_COMMON_RATIO 7
#define TOKEN_PLUGIN_WWAH_APP_EVENT_RETRY_BACKOFF_COMMON_RATIO_SIZE 1
#define TOKEN_PLUGIN_WWAH_APP_EVENT_RETRY_MAX_BACKOFF_TIME_S 8
#define TOKEN_PLUGIN_WWAH_APP_EVENT_RETRY_MAX_BACKOFF_TIME_S_SIZE 4
#define TOKEN_PLUGIN_WWAH_APP_EVENT_RETRY_MAX_REDELIVERY_ATTEMPTS 9
#define TOKEN_PLUGIN_WWAH_APP_EVENT_RETRY_MAX_REDELIVERY_ATTEMPTS_SIZE 1
#define TOKEN_PLUGIN_WWAH_USE_TC_FOR_CLUSTER_SERVER 10
#define TOKEN_PLUGIN_WWAH_USE_TC_FOR_CLUSTER_SERVER_SIZE 8
#endif //SILABS_WWAH_SERVER_SILABS_TEST_H
