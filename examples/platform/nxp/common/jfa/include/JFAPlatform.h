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

// Platform abstraction hooks for the Joint Fabric Administrator port.
//
// The reference jf-admin-app (Linux) relies on CommissionerMain.h helpers
// (InitCommissioner / GetDeviceCommissioner). Those live in the Linux example
// platform and are not available on the NXP FreeRTOS port. To keep the shared
// JFAManager / JFADatastoreSync logic platform agnostic, the application is
// expected to provide the following hooks. They are declared with C++ linkage
// in the chip namespace and implemented by the NXP application layer.

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
