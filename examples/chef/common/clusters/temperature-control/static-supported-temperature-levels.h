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

#include <app/clusters/temperature-control-server/supported-temperature-levels-manager.h>
#include <app/util/config.h>

namespace chef {
namespace Configuration {
namespace TemperatureControl {

/**
 * @brief Endpoint to temperature levels mapping. The endpoint must have a temperature control cluster.
 * Represents a pair of endpoints and temperature levels supported by that endpoint.
 */
struct EndpointPair
{
    /// An endpoint having temperature control cluster.
    chip::EndpointId mEndpointId;

    /// Temperature levels supported by the temperature control cluster at this endpoint.
    /// This should point to a const char span array initialized statically.
    chip::Span<const chip::CharSpan> mTemperatureLevels;

    EndpointPair() : mEndpointId(chip::kInvalidEndpointId), mTemperatureLevels() {}

    EndpointPair(chip::EndpointId aEndpointId, chip::Span<const chip::CharSpan> TemperatureLevels) :
        mEndpointId(aEndpointId), mTemperatureLevels(TemperatureLevels)
    {}
};
} // namespace TemperatureControl
} // namespace Configuration
} // namespace chef

namespace chip {
namespace app {
namespace Clusters {
namespace TemperatureControl {

class AppSupportedTemperatureLevelsDelegate : public SupportedTemperatureLevelsIteratorDelegate
{
    static chef::Configuration::TemperatureControl::EndpointPair
        supportedOptionsByEndpoints[MATTER_DM_TEMPERATURE_CONTROL_CLUSTER_SERVER_ENDPOINT_COUNT];

public:
    uint8_t Size() override;

    CHIP_ERROR Next(MutableCharSpan & item) override;

    static void SetSupportedEndpointPair(uint16_t index, chef::Configuration::TemperatureControl::EndpointPair endpointPair)
    {
        supportedOptionsByEndpoints[index] = endpointPair;
    }

    ~AppSupportedTemperatureLevelsDelegate() {}
};

} // namespace TemperatureControl
} // namespace Clusters
} // namespace app
} // namespace chip
