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

#include <app/util/config.h>
#include <static-supported-temperature-levels-manager.h>

using namespace std;
using namespace chip;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::TemperatureControl;
using chip::Protocols::InteractionModel::Status;

using TemperatureLevelStructType = Structs::TemperatureLevelStruct::Type;
using storage_value_type         = TemperatureLevelStructType;
namespace {
Structs::TemperatureLevelStruct::Type buildTemperatureLevelStruct(const char * label, uint8_t temperatureLevel)
{
    Structs::TemperatureLevelStruct::Type option;
    option.label            = CharSpan::fromCharString(label);
    option.temperatureLevel = temperatureLevel;
    return option;
}
} // namespace

// TODO: Configure your options for each endpoint
storage_value_type StaticSupportedTemperatureLevelsManager::temperatureLevelOptions[] = {
    buildTemperatureLevelStruct("wash Temperature", 1), buildTemperatureLevelStruct("rinse Temperature", 2),
    buildTemperatureLevelStruct("dry Temperature", 3)
};
const StaticSupportedTemperatureLevelsManager::EndpointPair StaticSupportedTemperatureLevelsManager::supportedOptionsByEndpoints
    [EMBER_AF_TEMPERATURE_CONTROL_CLUSTER_SERVER_ENDPOINT_COUNT] = {
        EndpointPair(1, StaticSupportedTemperatureLevelsManager::temperatureLevelOptions, 3) // Options for Endpoint 1
    };

bool StaticSupportedTemperatureLevelsManager::SupportedTemperatureLevelsManager::Iterator::Next(TemperatureLevelStructType & item)
{
    for (auto & endpointPair : StaticSupportedTemperatureLevelsManager::supportedOptionsByEndpoints)
    {
        if (endpointPair.mEndpointId == mEndpoint)
        {
            if (endpointPair.mSize > mIndex)
            {
                item = endpointPair.mData[mIndex];
                mIndex++;
                return true;
            }
        }
    }
    return false;
}
