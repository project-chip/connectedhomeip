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
#include <app/clusters/concentration-measurement-server/ConcentrationMeasurementDelegate.h>
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
    AirQualitySensorManager(EndpointId aEndpointId) :
        mEndpointId(aEndpointId),
        mAirQualityInstance(mEndpointId,
                            BitMask<AirQuality::Feature>(AirQuality::Feature::kModerate, AirQuality::Feature::kFair,
                                                         AirQuality::Feature::kVeryPoor, AirQuality::Feature::kExtremelyPoor)),
        // Delegates — fixed sensor hardware (kAir, kPpm)
        mCarbonDioxideDelegate(ConcentrationMeasurement::MeasurementMediumEnum::kAir,
                               ConcentrationMeasurement::MeasurementUnitEnum::kPpm, chip::app::DataModel::MakeNullable(0.0f),
                               chip::app::DataModel::MakeNullable(1000.0f), 0.0f),
        mCarbonMonoxideDelegate(ConcentrationMeasurement::MeasurementMediumEnum::kAir,
                                ConcentrationMeasurement::MeasurementUnitEnum::kPpm, chip::app::DataModel::MakeNullable(0.0f),
                                chip::app::DataModel::MakeNullable(1000.0f), 0.0f),
        mNitrogenDioxideDelegate(ConcentrationMeasurement::MeasurementMediumEnum::kAir,
                                 ConcentrationMeasurement::MeasurementUnitEnum::kPpm, chip::app::DataModel::MakeNullable(0.0f),
                                 chip::app::DataModel::MakeNullable(1000.0f), 0.0f),
        mPm1Delegate(ConcentrationMeasurement::MeasurementMediumEnum::kAir, ConcentrationMeasurement::MeasurementUnitEnum::kPpm,
                     chip::app::DataModel::MakeNullable(0.0f), chip::app::DataModel::MakeNullable(1000.0f), 0.0f),
        mPm10Delegate(ConcentrationMeasurement::MeasurementMediumEnum::kAir, ConcentrationMeasurement::MeasurementUnitEnum::kPpm,
                      chip::app::DataModel::MakeNullable(0.0f), chip::app::DataModel::MakeNullable(1000.0f), 0.0f),
        mPm25Delegate(ConcentrationMeasurement::MeasurementMediumEnum::kAir, ConcentrationMeasurement::MeasurementUnitEnum::kPpm,
                      chip::app::DataModel::MakeNullable(0.0f), chip::app::DataModel::MakeNullable(1000.0f), 0.0f),
        mRadonDelegate(ConcentrationMeasurement::MeasurementMediumEnum::kAir, ConcentrationMeasurement::MeasurementUnitEnum::kPpm,
                       chip::app::DataModel::MakeNullable(0.0f), chip::app::DataModel::MakeNullable(1000.0f), 0.0f),
        mTotalVolatileOrganicCompoundsDelegate(
            ConcentrationMeasurement::MeasurementMediumEnum::kAir, ConcentrationMeasurement::MeasurementUnitEnum::kPpm,
            chip::app::DataModel::MakeNullable(0.0f), chip::app::DataModel::MakeNullable(1000.0f), 0.0f),
        mOzoneDelegate(ConcentrationMeasurement::MeasurementMediumEnum::kAir, ConcentrationMeasurement::MeasurementUnitEnum::kPpm,
                       chip::app::DataModel::MakeNullable(0.0f), chip::app::DataModel::MakeNullable(1000.0f), 0.0f),
        mFormaldehydeDelegate(ConcentrationMeasurement::MeasurementMediumEnum::kAir,
                              ConcentrationMeasurement::MeasurementUnitEnum::kPpm, chip::app::DataModel::MakeNullable(0.0f),
                              chip::app::DataModel::MakeNullable(1000.0f), 0.0f),
        // Clusters
        mCarbonDioxideConcentrationMeasurementInstance(mEndpointId, CarbonDioxideConcentrationMeasurement::Id, kAllFeatures,
                                                       mCarbonDioxideDelegate),
        mCarbonMonoxideConcentrationMeasurementInstance(mEndpointId, CarbonMonoxideConcentrationMeasurement::Id, kAllFeatures,
                                                        mCarbonMonoxideDelegate),
        mNitrogenDioxideConcentrationMeasurementInstance(mEndpointId, NitrogenDioxideConcentrationMeasurement::Id, kAllFeatures,
                                                         mNitrogenDioxideDelegate),
        mPm1ConcentrationMeasurementInstance(mEndpointId, Pm1ConcentrationMeasurement::Id, kAllFeatures, mPm1Delegate),
        mPm10ConcentrationMeasurementInstance(mEndpointId, Pm10ConcentrationMeasurement::Id, kAllFeatures, mPm10Delegate),
        mPm25ConcentrationMeasurementInstance(mEndpointId, Pm25ConcentrationMeasurement::Id, kAllFeatures, mPm25Delegate),
        mRadonConcentrationMeasurementInstance(mEndpointId, RadonConcentrationMeasurement::Id, kAllFeatures, mRadonDelegate),
        mTotalVolatileOrganicCompoundsConcentrationMeasurementInstance(mEndpointId,
                                                                       TotalVolatileOrganicCompoundsConcentrationMeasurement::Id,
                                                                       kAllFeatures, mTotalVolatileOrganicCompoundsDelegate),
        mOzoneConcentrationMeasurementInstance(mEndpointId, OzoneConcentrationMeasurement::Id, kAllFeatures, mOzoneDelegate),
        mFormaldehydeConcentrationMeasurementInstance(mEndpointId, FormaldehydeConcentrationMeasurement::Id, kAllFeatures,
                                                      mFormaldehydeDelegate),
        // Registrations
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

    static constexpr BitFlags<ConcentrationMeasurement::Feature> kAllFeatures{
        ConcentrationMeasurement::Feature::kNumericMeasurement, ConcentrationMeasurement::Feature::kLevelIndication,
        ConcentrationMeasurement::Feature::kMediumLevel,        ConcentrationMeasurement::Feature::kCriticalLevel,
        ConcentrationMeasurement::Feature::kPeakMeasurement,    ConcentrationMeasurement::Feature::kAverageMeasurement,
    };

    EndpointId mEndpointId;
    AirQuality::Instance mAirQualityInstance;
    inline static AirQualitySensorManager * mInstance = nullptr;

    // Delegates declared before clusters — clusters hold a Delegate reference
    ConcentrationMeasurement::DefaultDelegate mCarbonDioxideDelegate;
    ConcentrationMeasurement::DefaultDelegate mCarbonMonoxideDelegate;
    ConcentrationMeasurement::DefaultDelegate mNitrogenDioxideDelegate;
    ConcentrationMeasurement::DefaultDelegate mPm1Delegate;
    ConcentrationMeasurement::DefaultDelegate mPm10Delegate;
    ConcentrationMeasurement::DefaultDelegate mPm25Delegate;
    ConcentrationMeasurement::DefaultDelegate mRadonDelegate;
    ConcentrationMeasurement::DefaultDelegate mTotalVolatileOrganicCompoundsDelegate;
    ConcentrationMeasurement::DefaultDelegate mOzoneDelegate;
    ConcentrationMeasurement::DefaultDelegate mFormaldehydeDelegate;

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

    // Registrations declared after clusters — hold a pointer to the cluster
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
