/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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
#include <app-common/zap-generated/callback.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/util/config.h>
#include <lib/core/DataModelTypes.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;

#if defined(EMBER_AF_PLUGIN_CARBON_MONOXIDE_CONCENTRATION_MEASUREMENT_SERVER) ||                                                   \
    defined(EMBER_AF_PLUGIN_CARBON_DIOXIDE_CONCENTRATION_MEASUREMENT_SERVER) ||                                                    \
    defined(EMBER_AF_PLUGIN_NITROGEN_DIOXIDE_CONCENTRATION_MEASUREMENT_SERVER) ||                                                  \
    defined(EMBER_AF_PLUGIN_OZONE_CONCENTRATION_MEASUREMENT_SERVER) ||                                                             \
    defined(EMBER_AF_PLUGIN_PM2__5_CONCENTRATION_MEASUREMENT_SERVER) ||                                                            \
    defined(EMBER_AF_PLUGIN_FORMALDEHYDE_CONCENTRATION_MEASUREMENT_SERVER) ||                                                      \
    defined(EMBER_AF_PLUGIN_PM1_CONCENTRATION_MEASUREMENT_SERVER) ||                                                               \
    defined(EMBER_AF_PLUGIN_PM10_CONCENTRATION_MEASUREMENT_SERVER) ||                                                              \
    defined(EMBER_AF_PLUGIN_TOTAL_VOLATILE_ORGANIC_COMPOUNDS_CONCENTRATION_MEASUREMENT_SERVER) ||                                  \
    defined(EMBER_AF_PLUGIN_RADON_CONCENTRATION_MEASUREMENT_SERVER)
#include <app/clusters/concentration-measurement-server/concentration-measurement-server.h>
using namespace chip::app::Clusters::ConcentrationMeasurement;
#endif

#ifdef EMBER_AF_PLUGIN_CARBON_DIOXIDE_CONCENTRATION_MEASUREMENT_SERVER
static Instance gCarbonDioxideConcentrationMeasurementInstance =
    CreateNumericMeasurementAndLevelIndicationConcentrationCluster<true, true, true, true>(
        EndpointId(1), CarbonDioxideConcentrationMeasurement::Id, MeasurementMediumEnum::kAir, MeasurementUnitEnum::kPpm);

void emberAfCarbonDioxideConcentrationMeasurementClusterInitCallback(EndpointId endpoint)
{
    VerifyOrDie(1 == endpoint);
    gCarbonDioxideConcentrationMeasurementInstance.Init();
}
#endif

#ifdef EMBER_AF_PLUGIN_CARBON_MONOXIDE_CONCENTRATION_MEASUREMENT_SERVER
static Instance gCarbonMonoxideConcentrationMeasurementInstance =
    CreateNumericMeasurementAndLevelIndicationConcentrationCluster<true, true, true, true>(
        EndpointId(1), CarbonMonoxideConcentrationMeasurement::Id, MeasurementMediumEnum::kAir, MeasurementUnitEnum::kPpm);

void emberAfCarbonMonoxideConcentrationMeasurementClusterInitCallback(EndpointId endpoint)
{
    VerifyOrDie(1 == endpoint);
    gCarbonMonoxideConcentrationMeasurementInstance.Init();
}
#endif

#ifdef EMBER_AF_PLUGIN_NITROGEN_DIOXIDE_CONCENTRATION_MEASUREMENT_SERVER
static Instance gNitrogenDioxideConcentrationMeasurementInstance =
    CreateNumericMeasurementAndLevelIndicationConcentrationCluster<true, true, true, true>(
        EndpointId(1), NitrogenDioxideConcentrationMeasurement::Id, MeasurementMediumEnum::kAir, MeasurementUnitEnum::kPpm);

void emberAfNitrogenDioxideConcentrationMeasurementClusterInitCallback(EndpointId endpoint)
{
    VerifyOrDie(1 == endpoint);
    gNitrogenDioxideConcentrationMeasurementInstance.Init();
}
#endif

#ifdef EMBER_AF_PLUGIN_PM1_CONCENTRATION_MEASUREMENT_SERVER
static Instance gPm1ConcentrationMeasurementInstance =
    CreateNumericMeasurementAndLevelIndicationConcentrationCluster<true, true, true, true>(
        EndpointId(1), Pm1ConcentrationMeasurement::Id, MeasurementMediumEnum::kAir, MeasurementUnitEnum::kPpm);

