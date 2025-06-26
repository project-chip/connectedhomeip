/*
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <clusters/SoftwareDiagnostics/Events.h>

namespace chip {
namespace app {
namespace Clusters {
namespace SoftwareDiagnostics {

/// A global class that listens for software fault detection.
///
/// Platforms are expected to notify listeners of faults. This is generally
/// used to trigger reports via the Software Diagnostics cluster.
class SoftwareFaultListener
{
public:
    SoftwareFaultListener()          = default;
    virtual ~SoftwareFaultListener() = default;

    /// Called by various layers to report software faults
    virtual void
    OnSoftwareFaultDetect(const chip::app::Clusters::SoftwareDiagnostics::Events::SoftwareFault::Type & softwareFault) = 0;

    /// Returs the set global listener (if any). May return nullptr if no such listener is set.
    static SoftwareFaultListener * GetGlobalListener();

    /// Set the global software fault listener, returns the old value
    static void SetGlobalListener(SoftwareFaultListener * newValue);

    /// Convenience method of "call software fault detect if global listener is not null"
    static void
    GlobalNotifySoftwareFaultDetect(const chip::app::Clusters::SoftwareDiagnostics::Events::SoftwareFault::Type & softwareFault)
    {
        if (SoftwareFaultListener * listener = GetGlobalListener(); listener != nullptr)
        {
            listener->OnSoftwareFaultDetect(softwareFault);
        }
    }
};

} // namespace SoftwareDiagnostics
} // namespace Clusters
} // namespace app
} // namespace chip
