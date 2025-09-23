/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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
#include "connectivity.h"

#include <imgui.h>

#include <platform/ConnectivityManager.h>
#include <platform/PlatformManager.h>

namespace example {
namespace Ui {
namespace Windows {

using namespace chip::DeviceLayer;

Connectivity::Connectivity()
{
    PlatformMgr().AddEventHandler(
        [](const ChipDeviceEvent * event, intptr_t self) { reinterpret_cast<Connectivity *>(self)->ChipEventHandler(event); },
        reinterpret_cast<intptr_t>(this));
}

void Connectivity::ChipEventHandler(const ChipDeviceEvent * event)
{
    switch (event->Type)
    {
#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
    case DeviceEventType::kCHIPoBLEAdvertisingChange:
        mBLEAdvertising = event->CHIPoBLEAdvertisingChange.Result == ActivityChange::kActivity_Started;
        break;
#endif
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
    case DeviceEventType::kWiFiConnectivityChange:
        mWiFiEnabled     = chip::DeviceLayer::ConnectivityMgr().IsWiFiStationEnabled();
        mWiFiProvisioned = chip::DeviceLayer::ConnectivityMgr().IsWiFiStationProvisioned();
        mWiFiAttached    = chip::DeviceLayer::ConnectivityMgr().IsWiFiStationConnected();
        break;
#endif
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
    case DeviceEventType::kThreadConnectivityChange:
        mThreadEnabled     = chip::DeviceLayer::ConnectivityMgr().IsThreadEnabled();
        mThreadProvisioned = chip::DeviceLayer::ConnectivityMgr().IsThreadProvisioned();
        break;
#endif
    default:
        break;
    }
}

void Connectivity::LoadInitialState()
{
#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
    mBLEAdvertising = chip::DeviceLayer::ConnectivityMgr().IsBLEAdvertising();
    mBLEConnected   = chip::DeviceLayer::ConnectivityMgr().NumBLEConnections() != 0;
#endif
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
    mWiFiEnabled     = chip::DeviceLayer::ConnectivityMgr().IsWiFiStationEnabled();
    mWiFiProvisioned = chip::DeviceLayer::ConnectivityMgr().IsWiFiStationProvisioned();
    mWiFiAttached    = chip::DeviceLayer::ConnectivityMgr().IsWiFiStationConnected();
#endif
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
    mThreadEnabled     = chip::DeviceLayer::ConnectivityMgr().IsThreadEnabled();
    mThreadProvisioned = chip::DeviceLayer::ConnectivityMgr().IsThreadProvisioned();
#endif
}

void Connectivity::UpdateState()
{
#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
    // TODO: Allow BLE connection state to be updated by ChipEventHandler. Unfortunately,
    //       as for now, the BLE connection/disconnection logic is a bit messy and because
    //       of that events are not posted for all connection state changes.
    mBLEConnected = chip::DeviceLayer::ConnectivityMgr().NumBLEConnections() != 0;
#endif
}

void Connectivity::Render()
{
    ImGui::Begin("Connectivity");

#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
    ImGui::Text("BLE:");
    ImGui::Indent();
    ImGui::RadioButton("Advertising", mBLEAdvertising);
    ImGui::RadioButton("Connected", mBLEConnected);
    ImGui::Unindent();
#endif

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
    ImGui::Text("Wi-Fi:");
    ImGui::Indent();
    ImGui::RadioButton("Enabled##wifi", mWiFiEnabled);
    ImGui::RadioButton("Provisioned##wifi", mWiFiProvisioned);
    ImGui::RadioButton("Attached##wifi", mWiFiAttached);
    ImGui::Unindent();
#endif

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
    ImGui::Text("Thread:");
    ImGui::Indent();
    ImGui::RadioButton("Enabled##thread", mThreadEnabled);
    ImGui::RadioButton("Provisioned##thread", mThreadProvisioned);
    ImGui::Unindent();
#endif

    ImGui::End();
}

} // namespace Windows
} // namespace Ui
} // namespace example
