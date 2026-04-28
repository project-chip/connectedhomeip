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

#pragma once

#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/clusters/air-quality-server/CodegenIntegration.h>
#include <app/clusters/concentration-measurement-server/ConcentrationMeasurementCluster.h>
#include <app/server-cluster/ServerClusterInterfaceRegistry.h>

#include <relative-humidity-sensor-manager.h>
#include <temperature-sensor-manager.h>

namespace chip {
namespace app {
namespace Clusters {

class AirQualitySensorManager
{
public:
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
    }
    static AirQualitySensorManager * GetInstance() { return mInstance; }

    AirQuality::AirQualityEnum GetAirQuality();

    void OnAirQualityChangeHandler(AirQuality::AirQualityEnum newValue);
    void OnCarbonDioxideMeasurementChangeHandler(float newValue);
    void OnCarbonMonoxideMeasurementChangeHandler(float newValue);
    void OnNitrogenDioxideMeasurementChangeHandler(float newValue);
    void OnPm1MeasurementChangeHandler(float newValue);
    void OnPm10MeasurementChangeHandler(float newValue);
    void OnPm25MeasurementChangeHandler(float newValue);
    void OnRadonMeasurementChangeHandler(float newValue);
    void OnTotalVolatileOrganicCompoundsMeasurementChangeHandler(float newValue);
    void OnOzoneMeasurementChangeHandler(float newValue);
    void OnFormaldehydeMeasurementChangeHandler(float newValue);
    void OnTemperatureMeasurementChangeHandler(int16_t newValue);
    void OnHumidityMeasurementChangeHandler(uint16_t newValue);

    void Init();

private:
    static constexpr ConcentrationMeasurement::MeasurementMediumEnum kMedium =
        ConcentrationMeasurement::MeasurementMediumEnum::kAir;
    static constexpr ConcentrationMeasurement::MeasurementUnitEnum kUnit = ConcentrationMeasurement::MeasurementUnitEnum::kPpm;
    static constexpr chip::BitFlags<ConcentrationMeasurement::Feature> kAllFeatures{
        ConcentrationMeasurement::Feature::kNumericMeasurement, ConcentrationMeasurement::Feature::kLevelIndication,
        ConcentrationMeasurement::Feature::kMediumLevel,        ConcentrationMeasurement::Feature::kCriticalLevel,
        ConcentrationMeasurement::Feature::kPeakMeasurement,    ConcentrationMeasurement::Feature::kAverageMeasurement,
    };

