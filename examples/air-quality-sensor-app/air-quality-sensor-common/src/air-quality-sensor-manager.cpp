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
    TEMPORARY_RETURN_IGNORED mAirQualityInstance.Init();
    mAirQualityInstance.UpdateAirQuality(AirQualityEnum::kGood);

    // CO2
    TEMPORARY_RETURN_IGNORED mCarbonDioxideConcentrationMeasurementInstance.Init();
    TEMPORARY_RETURN_IGNORED mCarbonDioxideConcentrationMeasurementInstance.SetMinMeasuredValue(MakeNullable(0.0f));
    TEMPORARY_RETURN_IGNORED mCarbonDioxideConcentrationMeasurementInstance.SetMaxMeasuredValue(MakeNullable(1000.0f));
    TEMPORARY_RETURN_IGNORED mCarbonDioxideConcentrationMeasurementInstance.SetMeasuredValue(MakeNullable(2.0f));
    TEMPORARY_RETURN_IGNORED mCarbonDioxideConcentrationMeasurementInstance.SetPeakMeasuredValue(MakeNullable(1.0f));
    TEMPORARY_RETURN_IGNORED mCarbonDioxideConcentrationMeasurementInstance.SetPeakMeasuredValueWindow(320);
    TEMPORARY_RETURN_IGNORED mCarbonDioxideConcentrationMeasurementInstance.SetAverageMeasuredValue(MakeNullable(1.0f));
    TEMPORARY_RETURN_IGNORED mCarbonDioxideConcentrationMeasurementInstance.SetAverageMeasuredValueWindow(320);
    TEMPORARY_RETURN_IGNORED mCarbonDioxideConcentrationMeasurementInstance.SetUncertainty(0.0f);
    TEMPORARY_RETURN_IGNORED mCarbonDioxideConcentrationMeasurementInstance.SetLevelValue(LevelValueEnum::kLow);

    // CO
    TEMPORARY_RETURN_IGNORED mCarbonMonoxideConcentrationMeasurementInstance.Init();
    TEMPORARY_RETURN_IGNORED mCarbonMonoxideConcentrationMeasurementInstance.SetMinMeasuredValue(MakeNullable(0.0f));
    TEMPORARY_RETURN_IGNORED mCarbonMonoxideConcentrationMeasurementInstance.SetMaxMeasuredValue(MakeNullable(1000.0f));
    TEMPORARY_RETURN_IGNORED mCarbonMonoxideConcentrationMeasurementInstance.SetMeasuredValue(MakeNullable(2.0f));
    TEMPORARY_RETURN_IGNORED mCarbonMonoxideConcentrationMeasurementInstance.SetPeakMeasuredValue(MakeNullable(1.0f));
    TEMPORARY_RETURN_IGNORED mCarbonMonoxideConcentrationMeasurementInstance.SetPeakMeasuredValueWindow(320);
    TEMPORARY_RETURN_IGNORED mCarbonMonoxideConcentrationMeasurementInstance.SetAverageMeasuredValue(MakeNullable(1.0f));
    TEMPORARY_RETURN_IGNORED mCarbonMonoxideConcentrationMeasurementInstance.SetAverageMeasuredValueWindow(320);
    TEMPORARY_RETURN_IGNORED mCarbonMonoxideConcentrationMeasurementInstance.SetUncertainty(0.0f);
    TEMPORARY_RETURN_IGNORED mCarbonMonoxideConcentrationMeasurementInstance.SetLevelValue(LevelValueEnum::kLow);

    // NO2
    TEMPORARY_RETURN_IGNORED mNitrogenDioxideConcentrationMeasurementInstance.Init();
    TEMPORARY_RETURN_IGNORED mNitrogenDioxideConcentrationMeasurementInstance.SetMinMeasuredValue(MakeNullable(0.0f));
    TEMPORARY_RETURN_IGNORED mNitrogenDioxideConcentrationMeasurementInstance.SetMaxMeasuredValue(MakeNullable(1000.0f));
    TEMPORARY_RETURN_IGNORED mNitrogenDioxideConcentrationMeasurementInstance.SetMeasuredValue(MakeNullable(2.0f));
    TEMPORARY_RETURN_IGNORED mNitrogenDioxideConcentrationMeasurementInstance.SetPeakMeasuredValue(MakeNullable(1.0f));
    TEMPORARY_RETURN_IGNORED mNitrogenDioxideConcentrationMeasurementInstance.SetPeakMeasuredValueWindow(320);
    TEMPORARY_RETURN_IGNORED mNitrogenDioxideConcentrationMeasurementInstance.SetAverageMeasuredValue(MakeNullable(1.0f));
    TEMPORARY_RETURN_IGNORED mNitrogenDioxideConcentrationMeasurementInstance.SetAverageMeasuredValueWindow(320);
    TEMPORARY_RETURN_IGNORED mNitrogenDioxideConcentrationMeasurementInstance.SetUncertainty(0.0f);
    TEMPORARY_RETURN_IGNORED mNitrogenDioxideConcentrationMeasurementInstance.SetLevelValue(LevelValueEnum::kLow);

    // PM1
    TEMPORARY_RETURN_IGNORED mPm1ConcentrationMeasurementInstance.Init();
    TEMPORARY_RETURN_IGNORED mPm1ConcentrationMeasurementInstance.SetMinMeasuredValue(MakeNullable(0.0f));
    TEMPORARY_RETURN_IGNORED mPm1ConcentrationMeasurementInstance.SetMaxMeasuredValue(MakeNullable(1000.0f));
    TEMPORARY_RETURN_IGNORED mPm1ConcentrationMeasurementInstance.SetMeasuredValue(MakeNullable(2.0f));
    TEMPORARY_RETURN_IGNORED mPm1ConcentrationMeasurementInstance.SetPeakMeasuredValue(MakeNullable(1.0f));
    TEMPORARY_RETURN_IGNORED mPm1ConcentrationMeasurementInstance.SetPeakMeasuredValueWindow(320);
    TEMPORARY_RETURN_IGNORED mPm1ConcentrationMeasurementInstance.SetAverageMeasuredValue(MakeNullable(1.0f));
    TEMPORARY_RETURN_IGNORED mPm1ConcentrationMeasurementInstance.SetAverageMeasuredValueWindow(320);
    TEMPORARY_RETURN_IGNORED mPm1ConcentrationMeasurementInstance.SetUncertainty(0.0f);
    TEMPORARY_RETURN_IGNORED mPm1ConcentrationMeasurementInstance.SetLevelValue(LevelValueEnum::kLow);

    // PM10
    TEMPORARY_RETURN_IGNORED mPm10ConcentrationMeasurementInstance.Init();
    TEMPORARY_RETURN_IGNORED mPm10ConcentrationMeasurementInstance.SetMinMeasuredValue(MakeNullable(0.0f));
    TEMPORARY_RETURN_IGNORED mPm10ConcentrationMeasurementInstance.SetMaxMeasuredValue(MakeNullable(1000.0f));
    TEMPORARY_RETURN_IGNORED mPm10ConcentrationMeasurementInstance.SetMeasuredValue(MakeNullable(2.0f));
    TEMPORARY_RETURN_IGNORED mPm10ConcentrationMeasurementInstance.SetPeakMeasuredValue(MakeNullable(1.0f));
    TEMPORARY_RETURN_IGNORED mPm10ConcentrationMeasurementInstance.SetPeakMeasuredValueWindow(320);
    TEMPORARY_RETURN_IGNORED mPm10ConcentrationMeasurementInstance.SetAverageMeasuredValue(MakeNullable(1.0f));
    TEMPORARY_RETURN_IGNORED mPm10ConcentrationMeasurementInstance.SetAverageMeasuredValueWindow(320);
    TEMPORARY_RETURN_IGNORED mPm10ConcentrationMeasurementInstance.SetUncertainty(0.0f);
    TEMPORARY_RETURN_IGNORED mPm10ConcentrationMeasurementInstance.SetLevelValue(LevelValueEnum::kLow);

    // PM2.5
    TEMPORARY_RETURN_IGNORED mPm25ConcentrationMeasurementInstance.Init();
    TEMPORARY_RETURN_IGNORED mPm25ConcentrationMeasurementInstance.SetMinMeasuredValue(MakeNullable(0.0f));
    TEMPORARY_RETURN_IGNORED mPm25ConcentrationMeasurementInstance.SetMaxMeasuredValue(MakeNullable(1000.0f));
    TEMPORARY_RETURN_IGNORED mPm25ConcentrationMeasurementInstance.SetMeasuredValue(MakeNullable(2.0f));
    TEMPORARY_RETURN_IGNORED mPm25ConcentrationMeasurementInstance.SetPeakMeasuredValue(MakeNullable(1.0f));
    TEMPORARY_RETURN_IGNORED mPm25ConcentrationMeasurementInstance.SetPeakMeasuredValueWindow(320);
    TEMPORARY_RETURN_IGNORED mPm25ConcentrationMeasurementInstance.SetAverageMeasuredValue(MakeNullable(1.0f));
    TEMPORARY_RETURN_IGNORED mPm25ConcentrationMeasurementInstance.SetAverageMeasuredValueWindow(320);
    TEMPORARY_RETURN_IGNORED mPm25ConcentrationMeasurementInstance.SetUncertainty(0.0f);
    TEMPORARY_RETURN_IGNORED mPm25ConcentrationMeasurementInstance.SetLevelValue(LevelValueEnum::kLow);

    // Radon
    TEMPORARY_RETURN_IGNORED mRadonConcentrationMeasurementInstance.Init();
    TEMPORARY_RETURN_IGNORED mRadonConcentrationMeasurementInstance.SetMinMeasuredValue(MakeNullable(0.0f));
    TEMPORARY_RETURN_IGNORED mRadonConcentrationMeasurementInstance.SetMaxMeasuredValue(MakeNullable(1000.0f));
    TEMPORARY_RETURN_IGNORED mRadonConcentrationMeasurementInstance.SetMeasuredValue(MakeNullable(2.0f));
    TEMPORARY_RETURN_IGNORED mRadonConcentrationMeasurementInstance.SetPeakMeasuredValue(MakeNullable(1.0f));
    TEMPORARY_RETURN_IGNORED mRadonConcentrationMeasurementInstance.SetPeakMeasuredValueWindow(320);
    TEMPORARY_RETURN_IGNORED mRadonConcentrationMeasurementInstance.SetAverageMeasuredValue(MakeNullable(1.0f));
    TEMPORARY_RETURN_IGNORED mRadonConcentrationMeasurementInstance.SetAverageMeasuredValueWindow(320);
    TEMPORARY_RETURN_IGNORED mRadonConcentrationMeasurementInstance.SetUncertainty(0.0f);
    TEMPORARY_RETURN_IGNORED mRadonConcentrationMeasurementInstance.SetLevelValue(LevelValueEnum::kLow);

    // TVOC
    TEMPORARY_RETURN_IGNORED mTotalVolatileOrganicCompoundsConcentrationMeasurementInstance.Init();
    TEMPORARY_RETURN_IGNORED mTotalVolatileOrganicCompoundsConcentrationMeasurementInstance.SetMinMeasuredValue(MakeNullable(0.0f));
    TEMPORARY_RETURN_IGNORED mTotalVolatileOrganicCompoundsConcentrationMeasurementInstance.SetMaxMeasuredValue(
        MakeNullable(1000.0f));
    TEMPORARY_RETURN_IGNORED mTotalVolatileOrganicCompoundsConcentrationMeasurementInstance.SetMeasuredValue(MakeNullable(2.0f));
    TEMPORARY_RETURN_IGNORED mTotalVolatileOrganicCompoundsConcentrationMeasurementInstance.SetPeakMeasuredValue(
        MakeNullable(1.0f));
    TEMPORARY_RETURN_IGNORED mTotalVolatileOrganicCompoundsConcentrationMeasurementInstance.SetPeakMeasuredValueWindow(320);
    TEMPORARY_RETURN_IGNORED mTotalVolatileOrganicCompoundsConcentrationMeasurementInstance.SetAverageMeasuredValue(
        MakeNullable(1.0f));
    TEMPORARY_RETURN_IGNORED mTotalVolatileOrganicCompoundsConcentrationMeasurementInstance.SetAverageMeasuredValueWindow(320);
    TEMPORARY_RETURN_IGNORED mTotalVolatileOrganicCompoundsConcentrationMeasurementInstance.SetUncertainty(0.0f);
    TEMPORARY_RETURN_IGNORED mTotalVolatileOrganicCompoundsConcentrationMeasurementInstance.SetLevelValue(LevelValueEnum::kLow);

    // Ozone
    TEMPORARY_RETURN_IGNORED mOzoneConcentrationMeasurementInstance.Init();
    TEMPORARY_RETURN_IGNORED mOzoneConcentrationMeasurementInstance.SetMinMeasuredValue(MakeNullable(0.0f));
    TEMPORARY_RETURN_IGNORED mOzoneConcentrationMeasurementInstance.SetMaxMeasuredValue(MakeNullable(1000.0f));
    TEMPORARY_RETURN_IGNORED mOzoneConcentrationMeasurementInstance.SetMeasuredValue(MakeNullable(2.0f));
    TEMPORARY_RETURN_IGNORED mOzoneConcentrationMeasurementInstance.SetPeakMeasuredValue(MakeNullable(1.0f));
    TEMPORARY_RETURN_IGNORED mOzoneConcentrationMeasurementInstance.SetPeakMeasuredValueWindow(320);
    TEMPORARY_RETURN_IGNORED mOzoneConcentrationMeasurementInstance.SetAverageMeasuredValue(MakeNullable(1.0f));
    TEMPORARY_RETURN_IGNORED mOzoneConcentrationMeasurementInstance.SetAverageMeasuredValueWindow(320);
    TEMPORARY_RETURN_IGNORED mOzoneConcentrationMeasurementInstance.SetUncertainty(0.0f);
    TEMPORARY_RETURN_IGNORED mOzoneConcentrationMeasurementInstance.SetLevelValue(LevelValueEnum::kLow);

    // Formaldehyde
    TEMPORARY_RETURN_IGNORED mFormaldehydeConcentrationMeasurementInstance.Init();
    TEMPORARY_RETURN_IGNORED mFormaldehydeConcentrationMeasurementInstance.SetMinMeasuredValue(MakeNullable(0.0f));
    TEMPORARY_RETURN_IGNORED mFormaldehydeConcentrationMeasurementInstance.SetMaxMeasuredValue(MakeNullable(1000.0f));
    TEMPORARY_RETURN_IGNORED mFormaldehydeConcentrationMeasurementInstance.SetMeasuredValue(MakeNullable(2.0f));
    TEMPORARY_RETURN_IGNORED mFormaldehydeConcentrationMeasurementInstance.SetPeakMeasuredValue(MakeNullable(1.0f));
    TEMPORARY_RETURN_IGNORED mFormaldehydeConcentrationMeasurementInstance.SetPeakMeasuredValueWindow(320);
    TEMPORARY_RETURN_IGNORED mFormaldehydeConcentrationMeasurementInstance.SetAverageMeasuredValue(MakeNullable(1.0f));
    TEMPORARY_RETURN_IGNORED mFormaldehydeConcentrationMeasurementInstance.SetAverageMeasuredValueWindow(320);
    TEMPORARY_RETURN_IGNORED mFormaldehydeConcentrationMeasurementInstance.SetUncertainty(0.0f);
    TEMPORARY_RETURN_IGNORED mFormaldehydeConcentrationMeasurementInstance.SetLevelValue(LevelValueEnum::kLow);
}

