/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
