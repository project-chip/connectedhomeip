#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/clusters/air-quality-server/air-quality-server.h>
#include <app/clusters/concentration-measurement-server/concentration-measurement-server.h>

#include <relative-humidity-sensor-manager.h>
#include <temperature-sensor-manager.h>

#pragma once

namespace chip {
namespace app {
namespace Clusters {

class AirQualitySensorManager
{
public:
    // Delete copy constructor and assignment operator.
    AirQualitySensorManager(const AirQualitySensorManager &)             = delete;
    AirQualitySensorManager(const AirQualitySensorManager &&)            = delete;
    AirQualitySensorManager & operator=(const AirQualitySensorManager &) = delete;

    static void InitInstance(EndpointId aEndpointId = 1)
    {
        if (mInstance == nullptr)
        {
            mInstance = new AirQualitySensorManager(aEndpointId);
            mInstance->Init();
        }
    };

    /**
     * @brief Get an Air Quality Manager object - this class acts as a singleton device manager for the air quality device
     * @param[in] aEndpointId    Endpoint that the air quality is on
     * @return mInstance    The AirQualitySensorManager instance, note this this could be nullptr if InitInstance has not been
     * called
     */
    static AirQualitySensorManager * GetInstance() { return mInstance; };

    /**
     * @brief Handles changes in Carbon Dioxide concentration measurement.
     * @param[in] newValue The new air value to be applied.
     */
    void Init();

    /**
     * @brief Handles changes in Air Quality measurement.
     * @param[in] newValue The new air value to be applied.
     */
    void OnAirQualityChangeHandler(AirQuality::AirQualityEnum newValue);

    /**
     * @brief Handles changes in Carbon Dioxide concentration measurement.
     * @param[in] newValue The new air value to be applied.
     */
    void OnCarbonDioxideMeasurementChangeHandler(float newValue);

    /**
     * @brief Handles changes in Carbon Monoxide concentration measurement.
     * @param[in] newValue The new air value to be applied.
     */
    void OnCarbonMonoxideMeasurementChangeHandler(float newValue);

    /**
     * @brief Handles changes in Nitrogen Dioxide concentration measurement.
     * @param[in] newValue The new air value to be applied.
     */
    void OnNitrogenDioxideMeasurementChangeHandler(float newValue);

    /**
     * @brief Handles changes in PM1 concentration measurement.
     * @param[in] newValue The new air value to be applied.
     */
    void OnPm1MeasurementChangeHandler(float newValue);

    /**
     * @brief Handles changes in PM10 concentration measurement.
     * @param[in] newValue The new air value to be applied.
     */
    void OnPm10MeasurementChangeHandler(float newValue);

    /**
     * @brief Handles changes in PM2.5 concentration measurement.
     * @param[in] newValue The new air value to be applied.
     */
    void OnPm25MeasurementChangeHandler(float newValue);

    /**
     * @brief Handles changes in Radon concentration measurement.
     * @param[in] newValue The new air value to be applied.
     */
    void OnRadonMeasurementChangeHandler(float newValue);

    /**
     * @brief Handles changes in Total Volatile Organic Compounds concentration measurement.
     * @param[in] newValue The new air value to be applied.
     */
    void OnTotalVolatileOrganicCompoundsMeasurementChangeHandler(float newValue);

    /**
     * @brief Handles changes in Ozone concentration measurement.
     * @param[in] newValue The new air value to be applied.
     */
    void OnOzoneMeasurementChangeHandler(float newValue);

    /**
     * @brief Handles changes in Formaldehyde concentration measurement.
     * @param[in] newValue The new air value to be applied.
     */
    void OnFormaldehydeMeasurementChangeHandler(float newValue);

    /**
     * @brief Handles changes in Temperature measurement.
     * @param[in] newValue The new value to be applied.
     */
    void OnTemperatureMeasurementChangeHandler(int16_t newValue);

