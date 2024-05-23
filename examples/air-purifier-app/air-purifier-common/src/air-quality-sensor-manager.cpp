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
    airQualityInstance.Init();
    airQualityInstance.UpdateAirQuality(AirQualityEnum::kGood);

    // CO2
    carbonDioxideConcentrationMeasurementInstance.Init();
    carbonDioxideConcentrationMeasurementInstance.SetMinMeasuredValue(MakeNullable(0.0f));
    carbonDioxideConcentrationMeasurementInstance.SetMaxMeasuredValue(MakeNullable(1000.0f));
    carbonDioxideConcentrationMeasurementInstance.SetMeasuredValue(MakeNullable(2.0f));
    carbonDioxideConcentrationMeasurementInstance.SetPeakMeasuredValue(MakeNullable(1.0f));
    carbonDioxideConcentrationMeasurementInstance.SetPeakMeasuredValueWindow(320);
    carbonDioxideConcentrationMeasurementInstance.SetAverageMeasuredValue(MakeNullable(1.0f));
    carbonDioxideConcentrationMeasurementInstance.SetAverageMeasuredValueWindow(320);
    carbonDioxideConcentrationMeasurementInstance.SetUncertainty(0.0f);
    carbonDioxideConcentrationMeasurementInstance.SetLevelValue(LevelValueEnum::kLow);

    // CO
    carbonMonoxideConcentrationMeasurementInstance.Init();
    carbonMonoxideConcentrationMeasurementInstance.SetMinMeasuredValue(MakeNullable(0.0f));
    carbonMonoxideConcentrationMeasurementInstance.SetMaxMeasuredValue(MakeNullable(1000.0f));
    carbonMonoxideConcentrationMeasurementInstance.SetMeasuredValue(MakeNullable(2.0f));
    carbonMonoxideConcentrationMeasurementInstance.SetPeakMeasuredValue(MakeNullable(1.0f));
    carbonMonoxideConcentrationMeasurementInstance.SetPeakMeasuredValueWindow(320);
    carbonMonoxideConcentrationMeasurementInstance.SetAverageMeasuredValue(MakeNullable(1.0f));
    carbonMonoxideConcentrationMeasurementInstance.SetAverageMeasuredValueWindow(320);
    carbonMonoxideConcentrationMeasurementInstance.SetUncertainty(0.0f);
    carbonMonoxideConcentrationMeasurementInstance.SetLevelValue(LevelValueEnum::kLow);

    // NO2
    nitrogenDioxideConcentrationMeasurementInstance.Init();
    nitrogenDioxideConcentrationMeasurementInstance.SetMinMeasuredValue(MakeNullable(0.0f));
    nitrogenDioxideConcentrationMeasurementInstance.SetMaxMeasuredValue(MakeNullable(1000.0f));
    nitrogenDioxideConcentrationMeasurementInstance.SetMeasuredValue(MakeNullable(2.0f));
    nitrogenDioxideConcentrationMeasurementInstance.SetPeakMeasuredValue(MakeNullable(1.0f));
    nitrogenDioxideConcentrationMeasurementInstance.SetPeakMeasuredValueWindow(320);
    nitrogenDioxideConcentrationMeasurementInstance.SetAverageMeasuredValue(MakeNullable(1.0f));
    nitrogenDioxideConcentrationMeasurementInstance.SetAverageMeasuredValueWindow(320);
    nitrogenDioxideConcentrationMeasurementInstance.SetUncertainty(0.0f);
    nitrogenDioxideConcentrationMeasurementInstance.SetLevelValue(LevelValueEnum::kLow);

    // PM1
    pm1ConcentrationMeasurementInstance.Init();
    pm1ConcentrationMeasurementInstance.SetMinMeasuredValue(MakeNullable(0.0f));
    pm1ConcentrationMeasurementInstance.SetMaxMeasuredValue(MakeNullable(1000.0f));
    pm1ConcentrationMeasurementInstance.SetMeasuredValue(MakeNullable(2.0f));
    pm1ConcentrationMeasurementInstance.SetPeakMeasuredValue(MakeNullable(1.0f));
    pm1ConcentrationMeasurementInstance.SetPeakMeasuredValueWindow(320);
    pm1ConcentrationMeasurementInstance.SetAverageMeasuredValue(MakeNullable(1.0f));
    pm1ConcentrationMeasurementInstance.SetAverageMeasuredValueWindow(320);
    pm1ConcentrationMeasurementInstance.SetUncertainty(0.0f);
    pm1ConcentrationMeasurementInstance.SetLevelValue(LevelValueEnum::kLow);

    // PM10
    pm10ConcentrationMeasurementInstance.Init();
    pm10ConcentrationMeasurementInstance.SetMinMeasuredValue(MakeNullable(0.0f));
    pm10ConcentrationMeasurementInstance.SetMaxMeasuredValue(MakeNullable(1000.0f));
    pm10ConcentrationMeasurementInstance.SetMeasuredValue(MakeNullable(2.0f));
    pm10ConcentrationMeasurementInstance.SetPeakMeasuredValue(MakeNullable(1.0f));
    pm10ConcentrationMeasurementInstance.SetPeakMeasuredValueWindow(320);
    pm10ConcentrationMeasurementInstance.SetAverageMeasuredValue(MakeNullable(1.0f));
    pm10ConcentrationMeasurementInstance.SetAverageMeasuredValueWindow(320);
    pm10ConcentrationMeasurementInstance.SetUncertainty(0.0f);
    pm10ConcentrationMeasurementInstance.SetLevelValue(LevelValueEnum::kLow);

    // PM2.5
    pm25ConcentrationMeasurementInstance.Init();
    pm25ConcentrationMeasurementInstance.SetMinMeasuredValue(MakeNullable(0.0f));
    pm25ConcentrationMeasurementInstance.SetMaxMeasuredValue(MakeNullable(1000.0f));
    pm25ConcentrationMeasurementInstance.SetMeasuredValue(MakeNullable(2.0f));
    pm25ConcentrationMeasurementInstance.SetPeakMeasuredValue(MakeNullable(1.0f));
    pm25ConcentrationMeasurementInstance.SetPeakMeasuredValueWindow(320);
    pm25ConcentrationMeasurementInstance.SetAverageMeasuredValue(MakeNullable(1.0f));
    pm25ConcentrationMeasurementInstance.SetAverageMeasuredValueWindow(320);
    pm25ConcentrationMeasurementInstance.SetUncertainty(0.0f);
    pm25ConcentrationMeasurementInstance.SetLevelValue(LevelValueEnum::kLow);

    // Radon
    radonConcentrationMeasurementInstance.Init();
    radonConcentrationMeasurementInstance.SetMinMeasuredValue(MakeNullable(0.0f));
    radonConcentrationMeasurementInstance.SetMaxMeasuredValue(MakeNullable(1000.0f));
    radonConcentrationMeasurementInstance.SetMeasuredValue(MakeNullable(2.0f));
    radonConcentrationMeasurementInstance.SetPeakMeasuredValue(MakeNullable(1.0f));
    radonConcentrationMeasurementInstance.SetPeakMeasuredValueWindow(320);
    radonConcentrationMeasurementInstance.SetAverageMeasuredValue(MakeNullable(1.0f));
    radonConcentrationMeasurementInstance.SetAverageMeasuredValueWindow(320);
    radonConcentrationMeasurementInstance.SetUncertainty(0.0f);
    radonConcentrationMeasurementInstance.SetLevelValue(LevelValueEnum::kLow);

    // TVOC
    totalVolatileOrganicCompoundsConcentrationMeasurementInstance.Init();
    totalVolatileOrganicCompoundsConcentrationMeasurementInstance.SetMinMeasuredValue(MakeNullable(0.0f));
    totalVolatileOrganicCompoundsConcentrationMeasurementInstance.SetMaxMeasuredValue(MakeNullable(1000.0f));
    totalVolatileOrganicCompoundsConcentrationMeasurementInstance.SetMeasuredValue(MakeNullable(2.0f));
    totalVolatileOrganicCompoundsConcentrationMeasurementInstance.SetPeakMeasuredValue(MakeNullable(1.0f));
    totalVolatileOrganicCompoundsConcentrationMeasurementInstance.SetPeakMeasuredValueWindow(320);
    totalVolatileOrganicCompoundsConcentrationMeasurementInstance.SetAverageMeasuredValue(MakeNullable(1.0f));
    totalVolatileOrganicCompoundsConcentrationMeasurementInstance.SetAverageMeasuredValueWindow(320);
    totalVolatileOrganicCompoundsConcentrationMeasurementInstance.SetUncertainty(0.0f);
    totalVolatileOrganicCompoundsConcentrationMeasurementInstance.SetLevelValue(LevelValueEnum::kLow);

    // Ozone
    ozoneConcentrationMeasurementInstance.Init();
    ozoneConcentrationMeasurementInstance.SetMinMeasuredValue(MakeNullable(0.0f));
    ozoneConcentrationMeasurementInstance.SetMaxMeasuredValue(MakeNullable(1000.0f));
    ozoneConcentrationMeasurementInstance.SetMeasuredValue(MakeNullable(2.0f));
    ozoneConcentrationMeasurementInstance.SetPeakMeasuredValue(MakeNullable(1.0f));
    ozoneConcentrationMeasurementInstance.SetPeakMeasuredValueWindow(320);
    ozoneConcentrationMeasurementInstance.SetAverageMeasuredValue(MakeNullable(1.0f));
    ozoneConcentrationMeasurementInstance.SetAverageMeasuredValueWindow(320);
    ozoneConcentrationMeasurementInstance.SetUncertainty(0.0f);
    ozoneConcentrationMeasurementInstance.SetLevelValue(LevelValueEnum::kLow);

    // Formaldehyde
    formaldehydeConcentrationMeasurementInstance.Init();
    formaldehydeConcentrationMeasurementInstance.SetMinMeasuredValue(MakeNullable(0.0f));
    formaldehydeConcentrationMeasurementInstance.SetMaxMeasuredValue(MakeNullable(1000.0f));
    formaldehydeConcentrationMeasurementInstance.SetMeasuredValue(MakeNullable(2.0f));
    formaldehydeConcentrationMeasurementInstance.SetPeakMeasuredValue(MakeNullable(1.0f));
    formaldehydeConcentrationMeasurementInstance.SetPeakMeasuredValueWindow(320);
    formaldehydeConcentrationMeasurementInstance.SetAverageMeasuredValue(MakeNullable(1.0f));
    formaldehydeConcentrationMeasurementInstance.SetAverageMeasuredValueWindow(320);
    formaldehydeConcentrationMeasurementInstance.SetUncertainty(0.0f);
    formaldehydeConcentrationMeasurementInstance.SetLevelValue(LevelValueEnum::kLow);
}