AirQualityEnum AirQualitySensorManager::GetAirQuality()
{
    return mAirQualityInstance.GetAirQuality();
}

void AirQualitySensorManager::OnAirQualityChangeHandler(AirQualityEnum newValue)
{
    mAirQualityInstance.UpdateAirQuality(newValue);
    ChipLogDetail(NotSpecified, "Updated AirQuality value: %huu", chip::to_underlying(newValue));
}

void AirQualitySensorManager::OnCarbonDioxideMeasurementChangeHandler(float newValue)
{
    TEMPORARY_RETURN_IGNORED mCarbonDioxideConcentrationMeasurementInstance.SetMeasuredValue(MakeNullable(newValue));
    ChipLogDetail(NotSpecified, "Updated Carbon Dioxide: %f", newValue);
}

void AirQualitySensorManager::OnCarbonMonoxideMeasurementChangeHandler(float newValue)
{
    TEMPORARY_RETURN_IGNORED mCarbonMonoxideConcentrationMeasurementInstance.SetMeasuredValue(MakeNullable(newValue));
    ChipLogDetail(NotSpecified, "Updated Carbon Monoxide value: %f", newValue);
}

void AirQualitySensorManager::OnNitrogenDioxideMeasurementChangeHandler(float newValue)
{
    TEMPORARY_RETURN_IGNORED mNitrogenDioxideConcentrationMeasurementInstance.SetMeasuredValue(MakeNullable(newValue));
    ChipLogDetail(NotSpecified, "Updated Nitrogen Dioxide value: %f", newValue);
}

