/*
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

#include <clusters/BridgedDeviceBasicInformation/Commands.h>
#include <protocols/interaction_model/StatusCode.h>

namespace chip::app::Clusters {

/// This handles application notifications for bridged devices that support ICD.
///
/// Specifically it allows being notified when a `KeepActive` is requested
class BridgedDeviceIcdDelegate
{
public:
    virtual ~BridgedDeviceIcdDelegate() = default;

    /// Notification that a keep active was requested.
    ///
    /// NOTE: Specification requires an `ActiveChanged` event to be generated once the device is active.
    ///       Implementations should ensure that the bridged device `GenerateActiveChangedEvent` is triggered
    ///       when applicable.
    virtual Protocols::InteractionModel::Status
    OnKeepActive(const BridgedDeviceBasicInformation::Commands::KeepActive::DecodableType & request) = 0;
};

} // namespace chip::app::Clusters
