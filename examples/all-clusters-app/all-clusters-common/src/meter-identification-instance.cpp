/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <app/util/af-types.h>
#include <meter-identification-instance.h>

using namespace chip::app::Clusters;
using namespace chip::app::Clusters::MeterIdentification;

namespace {
static std::unique_ptr<Instance> gMeterIdentificationCluster;
} // namespace

Instance * MeterIdentification::GetInstance()
{
    return gMeterIdentificationCluster.get();
}

void MeterIdentification::Shutdown()
{
    VerifyOrDie(gMeterIdentificationCluster);
    gMeterIdentificationCluster->Shutdown();
    gMeterIdentificationCluster.reset(nullptr);
}

void emberAfMeterIdentificationClusterInitCallback(chip::EndpointId endpointId)
{
    ChipLogProgress(Zcl, "emberAfMeterIdentificationClusterInitCallback %d", static_cast<int>(endpointId));
    VerifyOrDie(1 == endpointId); // this cluster is only enabled for endpoint 1.
    VerifyOrDie(!gMeterIdentificationCluster);
    gMeterIdentificationCluster = std::make_unique<Instance>(1, chip::BitMask<Feature, uint32_t>(Feature::kPowerThreshold));
    gMeterIdentificationCluster->Init();
}
