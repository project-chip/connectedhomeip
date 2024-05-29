/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include <app-common/zap-generated/cluster-objects.h>
#include <app/clusters/service-area-server/service-area-server.h>
#include <protocols/interaction_model/StatusCode.h>

#include <string>


namespace chip {
namespace app {
namespace Clusters {


class RvcDevice;

typedef void (RvcDevice::*GetRemainingTimeCallback)(DataModel::Nullable<uint32_t> & aRemainineTime);
typedef bool (RvcDevice::*IsSetSelectedLocationAllowedCallback)(std::string & statusText);


namespace ServiceArea {


// This is an application level delegate to handle service area cluster commands and attribute access according to the business logic of the iRobot device.
class RvcServiceAreaDelegate : public ServiceArea::Delegate
{
private:

    RvcDevice * mSetSelectedLocationAllowedInstance;
    IsSetSelectedLocationAllowedCallback mSetSelectedLocationAllowedCallback;

public:

void HandleVolatileCurrentLocation(const DataModel::Nullable<uint32_t> prevCurrentLocation) override;

void HandleVolatileEstimatedEndTime(const DataModel::Nullable<uint32_t> prevEstimatedEndTime) override;

void HandleSelectedLocationsChanged() override;

virtual void HandleVolatileProgressList() override;   

bool IsSetSelectedLocationAllowed(std::string & statusText) override;

void SetIsSetSelectedLocationCallback(IsSetSelectedLocationAllowedCallback aCallback, RvcDevice * aInstance);

bool HandleSetSelectLocations(const std::vector<uint32_t> & newSelectLocations, SelectLocationsStatus & locationStatus, std::string & locationStatusText) override;

bool HandleSkipCurrentLocation(std::string & skipStatusText) override;

};


} // namespace Location
} // namespace Clusters
} // namespace app
} // namespace chip
