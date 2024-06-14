/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include <MeterIdentificationDelegate.h>

using namespace chip;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::MeterIdentification;
using Feature = chip::app::Clusters::MeterIdentification::Feature;

static std::unique_ptr<MeterIdentificationDelegate> gMIDelegate;
static std::unique_ptr<MeterIdentificationInstance> gMIInstance;

void emberAfMeterIdentificationClusterInitCallback(chip::EndpointId endpointId)
{
    VerifyOrDie(endpointId == 1); // this cluster is only enabled for endpoint 1.
    VerifyOrDie(!gMIInstance);

    gMIDelegate = std::make_unique<MeterIdentificationDelegate>();
    if (gMIDelegate)
    {
        gMIInstance = std::make_unique<MeterIdentificationInstance>(
            endpointId, *gMIDelegate,
            BitMask<Feature, uint32_t>(Feature::kPowerThreshold));

        gMIInstance->Init();
    }
}
