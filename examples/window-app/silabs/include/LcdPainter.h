/*
 *
 *    Copyright (c) 2019 Google LLC.
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

#include "app-common/zap-generated/cluster-enums.h"

#include <lcd.h>

#include <stdint.h>

enum class LcdIcon
{
    None = 0,
    One,
    Two,
    Lift,
    Tilt
};

using chip::app::Clusters::WindowCovering::Type;

class LcdPainter
{
public:
    // Binds the display used by static Paint(); call once after GetLCD().Init()
    explicit LcdPainter(SilabsLCD & lcd);
    static void Paint(Type type, uint16_t lift, uint16_t tilt, LcdIcon icon);

private:
    static void ClearScreen();
    static SilabsLCD * sLcd;
    static bool mLcdCleared;
};