    AirQualitySensorManager(EndpointId aEndpointId) :
        mEndpointId(aEndpointId),
        mAirQualityInstance(mEndpointId,
                            BitMask<AirQuality::Feature>(AirQuality::Feature::kModerate, AirQuality::Feature::kFair,
                                                         AirQuality::Feature::kVeryPoor, AirQuality::Feature::kExtremelyPoor)),
        mCarbonDioxideConcentrationMeasurementInstance(mEndpointId, CarbonDioxideConcentrationMeasurement::Id, kAllFeatures,
                                                       kMedium, kUnit, chip::app::DataModel::MakeNullable(0.0f),
                                                       chip::app::DataModel::MakeNullable(1000.0f)),
        mCarbonMonoxideConcentrationMeasurementInstance(mEndpointId, CarbonMonoxideConcentrationMeasurement::Id, kAllFeatures,
                                                        kMedium, kUnit, chip::app::DataModel::MakeNullable(0.0f),
                                                        chip::app::DataModel::MakeNullable(1000.0f)),
        mNitrogenDioxideConcentrationMeasurementInstance(mEndpointId, NitrogenDioxideConcentrationMeasurement::Id, kAllFeatures,
                                                         kMedium, kUnit, chip::app::DataModel::MakeNullable(0.0f),
                                                         chip::app::DataModel::MakeNullable(1000.0f)),
        mPm1ConcentrationMeasurementInstance(mEndpointId, Pm1ConcentrationMeasurement::Id, kAllFeatures, kMedium, kUnit,
                                             chip::app::DataModel::MakeNullable(0.0f), chip::app::DataModel::MakeNullable(1000.0f)),
        mPm10ConcentrationMeasurementInstance(mEndpointId, Pm10ConcentrationMeasurement::Id, kAllFeatures, kMedium, kUnit,
                                              chip::app::DataModel::MakeNullable(0.0f),
                                              chip::app::DataModel::MakeNullable(1000.0f)),
        mPm25ConcentrationMeasurementInstance(mEndpointId, Pm25ConcentrationMeasurement::Id, kAllFeatures, kMedium, kUnit,
                                              chip::app::DataModel::MakeNullable(0.0f),
                                              chip::app::DataModel::MakeNullable(1000.0f)),
        mRadonConcentrationMeasurementInstance(mEndpointId, RadonConcentrationMeasurement::Id, kAllFeatures, kMedium, kUnit,
                                               chip::app::DataModel::MakeNullable(0.0f),
                                               chip::app::DataModel::MakeNullable(1000.0f)),
        mTotalVolatileOrganicCompoundsConcentrationMeasurementInstance(
            mEndpointId, TotalVolatileOrganicCompoundsConcentrationMeasurement::Id, kAllFeatures, kMedium, kUnit,
            chip::app::DataModel::MakeNullable(0.0f), chip::app::DataModel::MakeNullable(1000.0f)),
        mOzoneConcentrationMeasurementInstance(mEndpointId, OzoneConcentrationMeasurement::Id, kAllFeatures, kMedium, kUnit,
                                               chip::app::DataModel::MakeNullable(0.0f),
                                               chip::app::DataModel::MakeNullable(1000.0f)),
        mFormaldehydeConcentrationMeasurementInstance(mEndpointId, FormaldehydeConcentrationMeasurement::Id, kAllFeatures, kMedium,
                                                      kUnit, chip::app::DataModel::MakeNullable(0.0f),
                                                      chip::app::DataModel::MakeNullable(1000.0f)),
        mCarbonDioxideRegistration(mCarbonDioxideConcentrationMeasurementInstance),
        mCarbonMonoxideRegistration(mCarbonMonoxideConcentrationMeasurementInstance),
        mNitrogenDioxideRegistration(mNitrogenDioxideConcentrationMeasurementInstance),
        mPm1Registration(mPm1ConcentrationMeasurementInstance), mPm10Registration(mPm10ConcentrationMeasurementInstance),
        mPm25Registration(mPm25ConcentrationMeasurementInstance), mRadonRegistration(mRadonConcentrationMeasurementInstance),
        mTotalVolatileOrganicCompoundsRegistration(mTotalVolatileOrganicCompoundsConcentrationMeasurementInstance),
        mOzoneRegistration(mOzoneConcentrationMeasurementInstance),
        mFormaldehydeRegistration(mFormaldehydeConcentrationMeasurementInstance), mTemperatureSensorManager(mEndpointId),
        mHumiditySensorManager(mEndpointId)
    {}

    EndpointId mEndpointId;
    AirQuality::Instance mAirQualityInstance;
    inline static AirQualitySensorManager * mInstance = nullptr;

    ConcentrationMeasurement::ConcentrationMeasurementCluster mCarbonDioxideConcentrationMeasurementInstance;
    ConcentrationMeasurement::ConcentrationMeasurementCluster mCarbonMonoxideConcentrationMeasurementInstance;
    ConcentrationMeasurement::ConcentrationMeasurementCluster mNitrogenDioxideConcentrationMeasurementInstance;
    ConcentrationMeasurement::ConcentrationMeasurementCluster mPm1ConcentrationMeasurementInstance;
    ConcentrationMeasurement::ConcentrationMeasurementCluster mPm10ConcentrationMeasurementInstance;
    ConcentrationMeasurement::ConcentrationMeasurementCluster mPm25ConcentrationMeasurementInstance;
    ConcentrationMeasurement::ConcentrationMeasurementCluster mRadonConcentrationMeasurementInstance;
    ConcentrationMeasurement::ConcentrationMeasurementCluster mTotalVolatileOrganicCompoundsConcentrationMeasurementInstance;
    ConcentrationMeasurement::ConcentrationMeasurementCluster mOzoneConcentrationMeasurementInstance;
    ConcentrationMeasurement::ConcentrationMeasurementCluster mFormaldehydeConcentrationMeasurementInstance;

    chip::app::ServerClusterRegistration mCarbonDioxideRegistration;
    chip::app::ServerClusterRegistration mCarbonMonoxideRegistration;
    chip::app::ServerClusterRegistration mNitrogenDioxideRegistration;
    chip::app::ServerClusterRegistration mPm1Registration;
    chip::app::ServerClusterRegistration mPm10Registration;
    chip::app::ServerClusterRegistration mPm25Registration;
    chip::app::ServerClusterRegistration mRadonRegistration;
    chip::app::ServerClusterRegistration mTotalVolatileOrganicCompoundsRegistration;
    chip::app::ServerClusterRegistration mOzoneRegistration;
    chip::app::ServerClusterRegistration mFormaldehydeRegistration;

    TemperatureSensorManager mTemperatureSensorManager;
    RelativeHumiditySensorManager mHumiditySensorManager;
};

} // namespace Clusters
} // namespace app
} // namespace chip
