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
#include "BasicTypes.h"

namespace chip {
namespace access {

/**
 * @typedef SubjectId
 * 
 * @brief Access control subject identifier. Can store a PasscodeId, NodeId,
 * CatId, GroupId, etc. as appropriate.
 */
typedef uint64_t SubjectId;

/**
 * @class SubjectDescriptor
 * 
 * @brief Access control subject descriptor.
 */
struct SubjectDescriptor
{
    // Holds PasscodeId if kPase, NodeId if kCase, GroupId if kGroup
    SubjectId subject = 0;

    // Holds (optional) CAT1/CAT2 if kCase
    SubjectId subject1 = 0;

    // Holds (optional) CAT1/CAT2 if kCase
    SubjectId subject2 = 0;

    // Holds AuthMode of subject(s), kNone if no access
    AuthMode authMode = AuthMode::kNone;

    // Holds FabricIndex of fabric, 0 if no fabric
    FabricIndex fabricIndex = 0;

    // Holds whether the subject is commissioning
    bool isCommissioning = false;
};

} // namespace access
} // namespace chip
