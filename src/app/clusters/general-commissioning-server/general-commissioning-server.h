/**
 *
 *    Copyright (c) 2022 Project CHIP Authors
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
#include <platform/CHIPDeviceConfig.h>
#include <app-common/zap-generated/cluster-objects.h>

namespace chip {
namespace app {
namespace Clusters {
namespace GeneralCommissioning {

void SetBreadcrumb(Attributes::Breadcrumb::TypeInfo::Type breadcrumb);
#if CHIP_DEVICE_CONFIG_NETWORK_RECOVERY_REQUIRED
void SetNetworkRecoveryReasonValue(Attributes::NetworkRecoveryReason::TypeInfo::Type& value);
void GetNetworkRecoveryReasonValue(Attributes::NetworkRecoveryReason::TypeInfo::Type& value);
void GenerateAndSetRecoveryIdentifier(void);
uint64_t GetRecoveryIdentifier(void);
#endif // CHIP_DEVICE_CONFIG_NETWORK_RECOVERY_REQUIRED
} // namespace GeneralCommissioning
} // namespace Clusters
} // namespace app
} // namespace chip
