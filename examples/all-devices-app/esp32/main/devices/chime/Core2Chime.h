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

#include "TonePlayingChime.h"

namespace chip::app {

/// Chime on the M5Stack Core2: tones from TonePlayingChime through the onboard I2S speaker.
class Core2Chime : public TonePlayingChime
{
public:
    explicit Core2Chime(TimerDelegate & timerDelegate);
    ~Core2Chime() override = default;
};

} // namespace chip::app
