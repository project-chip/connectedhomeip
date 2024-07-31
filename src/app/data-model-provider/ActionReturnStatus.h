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
#include <protocols/interaction_model/StatusCode.h>

#include <optional>

namespace chip {
namespace app {
namespace DataModel {

/// An ActionReturnStatus encodes the result of a read/write/invoke.
///
/// Generally such actions result in a StatusIB in the interaction model,
/// which is a code (InteractionModel::Status) and may have an associated
/// cluster-specific code.
///
/// However some actions specifically may return additional errors for
/// chunking (i.e. out of space, retry or send existing partial data) hence
/// the exitance of this class:
///
///   - encapsulates a ClusterStatusCode for an actual action result
///   - encapsulates a underlying CHIP_ERROR for reporting purposes
///     and to check for out-of-space specific errors
///
/// The class is directly constructible from statuses (non-exlicit) to make
/// returning of values easy.
class ActionReturnStatus
{
public:
    ActionReturnStatus(CHIP_ERROR error): mError(error) {}
    ActionReturnStatus(Protocols::InteractionModel::Status status): mStatusCode(status) {}
    ActionReturnStatus(Protocols::InteractionModel::ClusterStatusCode status): mStatusCode(status) {}

    /// Constructs a status code. Either returns the underlying code directly
    /// or converts the underlying CHIP_ERROR into a cluster status code.
    Protocols::InteractionModel::ClusterStatusCode GetStatusCode() const;

    /// Considers if the underlying error is an error or not (CHIP_NO_ERROR is the only non-erro)
    /// or if the underlying statuscode is not an error (success and cluster specific successes
    /// are not an error).
    bool IsError() const;

    /// Checks if the underlying error is an out of space condition (i.e. something that
    /// chunking can handle by sending partial list data).
    bool IsOutOfSpaceError() const;

    /// Logs the underlying code as a ChipLogError. If the underlying data contains
    /// a CHIP_ERROR, the error is reported. Otherwise the cluster status code data
    /// is logged.
    void LogError(const char *prefix) const;

private:
    std::optional<CHIP_ERROR> mError;
    std::optional<Protocols::InteractionModel::ClusterStatusCode> mStatusCode;
};

} // namespace DataModel
} // namespace app
} // namespace chip
