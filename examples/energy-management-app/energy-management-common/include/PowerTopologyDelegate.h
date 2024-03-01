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

#include <app/clusters/power-topology-server/power-topology-server.h>

#include <app/util/af-types.h>
#include <lib/core/CHIPError.h>

namespace chip {
namespace app {
namespace Clusters {
namespace PowerTopology {

class PowerTopologyDelegate : public Delegate
{
public:
    ~PowerTopologyDelegate() = default;

    CHIP_ERROR GetAvailableEndpointAtIndex(size_t index, EndpointId & endpointId) override;
    CHIP_ERROR GetActiveEndpointAtIndex(size_t index, EndpointId & endpointId) override;
};

class PowerTopologyInstance : public Instance
{
public:
    PowerTopologyInstance(EndpointId aEndpointId, PowerTopologyDelegate & aDelegate, Feature aFeature,
                          OptionalAttributes aOptionalAttributes) :
        PowerTopology::Instance(aEndpointId, aDelegate, aFeature, aOptionalAttributes)
    {
        mDelegate = &aDelegate;
    }

    // Delete copy constructor and assignment operator.
    PowerTopologyInstance(const PowerTopologyInstance &)             = delete;
    PowerTopologyInstance(const PowerTopologyInstance &&)            = delete;
    PowerTopologyInstance & operator=(const PowerTopologyInstance &) = delete;

    CHIP_ERROR Init();
    void Shutdown();

    PowerTopologyDelegate * GetDelegate() { return mDelegate; };

private:
    PowerTopologyDelegate * mDelegate;
};

} // namespace PowerTopology
} // namespace Clusters
} // namespace app
} // namespace chip
