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

    // CO2
    TEMPORARY_RETURN_IGNORED carbonDioxideConcentrationMeasurementInstance.Init();
    TEMPORARY_RETURN_IGNORED carbonDioxideConcentrationMeasurementInstance.SetMinMeasuredValue(MakeNullable(0.0f));
    TEMPORARY_RETURN_IGNORED carbonDioxideConcentrationMeasurementInstance.SetMaxMeasuredValue(MakeNullable(1000.0f));
    TEMPORARY_RETURN_IGNORED carbonDioxideConcentrationMeasurementInstance.SetMeasuredValue(MakeNullable(2.0f));
    TEMPORARY_RETURN_IGNORED carbonDioxideConcentrationMeasurementInstance.SetPeakMeasuredValue(MakeNullable(1.0f));
    TEMPORARY_RETURN_IGNORED carbonDioxideConcentrationMeasurementInstance.SetPeakMeasuredValueWindow(320);
    TEMPORARY_RETURN_IGNORED carbonDioxideConcentrationMeasurementInstance.SetAverageMeasuredValue(MakeNullable(1.0f));
    TEMPORARY_RETURN_IGNORED carbonDioxideConcentrationMeasurementInstance.SetAverageMeasuredValueWindow(320);
    TEMPORARY_RETURN_IGNORED carbonDioxideConcentrationMeasurementInstance.SetUncertainty(0.0f);
    TEMPORARY_RETURN_IGNORED carbonDioxideConcentrationMeasurementInstance.SetLevelValue(LevelValueEnum::kLow);

    // CO
    TEMPORARY_RETURN_IGNORED carbonMonoxideConcentrationMeasurementInstance.Init();
    TEMPORARY_RETURN_IGNORED carbonMonoxideConcentrationMeasurementInstance.SetMinMeasuredValue(MakeNullable(0.0f));
    TEMPORARY_RETURN_IGNORED carbonMonoxideConcentrationMeasurementInstance.SetMaxMeasuredValue(MakeNullable(1000.0f));
    TEMPORARY_RETURN_IGNORED carbonMonoxideConcentrationMeasurementInstance.SetMeasuredValue(MakeNullable(2.0f));
    TEMPORARY_RETURN_IGNORED carbonMonoxideConcentrationMeasurementInstance.SetPeakMeasuredValue(MakeNullable(1.0f));
    TEMPORARY_RETURN_IGNORED carbonMonoxideConcentrationMeasurementInstance.SetPeakMeasuredValueWindow(320);
    TEMPORARY_RETURN_IGNORED carbonMonoxideConcentrationMeasurementInstance.SetAverageMeasuredValue(MakeNullable(1.0f));
    TEMPORARY_RETURN_IGNORED carbonMonoxideConcentrationMeasurementInstance.SetAverageMeasuredValueWindow(320);
    TEMPORARY_RETURN_IGNORED carbonMonoxideConcentrationMeasurementInstance.SetUncertainty(0.0f);
    TEMPORARY_RETURN_IGNORED carbonMonoxideConcentrationMeasurementInstance.SetLevelValue(LevelValueEnum::kLow);

    // NO2
    TEMPORARY_RETURN_IGNORED nitrogenDioxideConcentrationMeasurementInstance.Init();
    TEMPORARY_RETURN_IGNORED nitrogenDioxideConcentrationMeasurementInstance.SetMinMeasuredValue(MakeNullable(0.0f));
    TEMPORARY_RETURN_IGNORED nitrogenDioxideConcentrationMeasurementInstance.SetMaxMeasuredValue(MakeNullable(1000.0f));
    TEMPORARY_RETURN_IGNORED nitrogenDioxideConcentrationMeasurementInstance.SetMeasuredValue(MakeNullable(2.0f));
    TEMPORARY_RETURN_IGNORED nitrogenDioxideConcentrationMeasurementInstance.SetPeakMeasuredValue(MakeNullable(1.0f));
    TEMPORARY_RETURN_IGNORED nitrogenDioxideConcentrationMeasurementInstance.SetPeakMeasuredValueWindow(320);
    TEMPORARY_RETURN_IGNORED nitrogenDioxideConcentrationMeasurementInstance.SetAverageMeasuredValue(MakeNullable(1.0f));
    TEMPORARY_RETURN_IGNORED nitrogenDioxideConcentrationMeasurementInstance.SetAverageMeasuredValueWindow(320);
    TEMPORARY_RETURN_IGNORED nitrogenDioxideConcentrationMeasurementInstance.SetUncertainty(0.0f);
    TEMPORARY_RETURN_IGNORED nitrogenDioxideConcentrationMeasurementInstance.SetLevelValue(LevelValueEnum::kLow);

    // PM1
    TEMPORARY_RETURN_IGNORED pm1ConcentrationMeasurementInstance.Init();
    TEMPORARY_RETURN_IGNORED pm1ConcentrationMeasurementInstance.SetMinMeasuredValue(MakeNullable(0.0f));
    TEMPORARY_RETURN_IGNORED pm1ConcentrationMeasurementInstance.SetMaxMeasuredValue(MakeNullable(1000.0f));
    TEMPORARY_RETURN_IGNORED pm1ConcentrationMeasurementInstance.SetMeasuredValue(MakeNullable(2.0f));
    TEMPORARY_RETURN_IGNORED pm1ConcentrationMeasurementInstance.SetPeakMeasuredValue(MakeNullable(1.0f));
    TEMPORARY_RETURN_IGNORED pm1ConcentrationMeasurementInstance.SetPeakMeasuredValueWindow(320);
    TEMPORARY_RETURN_IGNORED pm1ConcentrationMeasurementInstance.SetAverageMeasuredValue(MakeNullable(1.0f));
    TEMPORARY_RETURN_IGNORED pm1ConcentrationMeasurementInstance.SetAverageMeasuredValueWindow(320);
    TEMPORARY_RETURN_IGNORED pm1ConcentrationMeasurementInstance.SetUncertainty(0.0f);
    TEMPORARY_RETURN_IGNORED pm1ConcentrationMeasurementInstance.SetLevelValue(LevelValueEnum::kLow);

    // PM10
    TEMPORARY_RETURN_IGNORED pm10ConcentrationMeasurementInstance.Init();
    TEMPORARY_RETURN_IGNORED pm10ConcentrationMeasurementInstance.SetMinMeasuredValue(MakeNullable(0.0f));
    TEMPORARY_RETURN_IGNORED pm10ConcentrationMeasurementInstance.SetMaxMeasuredValue(MakeNullable(1000.0f));
    TEMPORARY_RETURN_IGNORED pm10ConcentrationMeasurementInstance.SetMeasuredValue(MakeNullable(2.0f));
    TEMPORARY_RETURN_IGNORED pm10ConcentrationMeasurementInstance.SetPeakMeasuredValue(MakeNullable(1.0f));
    TEMPORARY_RETURN_IGNORED pm10ConcentrationMeasurementInstance.SetPeakMeasuredValueWindow(320);
    TEMPORARY_RETURN_IGNORED pm10ConcentrationMeasurementInstance.SetAverageMeasuredValue(MakeNullable(1.0f));
    TEMPORARY_RETURN_IGNORED pm10ConcentrationMeasurementInstance.SetAverageMeasuredValueWindow(320);
    TEMPORARY_RETURN_IGNORED pm10ConcentrationMeasurementInstance.SetUncertainty(0.0f);
    TEMPORARY_RETURN_IGNORED pm10ConcentrationMeasurementInstance.SetLevelValue(LevelValueEnum::kLow);

    // PM2.5
    TEMPORARY_RETURN_IGNORED pm25ConcentrationMeasurementInstance.Init();
    TEMPORARY_RETURN_IGNORED pm25ConcentrationMeasurementInstance.SetMinMeasuredValue(MakeNullable(0.0f));
    TEMPORARY_RETURN_IGNORED pm25ConcentrationMeasurementInstance.SetMaxMeasuredValue(MakeNullable(1000.0f));
    TEMPORARY_RETURN_IGNORED pm25ConcentrationMeasurementInstance.SetMeasuredValue(MakeNullable(2.0f));
    TEMPORARY_RETURN_IGNORED pm25ConcentrationMeasurementInstance.SetPeakMeasuredValue(MakeNullable(1.0f));
    TEMPORARY_RETURN_IGNORED pm25ConcentrationMeasurementInstance.SetPeakMeasuredValueWindow(320);
    TEMPORARY_RETURN_IGNORED pm25ConcentrationMeasurementInstance.SetAverageMeasuredValue(MakeNullable(1.0f));
    TEMPORARY_RETURN_IGNORED pm25ConcentrationMeasurementInstance.SetAverageMeasuredValueWindow(320);
    TEMPORARY_RETURN_IGNORED pm25ConcentrationMeasurementInstance.SetUncertainty(0.0f);
    TEMPORARY_RETURN_IGNORED pm25ConcentrationMeasurementInstance.SetLevelValue(LevelValueEnum::kLow);

    // Radon
    TEMPORARY_RETURN_IGNORED radonConcentrationMeasurementInstance.Init();
    TEMPORARY_RETURN_IGNORED radonConcentrationMeasurementInstance.SetMinMeasuredValue(MakeNullable(0.0f));
    TEMPORARY_RETURN_IGNORED radonConcentrationMeasurementInstance.SetMaxMeasuredValue(MakeNullable(1000.0f));
    TEMPORARY_RETURN_IGNORED radonConcentrationMeasurementInstance.SetMeasuredValue(MakeNullable(2.0f));
    TEMPORARY_RETURN_IGNORED radonConcentrationMeasurementInstance.SetPeakMeasuredValue(MakeNullable(1.0f));
    TEMPORARY_RETURN_IGNORED radonConcentrationMeasurementInstance.SetPeakMeasuredValueWindow(320);
    TEMPORARY_RETURN_IGNORED radonConcentrationMeasurementInstance.SetAverageMeasuredValue(MakeNullable(1.0f));
    TEMPORARY_RETURN_IGNORED radonConcentrationMeasurementInstance.SetAverageMeasuredValueWindow(320);
    TEMPORARY_RETURN_IGNORED radonConcentrationMeasurementInstance.SetUncertainty(0.0f);
    TEMPORARY_RETURN_IGNORED radonConcentrationMeasurementInstance.SetLevelValue(LevelValueEnum::kLow);

    // TVOC
    TEMPORARY_RETURN_IGNORED totalVolatileOrganicCompoundsConcentrationMeasurementInstance.Init();
    TEMPORARY_RETURN_IGNORED totalVolatileOrganicCompoundsConcentrationMeasurementInstance.SetMinMeasuredValue(MakeNullable(0.0f));
    TEMPORARY_RETURN_IGNORED totalVolatileOrganicCompoundsConcentrationMeasurementInstance.SetMaxMeasuredValue(
        MakeNullable(1000.0f));
    TEMPORARY_RETURN_IGNORED totalVolatileOrganicCompoundsConcentrationMeasurementInstance.SetMeasuredValue(MakeNullable(2.0f));
    TEMPORARY_RETURN_IGNORED totalVolatileOrganicCompoundsConcentrationMeasurementInstance.SetPeakMeasuredValue(MakeNullable(1.0f));
    TEMPORARY_RETURN_IGNORED totalVolatileOrganicCompoundsConcentrationMeasurementInstance.SetPeakMeasuredValueWindow(320);
    TEMPORARY_RETURN_IGNORED totalVolatileOrganicCompoundsConcentrationMeasurementInstance.SetAverageMeasuredValue(
        MakeNullable(1.0f));
    TEMPORARY_RETURN_IGNORED totalVolatileOrganicCompoundsConcentrationMeasurementInstance.SetAverageMeasuredValueWindow(320);
    TEMPORARY_RETURN_IGNORED totalVolatileOrganicCompoundsConcentrationMeasurementInstance.SetUncertainty(0.0f);
    TEMPORARY_RETURN_IGNORED totalVolatileOrganicCompoundsConcentrationMeasurementInstance.SetLevelValue(LevelValueEnum::kLow);

    // Ozone
    TEMPORARY_RETURN_IGNORED ozoneConcentrationMeasurementInstance.Init();
    TEMPORARY_RETURN_IGNORED ozoneConcentrationMeasurementInstance.SetMinMeasuredValue(MakeNullable(0.0f));
    TEMPORARY_RETURN_IGNORED ozoneConcentrationMeasurementInstance.SetMaxMeasuredValue(MakeNullable(1000.0f));
    TEMPORARY_RETURN_IGNORED ozoneConcentrationMeasurementInstance.SetMeasuredValue(MakeNullable(2.0f));
    TEMPORARY_RETURN_IGNORED ozoneConcentrationMeasurementInstance.SetPeakMeasuredValue(MakeNullable(1.0f));
    TEMPORARY_RETURN_IGNORED ozoneConcentrationMeasurementInstance.SetPeakMeasuredValueWindow(320);
    TEMPORARY_RETURN_IGNORED ozoneConcentrationMeasurementInstance.SetAverageMeasuredValue(MakeNullable(1.0f));
    TEMPORARY_RETURN_IGNORED ozoneConcentrationMeasurementInstance.SetAverageMeasuredValueWindow(320);
    TEMPORARY_RETURN_IGNORED ozoneConcentrationMeasurementInstance.SetUncertainty(0.0f);
    TEMPORARY_RETURN_IGNORED ozoneConcentrationMeasurementInstance.SetLevelValue(LevelValueEnum::kLow);

    // Formaldehyde
    TEMPORARY_RETURN_IGNORED formaldehydeConcentrationMeasurementInstance.Init();
    TEMPORARY_RETURN_IGNORED formaldehydeConcentrationMeasurementInstance.SetMinMeasuredValue(MakeNullable(0.0f));
    TEMPORARY_RETURN_IGNORED formaldehydeConcentrationMeasurementInstance.SetMaxMeasuredValue(MakeNullable(1000.0f));
    TEMPORARY_RETURN_IGNORED formaldehydeConcentrationMeasurementInstance.SetMeasuredValue(MakeNullable(2.0f));
    TEMPORARY_RETURN_IGNORED formaldehydeConcentrationMeasurementInstance.SetPeakMeasuredValue(MakeNullable(1.0f));
    TEMPORARY_RETURN_IGNORED formaldehydeConcentrationMeasurementInstance.SetPeakMeasuredValueWindow(320);
    TEMPORARY_RETURN_IGNORED formaldehydeConcentrationMeasurementInstance.SetAverageMeasuredValue(MakeNullable(1.0f));
    TEMPORARY_RETURN_IGNORED formaldehydeConcentrationMeasurementInstance.SetAverageMeasuredValueWindow(320);
    TEMPORARY_RETURN_IGNORED formaldehydeConcentrationMeasurementInstance.SetUncertainty(0.0f);
    TEMPORARY_RETURN_IGNORED formaldehydeConcentrationMeasurementInstance.SetLevelValue(LevelValueEnum::kLow);
}
