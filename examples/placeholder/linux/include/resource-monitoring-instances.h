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

/// This is an application level Instance to handle ActivatedCarbonfilterMonitoringInstance commands according to the specific
/// business logic.
class ActivatedCarbonFilterMonitoringInstance : public chip::app::Clusters::ResourceMonitoring::Instance
{
private:
    CHIP_ERROR AppInit() override;
    chip::Protocols::InteractionModel::Status PreResetCondition() override;
    chip::Protocols::InteractionModel::Status PostResetCondition() override;

public:
    ActivatedCarbonFilterMonitoringInstance(
        chip::EndpointId aEndpointId, uint32_t aFeature,
        chip::app::Clusters::ResourceMonitoring::Attributes::DegradationDirection::TypeInfo::Type aDegradationDirection,
        bool aResetConditionCommandSupported) :
        Instance(aEndpointId, chip::app::Clusters::ActivatedCarbonFilterMonitoring::Id, aFeature, aDegradationDirection,
                 aResetConditionCommandSupported){};
};

/// This is an application level instance to handle HepaFilterMonitoringInstance commands according to the specific business logic.
class HepaFilterMonitoringInstance : public chip::app::Clusters::ResourceMonitoring::Instance
{
private:
    CHIP_ERROR AppInit() override;
    chip::Protocols::InteractionModel::Status PreResetCondition() override;
    chip::Protocols::InteractionModel::Status PostResetCondition() override;

public:
    HepaFilterMonitoringInstance(
        chip::EndpointId aEndpointId, uint32_t aFeature,
        chip::app::Clusters::ResourceMonitoring::Attributes::DegradationDirection::TypeInfo::Type aDegradationDirection,
        bool aResetConditionCommandSupported) :
        Instance(aEndpointId, chip::app::Clusters::HepaFilterMonitoring::Id, aFeature, aDegradationDirection,
                 aResetConditionCommandSupported){};
};

class ImmutableReplacementProductListManager : public chip::app::Clusters::ResourceMonitoring::ReplacementProductListManager
{
public:
    CHIP_ERROR
    Next(chip::app::Clusters::ResourceMonitoring::ReplacementProductStruct & item) override;
};
