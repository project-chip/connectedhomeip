/*
 *    Copyright (c) 2021-2025 Project CHIP Authors
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
#include <app/clusters/time-format-localization-server/time-format-localization-cluster.h>
#include <app/static-cluster-config/TimeFormatLocalization.h>
#include <app/util/attribute-metadata.h>
#include <data-model-providers/codegen/ClusterIntegration.h>
#include <data-model-providers/codegen/CodegenDataModelProvider.h>
#include <data-model-providers/codegen/CodegenProcessingConfig.h>
#include <platform/DeviceInfoProvider.h>
#include <platform/PlatformManager.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace Protocols::InteractionModel;

#if CHIP_CODEGEN_CONFIG_ENABLE_CODEGEN_INTEGRATION_LOOKUP_ERRORS
#define CodegenInitError(...) ChipLogError(AppServer, __VA_ARGS__)
#else // CHIP_CODEGEN_CONFIG_ENABLE_CODEGEN_INTEGRATION_LOOKUP_ERRORS
#define CodegenInitError(...) (void) 0;
#endif // CHIP_CODEGEN_CONFIG_ENABLE_CODEGEN_INTEGRATION_LOOKUP_ERRORS

namespace {

void FetchDefaults(BitFlags<TimeFormatLocalization::Feature> featureMap, TimeFormatLocalization::HourFormatEnum & defaultHourFormat,
                   TimeFormatLocalization::CalendarTypeEnum & defaultCalendarType)
{
    // hour format always supported
    if (TimeFormatLocalization::Attributes::HourFormat::Get(kRootEndpointId, &defaultHourFormat) != Status::Success)
    {
        CodegenInitError("Failed to get HourFormat for endpoint %u", kRootEndpointId);
        defaultHourFormat = TimeFormatLocalization::HourFormatEnum::k12hr;
    }

    // Calendar format is feature-dependent. We set some default but still try to read it
    defaultCalendarType = TimeFormatLocalization::CalendarTypeEnum::kGregorian;
    if (featureMap.Has(TimeFormatLocalization::Feature::kCalendarFormat))
    {
        if (TimeFormatLocalization::Attributes::ActiveCalendarType::Get(kRootEndpointId, &defaultCalendarType) != Status::Success)
        {
            CodegenInitError("Failed to get ActiveCalendarType for endpoint %u", kRootEndpointId);
            defaultCalendarType = TimeFormatLocalization::CalendarTypeEnum::kGregorian;
        }
    }
}

LazyRegisteredServerCluster<TimeFormatLocalizationCluster> gServer;

class IntegrationDelegate : public CodegenClusterIntegration::Delegate
{
public:
    ServerClusterRegistration & CreateRegistration(EndpointId endpointId, unsigned clusterInstanceIndex,
                                                   uint32_t optionalAttributeBits, uint32_t rawFeatureMap) override
    {
        TimeFormatLocalization::HourFormatEnum defaultHourFormat;
        TimeFormatLocalization::CalendarTypeEnum defaultCalendarType;
        const BitFlags<TimeFormatLocalization::Feature> featureMap(rawFeatureMap);
        FetchDefaults(featureMap, defaultHourFormat, defaultCalendarType);

        gServer.Create(endpointId, featureMap, defaultHourFormat, defaultCalendarType);

        return gServer.Registration();
    }

    ServerClusterInterface * FindRegistration(unsigned clusterInstanceIndex) override
    {
        VerifyOrReturnValue(gServer.IsConstructed(), nullptr);
        return &gServer.Cluster();
    }

    // Nothing to destroy: separate singleton class without constructor/destructor is used
    void ReleaseRegistration(unsigned clusterInstanceIndex) override { gServer.Destroy(); }
};

} // namespace

void MatterTimeFormatLocalizationClusterInitCallback(EndpointId endpoint)
{
    // This cluster should only exist in Root endpoint.
    VerifyOrReturn(endpoint == kRootEndpointId);

    IntegrationDelegate integrationDelegate;
    CodegenClusterIntegration::RegisterServer(
        {
            .endpointId                = kRootEndpointId,
            .clusterId                 = TimeFormatLocalization::Id,
            .fixedClusterInstanceCount = TimeFormatLocalization::StaticApplicationConfig::kFixedClusterConfig.size(),
            .maxClusterInstanceCount   = 1, // Cluster is a singleton on the root node and this is the only thing supported
            .fetchFeatureMap           = true,
            .fetchOptionalAttributes   = false,
        },
        integrationDelegate);
}

void MatterTimeFormatLocalizationClusterShutdownCallback(EndpointId endpoint)
{
    // This cluster should only exist in Root endpoint.
    VerifyOrReturn(endpoint == kRootEndpointId);

    IntegrationDelegate integrationDelegate;
    CodegenClusterIntegration::UnregisterServer(
        {
            .endpointId                = kRootEndpointId,
            .clusterId                 = TimeFormatLocalization::Id,
            .fixedClusterInstanceCount = TimeFormatLocalization::StaticApplicationConfig::kFixedClusterConfig.size(),
            .maxClusterInstanceCount   = 1, // Cluster is a singleton on the root node and this is the only thing supported
        },
        integrationDelegate);
}

void MatterTimeFormatLocalizationPluginServerInitCallback() {}
