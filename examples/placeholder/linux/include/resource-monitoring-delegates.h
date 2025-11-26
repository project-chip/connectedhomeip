/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/clusters/resource-monitoring-server/resource-monitoring-cluster-objects.h>
#include <app/clusters/resource-monitoring-server/resource-monitoring-server.h>

namespace chip {
namespace app {
namespace Clusters {

namespace ActivatedCarbonFilterMonitoring {

/// This is an application level Delegate to handle ActivatedCarbonFilterMonitoringDelegate commands according to the specific
/// business logic.
class ActivatedCarbonFilterMonitoringDelegate : public ResourceMonitoring::Delegate
{
private:
    CHIP_ERROR Init() override;
    chip::Protocols::InteractionModel::Status PreResetCondition() override;
    chip::Protocols::InteractionModel::Status PostResetCondition() override;

public:
    ~ActivatedCarbonFilterMonitoringDelegate() override = default;
};

} // namespace ActivatedCarbonFilterMonitoring

namespace HepaFilterMonitoring {

/// This is an application level delegate to handle HepaFilterMonitoringDelegate commands according to the specific business logic.
class HepaFilterMonitoringDelegate : public ResourceMonitoring::Delegate
{
private:
    CHIP_ERROR Init() override;
    chip::Protocols::InteractionModel::Status PreResetCondition() override;
    chip::Protocols::InteractionModel::Status PostResetCondition() override;

public:
    ~HepaFilterMonitoringDelegate() override = default;
};

class ImmutableReplacementProductListManager : public chip::app::Clusters::ResourceMonitoring::ReplacementProductListManager
{
public:
    CHIP_ERROR
    Next(chip::app::Clusters::ResourceMonitoring::ReplacementProductStruct & item) override;
};

} // namespace HepaFilterMonitoring

} // namespace Clusters
} // namespace app
} // namespace chip
