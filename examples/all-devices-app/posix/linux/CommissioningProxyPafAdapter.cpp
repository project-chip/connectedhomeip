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

#include "CommissioningProxyPafAdapter.h"

#include <lib/support/CodeUtils.h>
#include <lib/support/Span.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/ConnectivityManager.h>
#include <platform/Linux/ConnectivityManagerImpl.h>

namespace chip {
namespace app {

CHIP_ERROR CommissioningProxyPafAdapter::StartForegroundScan(System::Clock::Seconds16 window, DiscoveryCallback onDevice,
                                                             ScanCompleteCallback onDone, void * context)
{
    VerifyOrReturnError(onDevice != nullptr && onDone != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    mForegroundCallback = onDevice;
    mForegroundDone     = onDone;
    mForegroundContext  = context;

    // WiFiPAFScan takes the window as uint8 seconds. The cluster's ScanMaxTime attribute
    // is itself a uint8, so the value cannot exceed what the platform call accepts.
    CHIP_ERROR err =
        DeviceLayer::ConnectivityMgrImpl().WiFiPAFScan(static_cast<uint8_t>(window.count()), OnPlatformScanComplete, this);
    if (err != CHIP_NO_ERROR)
    {
        // Leave nothing armed on failure, so a later stray platform callback cannot reach
        // a caller that believes its scan never started.
        mForegroundCallback = nullptr;
        mForegroundDone     = nullptr;
        mForegroundContext  = nullptr;
    }
    return err;
}

void CommissioningProxyPafAdapter::StopForegroundScan()
{
    // ConnectivityManagerImpl exposes no way to abort a one-shot WiFiPAFScan early - the
    // window is owned by wpa_supplicant and FinishWiFiPAFScan is private. Detaching the
    // callbacks is therefore the whole of what this can promise, and is what the caller
    // actually needs: a scan that lands after teardown is dropped here rather than
    // reaching a destroyed transport.
    mForegroundCallback = nullptr;
    mForegroundDone     = nullptr;
    mForegroundContext  = nullptr;
}

CHIP_ERROR CommissioningProxyPafAdapter::StartBackgroundScan(DiscoveryCallback cb, void * context)
{
    VerifyOrReturnError(cb != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    mBackgroundCallback = cb;
    mBackgroundContext  = context;

    CHIP_ERROR err = DeviceLayer::ConnectivityMgrImpl().WiFiPAFStartBackgroundScan(OnPlatformBgScanDiscovery, this);
    if (err != CHIP_NO_ERROR)
    {
        mBackgroundCallback = nullptr;
        mBackgroundContext  = nullptr;
    }
    return err;
}

void CommissioningProxyPafAdapter::StopBackgroundScan()
{
    DeviceLayer::ConnectivityMgrImpl().WiFiPAFStopBackgroundScan();
    mBackgroundCallback = nullptr;
    mBackgroundContext  = nullptr;
}

uint32_t CommissioningProxyPafAdapter::PendingConnectSubscribeId() const
{
    return DeviceLayer::ConnectivityMgrImpl().GetPendingConnectSubscribeId();
}

void CommissioningProxyPafAdapter::DisconnectPublishReceiveHandler()
{
    DeviceLayer::ConnectivityMgrImpl().WiFiPAFDisconnectPublishReceiveHandler();
}

void CommissioningProxyPafAdapter::ReportPeer(DiscoveryCallback cb, void * context, const DeviceLayer::NanPeerInfo & peer)
{
    // The platform keeps extended data in a heap buffer; hand it across as a span that is
    // only valid for this call, which is what the interface promises.
    ByteSpan extendedData;
    if (peer.hasExtendedData && !peer.storage.empty())
    {
        extendedData = ByteSpan(peer.storage.data(), peer.storage.size());
    }
    cb(context, ByteSpan(peer.mac, sizeof(peer.mac)), peer.discriminator, peer.vid, peer.pid, extendedData, peer.band);
}

void CommissioningProxyPafAdapter::OnPlatformScanComplete(void * context, const std::vector<DeviceLayer::NanPeerInfo> & peers)
{
    auto * self = static_cast<CommissioningProxyPafAdapter *>(context);
    VerifyOrReturn(self != nullptr && self->mForegroundCallback != nullptr && self->mForegroundDone != nullptr);

    // The platform reports the whole result set at the end of the window; the interface
    // reports device-by-device and then signals completion, so fan the batch out here.
    for (const auto & peer : peers)
    {
        ReportPeer(self->mForegroundCallback, self->mForegroundContext, peer);
    }

    ScanCompleteCallback onDone = self->mForegroundDone;
    void * doneContext          = self->mForegroundContext;
    self->mForegroundCallback   = nullptr;
    self->mForegroundDone       = nullptr;
    self->mForegroundContext    = nullptr;
    onDone(doneContext);
}

void CommissioningProxyPafAdapter::OnPlatformBgScanDiscovery(void * context, const DeviceLayer::NanPeerInfo & peer)
{
    auto * self = static_cast<CommissioningProxyPafAdapter *>(context);
    VerifyOrReturn(self != nullptr && self->mBackgroundCallback != nullptr);

    ReportPeer(self->mBackgroundCallback, self->mBackgroundContext, peer);
}

} // namespace app
} // namespace chip
