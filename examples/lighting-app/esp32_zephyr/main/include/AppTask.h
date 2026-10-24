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

#include <cstdint>

#include <lib/core/CHIPError.h>

class AppTask
{
public:
    static AppTask & Instance()
    {
        static AppTask sAppTask;
        return sAppTask;
    }

    // Brings up the whole Matter stack, then hands control to the CHIP event
    // loop thread. Blocks the calling (main) thread forever.
    CHIP_ERROR StartApp();

    // Reflects the On/Off cluster attribute onto the physical LED.
    // Safe to call from the Matter (CHIP) thread — it is a direct GPIO write.
    void SetOnOffLED(bool on);

private:
    CHIP_ERROR Init();
};
