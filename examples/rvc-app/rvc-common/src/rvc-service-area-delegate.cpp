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
#include <rvc-service-area-delegate.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ServiceArea;


void RvcServiceAreaDelegate::HandleVolatileCurrentLocation(const DataModel::Nullable<uint32_t> prevCurrentLocation)
{
    // TODO IMPLEMENT
};

void RvcServiceAreaDelegate::HandleVolatileEstimatedEndTime(const DataModel::Nullable<uint32_t> prevEstimatedEndTime)
{
    // TODO IMPLEMENT
};

void RvcServiceAreaDelegate::HandleSelectedLocationsChanged()
{
    // TODO IMPLEMENT
};

void RvcServiceAreaDelegate::HandleVolatileProgressList()
{
    // TODO IMPLEMENT
};   

bool RvcServiceAreaDelegate::IsSetSelectedLocationAllowed(std::string & statusText)
{
    return (mSetSelectedLocationAllowedInstance->*mSetSelectedLocationAllowedCallback)(statusText);
};

void RvcServiceAreaDelegate::SetIsSetSelectedLocationCallback(IsSetSelectedLocationAllowedCallback aCallback, RvcDevice * aInstance)
{
    mSetSelectedLocationAllowedCallback = aCallback;
    mSetSelectedLocationAllowedInstance = aInstance;
};

bool RvcServiceAreaDelegate::HandleSetSelectLocations(const std::vector<uint32_t> & newSelectLocations, 
                                            SelectLocationsStatus & locationStatus, std::string & locationStatusText) 
{
    bool ret_value = false;

    ret_value = true; // TODO IMPLEMENT

    return ret_value;
};

bool RvcServiceAreaDelegate::HandleSkipCurrentLocation(std::string & skipStatusText)
{
    bool ret_value = false;

    ret_value = true; // TODO IMPLEMENT

    return ret_value;
};
