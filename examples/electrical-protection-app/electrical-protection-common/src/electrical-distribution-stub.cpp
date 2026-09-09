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

#include <electrical-distribution-stub.h>

#include <lib/support/CodeUtils.h>

#include <memory>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ElectricalDistribution;
using namespace chip::app::Clusters::ElectricalDistribution::Attributes;

namespace {

// This app performs no real measurement, so publish representative fixed values.
const ElectricalDistributionCluster::StartupConfiguration kDefaultConfig = {
    .maxContinuousCurrent = DataModel::MakeNullable<int64_t>(100000), // 100 A, in mA
    .maxVoltage           = DataModel::MakeNullable<int64_t>(240000), // 240 V, in mV
    .numberOfPoles        = DataModel::MakeNullable<uint16_t>(2),
    .endOfLife            = DataModel::MakeNullable(EndOfLifeEnum::kNone),
    .serviceEntranceRated = DataModel::MakeNullable(true),
};

std::unique_ptr<ElectricalDistribution::Instance> gInstance;

} // namespace

namespace chip::app::Clusters::ElectricalDistribution {

CHIP_ERROR ElectricalDistributionInit(EndpointId endpointId)
{
    VerifyOrReturnError(gInstance == nullptr, CHIP_ERROR_INCORRECT_STATE);

    gInstance = std::make_unique<Instance>(endpointId, kDefaultConfig);
    VerifyOrReturnError(gInstance != nullptr, CHIP_ERROR_NO_MEMORY);

    CHIP_ERROR err = gInstance->Init();
    if (err != CHIP_NO_ERROR)
    {
        gInstance.reset();
    }
    return err;
}

void ElectricalDistributionShutdown()
{
    // ~Instance() unregisters.
    gInstance.reset();
}

CHIP_ERROR SetEndOfLife(const Attributes::EndOfLife::TypeInfo::Type & endOfLife)
{
    VerifyOrReturnError(gInstance != nullptr, CHIP_ERROR_INCORRECT_STATE);
    return gInstance->Cluster().SetEndOfLife(endOfLife);
}

} // namespace chip::app::Clusters::ElectricalDistribution
