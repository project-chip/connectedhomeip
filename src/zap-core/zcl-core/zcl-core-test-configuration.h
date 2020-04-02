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

#ifndef ZCL_CORE_TEST_CONFIGURATION_H
#define ZCL_CORE_TEST_CONFIGURATION_H

#define EMBER_AF_API_STACK "stack/include/ember.h"
#define EMBER_AF_API_BUFFER_MANAGEMENT "stack/framework/buffer-management.h"
#define EMBER_AF_API_BUFFER_QUEUE "stack/framework/buffer-queue.h"
#define EMBER_AF_API_HAL "hal/hal.h"
#define EMBER_ZCL_BINDING_TABLE_SIZE 8
#define EMBER_ZCL_CACHE_TABLE_SIZE 8
#define EMBER_ZCL_GROUP_TABLE_SIZE 8
#define EMBER_ZCL_REPORTING_CONFIGURATIONS_TABLE_SIZE 8

#define EMBER_AF_API_ZCL_CORE "app/thread/plugin/zcl/zcl-core/zcl-core.h"
#define EMBER_AF_API_ZCL_CORE_DTLS_MANAGER "app/thread/plugin/zcl/zcl-core/zcl-core-dtls-manager.h"
#define EMBER_AF_API_ZCL_CORE_WELL_KNOWN "app/thread/plugin/zcl/zcl-core/zcl-core-well-known.h"
#define EMBER_AF_API_ZCL_CORE_RESOURCE_DIRECTORY "app/thread/plugin/zcl/zcl-core/zcl-core-resource-directory.h"

#define EM_ZCL_MAX_REMOTE_ACCESS_TOKENS 2

#define EMBER_AF_PRINT_CORE
#define EMBER_AF_PRINT_APP
#define EMBER_AF_PRINT_BITS
#define EMBER_AF_PRINT_NAMES
#define emberAfAppPrint(...)
#define emberAfAppPrintln(...)
#define emberAfAppFlush()
#define emberAfAppDebugExec(x)
#define emberAfAppPrintBuffer(buffer, len, withSpace)
#define emberAfAppPrintString(buffer)

#endif // ZCL_CORE_TEST_CONFIGURATION_H
