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
#include <app/util/endpoint-config-api.h>
#include <data-model-providers/codegen/CodegenDataModelProvider.h>
#include <lib/support/BitFlags.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::BasicInformation::Attributes;
using namespace chip::app::Clusters::BasicInformation::StaticApplicationConfig;
using chip::Protocols::InteractionModel::Status;

namespace {

// BasicInformationCluster implementation is specifically implemented
// only for the root endpoint (endpoint 0)
// So either:
//   - we have a fixed config and it is endpoint 0 OR
//   - we have a fully dynamic config

static constexpr size_t kBasicInformationFixedClusterCount = BasicInformation::StaticApplicationConfig::kFixedClusterConfig.size();

static_assert((kBasicInformationFixedClusterCount == 0) ||
                  ((kBasicInformationFixedClusterCount == 1) &&
                   BasicInformation::StaticApplicationConfig::kFixedClusterConfig[0].endpointNumber == kRootEndpointId),
              "Basic Information cluster MUST be on endpoint 0");

ServerClusterRegistration gRegistration(BasicInformationCluster::Instance());

} // namespace

void emberAfBasicInformationClusterServerInitCallback(EndpointId endpointId)
{
    VerifyOrReturn(endpointId == kRootEndpointId);

    BasicInformationCluster::OptionalAttributesSet & attrs = BasicInformationCluster::Instance().OptionalAttributes();

    attrs.Set<ManufacturingDate::Id>(emberAfContainsAttribute(endpointId, BasicInformation::Id, ManufacturingDate::Id))
        .Set<PartNumber::Id>(emberAfContainsAttribute(endpointId, BasicInformation::Id, PartNumber::Id))
        .Set<ProductURL::Id>(emberAfContainsAttribute(endpointId, BasicInformation::Id, ProductURL::Id))
        .Set<ProductLabel::Id>(emberAfContainsAttribute(endpointId, BasicInformation::Id, ProductLabel::Id))
        .Set<SerialNumber::Id>(emberAfContainsAttribute(endpointId, BasicInformation::Id, SerialNumber::Id))
        .Set<LocalConfigDisabled::Id>(emberAfContainsAttribute(endpointId, BasicInformation::Id, LocalConfigDisabled::Id))
        .Set<Reachable::Id>(emberAfContainsAttribute(endpointId, BasicInformation::Id, Reachable::Id))
        .Set<ProductAppearance::Id>(emberAfContainsAttribute(endpointId, BasicInformation::Id, ProductAppearance::Id))

        // This is NOT typical, however we try to respect ZAP here. MCORE_FS tests require this:
        // Specifically builds need to support the "do not build with unique id (make it optional)"
        // to emulate the test case where UniqueID is missing as it was optional in previous versions of the spec
        .Set<UniqueID::Id>(emberAfContainsAttribute(endpointId, BasicInformation::Id, UniqueID::Id));

    CHIP_ERROR err = CodegenDataModelProvider::Instance().Registry().Register(gRegistration);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Basic Information register error: endpoint %u, %" CHIP_ERROR_FORMAT, endpointId, err.Format());
    }
}

void MatterBasicInformationClusterServerShutdownCallback(EndpointId endpointId)
{
    VerifyOrReturn(endpointId == kRootEndpointId);

    CHIP_ERROR err = CodegenDataModelProvider::Instance().Registry().Unregister(&BasicInformationCluster::Instance());
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Basic Information unregister error: endpoint %u, %" CHIP_ERROR_FORMAT, endpointId, err.Format());
    }
}

void MatterBasicInformationPluginServerInitCallback() {}
void MatterBasicInformationPluginServerShutdownCallback() {}
