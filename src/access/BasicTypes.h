/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <cinttypes>

namespace chip {
namespace Access {

// TODO: these basic types need to come from a lower layer
//       "app/util/basic-types.h" has a lot but isn't a lower layer
//       it was preferable to duplicate here for now rather than
//       hook up this module to a higher level module
//       but of course the correct solution is to refactor the
//       basic types into a lower level location that all modules can use
//       note that this will change the namespace of these (should be fine)

typedef uint64_t CatId;
typedef uint32_t ClusterId;
typedef uint32_t DeviceTypeId;
typedef uint16_t EndpointId;
typedef uint8_t FabricIndex;
typedef uint16_t GroupId;
typedef uint64_t NodeId;
typedef uint16_t PasscodeId;

} // namespace Access
} // namespace chip
