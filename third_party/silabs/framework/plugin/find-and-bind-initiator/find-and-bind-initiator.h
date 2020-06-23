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
 * @brief Initiator APIs and defines for the Find and Bind Initiator plugin.
 *******************************************************************************
   ******************************************************************************/

#ifndef SILABS_FIND_AND_BIND_INITIATOR_H
#define SILABS_FIND_AND_BIND_INITIATOR_H

// -----------------------------------------------------------------------------
// Constants

#define EMBER_AF_PLUGIN_FIND_AND_BIND_INITIATOR_PLUGIN_NAME "Find and Bind Initiator"

// -----------------------------------------------------------------------------
// API

/** @brief Starts initiator finding and binding operations.
 *
 * A call to this function will commence the initiator finding and
 * binding operations. Specifically, the initiator will attempt to start
 * searching for potential bindings that can be made with identifying
 * targets.
 *
 * @param endpoint The endpoint on which to begin initiator operations.
 *
 * @returns An ::EmberStatus value describing the success of the
 * commencement of the initiator operations.
 */
EmberStatus emberAfPluginFindAndBindInitiatorStart(uint8_t endpoint);

#endif /* __FIND_AND_BIND_INITIATOR_H__ */
