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

#include "DisplayBoard.h"

#include <bsp/esp-bsp.h>

namespace DisplayBoard {
namespace {

// bsp_display_lock() treats 0 as "wait indefinitely".
constexpr uint32_t kWaitForever = 0;

} // namespace

lv_display_t * Start()
{
    return bsp_display_start();
}

bool Lock()
{
    return bsp_display_lock(kWaitForever);
}

void Unlock()
{
    bsp_display_unlock();
}

int32_t EdgeInset()
{
    return 0;
}

void SetBrightness(int percent)
{
    ESP_ERROR_CHECK_WITHOUT_ABORT(bsp_display_brightness_set(percent));
}

const char * BoardName()
{
    return "M5Stack CoreS3";
}

} // namespace DisplayBoard
