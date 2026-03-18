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
#include <app-common/zap-generated/callback.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/util/config.h>
#include <lib/core/DataModelTypes.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;

#ifdef MATTER_DM_PLUGIN_AIR_QUALITY_SERVER
#include <app/clusters/air-quality-server/CodegenIntegration.h>
using namespace chip::app::Clusters::AirQuality;

Protocols::InteractionModel::Status chefAirQualityWriteCallback(EndpointId endpoint, ClusterId clusterId,
                                                                const EmberAfAttributeMetadata * attributeMetadata,
                                                                uint8_t * buffer)
{
    Protocols::InteractionModel::Status ret = Protocols::InteractionModel::Status::Success;

    AirQualityCluster * clusterInstance = FindClusterOnEndpoint(endpoint);
    if (clusterInstance == nullptr)
    {
        ChipLogError(DeviceLayer, "Invalid Endpoint ID: %d", endpoint);
        return Protocols::InteractionModel::Status::UnsupportedEndpoint;
    }

    AttributeId attributeId = attributeMetadata->attributeId;

    switch (attributeId)
    {
    case chip::app::Clusters::AirQuality::Attributes::AirQuality::Id: {
        AirQualityEnum m                           = static_cast<AirQualityEnum>(buffer[0]);
        Protocols::InteractionModel::Status status = clusterInstance->SetAirQuality(m);
        if (Protocols::InteractionModel::Status::Success == status)
        {
            break;
        }
        ret = Protocols::InteractionModel::Status::UnsupportedWrite;
        ChipLogError(DeviceLayer, "Invalid Attribute Update status: %d", static_cast<int>(status));
    }
    break;
    default:
        ret = Protocols::InteractionModel::Status::UnsupportedAttribute;
        ChipLogError(DeviceLayer, "Unsupported Attribute ID: %d", static_cast<int>(attributeId));
        break;
    }

    return ret;
}

Protocols::InteractionModel::Status chefAirQualityReadCallback(EndpointId endpoint, ClusterId clusterId,
                                                               const EmberAfAttributeMetadata * attributeMetadata, uint8_t * buffer,
                                                               uint16_t maxReadLength)
{
    Protocols::InteractionModel::Status ret = Protocols::InteractionModel::Status::Success;

    return ret;
}
#endif
