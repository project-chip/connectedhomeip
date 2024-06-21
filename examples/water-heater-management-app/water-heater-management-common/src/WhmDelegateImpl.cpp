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
#include <app/clusters/water-heater-management-server/water-heater-management-server.h>

#include <WhmDelegate.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::WaterHeaterManagement;

using Protocols::InteractionModel::Status;

static Instance * gWhmInstance = nullptr;
static Delegate * gWhmDelegate = nullptr;

WaterHeaterManagement::Instance * GetWaterHeaterManagementInstance()
{
    return gWhmInstance;
}

void WaterHeaterManagement::Shutdown()
{
    if (gWhmInstance != nullptr)
    {
        delete gWhmInstance;
        gWhmInstance = nullptr;
    }
    if (gWhmDelegate != nullptr)
    {
        delete gWhmDelegate;
        gWhmDelegate = nullptr;
    }
}

void emberAfWaterHeaterManagementClusterInitCallback(chip::EndpointId endpointId)
{
    VerifyOrDie(endpointId == 1); // this cluster is only enabled for endpoint 1.
    VerifyOrDie(gWhmInstance == nullptr && gWhmDelegate == nullptr);

    gWhmDelegate = new WaterHeaterManagementDelegate;
    EndpointId waterHeaterManagementEndpoint = 0x01;
    gWhmInstance =
        new WaterHeaterManagement::Instance(waterHeaterManagementEndpoint,
                                            *gWhmDelegate,
                                            BitMask<Feature, uint32_t>(Feature::kEnergyManagement, Feature::kTankPercent));

    gWhmInstance->Init();
}

WaterHeaterTypeBitmap WaterHeaterManagementDelegate::GetHeaterTypes()
{
    return mHeaterTypes;
}

WaterHeaterDemandBitmap WaterHeaterManagementDelegate::GetHeatDemand()
{
    return mHeatDemand;
}

uint16_t WaterHeaterManagementDelegate::GetTankVolume()
{
    return mTankVolume;
}

int64_t WaterHeaterManagementDelegate::GetEstimatedHeatRequired()
{
    return mEstimatedHeatRequired;
}

Percent WaterHeaterManagementDelegate::GetTankPercentage()
{
    return mTankPercentage;
}

BoostStateEnum WaterHeaterManagementDelegate::GetBoostState()
{
    return mBoostState;
}

Protocols::InteractionModel::Status WaterHeaterManagementDelegate::HandleBoost(uint32_t duration, Optional<bool> oneShot, Optional<bool> emergencyBoost, Optional<int16_t> temporarySetpoint, Optional<chip::Percent> targetPercentage, Optional<chip::Percent> targetReheat)
{
    ChipLogProgress(AppServer, "WaterHeaterManagementDelegate::HandleBoost");

    return Status::Success;
}

Protocols::InteractionModel::Status WaterHeaterManagementDelegate::HandleCancelBoost()
{
    ChipLogProgress(AppServer, "WaterHeaterManagementDelegate::HandleCancelBoost");

    return Status::Success;
}

