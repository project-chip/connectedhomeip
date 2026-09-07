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

#include <protocols/interaction_model/StatusCode.h>

namespace chip::app::Clusters {

/// This handles application notifications for bridged devices that support ICD.
///
/// Specifically, it allows the application to be notified when a `KeepActive` command is requested.
class BridgedDeviceIcdDelegate
{
public:
    virtual ~BridgedDeviceIcdDelegate() = default;

    /// Called when the cluster enters the PendingActive state.
    ///
    /// When the bridged device checks in, the application MUST query the cluster for
    /// the corresponding stay-active duration as it may change (multiple requests for KeepActive
    /// will keep a max value of the stay active duration).
    virtual Protocols::InteractionModel::Status OnEnterPendingActive() = 0;

    /// Called when the PendingActive state timer expires.
    virtual void OnPendingActiveExpired() = 0;
};

} // namespace chip::app::Clusters
