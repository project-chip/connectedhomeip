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

#include <app/clusters/level-control/LevelControlCluster.h>
#include <lvgl.h>

namespace chip::app {

/**
 * Creates an interactive LevelControl cluster widget displaying level slider and percentage.
 * `levelName` prefixes the value text (e.g. "Volume: 40% (101)"). It is not copied, so it must
 * outlive the widget (use a string literal).
 * Must be called while holding the LVGL lock.
 */
lv_obj_t * CreateLevelControlClusterWidget(lv_obj_t * parent, Clusters::LevelControlCluster & cluster,
                                           const char * levelName = "Brightness");

} // namespace chip::app
