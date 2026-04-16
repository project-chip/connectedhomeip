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

#include <clusters/OtaSoftwareUpdateRequestor/Events.h>

namespace chip {

// This interface is used by the DefaultOTARequestor to send events.
class DefaultOTARequestorEventSender
{
public:
    virtual ~DefaultOTARequestorEventSender() = default;

    using DownloadErrorEvent  = app::Clusters::OtaSoftwareUpdateRequestor::Events::DownloadError::Type;
    using VersionAppliedEvent = app::Clusters::OtaSoftwareUpdateRequestor::Events::VersionApplied::Type;

    virtual CHIP_ERROR SendVersionAppliedEvent(const VersionAppliedEvent & event) = 0;

    // Sends a download error event.
    virtual CHIP_ERROR SendDownloadErrorEvent(const DownloadErrorEvent & event) = 0;
};

} // namespace chip
