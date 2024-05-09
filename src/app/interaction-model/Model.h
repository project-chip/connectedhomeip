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

#include <app/interaction-model/Actions.h>
#include <app/interaction-model/InvokeResponder.h>
#include <app/interaction-model/IterationTypes.h>
#include <app/interaction-model/OperationTypes.h>

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
class Model : public AttributeTreeIterator
{
public:
    virtual ~Model() = default;

    // `actions` pointers  will be guaranteed valid until Shutdown is called()
    virtual CHIP_ERROR Startup(InteractionModelActions actions)
    {
        mActions = actions;
        return CHIP_NO_ERROR;
    }
    virtual CHIP_ERROR Shutdown() = 0;

    // During the transition phase, we expect a large subset of code to require access to
    // event emitting, path marking and other operations
    virtual InteractionModelActions CurrentActions() { return mActions; }

    /// Return codes:
    ///   CHIP_ERROR_ACCESS_DENIED:
    ///      - May be ignored if reads use path expansion (e.g. to skip inaccessible attributes
    ///        during subscription requests). This code MUST be used for ACL failures and only for ACL failures.
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
    /// List operation support:
    ///    - the first list write will have `request.writeFlags.Has(WriteFlags::kListBegin)`
    ///    - the last list write will have `request.writeFlags.Has(WriteFlags::kListEnd)`
    ///    - the last list write MAY have empty data (no list items)
    ///
    /// When `request.writeFlags.Has(WriteFlags::kForceInternal)` the request is from an internal app update
    /// and SHOULD bypass some internal checks (like timed enforcement, potentially read-only restrictions)
    ///
    /// Return codes
    ///   CHIP_IM_GLOBAL_STATUS(code):
    ///       - error codes that are translatable to specific IM codes
    ///       - in particular, the following codes are interesting/expected
    ///         - `UnsupportedWrite` for attempts to write read-only data
    ///         - `UnsupportedAccess` for ACL failures
    ///         - `NeedsTimedInteraction` for writes that are not timed however are required to be so
    virtual CHIP_ERROR WriteAttribute(const WriteAttributeRequest & request, AttributeValueDecoder & decoder) = 0;

    /// `responder` is used to send back the reply.
    ///    - calling Reply() or ReplyAsync() will let the application control the reply
    ///    - returning a CHIP_NO_ERROR without reply/reply_async implies a Status::Success reply without data
    ///    - returning a CHIP_*_ERROR implies an error reply (error and data are mutually exclusive)
    ///
    /// See InvokeReply/AutoCompleteInvokeResponder for details on how to send back replies and expected
    /// error handling. If you require knowledge if a response was successfully sent, use the underlying
    /// `reply` object instead of returning an error codes from Invoke.
    ///
    /// Return codes
    ///   CHIP_IM_GLOBAL_STATUS(code):
    ///       - error codes that are translatable to specific IM codes
    ///       - in particular, the following codes are interesting/expected
    ///         - `UnsupportedEndpoint` for invalid endpoint
    ///         - `UnsupportedCluster` for no such cluster on the endpoint
    ///         - `UnsupportedCommand` for no such command in the cluster
    ///         - `UnsupportedAccess` for permission errors (ACL or fabric scoped with invalid fabric)
    ///         - `NeedsTimedInteraction` if the invoke requires timed interaction support
    virtual CHIP_ERROR Invoke(const InvokeRequest & request, chip::TLV::TLVReader & input_arguments, InvokeReply & reply) = 0;

private:
    InteractionModelActions mActions = { nullptr };
};

} // namespace InteractionModel
} // namespace app
} // namespace chip
