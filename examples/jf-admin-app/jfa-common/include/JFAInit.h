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

#include <app/server/Server.h>
#include <lib/core/CHIPError.h>

namespace chip {

// OS-agnostic wire-up of the Joint Fabric Administrator into a running Server.
//
// Performs only the steps that are identical on every platform:
//   - initialize JFAManager (via JFAMgr()) and JFADatastoreSync (via JFADSync())
//   - register them as the JointFabricAdministrator / JointFabricDatastore delegates
//   - install a DeviceInstanceInfoProvider wrapper that reports the local JF mode
//     while forwarding all other queries to the platform provider
//   - register an event handler for commissioning-complete and DNS-SD-initialized
//
// Must be called after the Matter Server has been initialized. Platform-specific
// setup (for example initializing the JointFabricDatastore persistent storage) is
// deliberately not done here; each platform performs its own extra steps around
// this call.
CHIP_ERROR JFAInitialize(Server & server);

} // namespace chip
