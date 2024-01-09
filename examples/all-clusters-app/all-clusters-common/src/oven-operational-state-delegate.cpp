/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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
#include <oven-operational-state-delegate.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::OvenCavityOperationalState;

static OperationalState::Instance * gOvenCavityOperationalStateInstance         = nullptr;
static OvenCavityOperationalStateDelegate * gOvenCavityOperationalStateDelegate = nullptr;

void OvenCavityOperationalState::Shutdown()
{
    if (gOvenCavityOperationalStateInstance != nullptr)
    {
        delete gOvenCavityOperationalStateInstance;
        gOvenCavityOperationalStateInstance = nullptr;
    }
    if (gOvenCavityOperationalStateDelegate != nullptr)
    {
        delete gOvenCavityOperationalStateDelegate;
        gOvenCavityOperationalStateDelegate = nullptr;
    }
}

void emberAfOvenCavityOperationalStateClusterInitCallback(chip::EndpointId endpointId)
{
    VerifyOrDie(endpointId == 1); // this cluster is only enabled for endpoint 1.
    VerifyOrDie(gOvenCavityOperationalStateInstance == nullptr && gOvenCavityOperationalStateDelegate == nullptr);

    gOvenCavityOperationalStateDelegate = new OvenCavityOperationalStateDelegate;
    EndpointId operationalStateEndpoint = 0x01;
    gOvenCavityOperationalStateInstance =
        new OvenCavityOperationalState::Instance(gOvenCavityOperationalStateDelegate, operationalStateEndpoint);

    gOvenCavityOperationalStateInstance->SetOperationalState(to_underlying(OperationalState::OperationalStateEnum::kStopped));

    gOvenCavityOperationalStateInstance->Init();
}

CHIP_ERROR
OvenCavityOperationalStateDelegate::GetOperationalStateAtIndex(size_t index,
                                                               OperationalState::GenericOperationalState & operationalState)
{
    if (index >= ArraySize(mOperationalStateList))
    {
        return CHIP_ERROR_NOT_FOUND;
    }
    operationalState = mOperationalStateList[index];
    return CHIP_NO_ERROR;
}

CHIP_ERROR
OvenCavityOperationalStateDelegate::GetOperationalPhaseAtIndex(size_t index, MutableCharSpan & operationalPhase)
{
    return CHIP_ERROR_NOT_FOUND;
}
