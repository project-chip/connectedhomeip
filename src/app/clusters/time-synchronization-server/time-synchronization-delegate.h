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

#include <app-common/zap-generated/attributes/Accessors.h>

namespace chip {
namespace app {
namespace Clusters {
namespace TimeSynchronization {

using namespace chip::app::Clusters::TimeSynchronization;
using TimeZoneList = DataModel::List<chip::app::Clusters::TimeSynchronization::Structs::TimeZoneStruct::Type>;

/** @brief
 *    Defines methods for implementing application-specific logic for the Time Synchronization Cluster.
 */
class Delegate
{
public:
    inline bool HasFeature(chip::EndpointId ep, TimeSynchronizationFeature feature)
    {
        uint32_t map;
        bool success = (Attributes::FeatureMap::Get(ep, &map) == EMBER_ZCL_STATUS_SUCCESS);
        return success ? (map & to_underlying(feature)) : false;
    }

    virtual void HandleTimeZoneChanged(TimeZoneList timeZoneList) = 0;
    virtual CHIP_ERROR HandleDstoffsetlookup()                    = 0;
    virtual bool HandleDstoffsetavailable(chip::CharSpan name)    = 0;
    virtual CHIP_ERROR HandleGetdstoffset()                       = 0;
    virtual bool isNTPAddressValid(chip::CharSpan ntp)            = 0;
    virtual bool isNTPAddressDomain(chip::CharSpan ntp)           = 0;

    virtual ~Delegate() = default;
};

} // namespace TimeSynchronization
} // namespace Clusters
} // namespace app
} // namespace chip
