/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <app/data-model-provider/ActionReturnStatus.h>
#include <lib/core/CHIPError.h>
#include <lib/support/TypeTraits.h>
#include <lib/support/logging/CHIPLogging.h>
#include <protocols/interaction_model/StatusCode.h>

namespace chip::app {

/// Widgets drive clusters from a scheduled lambda on the CHIP thread, long after the touch event
/// that caused it. There is nowhere to report a rejected command back to, so the only sensible
/// handling is to log it: use these helpers instead of dropping the result silently.
///
/// `command` is the human readable operation name, e.g. "MoveToLevelWithOnOff".
inline void LogCommandFailure(const char * command, Protocols::InteractionModel::Status status)
{
    if (status != Protocols::InteractionModel::Status::Success)
    {
        ChipLogError(AppServer, "Display: %s rejected with status 0x%02x", command, to_underlying(status));
    }
}

inline void LogCommandFailure(const char * command, const DataModel::ActionReturnStatus & status)
{
    if (status.IsError())
    {
        DataModel::ActionReturnStatus::StringStorage storage;
        ChipLogError(AppServer, "Display: %s rejected: %s", command, status.c_str(storage));
    }
}

} // namespace chip::app
