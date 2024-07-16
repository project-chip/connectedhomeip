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

#include <lib/core/TLVReader.h>
#include <lib/core/TLVWriter.h>

#include <app/AttributeValueDecoder.h>
#include <app/AttributeValueEncoder.h>
#include <app/CommandHandler.h>

#include <app/data-model-interface/Context.h>
#include <app/data-model-interface/MetadataTypes.h>
#include <app/data-model-interface/OperationTypes.h>

namespace chip {
namespace app {
namespace InteractionModel {

/// Represents operations against a matter-defined data model.
///
/// Class is SINGLE-THREADED:
///   - operations are assumed to only be ever run in a single event-loop
///     thread or equivalent
///   - class is allowed to attempt to cache indexes/locations for faster
///     lookups of things (e.g during iterations)
class DataModel : public DataModelMetadataTree
{
public:
    virtual ~DataModel() = default;

    // `context` pointers  will be guaranteed valid until Shutdown is called()
    virtual CHIP_ERROR Startup(InteractionModelContext context)
    {
        mContext = context;
        return CHIP_NO_ERROR;
    }
    virtual CHIP_ERROR Shutdown() = 0;

    // During the transition phase, we expect a large subset of code to require access to
    // event emitting, path marking and other operations
    virtual InteractionModelContext CurrentContext() const { return mContext; }

    /// TEMPORARY/TRANSITIONAL requirement for transitioning from ember-specific code
    ///   ReadAttribute is REQUIRED to perform:
    ///     - ACL validation (see notes on OperationFlags::kInternal)
    ///     - Validation of readability/writability (also controlled by OperationFlags::kInternal)
    ///     - use request.path.mExpanded to skip encoding replies for data according
    ///       to 8.4.3.2 of the spec:
    ///         > If the path indicates attribute data that is not readable, then the path SHALL
    ///           be discarded.
    ///         > Else if reading from the attribute in the path requires a privilege that is not
    ///           granted to access the cluster in the path, then the path SHALL be discarded.
    ///
    /// Return codes:
    ///   CHIP_ERROR_NO_MEMORY or CHIP_ERROR_BUFFER_TOO_SMALL:
    ///      - Indicates that list encoding had insufficient buffer space to encode elements.
    ///      - encoder::GetState().AllowPartialData() determines if these errors are permanent (no partial
    ///        data allowed) or further encoding can be retried (AllowPartialData true for list encoding)
    ///   CHIP_IM_GLOBAL_STATUS(code):
    ///      - error codes that are translatable in IM status codes (otherwise we expect Failure to be reported)
    ///      - to check for this, CHIP_ERROR provides:
    ///        - ::IsPart(ChipError::SdkPart::kIMGlobalStatus) -> bool
    ///        - ::GetSdkCode() -> uint8_t to translate to the actual code
    ///   other internal falures
    virtual CHIP_ERROR ReadAttribute(const ReadAttributeRequest & request, AttributeValueEncoder & encoder) = 0;

    /// Requests a write of an attribute.
    ///
    /// When this is invoked, caller is expected to have already done some validations:
    ///    - cluster `data version` has been checked for the incoming request if applicable
    ///
    /// TEMPORARY/TRANSITIONAL requirement for transitioning from ember-specific code
    ///   WriteAttribute is REQUIRED to perform:
    ///     - ACL validation (see notes on OperationFlags::kInternal)
    ///     - Validation of readability/writability (also controlled by OperationFlags::kInternal)
    ///     - Validation of timed interaction required (also controlled by OperationFlags::kInternal)
    ///
    /// Return codes
    ///   CHIP_IM_GLOBAL_STATUS(code):
    ///       - error codes that are translatable to specific IM codes
    ///       - in particular, the following codes are interesting/expected
    ///         - `UnsupportedWrite` for attempts to write read-only data
    ///         - `UnsupportedAccess` for ACL failures
    ///         - `NeedsTimedInteraction` for writes that are not timed however are required to be so
    virtual CHIP_ERROR WriteAttribute(const WriteAttributeRequest & request, AttributeValueDecoder & decoder) = 0;

    /// `handler` is used to send back the reply.
    ///    - returning a value other than CHIP_NO_ERROR implies an error reply (error and data are mutually exclusive)
    virtual CHIP_ERROR Invoke(const InvokeRequest & request, chip::TLV::TLVReader & input_arguments, CommandHandler * handler) = 0;

private:
    InteractionModelContext mContext = { nullptr };
};

} // namespace InteractionModel
} // namespace app
} // namespace chip
