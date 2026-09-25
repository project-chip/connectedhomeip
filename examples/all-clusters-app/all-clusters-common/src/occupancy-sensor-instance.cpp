/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <occupancy-sensor-instance.h>

using namespace chip;
using namespace chip::app::Clusters;

CHIP_ERROR
OccupancySensingDelegateImpl::GetPredictedOccupancyAtIndex(size_t index,
                                                           OccupancySensing::Structs::PredictedOccupancyStruct::Type & prediction)
{
    switch (index)
    {
    case 0:
        prediction.startTimestamp = 1000;
        prediction.endTimestamp   = 2000;
        prediction.occupancy.Set(OccupancySensing::OccupancyBitmap::kOccupied);
        prediction.confidence = 85;
        return CHIP_NO_ERROR;
    case 1:
        prediction.startTimestamp = 2500;
        prediction.endTimestamp   = 3600;
        prediction.occupancy.ClearAll();
        prediction.confidence = 90;
        return CHIP_NO_ERROR;
    default:
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }
}
