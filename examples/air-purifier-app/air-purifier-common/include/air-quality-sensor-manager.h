#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/clusters/air-quality-server/air-quality-server.h>
#include <app/clusters/concentration-measurement-server/concentration-measurement-server.h>

#pragma once

namespace chip {
namespace app {
namespace Clusters {

class AirQualitySensorManager
{
public:
    AirQualitySensorManager(EndpointId aEndpointId) :
        mEndpointId(aEndpointId),
        airQualityInstance(mEndpointId,
                           BitMask<AirQuality::Feature, uint32_t>(AirQuality::Feature::kModerate, AirQuality::Feature::kFair,
                                                                  AirQuality::Feature::kVeryPoor,
                                                                  AirQuality::Feature::kExtremelyPoor)),
        carbonDioxideConcentrationMeasurementInstance(mEndpointId, CarbonDioxideConcentrationMeasurement::Id,
                                                      ConcentrationMeasurement::MeasurementMediumEnum::kAir,
                                                      ConcentrationMeasurement::MeasurementUnitEnum::kPpm),
        carbonMonoxideConcentrationMeasurementInstance(mEndpointId, CarbonMonoxideConcentrationMeasurement::Id,
                                                       ConcentrationMeasurement::MeasurementMediumEnum::kAir,
                                                       ConcentrationMeasurement::MeasurementUnitEnum::kPpm),
        nitrogenDioxideConcentrationMeasurementInstance(mEndpointId, NitrogenDioxideConcentrationMeasurement::Id,
                                                        ConcentrationMeasurement::MeasurementMediumEnum::kAir,
                                                        ConcentrationMeasurement::MeasurementUnitEnum::kPpm),
        pm1ConcentrationMeasurementInstance(mEndpointId, Pm1ConcentrationMeasurement::Id,
                                            ConcentrationMeasurement::MeasurementMediumEnum::kAir,
                                            ConcentrationMeasurement::MeasurementUnitEnum::kPpm),
        pm10ConcentrationMeasurementInstance(mEndpointId, Pm10ConcentrationMeasurement::Id,
                                             ConcentrationMeasurement::MeasurementMediumEnum::kAir,
                                             ConcentrationMeasurement::MeasurementUnitEnum::kPpm),
        pm25ConcentrationMeasurementInstance(mEndpointId, Pm25ConcentrationMeasurement::Id,
                                             ConcentrationMeasurement::MeasurementMediumEnum::kAir,
                                             ConcentrationMeasurement::MeasurementUnitEnum::kPpm),
        radonConcentrationMeasurementInstance(mEndpointId, RadonConcentrationMeasurement::Id,
                                              ConcentrationMeasurement::MeasurementMediumEnum::kAir,
                                              ConcentrationMeasurement::MeasurementUnitEnum::kPpm),
        totalVolatileOrganicCompoundsConcentrationMeasurementInstance(
            mEndpointId, TotalVolatileOrganicCompoundsConcentrationMeasurement::Id,
            ConcentrationMeasurement::MeasurementMediumEnum::kAir, ConcentrationMeasurement::MeasurementUnitEnum::kPpm),
        ozoneConcentrationMeasurementInstance(mEndpointId, OzoneConcentrationMeasurement::Id,
                                              ConcentrationMeasurement::MeasurementMediumEnum::kAir,
                                              ConcentrationMeasurement::MeasurementUnitEnum::kPpm),
        formaldehydeConcentrationMeasurementInstance(mEndpointId, FormaldehydeConcentrationMeasurement::Id,
                                                     ConcentrationMeasurement::MeasurementMediumEnum::kAir,
                                                     ConcentrationMeasurement::MeasurementUnitEnum::kPpm){};

    void Init();

private:
    EndpointId mEndpointId;
    AirQuality::Instance airQualityInstance;
    ConcentrationMeasurement::Instance<true, true, true, true, true, true> carbonDioxideConcentrationMeasurementInstance;
    ConcentrationMeasurement::Instance<true, true, true, true, true, true> carbonMonoxideConcentrationMeasurementInstance;
    ConcentrationMeasurement::Instance<true, true, true, true, true, true> nitrogenDioxideConcentrationMeasurementInstance;
    ConcentrationMeasurement::Instance<true, true, true, true, true, true> pm1ConcentrationMeasurementInstance;
    ConcentrationMeasurement::Instance<true, true, true, true, true, true> pm10ConcentrationMeasurementInstance;
    ConcentrationMeasurement::Instance<true, true, true, true, true, true> pm25ConcentrationMeasurementInstance;
    ConcentrationMeasurement::Instance<true, true, true, true, true, true> radonConcentrationMeasurementInstance;
    ConcentrationMeasurement::Instance<true, true, true, true, true, true>
        totalVolatileOrganicCompoundsConcentrationMeasurementInstance;
    ConcentrationMeasurement::Instance<true, true, true, true, true, true> ozoneConcentrationMeasurementInstance;
    ConcentrationMeasurement::Instance<true, true, true, true, true, true> formaldehydeConcentrationMeasurementInstance;

    // TODO: Add public OnChange handlers for the AQ and Concentrations - maybe not required?
};

} // namespace Clusters
} // namespace app
} // namespace chip