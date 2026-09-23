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

#include <app/clusters/valve-configuration-and-control-server/ValveConfigurationAndControlCluster.h>
#include <lvgl.h>

namespace chip::app {

/**
 * Creates an interactive ValveConfigurationAndControl cluster widget showing the valve state, the
 * remaining open duration, and Open/Close buttons. A target level slider is added when the cluster
 * supports the LVL feature.
 * Must be called while holding the LVGL lock.
 */
lv_obj_t * CreateValveConfigurationAndControlClusterWidget(lv_obj_t * parent,
                                                           Clusters::ValveConfigurationAndControlCluster & cluster);

} // namespace chip::app
