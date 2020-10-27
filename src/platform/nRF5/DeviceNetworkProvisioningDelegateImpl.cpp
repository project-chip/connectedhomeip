/*
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

#include "DeviceNetworkProvisioningDelegateImpl.h"

#if CHIP_ENABLE_OPENTHREAD
#include <platform/ThreadStackManager.h>
#endif

namespace chip {
namespace DeviceLayer {

void DeviceNetworkProvisioningDelegateImpl::_ProvisionThreadNetwork(DeviceLayer::Internal::DeviceNetworkInfo & threadData)
{
#if CHIP_ENABLE_OPENTHREAD
    ThreadStackMgr().SetThreadEnabled(false);
    ThreadStackMgr().SetThreadProvision(threadData);
    ThreadStackMgr().SetThreadEnabled(true);
#endif // CHIP_ENABLE_OPENTHREAD
}

} // namespace DeviceLayer
} // namespace chip
