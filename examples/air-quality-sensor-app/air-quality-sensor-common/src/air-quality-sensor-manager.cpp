#include <air-quality-sensor-manager.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::DataModel;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ConcentrationMeasurement;
using namespace chip::app::Clusters::AirQuality;

void AirQualitySensorManager::Init()
{
    // Air Quality
    mAirQualityInstance.Init();
    mAirQualityInstance.UpdateAirQuality(AirQualityEnum::kGood);

    // CO2
    mCarbonDioxideConcentrationMeasurementInstance.Init();
    mCarbonDioxideConcentrationMeasurementInstance.SetMinMeasuredValue(MakeNullable(0.0f));
    mCarbonDioxideConcentrationMeasurementInstance.SetMaxMeasuredValue(MakeNullable(1000.0f));
    mCarbonDioxideConcentrationMeasurementInstance.SetMeasuredValue(MakeNullable(2.0f));
    mCarbonDioxideConcentrationMeasurementInstance.SetPeakMeasuredValue(MakeNullable(1.0f));
    mCarbonDioxideConcentrationMeasurementInstance.SetPeakMeasuredValueWindow(320);
    mCarbonDioxideConcentrationMeasurementInstance.SetAverageMeasuredValue(MakeNullable(1.0f));
    mCarbonDioxideConcentrationMeasurementInstance.SetAverageMeasuredValueWindow(320);
    mCarbonDioxideConcentrationMeasurementInstance.SetUncertainty(0.0f);
    mCarbonDioxideConcentrationMeasurementInstance.SetLevelValue(LevelValueEnum::kLow);

    // CO
    mCarbonMonoxideConcentrationMeasurementInstance.Init();
    mCarbonMonoxideConcentrationMeasurementInstance.SetMinMeasuredValue(MakeNullable(0.0f));
    mCarbonMonoxideConcentrationMeasurementInstance.SetMaxMeasuredValue(MakeNullable(1000.0f));
    mCarbonMonoxideConcentrationMeasurementInstance.SetMeasuredValue(MakeNullable(2.0f));
    mCarbonMonoxideConcentrationMeasurementInstance.SetPeakMeasuredValue(MakeNullable(1.0f));
    mCarbonMonoxideConcentrationMeasurementInstance.SetPeakMeasuredValueWindow(320);
    mCarbonMonoxideConcentrationMeasurementInstance.SetAverageMeasuredValue(MakeNullable(1.0f));
    mCarbonMonoxideConcentrationMeasurementInstance.SetAverageMeasuredValueWindow(320);
    mCarbonMonoxideConcentrationMeasurementInstance.SetUncertainty(0.0f);
    mCarbonMonoxideConcentrationMeasurementInstance.SetLevelValue(LevelValueEnum::kLow);

    // NO2
    mNitrogenDioxideConcentrationMeasurementInstance.Init();
    mNitrogenDioxideConcentrationMeasurementInstance.SetMinMeasuredValue(MakeNullable(0.0f));
    mNitrogenDioxideConcentrationMeasurementInstance.SetMaxMeasuredValue(MakeNullable(1000.0f));
    mNitrogenDioxideConcentrationMeasurementInstance.SetMeasuredValue(MakeNullable(2.0f));
    mNitrogenDioxideConcentrationMeasurementInstance.SetPeakMeasuredValue(MakeNullable(1.0f));
    mNitrogenDioxideConcentrationMeasurementInstance.SetPeakMeasuredValueWindow(320);
    mNitrogenDioxideConcentrationMeasurementInstance.SetAverageMeasuredValue(MakeNullable(1.0f));
    mNitrogenDioxideConcentrationMeasurementInstance.SetAverageMeasuredValueWindow(320);
    mNitrogenDioxideConcentrationMeasurementInstance.SetUncertainty(0.0f);
    mNitrogenDioxideConcentrationMeasurementInstance.SetLevelValue(LevelValueEnum::kLow);

    // PM1
    mPm1ConcentrationMeasurementInstance.Init();
    mPm1ConcentrationMeasurementInstance.SetMinMeasuredValue(MakeNullable(0.0f));
    mPm1ConcentrationMeasurementInstance.SetMaxMeasuredValue(MakeNullable(1000.0f));
    mPm1ConcentrationMeasurementInstance.SetMeasuredValue(MakeNullable(2.0f));
    mPm1ConcentrationMeasurementInstance.SetPeakMeasuredValue(MakeNullable(1.0f));
    mPm1ConcentrationMeasurementInstance.SetPeakMeasuredValueWindow(320);
    mPm1ConcentrationMeasurementInstance.SetAverageMeasuredValue(MakeNullable(1.0f));
    mPm1ConcentrationMeasurementInstance.SetAverageMeasuredValueWindow(320);
    mPm1ConcentrationMeasurementInstance.SetUncertainty(0.0f);
    mPm1ConcentrationMeasurementInstance.SetLevelValue(LevelValueEnum::kLow);

    // PM10
    mPm10ConcentrationMeasurementInstance.Init();
    mPm10ConcentrationMeasurementInstance.SetMinMeasuredValue(MakeNullable(0.0f));
    mPm10ConcentrationMeasurementInstance.SetMaxMeasuredValue(MakeNullable(1000.0f));
    mPm10ConcentrationMeasurementInstance.SetMeasuredValue(MakeNullable(2.0f));
    mPm10ConcentrationMeasurementInstance.SetPeakMeasuredValue(MakeNullable(1.0f));
    mPm10ConcentrationMeasurementInstance.SetPeakMeasuredValueWindow(320);
    mPm10ConcentrationMeasurementInstance.SetAverageMeasuredValue(MakeNullable(1.0f));
    mPm10ConcentrationMeasurementInstance.SetAverageMeasuredValueWindow(320);
    mPm10ConcentrationMeasurementInstance.SetUncertainty(0.0f);
    mPm10ConcentrationMeasurementInstance.SetLevelValue(LevelValueEnum::kLow);

    // PM2.5
    mPm25ConcentrationMeasurementInstance.Init();
    mPm25ConcentrationMeasurementInstance.SetMinMeasuredValue(MakeNullable(0.0f));
    mPm25ConcentrationMeasurementInstance.SetMaxMeasuredValue(MakeNullable(1000.0f));
    mPm25ConcentrationMeasurementInstance.SetMeasuredValue(MakeNullable(2.0f));
    mPm25ConcentrationMeasurementInstance.SetPeakMeasuredValue(MakeNullable(1.0f));
    mPm25ConcentrationMeasurementInstance.SetPeakMeasuredValueWindow(320);
    mPm25ConcentrationMeasurementInstance.SetAverageMeasuredValue(MakeNullable(1.0f));
    mPm25ConcentrationMeasurementInstance.SetAverageMeasuredValueWindow(320);
    mPm25ConcentrationMeasurementInstance.SetUncertainty(0.0f);
    mPm25ConcentrationMeasurementInstance.SetLevelValue(LevelValueEnum::kLow);

    // Radon
    mRadonConcentrationMeasurementInstance.Init();
    mRadonConcentrationMeasurementInstance.SetMinMeasuredValue(MakeNullable(0.0f));
    mRadonConcentrationMeasurementInstance.SetMaxMeasuredValue(MakeNullable(1000.0f));
    mRadonConcentrationMeasurementInstance.SetMeasuredValue(MakeNullable(2.0f));
    mRadonConcentrationMeasurementInstance.SetPeakMeasuredValue(MakeNullable(1.0f));
    mRadonConcentrationMeasurementInstance.SetPeakMeasuredValueWindow(320);
    mRadonConcentrationMeasurementInstance.SetAverageMeasuredValue(MakeNullable(1.0f));
    mRadonConcentrationMeasurementInstance.SetAverageMeasuredValueWindow(320);
    mRadonConcentrationMeasurementInstance.SetUncertainty(0.0f);
    mRadonConcentrationMeasurementInstance.SetLevelValue(LevelValueEnum::kLow);

    // TVOC
    mTotalVolatileOrganicCompoundsConcentrationMeasurementInstance.Init();
    mTotalVolatileOrganicCompoundsConcentrationMeasurementInstance.SetMinMeasuredValue(MakeNullable(0.0f));
    mTotalVolatileOrganicCompoundsConcentrationMeasurementInstance.SetMaxMeasuredValue(MakeNullable(1000.0f));
    mTotalVolatileOrganicCompoundsConcentrationMeasurementInstance.SetMeasuredValue(MakeNullable(2.0f));
    mTotalVolatileOrganicCompoundsConcentrationMeasurementInstance.SetPeakMeasuredValue(MakeNullable(1.0f));
    mTotalVolatileOrganicCompoundsConcentrationMeasurementInstance.SetPeakMeasuredValueWindow(320);
    mTotalVolatileOrganicCompoundsConcentrationMeasurementInstance.SetAverageMeasuredValue(MakeNullable(1.0f));
    mTotalVolatileOrganicCompoundsConcentrationMeasurementInstance.SetAverageMeasuredValueWindow(320);
    mTotalVolatileOrganicCompoundsConcentrationMeasurementInstance.SetUncertainty(0.0f);
    mTotalVolatileOrganicCompoundsConcentrationMeasurementInstance.SetLevelValue(LevelValueEnum::kLow);

    // Ozone
    mOzoneConcentrationMeasurementInstance.Init();
    mOzoneConcentrationMeasurementInstance.SetMinMeasuredValue(MakeNullable(0.0f));
    mOzoneConcentrationMeasurementInstance.SetMaxMeasuredValue(MakeNullable(1000.0f));
    mOzoneConcentrationMeasurementInstance.SetMeasuredValue(MakeNullable(2.0f));
    mOzoneConcentrationMeasurementInstance.SetPeakMeasuredValue(MakeNullable(1.0f));
    mOzoneConcentrationMeasurementInstance.SetPeakMeasuredValueWindow(320);
    mOzoneConcentrationMeasurementInstance.SetAverageMeasuredValue(MakeNullable(1.0f));
    mOzoneConcentrationMeasurementInstance.SetAverageMeasuredValueWindow(320);
    mOzoneConcentrationMeasurementInstance.SetUncertainty(0.0f);
    mOzoneConcentrationMeasurementInstance.SetLevelValue(LevelValueEnum::kLow);

    // Formaldehyde
    mFormaldehydeConcentrationMeasurementInstance.Init();
    mFormaldehydeConcentrationMeasurementInstance.SetMinMeasuredValue(MakeNullable(0.0f));
    mFormaldehydeConcentrationMeasurementInstance.SetMaxMeasuredValue(MakeNullable(1000.0f));
    mFormaldehydeConcentrationMeasurementInstance.SetMeasuredValue(MakeNullable(2.0f));
    mFormaldehydeConcentrationMeasurementInstance.SetPeakMeasuredValue(MakeNullable(1.0f));
    mFormaldehydeConcentrationMeasurementInstance.SetPeakMeasuredValueWindow(320);
    mFormaldehydeConcentrationMeasurementInstance.SetAverageMeasuredValue(MakeNullable(1.0f));
    mFormaldehydeConcentrationMeasurementInstance.SetAverageMeasuredValueWindow(320);
    mFormaldehydeConcentrationMeasurementInstance.SetUncertainty(0.0f);
    mFormaldehydeConcentrationMeasurementInstance.SetLevelValue(LevelValueEnum::kLow);
}

