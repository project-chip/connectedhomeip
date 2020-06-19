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
 * @brief Routines for the Find and Bind Target plugin.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"

#include "find-and-bind-target.h"

#ifdef EMBER_SCRIPTED_TEST
  #include "../find-and-bind-initiator/find-and-bind-test-configuration.h"
#endif

// -----------------------------------------------------------------------------
// Public API

EmberAfStatus emberAfPluginFindAndBindTargetStart(uint8_t endpoint)
{
  // Write the identify time.
  uint16_t identifyTime = EMBER_AF_PLUGIN_FIND_AND_BIND_TARGET_COMMISSIONING_TIME;
  EmberAfStatus status = EMBER_ZCL_STATUS_UNSUPPORTED_ATTRIBUTE;

  if (emberAfContainsServer(endpoint, ZCL_IDENTIFY_CLUSTER_ID)) {
    status = emberAfWriteServerAttribute(endpoint,
                                         ZCL_IDENTIFY_CLUSTER_ID,
                                         ZCL_IDENTIFY_TIME_ATTRIBUTE_ID,
                                         (uint8_t *)&identifyTime,
                                         ZCL_INT16U_ATTRIBUTE_TYPE);
  }

  emberAfCorePrintln("%p: %p: 0x%X",
                     EMBER_AF_PLUGIN_FIND_AND_BIND_TARGET_PLUGIN_NAME,
                     "Start target",
                     status);

  return status;
}
