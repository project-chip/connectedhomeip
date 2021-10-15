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

/**
 *    @file
 *      Basic types used by access control.
 */

#pragma once

#include <cinttypes>

namespace chip {
namespace access {

// TODO: these basic types need to come from a lower layer
//       "app/util/basic-types.h" has a lot but isn't a lower layer
//       so organization and refactoring needs to happen sometime
//       and this will change the namespace also

typedef uint64_t CatId;
typedef uint32_t ClusterId;
typedef uint32_t DeviceTypeId;
typedef uint16_t EndpointId;
typedef uint8_t FabricIndex;
typedef uint16_t GroupId;
typedef uint64_t NodeId;
typedef uint16_t PasscodeId;

} // namespace access
} // namespace chip
