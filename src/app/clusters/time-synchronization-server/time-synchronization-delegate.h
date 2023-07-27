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

    // TODO: All of these will need to be async. That's going to be a LOT more code, so leaving that until the end since
    // none of them return anything useful right now anyway.
    /**
     * @brief If the delegate supports GNSS and is able to get a good time, it updates the system time if required (ex using
     * System::SystemClock().SetClock_RealTime) and returns true. Otherwise, it returns false.
     */
    virtual bool UpdateTimeUsingGNSS() = 0;
    /**
     * @brief If the delegate supports PTP and is able to get a good time, it updates the system time if required (ex using
     * System::SystemClock().SetClock_RealTime) and returns true. Otherwise, it returns false.
     */
    virtual bool UpdateTimeUsingPTP() = 0;
    /**
     * @brief If the delegate supports trusted external time source (ex network NTP, cloud-based) and is able to get a good time, it
     * updates the system time if required (ex using System::SystemClock().SetClock_RealTime) and returns true. Otherwise, it
     * returns false.
     */
    virtual bool UpdateTimeUsingExternalSource() = 0;
    /**
     * @brief This covers delegate-discovered NTP sources. If the delegate supports NTP, it should attempt to update first using
     * the DHCPv6 defined NTP server option, falling back to the DHCP server option if ipv4 is supported, followed by servers
     * given by _ntp._udp DNS-SD query, if DNS-SD is supported. If the delegate is successful in updating the time, it updates the
     * system time as required (ex using System::SystemClock().SetClock_RealTime) and sets the parameters according to selected
     * source. If the delegate is uncertain of any values, it should set the parameters to false. The delegate returns true if the
     * time was successfully set using NTP, false otherwise.
     */
    virtual bool UpdateTimeUsingNTP(bool & usedFullNTP, bool & usedNTS, bool & allSourcesFromMatterNetwork) = 0;

    /**
     * @brief If the delegate supports NTP, it should attempt to update its time using the provided fallbackNTP source.
     * If the delegate is successful in obtaining a time from the fallbackNTP, it updates the system time (ex using
     * System::SystemClock().SetClock_RealTime). The delegate returns true if it was successful in updating the time, false
     * otherwise.
     */
    virtual bool UpdateTimeUsingNTPFallback(const CharSpan & fallbackNTP) = 0;

    virtual ~Delegate() = default;

private:
    EndpointId mEndpoint = kRootEndpointId;
};

} // namespace TimeSynchronization
} // namespace Clusters
} // namespace app
} // namespace chip
