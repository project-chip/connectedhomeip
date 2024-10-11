/*
 *    Copyright (c) 2024 Project CHIP Authors
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

#include <access/SubjectDescriptor.h>
#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Testing {

constexpr FabricIndex kTestFabrixIndex = kMinValidFabricIndex;
constexpr NodeId kTestNodeId           = 0xFFFF'1234'ABCD'4321;

constexpr Access::SubjectDescriptor kAdminSubjectDescriptor{
    .fabricIndex = kTestFabrixIndex,
    .authMode    = Access::AuthMode::kCase,
    .subject     = kTestNodeId,
};

constexpr Access::SubjectDescriptor kViewSubjectDescriptor{
    .fabricIndex = kTestFabrixIndex + 1,
    .authMode    = Access::AuthMode::kCase,
    .subject     = kTestNodeId,
};

constexpr Access::SubjectDescriptor kDenySubjectDescriptor{
    .fabricIndex = kTestFabrixIndex + 2,
    .authMode    = Access::AuthMode::kCase,
    .subject     = kTestNodeId,
};

} // namespace Testing
} // namespace app
} // namespace chip
