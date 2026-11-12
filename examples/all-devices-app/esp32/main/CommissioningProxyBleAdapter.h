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
 * ESP32 (NimBLE) implementation of the Commissioning Proxy BLE adapter, over the proxy
 * scan hooks of the ESP32 BLEManagerImpl.
 *
 * The ESP32 BLE role is selected before the BLE host starts and cannot change at
 * runtime, so EnableCentralRole() only reports the role: the application restarts the
 * proxy in central mode once it has been commissioned.
 */
class CommissioningProxyBleAdapter : public Clusters::CommissioningProxy::CommissioningProxyBleAdapter
{
public:
    CHIP_ERROR EnableCentralRole() override;
    CHIP_ERROR StartScan(DiscoveryCallback cb, void * context) override;
    void StopScan() override;

private:
    static void OnPlatformScanResult(void * context, const uint8_t bdAddr[6], uint16_t discriminator, uint16_t vendorId,
                                     uint16_t productId);

    DiscoveryCallback mCallback = nullptr;
    void * mContext             = nullptr;
};

} // namespace app
} // namespace chip
