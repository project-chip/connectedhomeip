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

#include <lvgl.h>

/**
 * What the LVGL renderer needs from the board's display hardware. Exactly one
 * backend under display/lvgl/board/ is compiled per board.
 */
namespace DisplayBoard {

/// Returns the LVGL display, or nullptr if the panel did not come up.
lv_display_t * Start();

/// Waits indefinitely for the LVGL lock.
bool Lock();

void Unlock();

/// 0 blanks the panel; touch input keeps working.
void SetBrightness(int percent);

const char * BoardName();

/// Pixels to keep clear along every edge so that panel corners do not clip the UI.
int32_t EdgeInset();

} // namespace DisplayBoard
