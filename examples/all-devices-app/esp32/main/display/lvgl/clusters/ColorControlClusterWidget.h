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

#include <app/clusters/color-control-server/ColorControlCluster.h>
#include <lvgl.h>

namespace chip::app {

/// Colour card showing the controls of the colour mode the cluster is currently in: hue and
/// saturation, xy, or colour temperature. A mode is offered only when the cluster supports the
/// matching feature; the mode selector is omitted when only one mode is supported.
lv_obj_t * CreateColorControlClusterWidget(lv_obj_t * parent, Clusters::ColorControlCluster & cluster);

} // namespace chip::app
