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

#include <app-common/zap-generated/cluster-objects.h>
#include <app/util/af-enums.h>
#include <protocols/interaction_model/StatusCode.h>

namespace chip {
namespace app {
namespace Clusters {
namespace TemperatureControl {

/**
 * Interface to help manage the supported temperature levels of the Temperature Control Cluster.
 */
class SupportedTemperatureLevelsManager
{

    using TemperatureLevelStructType = Structs::TemperatureLevelStruct::Type;

public:
    /**
     * A class that can return the supported temperature levels for a specific endpoint.
     */
    struct TemperatureLevelOptionsProvider
    {
        using pointer = const TemperatureLevelStructType *;

        /**
         * Returns the TemperatureLevelStructIterator to the first option.
         */
        inline pointer begin() const { return mBegin; }

        /**
         * Returns the TemperatureLevelStructIterator to an element after the last option.
         */
        inline pointer end() const { return mEnd; }

        TemperatureLevelOptionsProvider() : mBegin(nullptr), mEnd(nullptr) {}

        TemperatureLevelOptionsProvider(const pointer aBegin, const pointer aEnd) : mBegin(aBegin), mEnd(aEnd) {}

        pointer mBegin;
        pointer mEnd;
    };

    /**
     * Given the endpointId, returns all its temperature level options.
     * @param endpointId
     * @return The temperature level provider for the endpoint.
     */
    virtual TemperatureLevelOptionsProvider GetTemperatureLevelOptionsProvider(EndpointId endpointId) const = 0;

    /**
     * Given the endpointId and a temperature level value, find the TemperatureLevelStruct that matches the temperature level.
     * @param endpointId The endpoint for which to search the TemperatureLevelStruct.
     * @param temperatureLevel The temperature level for which to search the TemperatureLevelStruct.
     * @param dataPtr The pointer to receive the TemperatureLevelStruct, if one is found.
     * @return Status::Success if successfully found the option. Otherwise, returns appropriate status code (found in
     * <app/util/af-enums.h>)
     */
    virtual Protocols::InteractionModel::Status
    GetTemperatureLevelOptionByTemperatureLevel(EndpointId endpointId, uint8_t temperatureLevel,
                                                const TemperatureLevelStructType ** dataPtr) const = 0;

    virtual ~SupportedTemperatureLevelsManager() {}
};

const SupportedTemperatureLevelsManager * GetSupportedTemperatureLevelsManager();

} // namespace TemperatureControl
} // namespace Clusters
} // namespace app
} // namespace chip