void AirQualitySensorManager::OnAirQualityChangeHandler(AirQualityEnum newValue)
{
    mAirQualityInstance.UpdateAirQuality(newValue);
    ChipLogDetail(NotSpecified, "Updated AirQuality value: %huu", chip::to_underlying(newValue));
}

void AirQualitySensorManager::OnCarbonDioxideMeasurementChangeHandler(float newValue)
{
    mCarbonDioxideConcentrationMeasurementInstance.SetMeasuredValue(MakeNullable(newValue));
    ChipLogDetail(NotSpecified, "Updated Carbon Dioxide: %f", newValue);
}

void AirQualitySensorManager::OnCarbonMonoxideMeasurementChangeHandler(float newValue)
{
    mCarbonMonoxideConcentrationMeasurementInstance.SetMeasuredValue(MakeNullable(newValue));
    ChipLogDetail(NotSpecified, "Updated Carbon Monoxide value: %f", newValue);
}

void AirQualitySensorManager::OnNitrogenDioxideMeasurementChangeHandler(float newValue)
{
    mNitrogenDioxideConcentrationMeasurementInstance.SetMeasuredValue(MakeNullable(newValue));
    ChipLogDetail(NotSpecified, "Updated Nitrogen Dioxide value: %f", newValue);
}

void AirQualitySensorManager::OnPm1MeasurementChangeHandler(float newValue)
{
    mPm1ConcentrationMeasurementInstance.SetMeasuredValue(MakeNullable(newValue));
    ChipLogDetail(NotSpecified, "Updated PM1 value: %f", newValue);
}

