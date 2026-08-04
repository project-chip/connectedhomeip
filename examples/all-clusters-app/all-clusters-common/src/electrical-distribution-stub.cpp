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

#include <data-model-providers/codegen/CodegenDataModelProvider.h>
#include <electrical-distribution-stub.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ElectricalDistribution;
using namespace chip::app::Clusters::ElectricalDistribution::Attributes;

namespace {

// all-clusters-app performs no real measurement, so publish representative fixed values.
const ElectricalDistributionCluster::StartupConfiguration kDefaultConfig = {
    .maxContinuousCurrent = DataModel::MakeNullable<int64_t>(100000), // 100 A, in mA
    .maxVoltage           = DataModel::MakeNullable<int64_t>(240000), // 240 V, in mV
    .numberOfPoles        = DataModel::MakeNullable<uint16_t>(2),
    .endOfLife            = DataModel::MakeNullable(EndOfLifeEnum::kNone),
    .serviceEntranceRated = DataModel::MakeNullable(true),
};

LazyRegisteredServerCluster<ElectricalDistributionCluster> gServer;

constexpr EndpointId kEndpointWithElectricalDistribution = 1;

bool ValidEndpointForElectricalDistribution(EndpointId endpoint)
{
    if (endpoint != kEndpointWithElectricalDistribution)
    {
        ChipLogError(AppServer, "ElectricalDistribution cluster invalid endpoint");
        return false;
    }
    return true;
}

} // namespace

void MatterElectricalDistributionClusterInitCallback(EndpointId endpoint)
{
    VerifyOrReturn(ValidEndpointForElectricalDistribution(endpoint));

    gServer.Create(endpoint, kDefaultConfig);

    LogErrorOnFailure(CodegenDataModelProvider::Instance().Registry().Register(gServer.Registration()));
}

void MatterElectricalDistributionClusterShutdownCallback(EndpointId endpoint, MatterClusterShutdownType)
{
    VerifyOrReturn(ValidEndpointForElectricalDistribution(endpoint));
    LogErrorOnFailure(CodegenDataModelProvider::Instance().Registry().Unregister(&gServer.Cluster()));

    gServer.Destroy();
}

namespace chip::app::Clusters::ElectricalDistribution {

CHIP_ERROR SetEndOfLife(const Attributes::EndOfLife::TypeInfo::Type & endOfLife)
{
    return gServer.Cluster().SetEndOfLife(endOfLife);
}

} // namespace chip::app::Clusters::ElectricalDistribution
