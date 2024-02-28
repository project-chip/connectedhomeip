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
#include <map>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;

#ifdef MATTER_DM_PLUGIN_AIR_QUALITY_SERVER
#include <app/clusters/air-quality-server/air-quality-server.h>
using namespace chip::app::Clusters::AirQuality;

static chip::BitMask<Feature, uint32_t> airQualityFeatures(Feature::kFair, Feature::kModerate, Feature::kVeryPoor,
                                                           Feature::kExtremelyPoor);
static std::map<int, Instance *> gAirQualityClusterInstance{};

void emberAfAirQualityClusterInitCallback(chip::EndpointId endpointId)
{
    Instance * clusterInstance = new Instance(endpointId, airQualityFeatures);
    clusterInstance->Init();
    gAirQualityClusterInstance[endpointId] = clusterInstance;
}

Protocols::InteractionModel::Status chefAirQualityWriteCallback(EndpointId endpoint, ClusterId clusterId,
                                                                const EmberAfAttributeMetadata * attributeMetadata,
                                                                uint8_t * buffer)
{
    Protocols::InteractionModel::Status ret = Protocols::InteractionModel::Status::Success;

    if (gAirQualityClusterInstance.find(endpoint) == gAirQualityClusterInstance.end())
    {
        ChipLogError(DeviceLayer, "Invalid Endpoind ID: %d", endpoint);
        return Protocols::InteractionModel::Status::UnsupportedEndpoint;
    }

    Instance * clusterInstance = gAirQualityClusterInstance[endpoint];
    AttributeId attributeId    = attributeMetadata->attributeId;

    switch (attributeId)
    {
    case chip::app::Clusters::AirQuality::Attributes::AirQuality::Id: {
        AirQualityEnum m                           = static_cast<AirQualityEnum>(buffer[0]);
        Protocols::InteractionModel::Status status = clusterInstance->UpdateAirQuality(m);
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
