/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <static-supported-temperature-levels.h>
#include <zap-generated/gen_config.h>

using namespace std;
using namespace chip;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::TemperatureControl;
using chip::Protocols::InteractionModel::Status;

// TODO: Configure your options for each endpoint
CharSpan AppSupportedTemperatureLevelsDelegate::temperatureLevelOptions[] = { CharSpan("Hot", 3), CharSpan("Warm", 4),
                                                                              CharSpan("Freezing", 8) };

const AppSupportedTemperatureLevelsDelegate::EndpointPair AppSupportedTemperatureLevelsDelegate::supportedOptionsByEndpoints
    [EMBER_AF_TEMPERATURE_CONTROL_CLUSTER_SERVER_ENDPOINT_COUNT] = {
        EndpointPair(1, AppSupportedTemperatureLevelsDelegate::temperatureLevelOptions, 3) // Options for Endpoint 1
    };

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
