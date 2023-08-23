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

#include "TimeSyncDataProvider.h"

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <lib/support/Span.h>

namespace chip {
namespace app {
namespace Clusters {
namespace TimeSynchronization {

typedef void (*OnTimeSyncCompletion)(void * context, TimeSourceEnum timeSource, GranularityEnum granularity);
typedef void (*OnFallbackNTPCompletion)(void * context, bool timeSyncSuccessful);

/** @brief
 *    Defines methods for implementing application-specific logic for the Time Synchronization Cluster.
 */
class Delegate
{
    // using TimeZoneList = Span<TimeSyncDataProvider::TimeZoneStore>;

public:
    inline bool HasFeature(Feature feature)
    {
        uint32_t map;
        bool success = (Attributes::FeatureMap::Get(mEndpoint, &map) == EMBER_ZCL_STATUS_SUCCESS);
        return success ? (map & to_underlying(feature)) : false;
    }

    inline EndpointId GetEndpoint() { return mEndpoint; }
    inline void SetEndpoint(EndpointId ep) { mEndpoint = ep; }

    /**
     * @brief Notifies the delegate that the cluster's configured list of time zones has changed.
     *
     * @param timeZoneList new time zone list
     */
    virtual void TimeZoneListChanged(const Span<TimeSyncDataProvider::TimeZoneStore> timeZoneList) = 0;
    /**
     * @brief Give the delegate the chance to call SetDSTOffset on the TimeSynchronizationServer with a list of
     * DST offsets based on the provided time zone name.  If the delegate does so, it should return true.
     * If the delegate does not want to set DST offsets based on the time zone, it should return false.
     *
     * @param name name of active time zone
     */
    virtual bool HandleUpdateDSTOffset(const CharSpan name) = 0;
    /**
     * @brief Returns true if the provided string is a valid NTP address (either domain name or IPv6 address).
     *
     * @param ntp NTP address
     */
    virtual bool IsNTPAddressValid(const CharSpan ntp) = 0;
    /**
     * @brief Returns true if a valid NTP address is a domain name as opposed to an IPv6 address.
     *
     * @param ntp NTP address
     */
    virtual bool IsNTPAddressDomain(const CharSpan ntp) = 0;

    /**
     * @brief Delegate should attempt to get time from a platform-defined source using the ordering defined in the
     * Time source prioritization spec section. Delegate may skip any unsupported sources
     * Order: GNSS -> trusted high-resolution external source (PTP, trusted network NTP, cloud) ->
     * local network defined NTP (DHCPv6 -> DHCP -> DNS-SD sources)
     * If the delegate is unable to support any source, it may return an error immediately. If the delegate is going
     * to attempt to obtain time from any source, it returns CHIP_NO_ERROR and calls the callback on completion.
     * If the delegate successfully obtains the time, it sets the time using the platform time API (SetClock_RealTime)
     * and calls the callback with the time source and granularity set as appropriate.
     * If the delegate is unsuccessful in obtaining the time, it calls the callback with timeSource set to kNone and
     * granularity set to kNoTimeGranularity.
     */
    virtual CHIP_ERROR UpdateTimeFromPlatformSource(chip::Callback::Callback<OnTimeSyncCompletion> * callback) = 0;

    /**
     * @brief If the delegate supports NTP, it should attempt to update its time using the provided fallbackNTP source.
     * If the delegate is successful in obtaining a time from the fallbackNTP, it updates the system time (ex using
     * System::SystemClock().SetClock_RealTime) and calls the callback. If the delegate is going to attempt to update
     * the time and call the callback, it returns CHIP_NO_ERROR. If the delegate does not support NTP, it may return
     * a CHIP_ERROR.
     */
    virtual CHIP_ERROR UpdateTimeUsingNTPFallback(const CharSpan & fallbackNTP,
                                                  chip::Callback::Callback<OnFallbackNTPCompletion> * callback) = 0;

    virtual ~Delegate() = default;

private:
    EndpointId mEndpoint = kRootEndpointId;
};

} // namespace TimeSynchronization
} // namespace Clusters
} // namespace app
} // namespace chip
