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

#include <app/clusters/temperature-control-server/supported-temperature-levels-manager.h>
#include <app/util/af.h>
#include <app/util/config.h>
#include <cstring>

namespace chip {
namespace app {
namespace Clusters {
namespace TemperatureControl {

/**
 * This implementation statically defines the options.
 */

class StaticSupportedTemperatureLevelsManager : public SupportedTemperatureLevelsManager
{
    using TemperatureLevelStructType = Structs::TemperatureLevelStruct::Type;
    using storage_value_type         = const TemperatureLevelStructType;

    struct EndpointSpanPair
    {
        const EndpointId mEndpointId;
        const Span<storage_value_type> mSpan;

        EndpointSpanPair(const EndpointId aEndpointId, const Span<storage_value_type> && aSpan) :
            mEndpointId(aEndpointId), mSpan(aSpan)
        {}

        EndpointSpanPair() : mEndpointId(0), mSpan(Span<storage_value_type>()) {}
    };

    static storage_value_type temperatureLevelOptions[];
    static const EndpointSpanPair supportedOptionsByEndpoints[EMBER_AF_MODE_SELECT_CLUSTER_SERVER_ENDPOINT_COUNT];

public:
    static const StaticSupportedTemperatureLevelsManager instance;

    SupportedTemperatureLevelsManager::TemperatureLevelOptionsProvider
    GetTemperatureLevelOptionsProvider(EndpointId endpointId) const override;

    Protocols::InteractionModel::Status
    GetTemperatureLevelOptionByTemperatureLevel(EndpointId endpointId, uint8_t temperatureLevel,
                                                const TemperatureLevelStructType ** dataPtr) const override;

    ~StaticSupportedTemperatureLevelsManager(){};

    StaticSupportedTemperatureLevelsManager() {}

    static inline const StaticSupportedTemperatureLevelsManager & GetStaticSupportedTemperatureLevelsManagerInstance()
    {
        return instance;
    }
};

const SupportedTemperatureLevelsManager * GetSupportedTemperatureLevelsManager();

} // namespace TemperatureControl
} // namespace Clusters
} // namespace app
} // namespace chip
