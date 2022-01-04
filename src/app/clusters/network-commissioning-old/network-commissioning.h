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

#include <app-common/zap-generated/enums.h>
#include <app/CommandHandler.h>
#include <lib/core/CHIPCore.h>
#include <platform/internal/DeviceNetworkProvisioning.h>

namespace chip {
namespace app {
namespace Clusters {
namespace NetworkCommissioning {
void OnAddOrUpdateThreadNetworkCommandCallbackInternal(app::CommandHandler *, const app::ConcreteCommandPath & commandPath,
                                                       ByteSpan operationalDataset, uint64_t breadcrumb, uint32_t timeoutMs);
void OnAddOrUpdateWiFiNetworkCommandCallbackInternal(app::CommandHandler *, const app::ConcreteCommandPath & commandPath,
                                                     ByteSpan ssid, ByteSpan credentials, uint64_t breadcrumb, uint32_t timeoutMs);
void OnConnectNetworkCommandCallbackInternal(app::CommandHandler *, const app::ConcreteCommandPath & commandPath,
                                             ByteSpan networkID, uint64_t breadcrumb, uint32_t timeoutMs);
} // namespace NetworkCommissioning

} // namespace Clusters
} // namespace app
} // namespace chip
