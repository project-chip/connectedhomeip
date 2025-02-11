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

#include <lib/core/CHIPError.h>
#include <lib/core/TLVReader.h>
#include <lib/core/TLVWriter.h>

#include <app/AttributeValueDecoder.h>
#include <app/AttributeValueEncoder.h>
#include <app/CommandHandler.h>
#include <app/EventPathParams.h>

#include <app/data-model-provider/ActionReturnStatus.h>
#include <app/data-model-provider/Context.h>
#include <app/data-model-provider/OperationTypes.h>
#include <app/data-model-provider/ProviderMetadataTree.h>

namespace chip {
namespace app {
namespace DataModel {

/// Represents operations against a matter-defined data model.
///
/// Class is SINGLE-THREADED:
///   - operations are assumed to only be ever run in a single event-loop
///     thread or equivalent
///   - class is allowed to attempt to cache indexes/locations for faster
///     lookups of things (e.g during iterations)
class Provider : public ProviderMetadataTree
{
public:
    virtual ~Provider() = default;

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

    /// NOTE: this code is NOT required to handle `List` global attributes:
    ///       AcceptedCommandsList, GeneratedCommandsList OR AttributeList
    ///
    ///       Users of DataModel::Provider are expected to get these lists
    ///       from ProviderMetadataTree (in particular IM Reads of these
    ///       attributes will the automatically filled from metadata).
    ///
    /// Return value notes:
    ///   ActionReturnStatus::IsOutOfSpaceEncodingResponse
    ///      - Indicates that list encoding had insufficient buffer space to encode elements.
    ///      - encoder::GetState().AllowPartialData() determines if these errors are permanent (no partial
    ///        data allowed) or further encoding can be retried (AllowPartialData true for list encoding)
    virtual ActionReturnStatus ReadAttribute(const ReadAttributeRequest & request, AttributeValueEncoder & encoder) = 0;

    /// Requests a write of an attribute.
    ///
    /// When this is invoked, caller is expected to have already done some validations:
    ///    - cluster `data version` has been checked for the incoming request if applicable
    ///    - validation of ACL/timed interaction flags/writability, if those checks are desired.
    virtual ActionReturnStatus WriteAttribute(const WriteAttributeRequest & request, AttributeValueDecoder & decoder) = 0;

    /// `handler` is used to send back the reply.
    ///    - returning `std::nullopt` means that return value was placed in handler directly.
    ///      This includes cases where command handling and value return will be done asynchronously.
    ///    - returning a value other than Success implies an error reply (error and data are mutually exclusive)
    ///
    /// Preconditions:
    ///    - `request.path` MUST refer to a command that actually exists.  This is because in practice
    ///       callers must do ACL and flag checks (e.g. for timed invoke) before calling this function.
    ///
    ///       Callers that do not care about those checks should use `ProviderMetadataTree::AcceptedCommands`
    ///       to check for command existence.
    ///
    ///    - TODO: as interfaces are updated, we may want to make the above requirement more
    ///            relaxed, as it seems desirable for users of this interface to have guaranteed
    ///            behavior (like error on invalid paths) whereas today this seems unclear as some
    ///            command intercepts do not validate that the command is in fact accepted on the
    ///            endpoint provided.
    ///
    /// Return value expectations:
    ///   - if a response has been placed into `handler` then std::nullopt MUST be returned. In particular
    ///     note that CHIP_NO_ERROR is NOT the same as std::nullopt:
    ///        > CHIP_NO_ERROR means handler had no status set and we expect the caller to AddStatus(success)
    ///        > std::nullopt means that handler has added an appropriate data/status response
    ///   - if a value is returned (not nullopt) then the handler response MUST NOT be filled. The caller
    ///     will then issue `handler->AddStatus(request.path, <return_value>->GetStatusCode())`. This is a
    ///     convenience to make writing Invoke calls easier.
    virtual std::optional<ActionReturnStatus> Invoke(const InvokeRequest & request, chip::TLV::TLVReader & input_arguments,
                                                     CommandHandler * handler) = 0;

private:
    InteractionModelContext mContext = { nullptr };
};

} // namespace DataModel
} // namespace app
} // namespace chip
