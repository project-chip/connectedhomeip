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
#include <lib/support/StringBuilder.h>
#include <protocols/interaction_model/StatusCode.h>

#include <variant>

namespace chip {
namespace app {
namespace DataModel {

/// An ActionReturnStatus encodes the result of a read/write/invoke.
///
/// Generally such actions result in a StatusIB in the interaction model,
/// which is a code (InteractionModel::Status) and may have an associated
/// cluster-specific code.
///
/// However some actions specifically may return additional information for
/// chunking, hence the existence of this class:
///
///   - encapsulates a ClusterStatusCode for an actual action result
///   - encapsulates a underlying CHIP_ERROR for reporting purposes
///   - has a way to check for "chunking needed" status.
///
/// The class is directly constructible from statuses (non-exlicit) to make
/// returning of values easy.
class ActionReturnStatus
{
public:
    /// Provides storage for the c_str() call for the action status.
    struct StringStorage
    {
        // Generally size should be sufficient.
        // The longest status code from StatusCodeList is NO_UPSTREAM_SUBSCRIPTION(197)
        // so we need space for one of:
        //    "NO_UPSTREAM_SUBSCRIPTION(197)\0" = 30   // explicit verbose status code
        //    "FAILURE(1), Code 255\0")                // cluster failure, verbose
        //    "SUCCESS(0), Code 255\0")                // cluster success, verbose
        //    "Status<197>, Code 255\0")               // Cluster failure, non-verbose
        //
        // CHIP_ERROR has its own (global/static!) storage
        chip::StringBuilder<32> formatBuffer;
    };

    ActionReturnStatus(CHIP_ERROR error) : mReturnStatus(error) {}
    ActionReturnStatus(Protocols::InteractionModel::Status status) :
        mReturnStatus(Protocols::InteractionModel::ClusterStatusCode(status))
    {}
    ActionReturnStatus(Protocols::InteractionModel::ClusterStatusCode status) : mReturnStatus(status) {}

    /// Constructs a status code. Either returns the underlying code directly
    /// or converts the underlying CHIP_ERROR into a cluster status code.
    Protocols::InteractionModel::ClusterStatusCode GetStatusCode() const;

    /// Gets the underlying CHIP_ERROR if it exists, otherwise it will
    /// return a CHIP_ERROR corresponding to the underlying return status.
    ///
    /// Success statusess will result in CHIP_NO_ERROR (i.e. cluster specitic success codes are lost)
    CHIP_ERROR GetUnderlyingError() const;

    /// If this is a CHIP_NO_ERROR or a Status::Success
    bool IsSuccess() const;

    /// Considers if the underlying error is an error or not (CHIP_NO_ERROR is the only non-erro)
    /// or if the underlying statuscode is not an error (success and cluster specific successes
    /// are not an error).
    bool IsError() const { return !IsSuccess(); }

    /// Checks if the underlying error is an out of space condition (i.e. something that
    /// chunking can handle by sending partial list data).
    ///
    /// Generally this is when the return is based on CHIP_ERROR_NO_MEMORY or CHIP_ERROR_BUFFER_TOO_SMALL
    bool IsOutOfSpaceEncodingResponse() const;

    // NOTE: operator== will treat a CHIP_GLOBAL_IM_ERROR and a raw cluster status as equal if the statuses match,
    //       even though a CHIP_ERROR has some formatting info like file/line
    bool operator==(const ActionReturnStatus & other) const;
    bool operator!=(const ActionReturnStatus & other) const { return !(*this == other); }

    /// Get the formatted string of this status.
    ///
    /// May use `storage` for storying the actual underlying character string.
    const char * c_str(StringStorage & storage) const;

private:
    std::variant<CHIP_ERROR, Protocols::InteractionModel::ClusterStatusCode> mReturnStatus;
};

} // namespace DataModel
} // namespace app
} // namespace chip
