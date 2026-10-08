/*
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <lib/support/IntrusiveList.h>

namespace chip::app::Clusters {

/// Interface for implementing application-specific logic for the OnOff Cluster.
class OnOffDelegate : public IntrusiveListNodeBase<IntrusiveMode::AutoUnlink>
{
public:
    virtual ~OnOffDelegate() = default;

    /// Cluster was started up with this given on/off value.
    ///
    /// For lighting applications, the value will be according to the startup
    /// enum (i.e. it would be forced on or off or toggled)
    virtual void OnOffStartup(bool on) = 0;

    /// Called when the OnOff attribute has changed.
    /// The delegate should update the hardware state to match the new value.
    ///
    /// This is NOT called as part of startup.
    virtual void OnOnOffChanged(bool on) = 0;
};

} // namespace chip::app::Clusters
