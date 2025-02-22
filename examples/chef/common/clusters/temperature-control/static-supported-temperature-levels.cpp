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

#include <app/util/config.h>

#ifdef MATTER_DM_PLUGIN_TEMPERATURE_CONTROL_SERVER
#include "static-supported-temperature-levels.h"
#include <app/clusters/temperature-control-server/supported-temperature-levels-manager.h>
#include <lib/support/CodeUtils.h>

using namespace chip;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::TemperatureControl;
using chip::Protocols::InteractionModel::Status;

app::Clusters::TemperatureControl::AppSupportedTemperatureLevelsDelegate sAppSupportedTemperatureLevelsDelegate;

CharSpan AppSupportedTemperatureLevelsDelegate::temperatureLevelOptions[] = { "Low"_span, "Medium"_span, "High"_span };

const AppSupportedTemperatureLevelsDelegate::EndpointPair AppSupportedTemperatureLevelsDelegate::supportedOptionsByEndpoints
    [MATTER_DM_TEMPERATURE_CONTROL_CLUSTER_SERVER_ENDPOINT_COUNT] = { EndpointPair(
        1 /* endpointId */, AppSupportedTemperatureLevelsDelegate::temperatureLevelOptions,
        MATTER_ARRAY_SIZE(AppSupportedTemperatureLevelsDelegate::temperatureLevelOptions)) };

uint8_t AppSupportedTemperatureLevelsDelegate::Size()
{
    for (auto & endpointPair : AppSupportedTemperatureLevelsDelegate::supportedOptionsByEndpoints)
    {
        if (endpointPair.mEndpointId == mEndpoint)
        {
            return endpointPair.mSize;
        }
    }
    return 0;
}

CHIP_ERROR AppSupportedTemperatureLevelsDelegate::Next(MutableCharSpan & item)
{
    for (auto & endpointPair : AppSupportedTemperatureLevelsDelegate::supportedOptionsByEndpoints)
    {
        if (endpointPair.mEndpointId == mEndpoint)
        {
            if (endpointPair.mSize > mIndex)
            {
                CHIP_ERROR err = CopyCharSpanToMutableCharSpan(endpointPair.mTemperatureLevels[mIndex], item);
                if (err != CHIP_NO_ERROR)
                {
                    ChipLogError(Zcl, "Error copying char span to mutable char span %s", ErrorStr(err));
                    return err;
                }
                mIndex++;
                return CHIP_NO_ERROR;
            }
        }
    }
    return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
}
void emberAfTemperatureControlClusterInitCallback(EndpointId endpoint)
{
    static_assert(MATTER_DM_TEMPERATURE_CONTROL_CLUSTER_SERVER_ENDPOINT_COUNT == 1, "This cluster is only enabled for endpoint 1");

    chip::app::Clusters::TemperatureControl::SetInstance(&sAppSupportedTemperatureLevelsDelegate);
}
#endif // MATTER_DM_PLUGIN_TEMPERATURE_CONTROL_SERVER
