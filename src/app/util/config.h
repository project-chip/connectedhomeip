/**
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include <lib/core/CHIPConfig.h>

#if !CHIP_CONFIG_SKIP_APP_SPECIFIC_GENERATED_HEADER_INCLUDES

#include <zap-generated/gen_config.h>

#include <zap-generated/endpoint_config.h>

#endif // !CHIP_CONFIG_SKIP_APP_SPECIFIC_GENERATED_HEADER_INCLUDES

// Provide numeric defaults for optional plugin macros so that feature checks
// using `#if MATTER_DM_PLUGIN_*` evaluate safely even when the underlying
// configuration does not declare the macro.
#ifdef MATTER_DM_PLUGIN_SCENES_MANAGEMENT
#undef MATTER_DM_PLUGIN_SCENES_MANAGEMENT
#define MATTER_DM_PLUGIN_SCENES_MANAGEMENT 1
#else
#define MATTER_DM_PLUGIN_SCENES_MANAGEMENT 0
#endif

#ifdef MATTER_DM_PLUGIN_MODE_BASE
#undef MATTER_DM_PLUGIN_MODE_BASE
#define MATTER_DM_PLUGIN_MODE_BASE 1
#else
#define MATTER_DM_PLUGIN_MODE_BASE 0
#endif

#ifdef MATTER_DM_PLUGIN_MODE_SELECT
#undef MATTER_DM_PLUGIN_MODE_SELECT
#define MATTER_DM_PLUGIN_MODE_SELECT 1
#else
#define MATTER_DM_PLUGIN_MODE_SELECT 0
#endif

#ifdef MATTER_DM_PLUGIN_LEVEL_CONTROL
#undef MATTER_DM_PLUGIN_LEVEL_CONTROL
#define MATTER_DM_PLUGIN_LEVEL_CONTROL 1
#else
#define MATTER_DM_PLUGIN_LEVEL_CONTROL 0
#endif

#ifndef MATTER_DM_PLUGIN_LEVEL_CONTROL_RATE
#define MATTER_DM_PLUGIN_LEVEL_CONTROL_RATE 0
#endif

#ifndef MATTER_DM_PLUGIN_LEVEL_CONTROL_MINIMUM_LEVEL
#define MATTER_DM_PLUGIN_LEVEL_CONTROL_MINIMUM_LEVEL 0
#endif

#ifndef MATTER_DM_PLUGIN_LEVEL_CONTROL_MAXIMUM_LEVEL
#define MATTER_DM_PLUGIN_LEVEL_CONTROL_MAXIMUM_LEVEL 0
#endif

#ifdef MATTER_DM_PLUGIN_ON_OFF
#undef MATTER_DM_PLUGIN_ON_OFF
#define MATTER_DM_PLUGIN_ON_OFF 1
#else
#define MATTER_DM_PLUGIN_ON_OFF 0
#endif

#ifdef MATTER_DM_PLUGIN_COLOR_CONTROL_SERVER_TEMP
#undef MATTER_DM_PLUGIN_COLOR_CONTROL_SERVER_TEMP
#define MATTER_DM_PLUGIN_COLOR_CONTROL_SERVER_TEMP 1
#else
#define MATTER_DM_PLUGIN_COLOR_CONTROL_SERVER_TEMP 0
#endif

#ifdef MATTER_DM_PLUGIN_COLOR_CONTROL_SERVER_XY
#undef MATTER_DM_PLUGIN_COLOR_CONTROL_SERVER_XY
#define MATTER_DM_PLUGIN_COLOR_CONTROL_SERVER_XY 1
#else
#define MATTER_DM_PLUGIN_COLOR_CONTROL_SERVER_XY 0
#endif

#ifdef MATTER_DM_PLUGIN_COLOR_CONTROL_SERVER_HSV
#undef MATTER_DM_PLUGIN_COLOR_CONTROL_SERVER_HSV
#define MATTER_DM_PLUGIN_COLOR_CONTROL_SERVER_HSV 1
#else
#define MATTER_DM_PLUGIN_COLOR_CONTROL_SERVER_HSV 0
#endif
