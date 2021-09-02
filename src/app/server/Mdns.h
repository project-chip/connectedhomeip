/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <lib/core/CHIPError.h>
#include <stddef.h>

namespace chip {
namespace app {
namespace Mdns {

enum class CommissioningMode
{
    kDisabled,       // Commissioning Mode is disabled, CM=0, AC=0 in DNS-SD key/value pairs
    kEnabledBasic,   // Basic Commissioning Mode, CM=1, AC=0 in DNS-SD key/value pairs
    kEnabledEnhanced // Enhanced Commissioning Mode, CM=1, AC=1 in DNS-SD key/value pairs
};

/// Sets the secure Matter port
void SetSecuredPort(uint16_t port);

/// Gets the secure Matter port
uint16_t GetSecuredPort();

/// Sets the unsecure Matter port
void SetUnsecuredPort(uint16_t port);

/// Gets the unsecure Matter port
uint16_t GetUnsecuredPort();

/// Start operational advertising
CHIP_ERROR AdvertiseOperational();

/// Set MDNS commissioner advertisement
CHIP_ERROR AdvertiseCommissioner();

/// Set MDNS commissionable node advertisement
CHIP_ERROR AdvertiseCommissionableNode(CommissioningMode mode);

/// (Re-)starts the minmdns server
/// - if device has not yet been commissioned, then commissioning mode will show as enabled (CM=1, AC=0)
/// - if device has been commissioned, then commissioning mode will reflect the state of mode argument
void StartServer(CommissioningMode mode = CommissioningMode::kDisabled);

CHIP_ERROR GenerateRotatingDeviceId(char rotatingDeviceIdHexBuffer[], size_t rotatingDeviceIdHexBufferSize);

/// Generates the (random) instance name that a CHIP device is to use for pre-commissioning DNS-SD
CHIP_ERROR GetCommissionableInstanceName(char * buffer, size_t bufferLen);

} // namespace Mdns
} // namespace app
} // namespace chip
