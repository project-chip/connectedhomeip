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

#include "CodegenIntegration.h"

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/server/Server.h>
#include <app/static-cluster-config/PowerSource.h>
#include <data-model-providers/codegen/ClusterIntegration.h>
#include <data-model-providers/codegen/CodegenDataModelProvider.h>
#include <include/platform/CHIPDeviceLayer.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::PowerSource;
using namespace chip::app::Clusters::PowerSource::Attributes;

namespace {

LazyRegisteredServerCluster<MinimalWiredPowerSourceCluster> gServers;

class IntegrationDelegate : public CodegenClusterIntegration::Delegate
{
public:
    ServerClusterRegistration & CreateRegistration(EndpointId endpointId, unsigned clusterInstanceIndex,
                                                   uint32_t optionalAttributeBits, uint32_t featureMap) override
    {
        using namespace chip::Protocols;

        // Enforce a valid configuration from ember

        char descriptionBuffer[Description::TypeInfo::MaxLength()];
        MutableCharSpan description(descriptionBuffer);
        if (Description::Get(endpointId, description) != InteractionModel::Status::Success)
        {
            // an acceptable default of empty string
            description.reduce_size(0);
        }

            PowerSourceCluster::WiredCurrentTypeEnum currentType;
            VerifyOrDie(WiredCurrentType::Get(endpointId, &currentType) == InteractionModel::Status::Success);

            PowerSourceCluster::WiredConfiguration config(description, currentType);

            gServers.Create(endpointId, config);

        MinimalWiredPowerSourceCluster & cluster = gServers.Cluster();

        // Get all set defaults for attributes from ember.

#define DieIfInvalidValue(expr, attr_name)                                                                                         \
    {                                                                                                                              \
        CHIP_ERROR error_val = (expr);                                                                                             \
        VerifyOrDieWithMsg(error_val == CHIP_NO_ERROR || error_val == CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE, Zcl,                    \
                           "Unexpected error %" CHIP_ERROR_FORMAT " when trying to set attribute `" #attr_name "`.",               \
                           error_val.Format());                                                                                    \
    }

#define SetAttributeDefaultFromEmber(type, attr_name)                                                                              \
    if (type val{}; attr_name::Get(endpointId, &val) == InteractionModel::Status::Success)                                         \
    {                                                                                                                              \
        DieIfInvalidValue(cluster.Set##attr_name(val), attr_name);                                                                 \
    }

#define SetNullableAttributeDefaultFromEmber(type, attr_name)                                                                      \
    if (DataModel::Nullable<type> val{}; attr_name::Get(endpointId, val) == InteractionModel::Status::Success)                     \
    {                                                                                                                              \
        if (val.IsNull())                                                                                                          \
        {                                                                                                                          \
            (void) cluster.Set##attr_name(NullOptional); /* null is valid, can ignore the error */                                 \
        }                                                                                                                          \
        else                                                                                                                       \
        {                                                                                                                          \
            DieIfInvalidValue(cluster.Set##attr_name(Optional(val.Value())), attr_name);                                           \
        }                                                                                                                          \
    }

        SetAttributeDefaultFromEmber(PowerSourceCluster::PowerSourceStatusEnum, Status);
        SetAttributeDefaultFromEmber(uint8_t, Order);

#undef DieIfInvalidValue
#undef SetAttributeDefaultFromEmber
#undef SetNullableAttributeDefaultFromEmber

        return gServers.Registration();
    }

    ServerClusterInterface * FindRegistration(unsigned clusterInstanceIndex) override
    {
        VerifyOrReturnValue(gServers.IsConstructed(), nullptr);
        return &gServers.Cluster();
    }

    void ReleaseRegistration(unsigned clusterInstanceIndex) override { gServers.Destroy(); }
};

} // namespace

void MatterPowerSourceClusterInitCallback(EndpointId endpointId)
{
    IntegrationDelegate integrationDelegate;

    CodegenClusterIntegration::RegisterServer(
        {
            .endpointId                = endpointId,
            .clusterId                 = PowerSource::Id,
            .fixedClusterInstanceCount = 1,
            .maxClusterInstanceCount   = 1,
            .fetchFeatureMap           = false,
            .fetchOptionalAttributes   = false,
        },
        integrationDelegate);
}

void MatterPowerSourceClusterShutdownCallback(EndpointId endpointId, MatterClusterShutdownType shutdownType)
{
    IntegrationDelegate integrationDelegate;

    CodegenClusterIntegration::UnregisterServer(
        {
            .endpointId                = endpointId,
            .clusterId                 = PowerSource::Id,
            .fixedClusterInstanceCount = 1,
            .maxClusterInstanceCount   = 1,
        },
        integrationDelegate, shutdownType);
}

void MatterPowerSourcePluginServerInitCallback() {}

namespace chip::app::Clusters::PowerSource {

MinimalWiredPowerSourceCluster * FindClusterOnEndpoint(EndpointId endpointId)
{
    IntegrationDelegate integrationDelegate;

    ServerClusterInterface * powerSource = CodegenClusterIntegration::FindClusterOnEndpoint(
        {
            .endpointId                = endpointId,
            .clusterId                 = PowerSource::Id,
            .fixedClusterInstanceCount = 1,
            .maxClusterInstanceCount   = 1,
        },
        integrationDelegate);

    return static_cast<MinimalWiredPowerSourceCluster *>(powerSource);
}

} // namespace chip::app::Clusters::PowerSource