void AirQualitySensorManager::OnPm1MeasurementChangeHandler(float newValue)
{
    TEMPORARY_RETURN_IGNORED mPm1ConcentrationMeasurementInstance.SetMeasuredValue(MakeNullable(newValue));
    ChipLogDetail(NotSpecified, "Updated PM1 value: %f", newValue);
}

void AirQualitySensorManager::OnPm10MeasurementChangeHandler(float newValue)
{
    TEMPORARY_RETURN_IGNORED mPm10ConcentrationMeasurementInstance.SetMeasuredValue(MakeNullable(newValue));
    ChipLogDetail(NotSpecified, "Updated PM10 value: %f", newValue);
}

void AirQualitySensorManager::OnPm25MeasurementChangeHandler(float newValue)
{
    TEMPORARY_RETURN_IGNORED mPm25ConcentrationMeasurementInstance.SetMeasuredValue(MakeNullable(newValue));
    ChipLogDetail(NotSpecified, "Updated PM2.5 value: %f", newValue);
}

void AirQualitySensorManager::OnRadonMeasurementChangeHandler(float newValue)
{
    TEMPORARY_RETURN_IGNORED mRadonConcentrationMeasurementInstance.SetMeasuredValue(MakeNullable(newValue));
    ChipLogDetail(NotSpecified, "Updated Radon value: %f", newValue);
}

void AirQualitySensorManager::OnTotalVolatileOrganicCompoundsMeasurementChangeHandler(float newValue)
{
    TEMPORARY_RETURN_IGNORED mTotalVolatileOrganicCompoundsConcentrationMeasurementInstance.SetMeasuredValue(
        MakeNullable(newValue));
    ChipLogDetail(NotSpecified, "Updated Total Volatile Organic Compounds value: %f", newValue);
}

void AirQualitySensorManager::OnOzoneMeasurementChangeHandler(float newValue)
{
    TEMPORARY_RETURN_IGNORED mOzoneConcentrationMeasurementInstance.SetMeasuredValue(MakeNullable(newValue));
    ChipLogDetail(NotSpecified, "Updated Ozone value: %f", newValue);
}

void AirQualitySensorManager::OnFormaldehydeMeasurementChangeHandler(float newValue)
{
    TEMPORARY_RETURN_IGNORED mFormaldehydeConcentrationMeasurementInstance.SetMeasuredValue(MakeNullable(newValue));
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
