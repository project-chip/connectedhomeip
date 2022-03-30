/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#include "OTARequestorDriverImpl.h"
#include <lib/support/logging/CHIPLogging.h>

namespace chip {
namespace DeviceLayer {

namespace {

OTARequestorDriverImpl * ToDriver(void * context)
{
    return static_cast<OTARequestorDriverImpl *>(context);
}

} // namespace

void OTARequestorDriverImpl::UpdateAvailable(const UpdateDescription & update, System::Clock::Seconds32 delay)
{
    VerifyOrDie(mRequestor != nullptr);
    if (onOtaUpdateAvailableCallback)
    {
        if (!onOtaUpdateAvailableCallback->mCall(onOtaUpdateAvailableCallback->mContext, update))
        {
            return;
        }
    }
    ScheduleDelayedAction(
        delay, [](System::Layer *, void * context) { ToDriver(context)->mRequestor->DownloadUpdate(); }, this);
}

void OTARequestorDriverImpl::UpdateDownloaded()
{
    VerifyOrDie(mRequestor != nullptr);
    if (onOtaUpdateApplyCallback)
    {
        if (!onOtaUpdateApplyCallback->mCall(onOtaUpdateApplyCallback->mContext))
        {
            return;
        }
    }
    mRequestor->ApplyUpdate();
}

} // namespace DeviceLayer
} // namespace chip
