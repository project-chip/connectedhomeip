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
#include <app/clusters/basic-information/BasicInformationCluster.h>
#include <app/static-cluster-config/BasicInformation.h>
#include <app/util/attribute-storage.h>
#include <data-model-providers/codegen/CodegenDataModelProvider.h>
#include <lib/support/BitFlags.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::BasicInformation::Attributes;
using namespace chip::app::Clusters::BasicInformation::StaticApplicationConfig;
using chip::Protocols::InteractionModel::Status;

namespace {

// AdministraotrCommissioningCluster implementation is specifically implemented
// only for the root endpoint (endpoint 0)
// So either:
//   - we have a fixed config and it is endpoint 0 OR
//   - we have a fully dynamic config

static constexpr size_t kBasicInformationFixedClusterCount = BasicInformation::StaticApplicationConfig::kFixedClusterConfig.size();

static_assert((kBasicInformationFixedClusterCount == 0) ||
                  ((kBasicInformationFixedClusterCount == 1) &&
                   BasicInformation::StaticApplicationConfig::kFixedClusterConfig[0].endpointNumber == kRootEndpointId),
              "Basic Information cluster MUST be on endpoint 0");

LazyRegisteredServerCluster<BasicInformationCluster> gServer;

} // namespace

void emberAfBasicInformationClusterInitCallback(EndpointId endpointId)
{
    VerifyOrReturn(endpointId == kRootEndpointId);

    // TODO:
    //  - set up ember data from zap:
    //     - nodelabel (we should have our own persistence!)
    constexpr auto enabledOptionalAttributes =
        BitFlags<OptionalBasicInformationAttributes>()
            .Set(OptionalBasicInformationAttributes::kManufacturingDate, IsAttributeEnabledOnSomeEndpoint(ManufacturingDate::Id))
            .Set(OptionalBasicInformationAttributes::kPartNumber, IsAttributeEnabledOnSomeEndpoint(PartNumber::Id))
            .Set(OptionalBasicInformationAttributes::kProductURL, IsAttributeEnabledOnSomeEndpoint(ProductURL::Id))
            .Set(OptionalBasicInformationAttributes::kProductLabel, IsAttributeEnabledOnSomeEndpoint(ProductLabel::Id))
            .Set(OptionalBasicInformationAttributes::kSerialNumber, IsAttributeEnabledOnSomeEndpoint(SerialNumber::Id))
            .Set(OptionalBasicInformationAttributes::kLocalConfigDisabled,
                 IsAttributeEnabledOnSomeEndpoint(LocalConfigDisabled::Id))
            .Set(OptionalBasicInformationAttributes::kReachable, IsAttributeEnabledOnSomeEndpoint(Reachable::Id))
            .Set(OptionalBasicInformationAttributes::kProductAppearance, IsAttributeEnabledOnSomeEndpoint(ProductAppearance::Id));

    gServer.Create(enabledOptionalAttributes);
    CHIP_ERROR err = CodegenDataModelProvider::Instance().Registry().Register(gServer.Registration());
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Basic Information register error: endpoint %u, %" CHIP_ERROR_FORMAT, endpointId, err.Format());
    }
}

void emberAfBasicInformationClusterShutdownCallback(EndpointId endpointId)
{
    VerifyOrReturn(endpointId == kRootEndpointId);

    CHIP_ERROR err = CodegenDataModelProvider::Instance().Registry().Unregister(&gServer.Cluster());
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Basic Information unregister error: endpoint %u, %" CHIP_ERROR_FORMAT, endpointId, err.Format());
    }
    gServer.Destroy();
}

void MatterBasicInformationPluginServerInitCallback() {}
void MatterBasicInformationPluginServerShutdownCallback() {}
