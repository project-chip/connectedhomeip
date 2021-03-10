/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#pragma once

#include <gen/enums.h>

#include <app/Command.h>
#include <lib/core/CHIPCore.h>
#include <platform/internal/DeviceNetworkProvisioning.h>

namespace chip {
namespace app {
namespace clusters {
namespace NetworkCommissioning {
EmberAfNetworkCommissioningError OnAddThreadNetworkCommandCallbackInternal(app::Command *, EndpointId,
                                                                           const uint8_t * operationalDataset, uint64_t breadcrumb,
                                                                           uint32_t timeoutMs);
EmberAfNetworkCommissioningError OnAddWiFiNetworkCommandCallbackInternal(app::Command *, EndpointId, const uint8_t * ssid,
                                                                         const uint8_t * credentials, uint64_t breadcrumb,
                                                                         uint32_t timeoutMs);
EmberAfNetworkCommissioningError OnEnableNetworkCommandCallbackInternal(app::Command *, EndpointId, const uint8_t * networkID,
                                                                        uint64_t breadcrumb, uint32_t timeoutMs);
} // namespace NetworkCommissioning

} // namespace clusters
} // namespace app
} // namespace chip
