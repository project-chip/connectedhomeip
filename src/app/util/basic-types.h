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
 * @file
 * @brief The include file for all the types for the data model that are not
 *        dependent on an individual application configuration.
 */

#pragma once

#include <stdint.h>

/**
 * Types for use by CHIP C code, until we switch to C++.
 */
typedef uint8_t CHIPEndpointId;
typedef uint16_t CHIPClusterId;
typedef uint16_t CHIPAttributeId;
typedef uint16_t CHIPGroupId;

/**
 * Types for use by generated Silicon Labs code until we convert the generator
 * to using CHIP types.
 */
typedef CHIPClusterId EmberAfClusterId;
typedef CHIPAttributeId EmberAfAttributeId;
typedef CHIPGroupId EmberMulticastId;

/**
 * @brief Type for referring to zigbee application profile id
 *        TODO: This is probably not needed for CHIP and should be removed.
 *        https://github.com/project-chip/connectedhomeip/issues/3444
 */
typedef uint16_t EmberAfProfileId;

#ifdef __cplusplus
namespace chip {
typedef CHIPEndpointId EndpointId;
typedef CHIPClusterId ClusterId;
typedef CHIPAttributeId AttributeId;
typedef CHIPGroupId GroupId;
} // namespace chip
#endif // __cplusplus