    /**
     * @brief Handles changes in Humidity measurement.
     * @param[in] newValue The new value to be applied.
     */
    void OnHumidityMeasurementChangeHandler(uint16_t newValue);

private:
    inline static AirQualitySensorManager * mInstance;
    EndpointId mEndpointId;
    AirQuality::Instance mAirQualityInstance;
    ConcentrationMeasurement::Instance<true, true, true, true, true, true> mCarbonDioxideConcentrationMeasurementInstance;
    ConcentrationMeasurement::Instance<true, true, true, true, true, true> mCarbonMonoxideConcentrationMeasurementInstance;
    ConcentrationMeasurement::Instance<true, true, true, true, true, true> mNitrogenDioxideConcentrationMeasurementInstance;
    ConcentrationMeasurement::Instance<true, true, true, true, true, true> mPm1ConcentrationMeasurementInstance;
    ConcentrationMeasurement::Instance<true, true, true, true, true, true> mPm10ConcentrationMeasurementInstance;
    ConcentrationMeasurement::Instance<true, true, true, true, true, true> mPm25ConcentrationMeasurementInstance;
    ConcentrationMeasurement::Instance<true, true, true, true, true, true> mRadonConcentrationMeasurementInstance;
    ConcentrationMeasurement::Instance<true, true, true, true, true, true>
        mTotalVolatileOrganicCompoundsConcentrationMeasurementInstance;
    ConcentrationMeasurement::Instance<true, true, true, true, true, true> mOzoneConcentrationMeasurementInstance;
    ConcentrationMeasurement::Instance<true, true, true, true, true, true> mFormaldehydeConcentrationMeasurementInstance;
    TemperatureSensorManager mTemperatureSensorManager;
    RelativeHumiditySensorManager mHumiditySensorManager;

    /**
     * @brief Construct a new Air Quality Manager object - this class acts as a singleton device manager for the air quality device
     * @param[in] endpointId    Endpoint that the air quality device is on
     */
    AirQualitySensorManager(EndpointId aEndpointId) :
        mEndpointId(aEndpointId),
        mAirQualityInstance(mEndpointId,
                            BitMask<AirQuality::Feature, uint32_t>(AirQuality::Feature::kModerate, AirQuality::Feature::kFair,
                                                                   AirQuality::Feature::kVeryPoor,
                                                                   AirQuality::Feature::kExtremelyPoor)),
        mCarbonDioxideConcentrationMeasurementInstance(mEndpointId, CarbonDioxideConcentrationMeasurement::Id,
                                                       ConcentrationMeasurement::MeasurementMediumEnum::kAir,
                                                       ConcentrationMeasurement::MeasurementUnitEnum::kPpm),
        mCarbonMonoxideConcentrationMeasurementInstance(mEndpointId, CarbonMonoxideConcentrationMeasurement::Id,
                                                        ConcentrationMeasurement::MeasurementMediumEnum::kAir,
                                                        ConcentrationMeasurement::MeasurementUnitEnum::kPpm),
        mNitrogenDioxideConcentrationMeasurementInstance(mEndpointId, NitrogenDioxideConcentrationMeasurement::Id,
                                                         ConcentrationMeasurement::MeasurementMediumEnum::kAir,
                                                         ConcentrationMeasurement::MeasurementUnitEnum::kPpm),
        mPm1ConcentrationMeasurementInstance(mEndpointId, Pm1ConcentrationMeasurement::Id,
                                             ConcentrationMeasurement::MeasurementMediumEnum::kAir,
                                             ConcentrationMeasurement::MeasurementUnitEnum::kPpm),
        mPm10ConcentrationMeasurementInstance(mEndpointId, Pm10ConcentrationMeasurement::Id,
                                              ConcentrationMeasurement::MeasurementMediumEnum::kAir,
                                              ConcentrationMeasurement::MeasurementUnitEnum::kPpm),
        mPm25ConcentrationMeasurementInstance(mEndpointId, Pm25ConcentrationMeasurement::Id,
                                              ConcentrationMeasurement::MeasurementMediumEnum::kAir,
                                              ConcentrationMeasurement::MeasurementUnitEnum::kPpm),
        mRadonConcentrationMeasurementInstance(mEndpointId, RadonConcentrationMeasurement::Id,
                                               ConcentrationMeasurement::MeasurementMediumEnum::kAir,
                                               ConcentrationMeasurement::MeasurementUnitEnum::kPpm),
        mTotalVolatileOrganicCompoundsConcentrationMeasurementInstance(
            mEndpointId, TotalVolatileOrganicCompoundsConcentrationMeasurement::Id,
            ConcentrationMeasurement::MeasurementMediumEnum::kAir, ConcentrationMeasurement::MeasurementUnitEnum::kPpm),
        mOzoneConcentrationMeasurementInstance(mEndpointId, OzoneConcentrationMeasurement::Id,
                                               ConcentrationMeasurement::MeasurementMediumEnum::kAir,
                                               ConcentrationMeasurement::MeasurementUnitEnum::kPpm),
        mFormaldehydeConcentrationMeasurementInstance(mEndpointId, FormaldehydeConcentrationMeasurement::Id,
                                                      ConcentrationMeasurement::MeasurementMediumEnum::kAir,
                                                      ConcentrationMeasurement::MeasurementUnitEnum::kPpm),
        mTemperatureSensorManager(mEndpointId), mHumiditySensorManager(mEndpointId){};
};

} // namespace Clusters
} // namespace app
} // namespace chip
