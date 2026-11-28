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
#include <platform/internal/BLEManager.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

#if !CHIP_DEVICE_CONFIG_SUPPORTS_CONCURRENT_CONNECTION
namespace {
constexpr System::Clock::Seconds16 kHandoffRetryInterval(1);
} // namespace
#endif

void BLEManagerImpl::NotifyChipConnectionClosed(BLE_CONNECTION_OBJECT conId)
{
#if !CHIP_DEVICE_CONFIG_SUPPORTS_CONCURRENT_CONNECTION
    // The handoff closes the physical links after the Matter BLE layer has shut down.
    VerifyOrReturn(mNetworkHandoff == NetworkHandoffState::kIdle);
#endif
    LogErrorOnFailure(CloseConnection(conId));
}

void BLEManagerImpl::CheckNonConcurrentBleClosing()
{
#if !CHIP_DEVICE_CONFIG_SUPPORTS_CONCURRENT_CONNECTION
    VerifyOrReturn(IsBleClosing() && mNetworkHandoff == NetworkHandoffState::kIdle);

    StartNetworkHandoff(NetworkHandoffState::kStartingNetwork);
#endif
}

#if !CHIP_DEVICE_CONFIG_SUPPORTS_CONCURRENT_CONNECTION

bool BLEManagerImpl::HandleBleHandoffEvent(const ChipDeviceEvent & event)
{
    switch (event.Type)
    {
    case DeviceEventType::kPlatformESP32BLELinkConnected:
    case DeviceEventType::kPlatformESP32BLELinkDisconnected: {
        const auto & link    = event.Platform.BLELink;
        const bool connected = event.Type == DeviceEventType::kPlatformESP32BLELinkConnected;
        auto existing        = std::find_if(mBleLinks.begin(), mBleLinks.end(), [&](auto & candidate) {
            if (!candidate.has_value())
            {
                return false;
            }
#ifdef CONFIG_BT_BLUEDROID_ENABLED
            // Both Bluedroid GATT roles can report the same physical link.
            if (connected)
            {
                return std::memcmp(candidate->PeerAddress, link.PeerAddress, sizeof(link.PeerAddress)) == 0;
            }
            return candidate->ConnectionFor(link.Role) == link.Connection;
#else
            return candidate->Connection == link.Connection;
#endif
        });
        if (connected)
        {
            if (existing == mBleLinks.end())
            {
                existing = std::find(mBleLinks.begin(), mBleLinks.end(), std::nullopt);
                VerifyOrDieWithMsg(existing != mBleLinks.end(), DeviceLayer, "No slot for physical BLE link");
#ifdef CONFIG_BT_BLUEDROID_ENABLED
                std::memcpy(existing->emplace().PeerAddress, link.PeerAddress, sizeof(link.PeerAddress));
#else
                *existing = link;
#endif
            }
#ifdef CONFIG_BT_BLUEDROID_ENABLED
            (*existing)->ConnectionFor(link.Role) = link.Connection;
#endif
        }
        else if (existing != mBleLinks.end())
        {
#ifdef CONFIG_BT_BLUEDROID_ENABLED
            (*existing)->ConnectionFor(link.Role).reset();
            if (!(*existing)->ClientConnection.has_value() && !(*existing)->ServerConnection.has_value())
            {
                existing->reset();
            }
#else
            existing->reset();
#endif
        }
        ScheduleNetworkHandoff();
        return true;
    }
    case DeviceEventType::kFailSafeTimerExpired:
        if (mNetworkHandoff != NetworkHandoffState::kIdle ||
            (mServiceMode == ConnectivityManager::kCHIPoBLEServiceMode_Enabled && (IsBleClosing() || !IsInitialized())))
        {
            StartNetworkHandoff(NetworkHandoffState::kRestoringBle);
        }
        return true;

    default:
        return false;
    }
}

void BLEManagerImpl::StartNetworkHandoff(NetworkHandoffState state)
{
    mNetworkHandoff = state;
    ScheduleNetworkHandoff();
    LogErrorOnFailure(SystemLayer().StartTimer(kHandoffRetryInterval, RetryNetworkHandoff, this));
}

void BLEManagerImpl::ScheduleNetworkHandoff()
{
    VerifyOrReturn(mNetworkHandoff != NetworkHandoffState::kIdle && !mNetworkHandoffPosted);
    const ChipDeviceEvent event{ .Type = DeviceEventType::kPlatformESP32BLEHandoff };
    const CHIP_ERROR err  = PlatformMgr().PostEvent(&event);
    mNetworkHandoffPosted = (err == CHIP_NO_ERROR);
    LogErrorOnFailure(err);
}

bool BLEManagerImpl::DisconnectBleLinks()
{
    VerifyOrReturnValue(mNetworkHandoff != NetworkHandoffState::kIdle && !IsInitialized(), false);
    for (auto & link : mBleLinks)
    {
        if (link.has_value())
        {
            LogErrorOnFailure(DisconnectBleLink(link));
        }
    }
    return std::none_of(mBleLinks.begin(), mBleLinks.end(), [](const auto & link) { return link.has_value(); });
}

void BLEManagerImpl::ResetNetworkHandoff()
{
    SystemLayer().CancelTimer(RetryNetworkHandoff, this);
    mNetworkHandoff       = NetworkHandoffState::kIdle;
    mNetworkHandoffPosted = false;
}

void BLEManagerImpl::RetryNetworkHandoff(System::Layer * layer, void * state)
{
    auto & manager = *static_cast<BLEManagerImpl *>(state);
    VerifyOrReturn(manager.mNetworkHandoff != NetworkHandoffState::kIdle);

    manager.ScheduleNetworkHandoff();
    LogErrorOnFailure(layer->StartTimer(kHandoffRetryInterval, RetryNetworkHandoff, &manager));
}

CHIP_ERROR BLEManagerImpl::EnsureBleLayerInitialized()
{
    VerifyOrReturnError(mNetworkHandoff == NetworkHandoffState::kIdle, CHIP_ERROR_BUSY);
    VerifyOrReturnError(mServiceMode == ConnectivityManager::kCHIPoBLEServiceMode_Enabled && !IsInitialized(), CHIP_NO_ERROR);

#ifdef CONFIG_ENABLE_ESP32_BLE_CONTROLLER
    return BleLayer::Init(this, this, this, &SystemLayer());
#else
    return BleLayer::Init(this, this, &SystemLayer());
#endif
}

#endif // !CHIP_DEVICE_CONFIG_SUPPORTS_CONCURRENT_CONNECTION

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
