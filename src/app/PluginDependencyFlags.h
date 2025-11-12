/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <app/util/config.h>

#if defined(MATTER_DM_PLUGIN_ON_OFF)
#undef MATTER_DM_PLUGIN_ON_OFF
#define MATTER_DM_PLUGIN_ON_OFF 1
#else
#define MATTER_DM_PLUGIN_ON_OFF 0
#endif

#if defined(MATTER_DM_PLUGIN_SCENES_MANAGEMENT)
#undef MATTER_DM_PLUGIN_SCENES_MANAGEMENT
#define MATTER_DM_PLUGIN_SCENES_MANAGEMENT 1
#else
#define MATTER_DM_PLUGIN_SCENES_MANAGEMENT 0
#endif

#if defined(MATTER_DM_PLUGIN_COLOR_CONTROL_SERVER_TEMP)
#undef MATTER_DM_PLUGIN_COLOR_CONTROL_SERVER_TEMP
#define MATTER_DM_PLUGIN_COLOR_CONTROL_SERVER_TEMP 1
#else
#define MATTER_DM_PLUGIN_COLOR_CONTROL_SERVER_TEMP 0
#endif
