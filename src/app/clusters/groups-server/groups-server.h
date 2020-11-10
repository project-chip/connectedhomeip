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

#pragma once

#include <app/util/basic-types.h>

/** @brief Get Group Name
 *
 * This function returns the name of a group with the provided group ID, should
 * it exist.
 *
 * @param endpoint Endpoint Ver.: always
 * @param groupId Group ID Ver.: always
 * @param groupName Group Name Ver.: always
 */
void emberAfPluginGroupsServerGetGroupNameCallback(CHIPEndpointId endpoint, CHIPGroupId groupId, uint8_t * groupName);

/** @brief Set Group Name
 *
 * This function sets the name of a group with the provided group ID.
 *
 * @param endpoint Endpoint Ver.: always
 * @param groupId Group ID Ver.: always
 * @param groupName Group Name Ver.: always
 */
void emberAfPluginGroupsServerSetGroupNameCallback(CHIPEndpointId endpoint, CHIPGroupId groupId, uint8_t * groupName);

/** @brief Group Names Supported
 *
 * This function returns whether or not group names are supported.
 *
 * @param endpoint Endpoint Ver.: always
 */
bool emberAfPluginGroupsServerGroupNamesSupportedCallback(CHIPEndpointId endpoint);
