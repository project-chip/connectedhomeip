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

#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <algorithm>
#include <cstring>

#include <lib/support/CodeUtils.h>
#include <platform/DeviceControlServer.h>
#include <platform/internal/BLEManager.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

#if !CHIP_DEVICE_CONFIG_SUPPORTS_CONCURRENT_CONNECTION
namespace {
constexpr System::Clock::Seconds16 kHandoffRetryInterval(1);
constexpr uint8_t kHandoffRetryCount = 2;
} // namespace
#endif

void BLEManagerImpl::HandleMatterBleWrite(const ChipDeviceEvent & event)
{
    const auto connection                = event.CHIPoBLEWriteReceived.ConId;
    [[maybe_unused]] const bool received = HandleWriteReceived(connection, &Ble::CHIP_BLE_SVC_ID, &Ble::CHIP_BLE_CHAR_1_UUID,
                                                               System::PacketBufferHandle::Adopt(event.CHIPoBLEWriteReceived.Data));
#if !CHIP_DEVICE_CONFIG_SUPPORTS_CONCURRENT_CONNECTION
    if (auto * link = FindBleLink(connection); received && link != nullptr)
    {
        link->UsedByMatter = true;
    }
#endif
}

void BLEManagerImpl::NotifyChipConnectionClosed(BLE_CONNECTION_OBJECT conId)
{
#if !CHIP_DEVICE_CONFIG_SUPPORTS_CONCURRENT_CONNECTION
    if (const auto * link = FindBleLink(conId); mNetworkHandoffPending && link != nullptr && link->UsedByMatter)
    {
        // The handoff owns physical disconnects for these links, even after their Matter endpoints are gone.
        return;
    }
#endif
    LogErrorOnFailure(CloseConnection(conId));
}

void BLEManagerImpl::CheckNonConcurrentBleClosing()
{
#if !CHIP_DEVICE_CONFIG_SUPPORTS_CONCURRENT_CONNECTION
    VerifyOrReturn(IsBleClosing() && !mNetworkHandoffPending);

    mNetworkHandoffPending          = true;
    mNetworkHandoffRetriesRemaining = kHandoffRetryCount;
    PostCloseAllBleConnections();
    LogErrorOnFailure(SystemLayer().StartTimer(kHandoffRetryInterval, RetryNetworkHandoff, this));
#endif
}

#if !CHIP_DEVICE_CONFIG_SUPPORTS_CONCURRENT_CONNECTION

BLEManagerImpl::BleLink * BLEManagerImpl::FindBleLink(uint16_t connection)
{
    auto link = std::find_if(mBleLinks.begin(), mBleLinks.end(),
                             [connection](const auto & candidate) { return candidate.Connection == connection; });
    return link == mBleLinks.end() ? nullptr : &*link;
}

bool BLEManagerImpl::HandleBleHandoffEvent(const ChipDeviceEvent & event)
{
    switch (event.Type)
    {
    case DeviceEventType::kPlatformESP32BLELinkConnected: {
        auto * link = FindBleLink(event.Platform.BLELink.Connection);
        if (link == nullptr)
        {
            link = FindBleLink(BLE_CONNECTION_UNINITIALIZED);
        }
        VerifyOrReturnValue(link != nullptr, true, ChipLogError(DeviceLayer, "No slot for BLE link"));
        *link            = {};
        link->Connection = event.Platform.BLELink.Connection;
#ifdef CONFIG_BT_BLUEDROID_ENABLED
        std::memcpy(link->PeerAddress, event.Platform.BLELink.PeerAddress, sizeof(link->PeerAddress));
#endif
        return true;
    }
    case DeviceEventType::kPlatformESP32BLELinkDisconnected:
        if (auto * link = FindBleLink(event.Platform.BLELink.Connection); link != nullptr)
        {
            *link = {};
        }
        FinishNetworkHandoffIfReady();
        return true;

    case DeviceEventType::kCloseAllBleConnections:
        if (mNetworkHandoffPending)
        {
            // The commissioning window manager closes the Matter endpoints during this event dispatch.
            LogErrorOnFailure(PlatformMgr().ScheduleWork([](intptr_t) { sInstance.DisconnectCommissioningLinks(); }));
        }
        return true;

    case DeviceEventType::kFailSafeTimerExpired:
        if (mNetworkHandoffPending || IsBleClosing() || !IsInitialized())
        {
            ResetNetworkHandoff();
            LogErrorOnFailure(PlatformMgr().ScheduleWork(RestoreBleLayerAfterFailedHandoff));
        }
        return true;

    default:
        return false;
    }
}

