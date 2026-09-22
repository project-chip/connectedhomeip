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

#include <app/clusters/boolean-state-server/BooleanStateCluster.h>
#include <lvgl.h>

namespace chip::app {

/**
 * Creates a compact LVGL card displaying BooleanState cluster status and an interactive toggle button
 * to simulate sensor state transitions.
 * Must be called while holding the LVGL lock.
 */
lv_obj_t * CreateBooleanStateClusterWidget(lv_obj_t * parent, Clusters::BooleanStateCluster & cluster,
                                           const char * trueLabel = "Active (True)", const char * falseLabel = "Inactive (False)");

} // namespace chip::app
