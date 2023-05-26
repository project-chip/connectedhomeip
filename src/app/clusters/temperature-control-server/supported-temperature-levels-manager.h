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

public:
    using TemperatureLevelStructType = Structs::TemperatureLevelStruct::Type;

    class Iterator
    {
    public:
        virtual ~Iterator() = default;

        Iterator(EndpointId endpoint)
        {
            mEndpoint = endpoint;
            mIndex    = 0;
        }

        virtual bool Next(TemperatureLevelStructType & item);

    private:
        EndpointId mEndpoint;
        uint8_t mIndex;
    };

    virtual ~SupportedTemperatureLevelsManager() {}
};

} // namespace TemperatureControl
} // namespace Clusters
} // namespace app
} // namespace chip
