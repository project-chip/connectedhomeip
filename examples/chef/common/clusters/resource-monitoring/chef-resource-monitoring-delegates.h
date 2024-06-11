/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

namespace ResourceMonitoring {

// Inherit ResourceMonitoring class to able to write external attributes
class ChefResourceMonitorInstance : public ResourceMonitoring::Instance
{
public:
    ChefResourceMonitorInstance(Delegate * aDelegate, EndpointId aEndpointId, ClusterId aClusterId, uint32_t aFeatureMap,
                                ResourceMonitoring::Attributes::DegradationDirection::TypeInfo::Type aDegradationDirection,
                                bool aResetConditionCommandSupported) :
        ResourceMonitoring::Instance(aDelegate, aEndpointId, aClusterId, aFeatureMap, aDegradationDirection,
                                     aResetConditionCommandSupported){};

    chip::Protocols::InteractionModel::Status ExternalAttributeWrite(const EmberAfAttributeMetadata * attributeMetadata,
                                                                     uint8_t * buffer);
    chip::Protocols::InteractionModel::Status ExternalAttributeRead(const EmberAfAttributeMetadata * attributeMetadata,
                                                                    uint8_t * buffer, uint16_t maxReadLength);
};

} // namespace ResourceMonitoring

namespace HepaFilterMonitoring {
class HepaFilterMonitoringDelegate : public ResourceMonitoring::Delegate
{
private:
    CHIP_ERROR Init() override;
    chip::Protocols::InteractionModel::Status PreResetCondition() override;
    chip::Protocols::InteractionModel::Status PostResetCondition() override;

public:
    ~HepaFilterMonitoringDelegate() override = default;
};

class ImmutableReplacementProductListManager : public ResourceMonitoring::ReplacementProductListManager
{
public:
    CHIP_ERROR
    Next(chip::app::Clusters::ResourceMonitoring::ReplacementProductStruct & item) override;
};

void Shutdown();

} // namespace HepaFilterMonitoring

namespace ActivatedCarbonFilterMonitoring {
class ActivatedCarbonFilterMonitoringDelegate : public ResourceMonitoring::Delegate
{
private:
    CHIP_ERROR Init() override;
    chip::Protocols::InteractionModel::Status PreResetCondition() override;
    chip::Protocols::InteractionModel::Status PostResetCondition() override;

public:
    ~ActivatedCarbonFilterMonitoringDelegate() override = default;
};

void Shutdown();

} // namespace ActivatedCarbonFilterMonitoring

} // namespace Clusters
} // namespace app
} // namespace chip

chip::Protocols::InteractionModel::Status
chefResourceMonitoringExternalWriteCallback(chip::EndpointId endpoint, chip::ClusterId clusterId,
                                            const EmberAfAttributeMetadata * attributeMetadata, uint8_t * buffer);

chip::Protocols::InteractionModel::Status
chefResourceMonitoringExternalReadCallback(chip::EndpointId endpoint, chip::ClusterId clusterId,
                                           const EmberAfAttributeMetadata * attributeMetadata, uint8_t * buffer,
                                           uint16_t maxReadLength);
