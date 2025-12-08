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

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/clusters/localization-configuration-server/LocalizationConfigurationCluster.h>
#include <app/static-cluster-config/LocalizationConfiguration.h>
#include <data-model-providers/codegen/ClusterIntegration.h>
#include <lib/support/CodeUtils.h>
#include <platform/DeviceInfoProvider.h>
#include <protocols/interaction_model/StatusCode.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::LocalizationConfiguration;
using namespace chip::app::Clusters::LocalizationConfiguration::Attributes;
using chip::Protocols::InteractionModel::Status;

// for fixed endpoint, this file is ever only included IF localization configuration is enabled and that MUST happen only on
// endpoint 0 the static assert is skipped in case of dynamic endpoints.
static_assert((LocalizationConfiguration::StaticApplicationConfig::kFixedClusterConfig.size() == 1 &&
               LocalizationConfiguration::StaticApplicationConfig::kFixedClusterConfig[0].endpointNumber == kRootEndpointId) ||
              LocalizationConfiguration::StaticApplicationConfig::kFixedClusterConfig.size() == 0);

namespace {

LazyRegisteredServerCluster<LocalizationConfigurationCluster> gServer;

class IntegrationDelegate : public CodegenClusterIntegration::Delegate
{
public:
    ServerClusterRegistration & CreateRegistration(EndpointId endpointId, unsigned clusterInstanceIndex,
                                                   uint32_t optionalAttributeBits, uint32_t featureMap) override
    {
        char outBuf[Attributes::ActiveLocale::TypeInfo::MaxLength()];
        MutableCharSpan activeLocale(outBuf);
        Status status = ActiveLocale::Get(endpointId, activeLocale);
        if (status != Status::Success)
        {
            ChipLogError(AppServer, "Failed to get active locale on endpoint %u: 0x%02x", endpointId, to_underlying(status));
        }

        gServer.Create(*DeviceLayer::GetDeviceInfoProvider(), activeLocale);
        return gServer.Registration();
    }

    ServerClusterInterface * FindRegistration(unsigned clusterInstanceIndex) override { return &gServer.Cluster(); }
    void ReleaseRegistration(unsigned clusterInstanceIndex) override { gServer.Destroy(); }
};

} // namespace

void MatterLocalizationConfigurationClusterInitCallback(EndpointId endpointId)
{
    IntegrationDelegate integrationDelegate;
    CodegenClusterIntegration::RegisterServer(
        {
            .endpointId = kRootEndpointId,
            .clusterId  = LocalizationConfiguration::Id,
            .fixedClusterInstanceCount =
                static_cast<uint16_t>(LocalizationConfiguration::StaticApplicationConfig::kFixedClusterConfig.size()),
            .maxClusterInstanceCount = 1, // only root-node functionality supported by this implementation
            .fetchFeatureMap         = false,
            .fetchOptionalAttributes = false,
        },
        integrationDelegate);
}

void MatterLocalizationConfigurationClusterShutdownCallback(EndpointId endpointId)
{
    IntegrationDelegate integrationDelegate;
    CodegenClusterIntegration::UnregisterServer(
        {
            .endpointId = kRootEndpointId,
            .clusterId  = LocalizationConfiguration::Id,
            .fixedClusterInstanceCount =
                static_cast<uint16_t>(LocalizationConfiguration::StaticApplicationConfig::kFixedClusterConfig.size()),
            .maxClusterInstanceCount = 1, // only root-node functionality supported by this implementation
        },
        integrationDelegate);
}
