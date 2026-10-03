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

#include <lib/core/CHIPError.h>

namespace chip {
namespace NXP {
namespace App {
namespace JFA {

// Wire up the Joint Fabric Administrator into the running Server:
//  - initialize JFAManager and JFADatastoreSync
//  - register them as the JointFabricAdministrator / JointFabricDatastore delegates
//  - register a commissioning-complete event handler
// Must be called after the Matter Server has been initialized.
CHIP_ERROR Init();

} // namespace JFA
} // namespace App
} // namespace NXP
} // namespace chip
