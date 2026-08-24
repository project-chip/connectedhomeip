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

#include <app/clusters/commissioning-proxy-server/CommissioningProxyPafAdapter.h>
#include <lib/core/CHIPError.h>
#include <system/SystemClock.h>

#include <cstdint>
#include <vector>

namespace chip {
namespace DeviceLayer {
// Declared in platform/Linux/ConnectivityManagerImpl.h. Forward-declared here so this
// header stays free of the platform implementation header; the .cpp includes it.
struct NanPeerInfo;
} // namespace DeviceLayer

namespace app {

/**
 * @brief Linux implementation of the commissioning proxy's Wi-Fi PAF discovery hooks.
 *
 * Forwards to ConnectivityManagerImpl, which owns the wpa_supplicant NAN interface and
 * the single subscribe slot. This is also where the platform's peer descriptor is
 * translated: NanPeerInfo is Linux-only and owns heap storage for its extended data, so
 * it is unpacked into the interface's scalars here and never crosses into the cluster.
 *
 * The platform scan APIs hand back one void*, so this holds the caller's callbacks and
 * context and passes itself instead.
 */
class LinuxCommissioningProxyPafAdapter : public Clusters::CommissioningProxy::CommissioningProxyPafAdapter
{
public:
    CHIP_ERROR StartForegroundScan(System::Clock::Seconds16 window, DiscoveryCallback onDevice, ScanCompleteCallback onDone,
                                   void * context) override;
    void StopForegroundScan() override;
    CHIP_ERROR StartBackgroundScan(DiscoveryCallback cb, void * context) override;
    void StopBackgroundScan() override;
    uint32_t PendingConnectSubscribeId() const override;

private:
    /// Trampolines matching the platform callback shapes; they re-report each peer in the
    /// interface's own terms.
    static void OnPlatformScanComplete(void * context, const std::vector<DeviceLayer::NanPeerInfo> & peers);
    static void OnPlatformBgScanDiscovery(void * context, const DeviceLayer::NanPeerInfo & peer);

    /// Unpack one platform peer descriptor and hand it to @p cb.
    static void ReportPeer(DiscoveryCallback cb, void * context, const DeviceLayer::NanPeerInfo & peer);

    DiscoveryCallback mForegroundCallback = nullptr;
    ScanCompleteCallback mForegroundDone  = nullptr;
    void * mForegroundContext             = nullptr;

    DiscoveryCallback mBackgroundCallback = nullptr;
    void * mBackgroundContext             = nullptr;
};

} // namespace app
} // namespace chip
