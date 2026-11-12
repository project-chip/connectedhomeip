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

// Platform abstraction hooks for the Joint Fabric Administrator example.
//
// The jf-admin-app shared code lives in examples/jf-admin-app/jfa-common and
// is OS-agnostic. The reference Linux example brings up its commissioner role
// via CommissionerMain.h helpers (InitCommissioner / GetDeviceCommissioner).
// Those helpers are Linux-only, so the shared code must not call them
// directly. Instead, each platform implements the two hooks declared below
// (for Linux: examples/jf-admin-app/linux/JFAPlatformLinux.cpp) and the shared
// JFAManagerBase / JFADatastoreSync code calls through this interface.

#pragma once

#include <controller/CHIPDeviceController.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>

namespace chip {

// Initialize the local commissioner role on the joint fabric. Called after the
// device has been commissioned as a Joint Fabric Administrator. Implementations
// bring up a DeviceCommissioner bound to the given fabric.
CHIP_ERROR JFAInitCommissioner(FabricId fabricId, FabricIndex fabricIndex);

// Returns the DeviceCommissioner initialized by JFAInitCommissioner(), or
// nullptr if the commissioner role has not been initialized yet.
Controller::DeviceCommissioner * JFAGetDeviceCommissioner();

} // namespace chip