void BLEManagerImpl::PostCloseAllBleConnections()
{
    VerifyOrReturn(!mCloseAllBleConnectionsPosted);
    const CHIP_ERROR err          = DeviceControlServer::DeviceControlSvr().PostCloseAllBLEConnectionsToOperationalNetworkEvent();
    mCloseAllBleConnectionsPosted = (err == CHIP_NO_ERROR);
    LogErrorOnFailure(err);
}

void BLEManagerImpl::DisconnectCommissioningLinks()
{
    VerifyOrReturn(mNetworkHandoffPending && !IsInitialized());

    for (auto & link : mBleLinks)
    {
        if (link.UsedByMatter)
        {
            LogErrorOnFailure(DisconnectBleLink(link));
        }
    }
    FinishNetworkHandoffIfReady();
}

void BLEManagerImpl::FinishNetworkHandoffIfReady()
{
    VerifyOrReturn(mNetworkHandoffPending && !IsInitialized());
    VerifyOrReturn(std::none_of(mBleLinks.begin(), mBleLinks.end(), [](const auto & link) { return link.UsedByMatter; }));
    const CHIP_ERROR err = DeviceControlServer::DeviceControlSvr().PostOperationalNetworkStartedEvent();
    LogErrorOnFailure(err);
    if (err == CHIP_NO_ERROR)
    {
        ResetNetworkHandoff();
    }
}

void BLEManagerImpl::ResetNetworkHandoff()
{
    SystemLayer().CancelTimer(RetryNetworkHandoff, this);
    mNetworkHandoffPending          = false;
    mCloseAllBleConnectionsPosted   = false;
    mNetworkHandoffRetriesRemaining = 0;
}

void BLEManagerImpl::RetryNetworkHandoff(System::Layer * layer, void * state)
{
    auto & manager = *static_cast<BLEManagerImpl *>(state);
    VerifyOrReturn(manager.mNetworkHandoffPending && manager.mNetworkHandoffRetriesRemaining != 0);

    --manager.mNetworkHandoffRetriesRemaining;
    if (manager.IsInitialized())
    {
        manager.PostCloseAllBleConnections();
    }
    else
    {
        manager.DisconnectCommissioningLinks();
    }
    if (manager.mNetworkHandoffRetriesRemaining != 0)
    {
        LogErrorOnFailure(layer->StartTimer(kHandoffRetryInterval, RetryNetworkHandoff, &manager));
    }
}

void BLEManagerImpl::RestoreBleLayerAfterFailedHandoff(intptr_t)
{
    VerifyOrReturn(sInstance.mServiceMode == ConnectivityManager::kCHIPoBLEServiceMode_Enabled);

    if (sInstance.IsBleClosing())
    {
        sInstance.BleLayer::Shutdown();
    }
    if (!sInstance.IsInitialized())
    {
#ifdef CONFIG_ENABLE_ESP32_BLE_CONTROLLER
        LogErrorOnFailure(sInstance.BleLayer::Init(&sInstance, &sInstance, &sInstance, &SystemLayer()));
#else
        LogErrorOnFailure(sInstance.BleLayer::Init(&sInstance, &sInstance, &SystemLayer()));
#endif
    }
}

#endif // !CHIP_DEVICE_CONFIG_SUPPORTS_CONCURRENT_CONNECTION

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
