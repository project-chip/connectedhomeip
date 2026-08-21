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

#pragma once

#include <app/clusters/commissioning-proxy-server/CommissioningProxyBleAdapter.h>
#include <lib/core/CHIPError.h>

#include <cstdint>

namespace chip {
namespace app {

/**
 * @brief Linux implementation of the commissioning proxy's BLE platform hooks.
 *
 * Forwards to BLEManagerImpl, which owns the BlueZ adapter, the single
 * ChipDeviceScanner and the peripheral/central role. Stateless apart from the
 * scan callback it has to remember: the platform scan API hands back one void*,
 * so this holds the caller's callback and context and passes itself instead.
 */
class LinuxCommissioningProxyBleAdapter : public Clusters::CommissioningProxy::CommissioningProxyBleAdapter
{
public:
    CHIP_ERROR EnableCentralRole() override;
    CHIP_ERROR StartScan(DiscoveryCallback cb, void * context) override;
    void StopScan() override;

private:
    /// Trampoline matching BLEManagerImpl::BleScanResultCallback; re-reports in the
    /// interface's own terms.
    static void OnPlatformScanResult(void * context, const uint8_t bdAddr[6], uint16_t discriminator, uint16_t vendorId,
                                     uint16_t productId);

    DiscoveryCallback mCallback = nullptr;
    void * mContext             = nullptr;
};

} // namespace app
} // namespace chip
