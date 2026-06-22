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
#include <DeviceFactoryPlatformOverride.h>
#include <PosixChimeDevice.h>
#include <app_config/enabled_devices.h>
#include <devices/device-factory/DeviceFactory.h>

namespace chip {
namespace app {

void RegisterDeviceFactoryOverrides(TimerDelegate & timerDelegate)
{
    if constexpr (ALL_DEVICES_ENABLE_CHIME)
    {
        DeviceFactory::GetInstance().RegisterCreator("chime", [&timerDelegate]() {
            static const ChimeDevice::Sound kDefaultSounds[] = {
                { 0, "Ding Dong"_span },
                { 1, "Ring Ring"_span },
            };
            return std::make_unique<PosixChimeDevice>(timerDelegate, Span<const ChimeDevice::Sound>(kDefaultSounds));
        });
    }
}

} // namespace app
} // namespace chip
