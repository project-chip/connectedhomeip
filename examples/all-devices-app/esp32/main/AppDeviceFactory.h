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

#include "sdkconfig.h"
#include <device-factory/DeviceFactory.h>

#if CONFIG_DISPLAY_LVGL
#include "DeviceScreenHook.h"

namespace chip::app {
using AppDeviceFactory = DeviceFactory<DeviceScreenHook>;
} // namespace chip::app
#else
namespace chip::app {
using AppDeviceFactory = NoHooksDeviceFactory;
} // namespace chip::app
#endif
