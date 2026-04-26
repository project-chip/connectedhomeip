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

#include <air-quality-sensor-manager.h>
#include <data-model-providers/codegen/CodegenDataModelProvider.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::DataModel;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ConcentrationMeasurement;
using namespace chip::app::Clusters::AirQuality;

void AirQualitySensorManager::Init()
{
    auto & registry = CodegenDataModelProvider::Instance().Registry();

    // Air Quality
    TEMPORARY_RETURN_IGNORED mAirQualityInstance.Init();
    mAirQualityInstance.UpdateAirQuality(AirQualityEnum::kGood);

    // CO2
    LogErrorOnFailure(registry.Register(mCarbonDioxideRegistration));
    TEMPORARY_RETURN_IGNORED mCarbonDioxideConcentrationMeasurementInstance.SetMeasuredValue(MakeNullable(2.0f));
    TEMPORARY_RETURN_IGNORED mCarbonDioxideConcentrationMeasurementInstance.SetPeakMeasuredValue(MakeNullable(1.0f));
    TEMPORARY_RETURN_IGNORED mCarbonDioxideConcentrationMeasurementInstance.SetPeakMeasuredValueWindow(320);
    TEMPORARY_RETURN_IGNORED mCarbonDioxideConcentrationMeasurementInstance.SetAverageMeasuredValue(MakeNullable(1.0f));
    TEMPORARY_RETURN_IGNORED mCarbonDioxideConcentrationMeasurementInstance.SetAverageMeasuredValueWindow(320);
    TEMPORARY_RETURN_IGNORED mCarbonDioxideConcentrationMeasurementInstance.SetLevelValue(LevelValueEnum::kLow);

    // CO
    LogErrorOnFailure(registry.Register(mCarbonMonoxideRegistration));
    TEMPORARY_RETURN_IGNORED mCarbonMonoxideConcentrationMeasurementInstance.SetMeasuredValue(MakeNullable(2.0f));
    TEMPORARY_RETURN_IGNORED mCarbonMonoxideConcentrationMeasurementInstance.SetPeakMeasuredValue(MakeNullable(1.0f));
    TEMPORARY_RETURN_IGNORED mCarbonMonoxideConcentrationMeasurementInstance.SetPeakMeasuredValueWindow(320);
    TEMPORARY_RETURN_IGNORED mCarbonMonoxideConcentrationMeasurementInstance.SetAverageMeasuredValue(MakeNullable(1.0f));
    TEMPORARY_RETURN_IGNORED mCarbonMonoxideConcentrationMeasurementInstance.SetAverageMeasuredValueWindow(320);
    TEMPORARY_RETURN_IGNORED mCarbonMonoxideConcentrationMeasurementInstance.SetLevelValue(LevelValueEnum::kLow);

    // NO2
    LogErrorOnFailure(registry.Register(mNitrogenDioxideRegistration));
    TEMPORARY_RETURN_IGNORED mNitrogenDioxideConcentrationMeasurementInstance.SetMeasuredValue(MakeNullable(2.0f));
    TEMPORARY_RETURN_IGNORED mNitrogenDioxideConcentrationMeasurementInstance.SetPeakMeasuredValue(MakeNullable(1.0f));
    TEMPORARY_RETURN_IGNORED mNitrogenDioxideConcentrationMeasurementInstance.SetPeakMeasuredValueWindow(320);
    TEMPORARY_RETURN_IGNORED mNitrogenDioxideConcentrationMeasurementInstance.SetAverageMeasuredValue(MakeNullable(1.0f));
    TEMPORARY_RETURN_IGNORED mNitrogenDioxideConcentrationMeasurementInstance.SetAverageMeasuredValueWindow(320);
    TEMPORARY_RETURN_IGNORED mNitrogenDioxideConcentrationMeasurementInstance.SetLevelValue(LevelValueEnum::kLow);

    // PM1
    LogErrorOnFailure(registry.Register(mPm1Registration));
    TEMPORARY_RETURN_IGNORED mPm1ConcentrationMeasurementInstance.SetMeasuredValue(MakeNullable(2.0f));
    TEMPORARY_RETURN_IGNORED mPm1ConcentrationMeasurementInstance.SetPeakMeasuredValue(MakeNullable(1.0f));
    TEMPORARY_RETURN_IGNORED mPm1ConcentrationMeasurementInstance.SetPeakMeasuredValueWindow(320);
    TEMPORARY_RETURN_IGNORED mPm1ConcentrationMeasurementInstance.SetAverageMeasuredValue(MakeNullable(1.0f));
    TEMPORARY_RETURN_IGNORED mPm1ConcentrationMeasurementInstance.SetAverageMeasuredValueWindow(320);
    TEMPORARY_RETURN_IGNORED mPm1ConcentrationMeasurementInstance.SetLevelValue(LevelValueEnum::kLow);

    // PM10
    LogErrorOnFailure(registry.Register(mPm10Registration));
    TEMPORARY_RETURN_IGNORED mPm10ConcentrationMeasurementInstance.SetMeasuredValue(MakeNullable(2.0f));
    TEMPORARY_RETURN_IGNORED mPm10ConcentrationMeasurementInstance.SetPeakMeasuredValue(MakeNullable(1.0f));
    TEMPORARY_RETURN_IGNORED mPm10ConcentrationMeasurementInstance.SetPeakMeasuredValueWindow(320);
    TEMPORARY_RETURN_IGNORED mPm10ConcentrationMeasurementInstance.SetAverageMeasuredValue(MakeNullable(1.0f));
    TEMPORARY_RETURN_IGNORED mPm10ConcentrationMeasurementInstance.SetAverageMeasuredValueWindow(320);
    TEMPORARY_RETURN_IGNORED mPm10ConcentrationMeasurementInstance.SetLevelValue(LevelValueEnum::kLow);

    // PM2.5
    LogErrorOnFailure(registry.Register(mPm25Registration));
    TEMPORARY_RETURN_IGNORED mPm25ConcentrationMeasurementInstance.SetMeasuredValue(MakeNullable(2.0f));
    TEMPORARY_RETURN_IGNORED mPm25ConcentrationMeasurementInstance.SetPeakMeasuredValue(MakeNullable(1.0f));
    TEMPORARY_RETURN_IGNORED mPm25ConcentrationMeasurementInstance.SetPeakMeasuredValueWindow(320);
    TEMPORARY_RETURN_IGNORED mPm25ConcentrationMeasurementInstance.SetAverageMeasuredValue(MakeNullable(1.0f));
    TEMPORARY_RETURN_IGNORED mPm25ConcentrationMeasurementInstance.SetAverageMeasuredValueWindow(320);
    TEMPORARY_RETURN_IGNORED mPm25ConcentrationMeasurementInstance.SetLevelValue(LevelValueEnum::kLow);

    // Radon
    LogErrorOnFailure(registry.Register(mRadonRegistration));
    TEMPORARY_RETURN_IGNORED mRadonConcentrationMeasurementInstance.SetMeasuredValue(MakeNullable(2.0f));
    TEMPORARY_RETURN_IGNORED mRadonConcentrationMeasurementInstance.SetPeakMeasuredValue(MakeNullable(1.0f));
    TEMPORARY_RETURN_IGNORED mRadonConcentrationMeasurementInstance.SetPeakMeasuredValueWindow(320);
    TEMPORARY_RETURN_IGNORED mRadonConcentrationMeasurementInstance.SetAverageMeasuredValue(MakeNullable(1.0f));
    TEMPORARY_RETURN_IGNORED mRadonConcentrationMeasurementInstance.SetAverageMeasuredValueWindow(320);
    TEMPORARY_RETURN_IGNORED mRadonConcentrationMeasurementInstance.SetLevelValue(LevelValueEnum::kLow);

    // TVOC
    LogErrorOnFailure(registry.Register(mTotalVolatileOrganicCompoundsRegistration));
    TEMPORARY_RETURN_IGNORED mTotalVolatileOrganicCompoundsConcentrationMeasurementInstance.SetMeasuredValue(MakeNullable(2.0f));
    TEMPORARY_RETURN_IGNORED mTotalVolatileOrganicCompoundsConcentrationMeasurementInstance.SetPeakMeasuredValue(
        MakeNullable(1.0f));
    TEMPORARY_RETURN_IGNORED mTotalVolatileOrganicCompoundsConcentrationMeasurementInstance.SetPeakMeasuredValueWindow(320);
    TEMPORARY_RETURN_IGNORED mTotalVolatileOrganicCompoundsConcentrationMeasurementInstance.SetAverageMeasuredValue(
        MakeNullable(1.0f));
    TEMPORARY_RETURN_IGNORED mTotalVolatileOrganicCompoundsConcentrationMeasurementInstance.SetAverageMeasuredValueWindow(320);
    TEMPORARY_RETURN_IGNORED mTotalVolatileOrganicCompoundsConcentrationMeasurementInstance.SetLevelValue(LevelValueEnum::kLow);

    // Ozone
    LogErrorOnFailure(registry.Register(mOzoneRegistration));
    TEMPORARY_RETURN_IGNORED mOzoneConcentrationMeasurementInstance.SetMeasuredValue(MakeNullable(2.0f));
    TEMPORARY_RETURN_IGNORED mOzoneConcentrationMeasurementInstance.SetPeakMeasuredValue(MakeNullable(1.0f));
    TEMPORARY_RETURN_IGNORED mOzoneConcentrationMeasurementInstance.SetPeakMeasuredValueWindow(320);
    TEMPORARY_RETURN_IGNORED mOzoneConcentrationMeasurementInstance.SetAverageMeasuredValue(MakeNullable(1.0f));
    TEMPORARY_RETURN_IGNORED mOzoneConcentrationMeasurementInstance.SetAverageMeasuredValueWindow(320);
    TEMPORARY_RETURN_IGNORED mOzoneConcentrationMeasurementInstance.SetLevelValue(LevelValueEnum::kLow);

    // Formaldehyde
    LogErrorOnFailure(registry.Register(mFormaldehydeRegistration));
    TEMPORARY_RETURN_IGNORED mFormaldehydeConcentrationMeasurementInstance.SetMeasuredValue(MakeNullable(2.0f));
    TEMPORARY_RETURN_IGNORED mFormaldehydeConcentrationMeasurementInstance.SetPeakMeasuredValue(MakeNullable(1.0f));
    TEMPORARY_RETURN_IGNORED mFormaldehydeConcentrationMeasurementInstance.SetPeakMeasuredValueWindow(320);
    TEMPORARY_RETURN_IGNORED mFormaldehydeConcentrationMeasurementInstance.SetAverageMeasuredValue(MakeNullable(1.0f));
    TEMPORARY_RETURN_IGNORED mFormaldehydeConcentrationMeasurementInstance.SetAverageMeasuredValueWindow(320);
    TEMPORARY_RETURN_IGNORED mFormaldehydeConcentrationMeasurementInstance.SetLevelValue(LevelValueEnum::kLow);
}
