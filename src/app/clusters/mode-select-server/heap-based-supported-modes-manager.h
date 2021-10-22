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

#include <app/clusters/mode-select-server/supported-modes-manager.h>
#include <vector>
#include <map>

namespace chip{
namespace app{
namespace Clusters{
namespace ModeSelectCluster{

/**
 * This implementation uses the heap to store the supported options
 */
class HeapBasedSupportedModesManager: public chip::app::Clusters::ModeSelectCluster::SupportedModesManager
{
    using ModeOptionStructType = Structs::ModeOptionStruct::Type;

public:
    /**
     * A builder for HeapBasedSupportedModesManager, for conveniently injecting the supported options and building the SupportedModesManager.
     */
    class Builder
    {
    public:
        /**
         * Fluent method for adding a ModeOptionStruct::Type.
         * @param endpointId The endpoint to which to add the supported mode
         * @return this builder instance.
         */
        Builder & addSupportedMode(EndpointId endpointId, ModeOptionStructType & modeOptionStruct);
        /**
         * Fluent method for adding a ModeOptionStruct::Type.
         * @param endpointId The endpoint to which to add the supported mode
         * @return this builder instance.
         */
        Builder & addSupportedMode(EndpointId endpointId, ModeOptionStructType && modeOptionStruct);
        HeapBasedSupportedModesManager build();

    private:
        std::map<EndpointId, std::vector<ModeOptionStructType> *> mSupportedModesByEndpoints;
    };


    const std::vector<ModeOptionStructType> getSupportedModesForEndpoint(EndpointId endpointId) const override;

    EmberAfStatus getModeOptionByMode(EndpointId endpointId, uint8_t mode, const ModeOptionStructType *& dataPtr) const override;


private:
    HeapBasedSupportedModesManager(std::map<EndpointId, const std::vector<ModeOptionStructType>> supportedModes) :
        _supportedModesForAllEndpoints(supportedModes) {}
    // TODO: Implement move constructor?

    // endpoint index -> **ModeOptionStruct;
    std::map<EndpointId, const std::vector<ModeOptionStructType>> _supportedModesForAllEndpoints;
};
}
}
}
}
