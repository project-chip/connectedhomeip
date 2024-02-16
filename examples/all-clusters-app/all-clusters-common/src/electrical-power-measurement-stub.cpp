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

#include <ElectricalPowerMeasurementDelegate.h>

using namespace chip;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ElectricalPowerMeasurement;

static std::unique_ptr<ElectricalPowerMeasurementDelegate> gEPMDelegate;
static std::unique_ptr<ElectricalPowerMeasurementInstance> gEPMInstance;

void emberAfElectricalPowerMeasurementClusterInitCallback(chip::EndpointId endpointId)
{
    VerifyOrDie(endpointId == 1); // this cluster is only enabled for endpoint 1.
    VerifyOrDie(!gEPMInstance);

    gEPMDelegate = std::make_unique<ElectricalPowerMeasurementDelegate>();
    if (gEPMDelegate)
    {
        gEPMInstance = std::make_unique<ElectricalPowerMeasurementInstance>(
            endpointId, *gEPMDelegate,
            BitMask<Feature, uint32_t>(Feature::kDirectCurrent, Feature::kAlternatingCurrent, Feature::kPolyphasePower,
                                       Feature::kHarmonics, Feature::kPowerQuality),
            BitMask<OptionalAttributes, uint32_t>(
                OptionalAttributes::kOptionalAttributeRanges, OptionalAttributes::kOptionalAttributeVoltage,
                OptionalAttributes::kOptionalAttributeActiveCurrent, OptionalAttributes::kOptionalAttributeReactiveCurrent,
                OptionalAttributes::kOptionalAttributeApparentCurrent, OptionalAttributes::kOptionalAttributeReactivePower,
                OptionalAttributes::kOptionalAttributeApparentPower, OptionalAttributes::kOptionalAttributeRMSVoltage,
                OptionalAttributes::kOptionalAttributeRMSCurrent, OptionalAttributes::kOptionalAttributeRMSPower,
                OptionalAttributes::kOptionalAttributeFrequency, OptionalAttributes::kOptionalAttributePowerFactor,
                OptionalAttributes::kOptionalAttributeNeutralCurrent));

        gEPMInstance->Init();

        gEPMDelegate->SetPowerMode(PowerModeEnum::kAc);
    }
}
