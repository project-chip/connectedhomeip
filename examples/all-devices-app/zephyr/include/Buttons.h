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

#include <zephyr/devicetree.h>

#if DT_NODE_EXISTS(DT_ALIAS(sw0))
#define ALL_DEVICES_FACTORY_RESET_SW_NODE DT_ALIAS(sw0)
#endif

#if defined(ALL_DEVICES_FACTORY_RESET_SW_NODE)
#define ALL_DEVICES_HAS_FACTORY_RESET_SW 1
#else
#define ALL_DEVICES_HAS_FACTORY_RESET_SW 0
#endif

namespace chip::app::AllDevices::Button {

/// Configures the factory-reset button when it exists.
void Init();

} // namespace chip::app::AllDevices::Button
