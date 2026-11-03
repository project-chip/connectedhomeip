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
 * Mirrors a commissioning session on the display: the QR code screen shows the
 * current stage, the bar shows connectivity, and a successful commissioning
 * returns the UI to Home.
 */
namespace CommissioningMonitor {

/// Call on the CHIP thread once Server::Init() has run.
void Init();

/// Icon plus stage text, kept current while it exists.
lv_obj_t * CreateStatusView(lv_obj_t * parent);

/// Fabrics on the node, kept current as fabrics are added and removed. Call with the display lock held.
uint8_t FabricCount();

} // namespace CommissioningMonitor
