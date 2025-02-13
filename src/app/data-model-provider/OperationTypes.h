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

/// Contains common flags among all interaction model operations: read/write/invoke
enum class OperationFlags : uint32_t
{
    // NOTE: temporary flag. This flag exists to faciliate transition from ember-compatibilty-functions
    //       implementation to DataModel Interface functionality. Specifically currently the
    //       model is expected to perform ACL and readability/writability checks.
    //
    //       In the future, this flag will be removed and InteractionModelEngine/ReportingEngine
    //       will perform the required validation.
    //
    //       Currently the flag FORCES a bypass of:
    //         - ACL validation (will allow any read/write)
    //         - Access validation (will allow reading write-only data for example)
    kInternal = 0x0001,
};

/// This information is available for ALL interactions: read/write/invoke
struct OperationRequest
{
    BitFlags<OperationFlags> operationFlags;

    /// Current authentication data EXCEPT for internal requests.
    ///  - Non-internal requests MUST have this set.
    ///  - operationFlags.Has(OperationFlags::kInternal) MUST NOT have this set
    ///
    /// NOTE: once kInternal flag is removed, this will become non-optional
    const chip::Access::SubjectDescriptor * subjectDescriptor = nullptr;

    /// Accessing fabric index is the subjectDescriptor fabric index (if any).
    /// This is a readability convenience function.
    ///
    /// Returns kUndefinedFabricIndex if no subject descriptor is available
    FabricIndex GetAccessingFabricIndex() const
    {
        VerifyOrReturnValue(subjectDescriptor != nullptr, kUndefinedFabricIndex);
        return subjectDescriptor->fabricIndex;
    }
};

enum class ReadFlags : uint32_t
{
    kFabricFiltered = 0x0001, // reading is performed fabric-filtered
};

struct ReadAttributeRequest : OperationRequest
{
    ConcreteAttributePath path;
    BitFlags<ReadFlags> readFlags;
};

enum class WriteFlags : uint32_t
{
    kTimed = 0x0001, // Write is a timed write (i.e. a Timed Request Action preceeded it)
};

struct WriteAttributeRequest : OperationRequest
{
    ConcreteDataAttributePath path; // NOTE: this also contains LIST operation options (i.e. "data" path type)
    BitFlags<WriteFlags> writeFlags;
};

enum class InvokeFlags : uint32_t
{
    kTimed = 0x0001, // Command received as part of a timed invoke interaction.
};

struct InvokeRequest : OperationRequest
{
    ConcreteCommandPath path;
    BitFlags<InvokeFlags> invokeFlags;
};

} // namespace DataModel
} // namespace app
} // namespace chip
