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
/****************************************************************************
 * @file
 * @brief Include file for ias zone server's unit
 *tests
 *******************************************************************************
 ******************************************************************************/

#ifndef SILABS_IAS_ZONE_SERVER_TEST_H
#define SILABS_IAS_ZONE_SERVER_TEST_H

#ifdef EMBER_SCRIPTED_TEST
#define EMBER_AF_PLUGIN_IAS_ZONE_SERVER_ZONE_TYPE 21
#define EMBER_AF_PLUGIN_IAS_ZONE_SERVER_QUEUE_DEPTH 5
#define EMBER_AF_PLUGIN_IAS_ZONE_SERVER_ENABLE_QUEUE 1
#define EMBER_AF_PLUGIN_IAS_ZONE_SERVER_MIN_OTA_TIME_MS (500u)
#define EMBER_AF_PLUGIN_IAS_ZONE_SERVER_FIRST_BACKOFF_TIME_SEC (3u)
#define EMBER_AF_PLUGIN_IAS_ZONE_SERVER_BACKOFF_SEQUENCE_COMMON_RATIO (2u)
#define EMBER_AF_PLUGIN_IAS_ZONE_SERVER_MAX_BACKOFF_TIME_SEC (12u)
// Studio does not generate value for BOOLEAN type plugin options. Making it aligned...
#define EMBER_AF_PLUGIN_IAS_ZONE_SERVER_UNLIMITED_RETRIES
#define EMBER_AF_PLUGIN_IAS_ZONE_SERVER_MAX_RETRY_ATTEMPTS (100)

#endif

#define TOKEN_PLUGIN_IAS_ZONE_SERVER_ENROLLMENT_METHOD 0
#define TOKEN_PLUGIN_IAS_ZONE_SERVER_ENROLLMENT_METHOD_SIZE 1

#undef EMBER_BINDING_TABLE_SIZE
#define EMBER_BINDING_TABLE_SIZE 12

#endif //__IAS_ZONE_SERVER_TEST_H__
