/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <LinuxCommissioningProxyBleAdapter.h>

#include <lib/support/CodeUtils.h>
#include <lib/support/Span.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/ConnectivityManager.h>

namespace chip {
namespace app {

namespace {
/// BD_ADDR length, as reported by BLEManagerImpl::BleScanResultCallback.
constexpr size_t kBleAddressLength = 6;
} // namespace

CHIP_ERROR LinuxCommissioningProxyBleAdapter::EnableCentralRole()
{
    // One-way switch: tears down peripheral advertising and re-inits BlueZ as a central.
    // Idempotent, and reports CHIP_ERROR_BUSY while prior peripheral activity is still
    // winding down, which is exactly what the interface asks for.
    return DeviceLayer::Internal::BLEMgrImpl().SwitchToCentralMode();
}

CHIP_ERROR LinuxCommissioningProxyBleAdapter::StartScan(DiscoveryCallback cb, void * context)
{
    VerifyOrReturnError(cb != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    mCallback = cb;
    mContext  = context;

    CHIP_ERROR err = DeviceLayer::Internal::BLEMgrImpl().StartProxyScan(OnPlatformScanResult, this);
    if (err != CHIP_NO_ERROR)
    {
        // Leave nothing armed on failure, so a later stray platform callback cannot
        // reach a caller that believes its scan never started.
        mCallback = nullptr;
        mContext  = nullptr;
    }
    return err;
}

void LinuxCommissioningProxyBleAdapter::StopScan()
{
    (void) DeviceLayer::Internal::BLEMgrImpl().StopProxyScan();
    mCallback = nullptr;
    mContext  = nullptr;
}

void LinuxCommissioningProxyBleAdapter::OnPlatformScanResult(void * context, const uint8_t bdAddr[6], uint16_t discriminator,
                                                             uint16_t vendorId, uint16_t productId)
{
    auto * self = static_cast<LinuxCommissioningProxyBleAdapter *>(context);
    VerifyOrReturn(self != nullptr && self->mCallback != nullptr);

    self->mCallback(self->mContext, ByteSpan(bdAddr, kBleAddressLength), discriminator, vendorId, productId);
}

} // namespace app
} // namespace chip
