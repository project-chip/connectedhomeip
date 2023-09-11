/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "DefaultTimeSyncDelegate.h"
#include "inet/IPAddress.h"
#include <platform/RuntimeOptionsProvider.h>
#include <system/SystemClock.h>

using chip::TimeSyncDataProvider;
using namespace chip::app::Clusters::TimeSynchronization;

void DefaultTimeSyncDelegate::TimeZoneListChanged(const Span<TimeSyncDataProvider::TimeZoneStore> timeZoneList)
{
    // placeholder implementation
}

bool DefaultTimeSyncDelegate::HandleUpdateDSTOffset(chip::CharSpan name)
{
    // placeholder implementation
    return false;
}

bool DefaultTimeSyncDelegate::IsNTPAddressValid(chip::CharSpan ntp)
{
    // placeholder implementation
    chip::Inet::IPAddress addr;
    return chip::Inet::IPAddress::FromString(ntp.data(), ntp.size(), addr) && addr.IsIPv6();
}

bool DefaultTimeSyncDelegate::IsNTPAddressDomain(chip::CharSpan ntp)
{
    // placeholder implementation
    return false;
}

CHIP_ERROR DefaultTimeSyncDelegate::UpdateTimeFromPlatformSource(chip::Callback::Callback<OnTimeSyncCompletion> * callback)
{
    System::Clock::Microseconds64 utcTime;
    if (chip::app::RuntimeOptionsProvider::Instance().GetSimulateNoInternalTime())
    {
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }
    if (System::SystemClock().GetClock_RealTime(utcTime) == CHIP_NO_ERROR)
    {
        // Default assumes the time came from NTP. Platforms using other sources should overwrite this
        // with their own delegates
        // Call the callback right away from within this function
        callback->mCall(callback->mContext, TimeSourceEnum::kMixedNTP, GranularityEnum::kMillisecondsGranularity);
        return CHIP_NO_ERROR;
    }
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR DefaultTimeSyncDelegate::UpdateTimeUsingNTPFallback(const CharSpan & fallbackNTP,
                                                               chip::Callback::Callback<OnFallbackNTPCompletion> * callback)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}
