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

#include <app/clusters/identify-server/IdentifyCluster.h>
#include <lvgl.h>

namespace chip::app {

/// Shows the remaining identify time and allows stopping it. Identify cannot be started here:
/// IdentifyCluster::SetIdentifyTime is private, so only a client write or command starts it.
lv_obj_t * CreateIdentifyClusterWidget(lv_obj_t * parent, Clusters::IdentifyCluster & cluster);

} // namespace chip::app
