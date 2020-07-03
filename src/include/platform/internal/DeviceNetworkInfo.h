/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2018 Nest Labs, Inc.
 *    All rights reserved.
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

#ifndef DEVICE_NETWORK_INFO_H
#define DEVICE_NETWORK_INFO_H

#include <stddef.h>
#include <stdint.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

/**
 * Ids for well-known network provision types.
 */
constexpr size_t kMaxThreadNetworkNameLength = 16;
constexpr size_t kThreadExtendedPANIdLength  = 8;
constexpr size_t kThreadMeshPrefixLength     = 8;
constexpr size_t kThreadNetworkKeyLength     = 16;
constexpr size_t kThreadPSKcLength           = 16;
constexpr size_t kThreadChannel_NotSpecified = UINT8_MAX;
constexpr size_t kThreadPANId_NotSpecified   = UINT16_MAX;

class DeviceNetworkInfo
{
public:
    // ---- Thread-specific Fields ----
    char ThreadNetworkName[kMaxThreadNetworkNameLength + 1];
    /**< The Thread network name as a NULL-terminated string. */
    uint8_t ThreadExtendedPANId[kThreadExtendedPANIdLength];
    /**< The Thread extended PAN ID. */
    uint8_t ThreadMeshPrefix[kThreadMeshPrefixLength];
    /**< The Thread mesh prefix. */
    uint8_t ThreadNetworkKey[kThreadNetworkKeyLength];
    /**< The Thread master network key (NOT NULL-terminated). */
    uint8_t ThreadPSKc[kThreadPSKcLength];
    /**< The Thread pre-shared commissioner key (NOT NULL-terminated). */
    uint16_t ThreadPANId;  /**< The 16-bit Thread PAN ID, or kThreadPANId_NotSpecified */
    uint8_t ThreadChannel; /**< The Thread channel (currently [11..26]), or kThreadChannel_NotSpecified */

    struct
    {
        bool ThreadExtendedPANId : 1;
        bool ThreadMeshPrefix : 1;
        bool ThreadPSKc : 1;
    } FieldPresent;
};

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip

#endif // DEVICE_NETWORK_INFO_H
