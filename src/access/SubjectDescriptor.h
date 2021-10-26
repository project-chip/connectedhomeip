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

#include "AuthMode.h"

#include <lib/core/DataModelTypes.h>
#include <lib/core/GroupId.h>
#include <lib/core/NodeId.h>
#include <lib/core/PasscodeId.h>

namespace chip {
namespace Access {

union SubjectId
{
    PasscodeId passcode;
    NodeId node;
    GroupId group;
};

struct SubjectDescriptor
{
    // Holds FabricIndex of fabric, 0 if no fabric.
    FabricIndex fabricIndex = 0;

    // Holds AuthMode of subject(s), kNone if no access.
    AuthMode authMode = AuthMode::kNone;

    // NOTE: due to packing there should be free bytes here

    // Holds subjects according to auth mode, and the latter two are only valid
    // if auth mode is CASE.
    SubjectId subjects[3] = {};
};

} // namespace Access
} // namespace chip
