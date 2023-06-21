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

#include <app/clusters/resource-monitoring-server/resource-monitoring-delegate.h>

namespace chip {
namespace app {
namespace Clusters {

// Defining mode select delegates for all mode select aliased clusters.
namespace ResourceMonitoring {

/// This is an application level delegate to handle HepaFilterMonitoringDelegate commands according to the specific business logic.
//class HepaFilterMonitoringDelegate : public Delegate
class HepaFilterMonitoringDelegate : public Delegate
{
private:
    CHIP_ERROR Init() override;
    Status HandleResetCondition() override;

public:
    explicit HepaFilterMonitoringDelegate() : Delegate() {}

    ~HepaFilterMonitoringDelegate() override = default;
};

} // namespace ResourceMonitoring
} // namespace Clusters
} // namespace app
} // namespace chip