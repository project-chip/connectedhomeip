/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "time-synchronization-delegate.h"

namespace chip {
namespace app {
namespace Clusters {
namespace TimeSynchronization {
class DefaultTimeSyncDelegate : public Delegate
{

public:
    DefaultTimeSyncDelegate() : Delegate(){};
    void TimeZoneListChanged(const Span<TimeSyncDataProvider::TimeZoneStore> timeZoneList) override;
    bool HandleUpdateDSTOffset(CharSpan name) override;
    bool IsNTPAddressValid(CharSpan ntp) override;
    bool IsNTPAddressDomain(CharSpan ntp) override;
    CHIP_ERROR UpdateTimeFromPlatformSource(chip::Callback::Callback<OnTimeSyncCompletion> * callback) override;
    CHIP_ERROR UpdateTimeUsingNTPFallback(const CharSpan & fallbackNTP,
                                          chip::Callback::Callback<OnFallbackNTPCompletion> * callback) override;
};

} // namespace TimeSynchronization
} // namespace Clusters
} // namespace app
} // namespace chip
