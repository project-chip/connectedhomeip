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

#include <EnergyEvseManager.h>

using namespace chip;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::EnergyEvse;

static EnergyEvseDelegate * gDelegate = nullptr;
static EnergyEvseManager * gInstance  = nullptr;

void emberAfEnergyEvseClusterInitCallback(chip::EndpointId endpointId)
{
    VerifyOrDie(endpointId == 1); // this cluster is only enabled for endpoint 1.
    VerifyOrDie(gInstance == nullptr);
    gDelegate = new EnergyEvseDelegate();
    if (gDelegate != nullptr)
    {
        gInstance =
            new EnergyEvseManager(endpointId, *gDelegate,
                                  BitMask<EnergyEvse::Feature, uint32_t>(
                                      EnergyEvse::Feature::kChargingPreferences, EnergyEvse::Feature::kPlugAndCharge,
                                      EnergyEvse::Feature::kRfid, EnergyEvse::Feature::kSoCReporting, EnergyEvse::Feature::kV2x),
                                  BitMask<OptionalAttributes, uint32_t>(OptionalAttributes::kSupportsUserMaximumChargingCurrent,
                                                                        OptionalAttributes::kSupportsRandomizationWindow,
                                                                        OptionalAttributes::kSupportsApproximateEvEfficiency),
                                  BitMask<OptionalCommands, uint32_t>(OptionalCommands::kSupportsStartDiagnostics));

        gInstance->Init(); /* Register Attribute & Command handlers */
    }
}
