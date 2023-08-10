/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include <app/ConcreteCommandPath.h>
#include <protocols/interaction_model/Constants.h>

namespace chip {
namespace app {

/// Wrapper around a command path with an associated status
///
/// NOTE: points to a REFERENCE of a command path, so lifetime of this
///       depends on the lifetime of the reference
class CommandPathStatus {
  public:
    CommandPathStatus(const ConcreteCommandPath &path, 
                      const Protocols::InteractionModel::Status status)
            : mPath(path), mStatus(status) {}

    const ConcreteCommandPath & GetPath() const { return mPath; }
    Protocols::InteractionModel::Status GetStatus() const { return mStatus; }

    /// Adds a ChipLogError containing information about a failed status
    ///
    /// `contextMessage` will be part of the logged message.
    ///
    /// returns self
    CommandPathStatus &LogIfFailure(const char *contextMessage);


    /// Logs the status given the current path
    ///
    /// If status is success, logs will be progress. If status
    /// is failure, log will be at error level.
    CommandPathStatus &LogStatus(const char *contextMessage);
  private:
    const ConcreteCommandPath &mPath;
    const Protocols::InteractionModel::Status mStatus;
    
};

} // namespace app
} // namespace chip
