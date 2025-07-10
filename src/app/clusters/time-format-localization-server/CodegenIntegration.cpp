/*
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
#include <app/clusters/time-format-localization-server/time-format-localization-cluster.h>
#include <app/static-cluster-config/TimeFormatLocalization.h>
#include <platform/DeviceInfoProvider.h>
#include <platform/PlatformManager.h>
#include <app/util/attribute-metadata.h>
#include <data-model-providers/codegen/CodegenDataModelProvider.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace Protocols::InteractionModel;

namespace {

LazyRegisteredServerCluster<TimeFormatLocalizationCluster> gServer;

#if 0
constexpr bool IsAttributeEnabled(EndpointId endpointId, AttributeId attributeId) {
    for(auto & config : TimeFormatLocalization::StaticApplicationConfig::kFixedClusterConfig)
    {
        if(config.endpointNumber != endpointId)
        {
            continue;
        }
        for(auto & attr : config.enabledAttributes)
        {
            if(attr == attributeId)
            {
                return true;
            }
        }
    }
    return false;
}
#endif

}

void emberAfTimeFormatLocalizationClusterServerInitCallback(EndpointId endpoint) 
{

}

Protocols::InteractionModel::Status MatterTimeFormatLocalizationClusterServerPreAttributeChangedCallback(
    const ConcreteAttributePath & attributePath, EmberAfAttributeType attributeType, uint16_t size, uint8_t * value) {

    return Protocols::InteractionModel::Status::Success;
    
}
// TODO: This is not a proper initialization will probable need to move it to something like 
// emberAfTimeFormatLocalizationClusterServerInitCallback to be able to get the information from the endpoint.
void MatterTimeFormatLocalizationPluginServerInitCallback()
{
    uint32_t rawFeatureMap = 0;
    TimeFormatLocalizationEnabledAttributes attr {1, 1};
    gServer.Create(kRootEndpointId, BitFlags<TimeFormatLocalization::Feature>(rawFeatureMap), attr);

    CHIP_ERROR err = CodegenDataModelProvider::Instance().Registry().Register(gServer.Registration());
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "TimeFormatLocalization cluster error registration");
    }
}

void MatterTimeFormatLocalizationPluginServerShutdownCallback()
{
    CHIP_ERROR err = CodegenDataModelProvider::Instance().Registry().Unregister(&gServer.Cluster());
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "TimeFormatLocalization unregister error");
    }
    gServer.Destroy();
}