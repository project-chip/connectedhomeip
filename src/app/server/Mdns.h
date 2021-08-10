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

#include <core/CHIPError.h>
#include <stddef.h>

namespace chip {
namespace app {
namespace Mdns {

/// Start operational advertising
CHIP_ERROR AdvertiseOperational();

/// Set MDNS commissioner advertisement
CHIP_ERROR AdvertiseCommissioner();

/// Set MDNS commissionable node advertisement
CHIP_ERROR AdvertiseCommissionableNode(bool commissioningMode, bool additionalCommissioning);

/// Overloaded utility method for commissioner and commissionable advertisement
// This method is used for both commissioner discovery and commissionable node discovery since
// they share many fields.
CHIP_ERROR Advertise(bool commissionableNode, bool commissioningMode, bool additionalCommissioning);

/// (Re-)starts the minmdns server
void StartServer();
void StartServer(bool commissioningMode, bool additionalCommissioning);

CHIP_ERROR GenerateRotatingDeviceId(char rotatingDeviceIdHexBuffer[], size_t rotatingDeviceIdHexBufferSize);

/// Generates the (random) instance name that a CHIP device is to use for pre-commissioning DNS-SD
CHIP_ERROR GetCommissionableInstanceName(char * buffer, size_t bufferLen);

} // namespace Mdns
} // namespace app
} // namespace chip
