/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include "ConnectivityManagerImpl_NetworkManagementBasis.h"

#include <lib/support/CodeUtils.h>

#include "ConnectivityManagerImpl_NetworkManagementDelegate.h"

namespace chip {
namespace DeviceLayer {
namespace Internal {

CHIP_ERROR NetworkManagementBasis::Init() noexcept
{
    mDelegate = nullptr;

    return CHIP_NO_ERROR;
}

void NetworkManagementBasis::SetDelegate(NetworkManagementDelegate * inNetworkManagementDelegate) noexcept
{
    mDelegate = inNetworkManagementDelegate;
}

void NetworkManagementBasis::OnWiFiMediumAvailable(bool inAvailable) noexcept
{
    VerifyOrReturn(mDelegate != nullptr);

    mDelegate->OnWiFiMediumAvailable(*this, inAvailable);
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
