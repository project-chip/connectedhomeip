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

#include <WhmDelegate.h>
#include <app/clusters/water-heater-management-server/water-heater-management-server.h>
#include <app/util/af-types.h>
#include <lib/core/CHIPError.h>

namespace chip {
namespace app {
namespace Clusters {
namespace WaterHeaterManagement {
using namespace chip::app::Clusters::WaterHeaterManagement;

class WaterHeaterManagementInstance : public Instance
{
public:
    WaterHeaterManagementInstance(EndpointId aEndpointId, WaterHeaterManagementDelegate & aDelegate, Feature aFeature) :
        WaterHeaterManagement::Instance(aEndpointId, aDelegate, aFeature)
    {
        mDelegate = &aDelegate;
    }

    // Delete copy constructor and assignment operator.
    WaterHeaterManagementInstance(const WaterHeaterManagementInstance &)             = delete;
    WaterHeaterManagementInstance(const WaterHeaterManagementInstance &&)            = delete;
    WaterHeaterManagementInstance & operator=(const WaterHeaterManagementInstance &) = delete;

    CHIP_ERROR Init();
    void Shutdown();

    WaterHeaterManagementDelegate * GetDelegate() { return mDelegate; };

private:
    WaterHeaterManagementDelegate * mDelegate;
};

} // namespace WaterHeaterManagement
} // namespace Clusters
} // namespace app
} // namespace chip
