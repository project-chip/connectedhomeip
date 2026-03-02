/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <app/clusters/temperature-control-server/CodegenIntegration.h>
#include <app/clusters/temperature-control-server/TemperatureControlCluster.h>
#include <app/static-cluster-config/TemperatureControl.h>
#include <app/util/attribute-storage.h>
#include <data-model-providers/codegen/ClusterIntegration.h>
#include <data-model-providers/codegen/CodegenDataModelProvider.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::TemperatureControl;
using namespace chip::app::Clusters::TemperatureControl::Attributes;

namespace {

constexpr size_t kTemperatureControlFixedClusterCount = TemperatureControl::StaticApplicationConfig::kFixedClusterConfig.size();
constexpr size_t kTemperatureControlMaxClusterCount =
    kTemperatureControlFixedClusterCount + CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT;

LazyRegisteredServerCluster<TemperatureControlCluster> gServers[kTemperatureControlMaxClusterCount];

static SupportedTemperatureLevelsIteratorDelegate * gDelegate = nullptr;

class IntegrationDelegate : public CodegenClusterIntegration::Delegate
{
public:
    ServerClusterRegistration & CreateRegistration(EndpointId endpointId, unsigned clusterInstanceIndex,
                                                   uint32_t optionalAttributeBits, uint32_t featureMap) override
    {
        app::OptionalAttributeSet<TemperatureSetpoint::Id, MinTemperature::Id, MaxTemperature::Id, Step::Id,
                                  SelectedTemperatureLevel::Id>
            optionalAttributeSet;
        BitFlags<Feature> features(featureMap);
        using namespace chip::Protocols::InteractionModel;

        int16_t temperatureSetpoint{};
        int16_t minTemperature{};
        int16_t maxTemperature{};
        int16_t step{};
        uint8_t selectedTemperatureLevel{};
        // Enforce a valid configuration from ember
        if (features.Has(Feature::kTemperatureNumber))
        {
            VerifyOrDie(optionalAttributeSet.IsSet(TemperatureSetpoint::Id));
            VerifyOrDie(TemperatureSetpoint::Get(endpointId, &temperatureSetpoint) == Status::Success);

            VerifyOrDie(optionalAttributeSet.IsSet(MinTemperature::Id));
            VerifyOrDie(MinTemperature::Get(endpointId, &minTemperature) == Status::Success);

            VerifyOrDie(optionalAttributeSet.IsSet(MaxTemperature::Id));
            VerifyOrDie(MaxTemperature::Get(endpointId, &maxTemperature) == Status::Success);

            if (features.Has(Feature::kTemperatureStep))
            {
                VerifyOrDie(optionalAttributeSet.IsSet(Step::Id));
                VerifyOrDie(Step::Get(endpointId, &step) == Status::Success);
            }
        }
        if (features.Has(Feature::kTemperatureLevel))
        {
            // VerifyOrDie(optionalAttributeSet.IsSet(SelectedTemperatureLevel::Id));
            VerifyOrDie(SelectedTemperatureLevel::Get(endpointId, &selectedTemperatureLevel) == Status::Success);
        }

        gServers[clusterInstanceIndex].Create(
            endpointId, features,
            TemperatureControlCluster::StartupConfiguration{ .temperatureSetpoint      = temperatureSetpoint,
                                                             .minTemperature           = minTemperature,
                                                             .maxTemperature           = maxTemperature,
                                                             .step                     = step,
                                                             .selectedTemperatureLevel = selectedTemperatureLevel });
        return gServers[clusterInstanceIndex].Registration();
    }

    ServerClusterInterface * FindRegistration(unsigned clusterInstanceIndex) override
    {
        VerifyOrReturnValue(gServers[clusterInstanceIndex].IsConstructed(), nullptr);
        return &gServers[clusterInstanceIndex].Cluster();
    }

    void ReleaseRegistration(unsigned clusterInstanceIndex) override { gServers[clusterInstanceIndex].Destroy(); }
};

} // namespace

void MatterTemperatureControlClusterInitCallback(EndpointId endpointId)
{
    IntegrationDelegate integrationDelegate;

    CodegenClusterIntegration::RegisterServer(
        {
            .endpointId                = endpointId,
            .clusterId                 = TemperatureControl::Id,
            .fixedClusterInstanceCount = kTemperatureControlFixedClusterCount,
            .maxClusterInstanceCount   = kTemperatureControlMaxClusterCount,
            .fetchFeatureMap           = true,
            .fetchOptionalAttributes   = true,
        },
        integrationDelegate);
}

void MatterTemperatureControlClusterShutdownCallback(EndpointId endpointId, MatterClusterShutdownType shutdownType)
{
    IntegrationDelegate integrationDelegate;

    CodegenClusterIntegration::UnregisterServer(
        {
            .endpointId                = endpointId,
            .clusterId                 = TemperatureControl::Id,
            .fixedClusterInstanceCount = kTemperatureControlFixedClusterCount,
            .maxClusterInstanceCount   = kTemperatureControlMaxClusterCount,
        },
        integrationDelegate, shutdownType);
}

void MatterTemperatureControlPluginServerInitCallback() {}

namespace chip::app::Clusters::TemperatureControl {

TemperatureControlCluster * FindClusterOnEndpoint(EndpointId endpointId)
{
    IntegrationDelegate integrationDelegate;

    ServerClusterInterface * temperatureControl = CodegenClusterIntegration::FindClusterOnEndpoint(
        {
            .endpointId                = endpointId,
            .clusterId                 = TemperatureControl::Id,
            .fixedClusterInstanceCount = kTemperatureControlFixedClusterCount,
            .maxClusterInstanceCount   = kTemperatureControlMaxClusterCount,
        },
        integrationDelegate);

    return static_cast<TemperatureControlCluster *>(temperatureControl);
}

CHIP_ERROR SetTemperatureSetpoint(EndpointId endpointId, int16_t temperatureSetpoint)
{
    auto temperatureControl = FindClusterOnEndpoint(endpointId);
    VerifyOrReturnError(temperatureControl != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    return temperatureControl->SetTemperatureSetpoint(temperatureSetpoint);
}

SupportedTemperatureLevelsIteratorDelegate * GetDelegate()
{
    return gDelegate;
}

void SetDelegate(SupportedTemperatureLevelsIteratorDelegate * delegate)
{
    gDelegate = delegate;
}

} // namespace chip::app::Clusters::TemperatureControl