void emberAfPm1ConcentrationMeasurementClusterInitCallback(EndpointId endpoint)
{
    VerifyOrDie(1 == endpoint);
    gPm1ConcentrationMeasurementInstance.Init();
}
#endif

#ifdef EMBER_AF_PLUGIN_PM10_CONCENTRATION_MEASUREMENT_SERVER
static Instance gPm10ConcentrationMeasurementInstance =
    CreateNumericMeasurementAndLevelIndicationConcentrationCluster<true, true, true, true>(
        EndpointId(1), Pm10ConcentrationMeasurement::Id, MeasurementMediumEnum::kAir, MeasurementUnitEnum::kPpm);

void emberAfPm10ConcentrationMeasurementClusterInitCallback(EndpointId endpoint)
{
    VerifyOrDie(1 == endpoint);
    gPm10ConcentrationMeasurementInstance.Init();
}
#endif

#ifdef EMBER_AF_PLUGIN_PM2__5_CONCENTRATION_MEASUREMENT_SERVER
static Instance gPm25ConcentrationMeasurementInstance =
    CreateNumericMeasurementAndLevelIndicationConcentrationCluster<true, true, true, true>(
        EndpointId(1), Pm25ConcentrationMeasurement::Id, MeasurementMediumEnum::kAir, MeasurementUnitEnum::kPpm);

void emberAfPm25ConcentrationMeasurementClusterInitCallback(EndpointId endpoint)
{
    VerifyOrDie(1 == endpoint);
    gPm25ConcentrationMeasurementInstance.Init();
}
#endif

#ifdef EMBER_AF_PLUGIN_RADON_CONCENTRATION_MEASUREMENT_SERVER
static Instance gRadonConcentrationMeasurementInstance =
    CreateNumericMeasurementAndLevelIndicationConcentrationCluster<true, true, true, true>(
        EndpointId(1), RadonConcentrationMeasurement::Id, MeasurementMediumEnum::kAir, MeasurementUnitEnum::kPpm);

void emberAfRadonConcentrationMeasurementClusterInitCallback(EndpointId endpoint)
{
    VerifyOrDie(1 == endpoint);
    gRadonConcentrationMeasurementInstance.Init();
}
#endif

#ifdef EMBER_AF_PLUGIN_TOTAL_VOLATILE_ORGANIC_COMPOUNDS_CONCENTRATION_MEASUREMENT_SERVER
static Instance gTotalVolatileOrganicCompoundsConcentrationMeasurementInstance =
    CreateNumericMeasurementAndLevelIndicationConcentrationCluster<true, true, true, true>(
        EndpointId(1), TotalVolatileOrganicCompoundsConcentrationMeasurement::Id, MeasurementMediumEnum::kAir,
        MeasurementUnitEnum::kPpm);

void emberAfTotalVolatileOrganicCompoundsConcentrationMeasurementClusterInitCallback(EndpointId endpoint)
{
    VerifyOrDie(1 == endpoint);
    gTotalVolatileOrganicCompoundsConcentrationMeasurementInstance.Init();
}
#endif

#ifdef EMBER_AF_PLUGIN_OZONE_CONCENTRATION_MEASUREMENT_SERVER
static Instance gOzoneConcentrationMeasurementInstance =
    CreateNumericMeasurementAndLevelIndicationConcentrationCluster<true, true, true, true>(
        EndpointId(1), OzoneConcentrationMeasurement::Id, MeasurementMediumEnum::kAir, MeasurementUnitEnum::kPpm);

void emberAfOzoneConcentrationMeasurementClusterInitCallback(EndpointId endpoint)
{
    VerifyOrDie(1 == endpoint);
    gOzoneConcentrationMeasurementInstance.Init();
}
#endif

#ifdef EMBER_AF_PLUGIN_FORMALDEHYDE_CONCENTRATION_MEASUREMENT_SERVER
static Instance gFormaldehydeConcentrationMeasurementInstance =
    CreateNumericMeasurementAndLevelIndicationConcentrationCluster<true, true, true, true>(
        EndpointId(1), FormaldehydeConcentrationMeasurement::Id, MeasurementMediumEnum::kAir, MeasurementUnitEnum::kPpm);

void emberAfFormaldehydeConcentrationMeasurementClusterInitCallback(EndpointId endpoint)
{
    VerifyOrDie(1 == endpoint);
    gFormaldehydeConcentrationMeasurementInstance.Init();
}
#endif
