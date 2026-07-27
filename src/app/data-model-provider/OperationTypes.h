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
#include <lib/core/DataModelTypes.h>
#include <lib/support/BitFlags.h>

#include <cstdint>
#include <optional>

namespace chip {
namespace app {
namespace DataModel {

/// This information is available for ALL interactions: read/write/invoke
struct OperationRequest
{
    /// Current authentication data.
    const chip::Access::SubjectDescriptor & subjectDescriptor;

    /// Accessing fabric index is the subjectDescriptor fabric index (if any).
    /// This is a readability convenience function.
    FabricIndex GetAccessingFabricIndex() const { return subjectDescriptor.fabricIndex; }

protected:
    OperationRequest(const Access::SubjectDescriptor & aSubjectDescriptor) : subjectDescriptor(aSubjectDescriptor) {}
};

enum class ReadFlags : uint32_t
{
    kFabricFiltered     = 0x0001, // reading is performed fabric-filtered
    kAllowsLargePayload = 0x0002, // reading is performed over a transport supporting large payload
};

enum class ListWriteOperation : uint8_t
{
    kListWriteBegin = 0,
    kListWriteSuccess,
    kListWriteFailure
};

struct ReadAttributeRequest : OperationRequest
{
    ConcreteAttributePath path;
    BitFlags<ReadFlags> readFlags;

    ReadAttributeRequest(const ConcreteAttributePath & aPath, const Access::SubjectDescriptor & aSubjectDescriptor) :
        OperationRequest(aSubjectDescriptor), path(aPath)
    {}
};

enum class WriteFlags : uint32_t
{
    kTimed = 0x0001, // Write is a timed write (i.e. a Timed Request Action preceeded it)
};

struct WriteAttributeRequest : OperationRequest
{
    ConcreteDataAttributePath path; // NOTE: this also contains LIST operation options (i.e. "data" path type)
    BitFlags<WriteFlags> writeFlags;

    WriteAttributeRequest(const ConcreteDataAttributePath & aPath, const Access::SubjectDescriptor & aSubjectDescriptor) :
        OperationRequest(aSubjectDescriptor), path(aPath)
    {}
};

enum class InvokeFlags : uint32_t
{
    kTimed = 0x0001, // Command received as part of a timed invoke interaction.
};

struct InvokeRequest : OperationRequest
{
    ConcreteCommandPath path;
    BitFlags<InvokeFlags> invokeFlags;

    InvokeRequest(const ConcreteCommandPath & aPath, const Access::SubjectDescriptor & aSubjectDescriptor) :
        OperationRequest(aSubjectDescriptor), path(aPath)
    {}
};

} // namespace DataModel
} // namespace app
} // namespace chip
