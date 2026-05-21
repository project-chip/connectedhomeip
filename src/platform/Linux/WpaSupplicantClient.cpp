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

#include "WpaSupplicantClient.h"

#include <mutex>

#include <platform/CHIPDeviceConfig.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

void WpaSupplicantClient::GDBusWpaSupplicant::Reset()
{
    iface.reset();
    proxy.reset();
    interfacePath.reset();
    networkPath.reset();
}

CHIP_ERROR WpaSupplicantClient::Init(ConnectivityManagerImpl & inConnectivityManagerImpl) noexcept
{
    // Initially, and again at some future point, this should have an
    // assertion that mConnectivityManagerImpl is null and otherwise
    // return CHIP_ERROR_ALREADY_INITIALIZED to indicate to the caller
    // that 'Init' has been double-tripped without a bookending call
    // to Shutdown. However, the ConnectivityManager stack of which
    // this is a part has no peer 'Shutdown' method to go with its
    // 'Init' and subsequently, there is not yet a reasonable place to
    // hook the 'Shutdown' for this class.
    //
    // At some point, revisit the top-down Init/Shutdown in
    // ConnectivityManager. When that happens, plumb the call
    // propagation through to this client 'Shutdown' and re-add the
    // assertion.

    mConnectivityManagerImpl = &inConnectivityManagerImpl;

    return CHIP_NO_ERROR;
}

void WpaSupplicantClient::Shutdown() noexcept
{
    Reset();

    mConnectivityManagerImpl = nullptr;
}

void WpaSupplicantClient::Reset() noexcept
{
    std::lock_guard<std::mutex> lock(mWpaSupplicantMutex);

    mWpaSupplicant.Reset();
}

bool WpaSupplicantClient::IsStarted() const noexcept
{
    std::lock_guard<std::mutex> lock(mWpaSupplicantMutex);

    return !!mWpaSupplicant.iface;
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
