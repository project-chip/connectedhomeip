#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/clusters/concentration-measurement-server/concentration-measurement-server.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::DataModel;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ConcentrationMeasurement;

class AirQualitySensorManager
{
public:
    AirQualitySensorManager();

private:
    static const EndpointId mEndpointId = 2;

    Instance carbonDioxideConcentrationMeasurementInstance =
        CreateNumericMeasurementAndLevelIndicationConcentrationCluster<true, true, true, true>(
            mEndpointId, CarbonDioxideConcentrationMeasurement::Id, MeasurementMediumEnum::kAir, MeasurementUnitEnum::kPpm);

    Instance carbonMonoxideConcentrationMeasurementInstance =
        CreateNumericMeasurementAndLevelIndicationConcentrationCluster<true, true, true, true>(
            mEndpointId, CarbonMonoxideConcentrationMeasurement::Id, MeasurementMediumEnum::kAir, MeasurementUnitEnum::kPpm);

    Instance nitrogenDioxideConcentrationMeasurementInstance =
        CreateNumericMeasurementAndLevelIndicationConcentrationCluster<true, true, true, true>(
            mEndpointId, NitrogenDioxideConcentrationMeasurement::Id, MeasurementMediumEnum::kAir, MeasurementUnitEnum::kPpm);

    Instance pm1ConcentrationMeasurementInstance =
        CreateNumericMeasurementAndLevelIndicationConcentrationCluster<true, true, true, true>(
            mEndpointId, Pm1ConcentrationMeasurement::Id, MeasurementMediumEnum::kAir, MeasurementUnitEnum::kPpm);

    Instance pm10ConcentrationMeasurementInstance =
        CreateNumericMeasurementAndLevelIndicationConcentrationCluster<true, true, true, true>(
            mEndpointId, Pm10ConcentrationMeasurement::Id, MeasurementMediumEnum::kAir, MeasurementUnitEnum::kPpm);

    Instance pm25ConcentrationMeasurementInstance =
        CreateNumericMeasurementAndLevelIndicationConcentrationCluster<true, true, true, true>(
            mEndpointId, Pm25ConcentrationMeasurement::Id, MeasurementMediumEnum::kAir, MeasurementUnitEnum::kPpm);

    Instance radonConcentrationMeasurementInstance =
        CreateNumericMeasurementAndLevelIndicationConcentrationCluster<true, true, true, true>(
            mEndpointId, RadonConcentrationMeasurement::Id, MeasurementMediumEnum::kAir, MeasurementUnitEnum::kPpm);

    Instance totalVolatileOrganicCompoundsConcentrationMeasurementInstance =
        CreateNumericMeasurementAndLevelIndicationConcentrationCluster<true, true, true, true>(
            mEndpointId, TotalVolatileOrganicCompoundsConcentrationMeasurement::Id, MeasurementMediumEnum::kAir,
            MeasurementUnitEnum::kPpm);

    Instance ozoneConcentrationMeasurementInstance =
        CreateNumericMeasurementAndLevelIndicationConcentrationCluster<true, true, true, true>(
            mEndpointId, OzoneConcentrationMeasurement::Id, MeasurementMediumEnum::kAir, MeasurementUnitEnum::kPpm);

    Instance formaldehydeConcentrationMeasurementInstance =
        CreateNumericMeasurementAndLevelIndicationConcentrationCluster<true, true, true, true>(
            mEndpointId, FormaldehydeConcentrationMeasurement::Id, MeasurementMediumEnum::kAir, MeasurementUnitEnum::kPpm);

    void InitializeConcentrations();
};