/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <app/clusters/electrical-grid-conditions-server/electrical-grid-conditions-server.h>
#include <app/util/af-types.h>
#include <lib/core/CHIPError.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ElectricalGridConditions {

class ElectricalGridConditionsDelegate : public ElectricalGridConditions::Delegate
{
public:
    ElectricalGridConditionsDelegate();
    ~ElectricalGridConditionsDelegate() = default;

    /* This class is a placeholder - most of the functionality is in the
     * cluster server. Consider removing this.
     */
private:
};

class ElectricalGridConditionsInstance : public Instance
{
public:
    ElectricalGridConditionsInstance(EndpointId aEndpointId, ElectricalGridConditionsDelegate & aDelegate, Feature aFeature) :
        ElectricalGridConditions::Instance(aEndpointId, aDelegate, aFeature)
    {
        mDelegate = &aDelegate;
    }

    // Delete copy constructor and assignment operator.
    ElectricalGridConditionsInstance(const ElectricalGridConditionsInstance &)             = delete;
    ElectricalGridConditionsInstance(const ElectricalGridConditionsInstance &&)            = delete;
    ElectricalGridConditionsInstance & operator=(const ElectricalGridConditionsInstance &) = delete;

    CHIP_ERROR Init();
    void Shutdown();
    CHIP_ERROR AppInit();

    ElectricalGridConditionsDelegate * GetDelegate() { return mDelegate; };

private:
    ElectricalGridConditionsDelegate * mDelegate;
};

} // namespace ElectricalGridConditions
} // namespace Clusters
} // namespace app
} // namespace chip
