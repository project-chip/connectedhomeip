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

using TimeZoneList = Span<TimeZoneStore>;

/** @brief
 *    Defines methods for implementing application-specific logic for the Time Synchronization Cluster.
 */
class Delegate
{
public:
    inline bool HasFeature(chip::EndpointId ep, Feature feature)
    {
        uint32_t map;
        bool success = (Attributes::FeatureMap::Get(ep, &map) == EMBER_ZCL_STATUS_SUCCESS);
        return success ? (map & to_underlying(feature)) : false;
    }

    /**
     * @brief Notifies through the delegate that time zone has changed.
     *
     * @param timeZoneList new time zone list
     */
    virtual void HandleTimeZoneChanged(const TimeZoneList timeZoneList) = 0;
    /**
     * @brief Give the delegate the chance to populate DNSOffset based on active time zone.
     *
     * @param name name of active time zone
     */
    virtual bool HandleUpdateDSTOffset(const chip::CharSpan name) = 0;
    /**
     * @brief Validate NTP address
     *
     * @param ntp NTP address
     */
    virtual bool IsNTPAddressValid(const chip::CharSpan ntp) = 0;
    /**
     * @brief Check if NTP address is domain
     *
     * @param ntp NTP address
     */
    virtual bool IsNTPAddressDomain(const chip::CharSpan ntp) = 0;

    virtual ~Delegate() = default;
};

} // namespace TimeSynchronization
} // namespace Clusters
} // namespace app
} // namespace chip
