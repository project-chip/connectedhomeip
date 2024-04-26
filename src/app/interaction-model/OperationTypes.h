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
#include <app/ConcreteAttributePath.h>
#include <app/ConcreteCommandPath.h>
#include <lib/support/BitFlags.h>

#include <cstdint>
#include <optional>

namespace chip {
namespace app {
namespace InteractionModel {

/// Contains common flags among all interaction model operations: read/write/invoke
enum class OperationFlags : uint32_t
{
    kInternal = 0x0001, // Internal request for data changes (can bypass checks/ACL etc.)
};

/// This information is available for ALL interactions: read/write/invoke
struct OperationRequest
{
    OperationFlags operationFlags;

    /// Current authentication data EXCEPT for internal requests.
    ///  - Non-internal requests MUST have this set.
    ///  - operationFlags.Has(OperationFlags::kInternal) MUST NOT have this set
    std::optional<chip::Access::SubjectDescriptor> subjectDescriptor;
};

enum class ReadFlags : uint32_t
{
    kFabricFiltered = 0x0001, // reading is performed fabric-filtered
};

struct ReadAttributeRequest : OperationRequest
{
    ConcreteAttributePath path;
    std::optional<DataVersion> dataVersion;
    BitFlags<ReadFlags> readFlags;
};

struct ReadState
{
    // When reading lists, reading will start at this index.
    // As list data is read, this index is incremented
    ListIndex listEncodeStart = kInvalidListIndex;
};

enum class WriteFlags : uint32_t
{
    kTimed     = 0x0001, // Received as a 2nd command after a timed invoke
    kListBegin = 0x0002, // This is the FIRST list data element in a series of data
    kListEnd   = 0x0004, // This is the LAST list element to write
};

struct WriteAttributeRequest : OperationRequest
{
    ConcreteDataAttributePath path; // NOTE: this also contains LIST operation options (i.e. "data" path type)
    BitFlags<WriteFlags> writeFlags;
};

enum class InvokeFlags : uint32_t
{
    kTimed = 0x0001, // Received as a 2nd command after a timed invoke
};

struct InvokeRequest : OperationRequest
{
    ConcreteCommandPath path;
  std::optional<GroupId> groupRequestId; // set if and only if this was a group request
    BitFlags<InvokeFlags> invokeFlags;
};

} // namespace InteractionModel
} // namespace app
} // namespace chip
