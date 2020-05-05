/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2019 Google LLC.
 *    Copyright (c) 2013-2017 Nest Labs, Inc.
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

#ifndef ID_CONSTANTS_H
#define ID_CONSTANTS_H

namespace chip {

// Special node id values.
enum
{
    kNodeIdNotSpecified = 0ULL,
    kAnyNodeId          = 0xFFFFFFFFFFFFFFFFULL
};

// Special fabric id values.
enum
{
    kFabricIdNotSpecified = 0ULL,

    /** Default fabric ID, which should only be used for testing purposes. */
    kFabricIdDefaultForTest = 1ULL,

    // Ids >= kReservedFabricIdStart and <= kMaxFabricId are reserved for special uses.  The
    // meanings of values in this range are context-specific; e.g. in the IdentifyRequest
    // message, these value are used to match devices that are/are not members of a fabric.
    kReservedFabricIdStart = 0xFFFFFFFFFFFFFF00ULL,

    kMaxFabricId = 0xFFFFFFFFFFFFFFFFULL
};

} // namespace chip

#endif