/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <map>
#include <vector>

#include <app-common/zap-generated/cluster-objects.h>
#include <app/util/af-enums.h>
#include <app/util/af.h>
#include <app/util/attribute-storage.h>
#include <app/util/basic-types.h>


namespace chip {
namespace app {
namespace Clusters {
namespace ModeSelectCluster {

/**
 * Interface to help manage the supported modes of the Mode Select Cluster.
 */
class SupportedModesManager
{

    using ModeOptionStructType = Structs::ModeOptionStruct::Type;

public:

    /**
     * Given the endpointId, returns all its supported modes options.
     * @param endpointId
     * @return
     */
    virtual const std::vector<ModeOptionStructType> getSupportedModesForEndpoint(EndpointId endpointId) const = 0;

    /**
     * Given the endpointId and a mode value, find the ModeOptionStruct that matches the mode.
     * @param endpointId
     * @param mode
     * @param dataPtr
     * @return
     */
    virtual EmberAfStatus getModeOptionByMode(EndpointId endpointId, uint8_t mode, const ModeOptionStructType *& dataPtr) const = 0;

    virtual ~SupportedModesManager() {};

};


} // namespace ModeSelectCluster
} // namespace Clusters
} // namespace app
} // namespace chip