void AirQualitySensorManager::OnPm10MeasurementChangeHandler(float newValue)
{
    mPm10ConcentrationMeasurementInstance.SetMeasuredValue(MakeNullable(newValue));
    ChipLogDetail(NotSpecified, "Updated PM10 value: %f", newValue);
}

void AirQualitySensorManager::OnPm25MeasurementChangeHandler(float newValue)
{
    mPm25ConcentrationMeasurementInstance.SetMeasuredValue(MakeNullable(newValue));
    ChipLogDetail(NotSpecified, "Updated PM2.5 value: %f", newValue);
}

void AirQualitySensorManager::OnRadonMeasurementChangeHandler(float newValue)
{
    mRadonConcentrationMeasurementInstance.SetMeasuredValue(MakeNullable(newValue));
    ChipLogDetail(NotSpecified, "Updated Radon value: %f", newValue);
}

void AirQualitySensorManager::OnTotalVolatileOrganicCompoundsMeasurementChangeHandler(float newValue)
{
    mTotalVolatileOrganicCompoundsConcentrationMeasurementInstance.SetMeasuredValue(MakeNullable(newValue));
    ChipLogDetail(NotSpecified, "Updated Total Volatile Organic Compounds value: %f", newValue);
}

void AirQualitySensorManager::OnOzoneMeasurementChangeHandler(float newValue)
{
    mOzoneConcentrationMeasurementInstance.SetMeasuredValue(MakeNullable(newValue));
    ChipLogDetail(NotSpecified, "Updated Ozone value: %f", newValue);
}

void AirQualitySensorManager::OnFormaldehydeMeasurementChangeHandler(float newValue)
{
    mFormaldehydeConcentrationMeasurementInstance.SetMeasuredValue(MakeNullable(newValue));
    ChipLogDetail(NotSpecified, "Updated Formaldehyde value: %f", newValue);
}

void AirQualitySensorManager::OnTemperatureMeasurementChangeHandler(int16_t newValue)
{
    mTemperatureSensorManager.OnTemperatureChangeHandler(newValue);
    ChipLogDetail(NotSpecified, "Updated Temperature value: %hu", newValue);
}

void AirQualitySensorManager::OnHumidityMeasurementChangeHandler(uint16_t newValue)
{
    mHumiditySensorManager.OnHumidityChangeHandler(newValue);
    ChipLogDetail(NotSpecified, "Updated Humidity value: %hu", newValue);
}
