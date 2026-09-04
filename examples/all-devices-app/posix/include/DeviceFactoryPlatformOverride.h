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
#pragma once

#include <credentials/FabricTable.h>
#include <lib/core/CHIPPersistentStorageDelegate.h>
#include <lib/support/TimerDelegate.h>

#include <PosixAudioManager.h>
#include <PosixChime.h>
#include <PosixSpeaker.h>
#include <app_config/enabled_devices.h>
#include <device-factory/DeviceFactory.h>

namespace chip {
namespace app {

template <typename Factory>
void RegisterDeviceFactoryOverrides(Factory & factory, TimerDelegate & timerDelegate, PersistentStorageDelegate * storageDelegate,
                                    PosixAudioManager & audioManager)
{
    if constexpr (ALL_DEVICES_ENABLE_SPEAKER)
    {
        factory.RegisterCreator("speaker", [&timerDelegate, &audioManager]() {
            return Factory::template MakeCreatedDevice<PosixSpeaker>(PosixSpeaker::Context{ timerDelegate }, audioManager);
        });
    }

    if constexpr (ALL_DEVICES_ENABLE_CHIME)
    {
        factory.RegisterCreator("chime", [&timerDelegate, &audioManager]() {
            return Factory::template MakeCreatedDevice<PosixChime>(timerDelegate, audioManager);
        });
    }
}

} // namespace app
} // namespace chip
