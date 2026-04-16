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

#include <air-quality-sensor-manager.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::DataModel;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ConcentrationMeasurement;
using namespace chip::app::Clusters::AirQuality;

void AirQualitySensorManager::Init()
{
    /*
     * Note these values are for testing purposes only and are not actual values for the air quality sensor.
     * They are also fixed.
     */

    // Air Quality
    TEMPORARY_RETURN_IGNORED airQualityInstance.Init();
    airQualityInstance.UpdateAirQuality(AirQualityEnum::kGood);

    // CO2 — Instance::GetDelegate() returns the DefaultDelegate; Handle*() stores + notifies.
    // No Init() call needed — ServerClusterRegistration (inside Instance) calls Startup() automatically.
    carbonDioxideConcentrationMeasurementInstance.GetDelegate().HandleNewMinMeasuredValue(MakeNullable(0.0f));
    carbonDioxideConcentrationMeasurementInstance.GetDelegate().HandleNewMaxMeasuredValue(MakeNullable(1000.0f));
    carbonDioxideConcentrationMeasurementInstance.GetDelegate().HandleNewMeasuredValue(MakeNullable(2.0f));
    carbonDioxideConcentrationMeasurementInstance.GetDelegate().HandleNewPeakMeasuredValue(MakeNullable(1.0f));
    carbonDioxideConcentrationMeasurementInstance.GetDelegate().HandleNewPeakMeasuredValueWindow(320);
    carbonDioxideConcentrationMeasurementInstance.GetDelegate().HandleNewAverageMeasuredValue(MakeNullable(1.0f));
    carbonDioxideConcentrationMeasurementInstance.GetDelegate().HandleNewAverageMeasuredValueWindow(320);
    carbonDioxideConcentrationMeasurementInstance.GetDelegate().HandleNewUncertainty(0.0f);
    carbonDioxideConcentrationMeasurementInstance.GetDelegate().HandleNewLevelValue(LevelValueEnum::kLow);

    // CO
    carbonMonoxideConcentrationMeasurementInstance.GetDelegate().HandleNewMinMeasuredValue(MakeNullable(0.0f));
    carbonMonoxideConcentrationMeasurementInstance.GetDelegate().HandleNewMaxMeasuredValue(MakeNullable(1000.0f));
    carbonMonoxideConcentrationMeasurementInstance.GetDelegate().HandleNewMeasuredValue(MakeNullable(2.0f));
    carbonMonoxideConcentrationMeasurementInstance.GetDelegate().HandleNewPeakMeasuredValue(MakeNullable(1.0f));
    carbonMonoxideConcentrationMeasurementInstance.GetDelegate().HandleNewPeakMeasuredValueWindow(320);
    carbonMonoxideConcentrationMeasurementInstance.GetDelegate().HandleNewAverageMeasuredValue(MakeNullable(1.0f));
    carbonMonoxideConcentrationMeasurementInstance.GetDelegate().HandleNewAverageMeasuredValueWindow(320);
    carbonMonoxideConcentrationMeasurementInstance.GetDelegate().HandleNewUncertainty(0.0f);
    carbonMonoxideConcentrationMeasurementInstance.GetDelegate().HandleNewLevelValue(LevelValueEnum::kLow);

    // NO2
    nitrogenDioxideConcentrationMeasurementInstance.GetDelegate().HandleNewMinMeasuredValue(MakeNullable(0.0f));
    nitrogenDioxideConcentrationMeasurementInstance.GetDelegate().HandleNewMaxMeasuredValue(MakeNullable(1000.0f));
    nitrogenDioxideConcentrationMeasurementInstance.GetDelegate().HandleNewMeasuredValue(MakeNullable(2.0f));
    nitrogenDioxideConcentrationMeasurementInstance.GetDelegate().HandleNewPeakMeasuredValue(MakeNullable(1.0f));
    nitrogenDioxideConcentrationMeasurementInstance.GetDelegate().HandleNewPeakMeasuredValueWindow(320);
    nitrogenDioxideConcentrationMeasurementInstance.GetDelegate().HandleNewAverageMeasuredValue(MakeNullable(1.0f));
    nitrogenDioxideConcentrationMeasurementInstance.GetDelegate().HandleNewAverageMeasuredValueWindow(320);
    nitrogenDioxideConcentrationMeasurementInstance.GetDelegate().HandleNewUncertainty(0.0f);
    nitrogenDioxideConcentrationMeasurementInstance.GetDelegate().HandleNewLevelValue(LevelValueEnum::kLow);

    // PM1
    pm1ConcentrationMeasurementInstance.GetDelegate().HandleNewMinMeasuredValue(MakeNullable(0.0f));
    pm1ConcentrationMeasurementInstance.GetDelegate().HandleNewMaxMeasuredValue(MakeNullable(1000.0f));
    pm1ConcentrationMeasurementInstance.GetDelegate().HandleNewMeasuredValue(MakeNullable(2.0f));
    pm1ConcentrationMeasurementInstance.GetDelegate().HandleNewPeakMeasuredValue(MakeNullable(1.0f));
    pm1ConcentrationMeasurementInstance.GetDelegate().HandleNewPeakMeasuredValueWindow(320);
    pm1ConcentrationMeasurementInstance.GetDelegate().HandleNewAverageMeasuredValue(MakeNullable(1.0f));
    pm1ConcentrationMeasurementInstance.GetDelegate().HandleNewAverageMeasuredValueWindow(320);
    pm1ConcentrationMeasurementInstance.GetDelegate().HandleNewUncertainty(0.0f);
    pm1ConcentrationMeasurementInstance.GetDelegate().HandleNewLevelValue(LevelValueEnum::kLow);

    // PM10
    pm10ConcentrationMeasurementInstance.GetDelegate().HandleNewMinMeasuredValue(MakeNullable(0.0f));
    pm10ConcentrationMeasurementInstance.GetDelegate().HandleNewMaxMeasuredValue(MakeNullable(1000.0f));
    pm10ConcentrationMeasurementInstance.GetDelegate().HandleNewMeasuredValue(MakeNullable(2.0f));
    pm10ConcentrationMeasurementInstance.GetDelegate().HandleNewPeakMeasuredValue(MakeNullable(1.0f));
    pm10ConcentrationMeasurementInstance.GetDelegate().HandleNewPeakMeasuredValueWindow(320);
    pm10ConcentrationMeasurementInstance.GetDelegate().HandleNewAverageMeasuredValue(MakeNullable(1.0f));
    pm10ConcentrationMeasurementInstance.GetDelegate().HandleNewAverageMeasuredValueWindow(320);
    pm10ConcentrationMeasurementInstance.GetDelegate().HandleNewUncertainty(0.0f);
    pm10ConcentrationMeasurementInstance.GetDelegate().HandleNewLevelValue(LevelValueEnum::kLow);

    // PM2.5
    pm25ConcentrationMeasurementInstance.GetDelegate().HandleNewMinMeasuredValue(MakeNullable(0.0f));
    pm25ConcentrationMeasurementInstance.GetDelegate().HandleNewMaxMeasuredValue(MakeNullable(1000.0f));
    pm25ConcentrationMeasurementInstance.GetDelegate().HandleNewMeasuredValue(MakeNullable(2.0f));
    pm25ConcentrationMeasurementInstance.GetDelegate().HandleNewPeakMeasuredValue(MakeNullable(1.0f));
    pm25ConcentrationMeasurementInstance.GetDelegate().HandleNewPeakMeasuredValueWindow(320);
    pm25ConcentrationMeasurementInstance.GetDelegate().HandleNewAverageMeasuredValue(MakeNullable(1.0f));
    pm25ConcentrationMeasurementInstance.GetDelegate().HandleNewAverageMeasuredValueWindow(320);
    pm25ConcentrationMeasurementInstance.GetDelegate().HandleNewUncertainty(0.0f);
    pm25ConcentrationMeasurementInstance.GetDelegate().HandleNewLevelValue(LevelValueEnum::kLow);

    // Radon
    radonConcentrationMeasurementInstance.GetDelegate().HandleNewMinMeasuredValue(MakeNullable(0.0f));
    radonConcentrationMeasurementInstance.GetDelegate().HandleNewMaxMeasuredValue(MakeNullable(1000.0f));
    radonConcentrationMeasurementInstance.GetDelegate().HandleNewMeasuredValue(MakeNullable(2.0f));
    radonConcentrationMeasurementInstance.GetDelegate().HandleNewPeakMeasuredValue(MakeNullable(1.0f));
    radonConcentrationMeasurementInstance.GetDelegate().HandleNewPeakMeasuredValueWindow(320);
    radonConcentrationMeasurementInstance.GetDelegate().HandleNewAverageMeasuredValue(MakeNullable(1.0f));
    radonConcentrationMeasurementInstance.GetDelegate().HandleNewAverageMeasuredValueWindow(320);
    radonConcentrationMeasurementInstance.GetDelegate().HandleNewUncertainty(0.0f);
    radonConcentrationMeasurementInstance.GetDelegate().HandleNewLevelValue(LevelValueEnum::kLow);

    // TVOC
    totalVolatileOrganicCompoundsConcentrationMeasurementInstance.GetDelegate().HandleNewMinMeasuredValue(MakeNullable(0.0f));
    totalVolatileOrganicCompoundsConcentrationMeasurementInstance.GetDelegate().HandleNewMaxMeasuredValue(MakeNullable(1000.0f));
    totalVolatileOrganicCompoundsConcentrationMeasurementInstance.GetDelegate().HandleNewMeasuredValue(MakeNullable(2.0f));
    totalVolatileOrganicCompoundsConcentrationMeasurementInstance.GetDelegate().HandleNewPeakMeasuredValue(MakeNullable(1.0f));
    totalVolatileOrganicCompoundsConcentrationMeasurementInstance.GetDelegate().HandleNewPeakMeasuredValueWindow(320);
    totalVolatileOrganicCompoundsConcentrationMeasurementInstance.GetDelegate().HandleNewAverageMeasuredValue(MakeNullable(1.0f));
    totalVolatileOrganicCompoundsConcentrationMeasurementInstance.GetDelegate().HandleNewAverageMeasuredValueWindow(320);
    totalVolatileOrganicCompoundsConcentrationMeasurementInstance.GetDelegate().HandleNewUncertainty(0.0f);
    totalVolatileOrganicCompoundsConcentrationMeasurementInstance.GetDelegate().HandleNewLevelValue(LevelValueEnum::kLow);

    // Ozone
    ozoneConcentrationMeasurementInstance.GetDelegate().HandleNewMinMeasuredValue(MakeNullable(0.0f));
    ozoneConcentrationMeasurementInstance.GetDelegate().HandleNewMaxMeasuredValue(MakeNullable(1000.0f));
    ozoneConcentrationMeasurementInstance.GetDelegate().HandleNewMeasuredValue(MakeNullable(2.0f));
    ozoneConcentrationMeasurementInstance.GetDelegate().HandleNewPeakMeasuredValue(MakeNullable(1.0f));
    ozoneConcentrationMeasurementInstance.GetDelegate().HandleNewPeakMeasuredValueWindow(320);
    ozoneConcentrationMeasurementInstance.GetDelegate().HandleNewAverageMeasuredValue(MakeNullable(1.0f));
    ozoneConcentrationMeasurementInstance.GetDelegate().HandleNewAverageMeasuredValueWindow(320);
    ozoneConcentrationMeasurementInstance.GetDelegate().HandleNewUncertainty(0.0f);
    ozoneConcentrationMeasurementInstance.GetDelegate().HandleNewLevelValue(LevelValueEnum::kLow);

    // Formaldehyde
    formaldehydeConcentrationMeasurementInstance.GetDelegate().HandleNewMinMeasuredValue(MakeNullable(0.0f));
    formaldehydeConcentrationMeasurementInstance.GetDelegate().HandleNewMaxMeasuredValue(MakeNullable(1000.0f));
    formaldehydeConcentrationMeasurementInstance.GetDelegate().HandleNewMeasuredValue(MakeNullable(2.0f));
    formaldehydeConcentrationMeasurementInstance.GetDelegate().HandleNewPeakMeasuredValue(MakeNullable(1.0f));
    formaldehydeConcentrationMeasurementInstance.GetDelegate().HandleNewPeakMeasuredValueWindow(320);
    formaldehydeConcentrationMeasurementInstance.GetDelegate().HandleNewAverageMeasuredValue(MakeNullable(1.0f));
    formaldehydeConcentrationMeasurementInstance.GetDelegate().HandleNewAverageMeasuredValueWindow(320);
    formaldehydeConcentrationMeasurementInstance.GetDelegate().HandleNewUncertainty(0.0f);
    formaldehydeConcentrationMeasurementInstance.GetDelegate().HandleNewLevelValue(LevelValueEnum::kLow);
}
