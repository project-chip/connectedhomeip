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
#pragma once

#include <platform/CHIPDeviceConfig.h>
#include <platform/CHIPDeviceEvent.h>

#include "window.h"

namespace example {
namespace Ui {
namespace Windows {

class Connectivity : public Window
{
public:
    Connectivity();

    void LoadInitialState() override;
    void UpdateState() override;
    void Render() override;

private:
    void ChipEventHandler(const chip::DeviceLayer::ChipDeviceEvent * event);

#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
    bool mBLEAdvertising = false;
    bool mBLEConnected   = false;
#endif
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
    bool mWiFiEnabled     = false;
    bool mWiFiProvisioned = false;
    bool mWiFiAttached    = false;
#endif
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
    bool mThreadEnabled     = false;
    bool mThreadProvisioned = false;
#endif
};

} // namespace Windows
} // namespace Ui
} // namespace example
