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

#include <app/clusters/electrical-energy-measurement-server/CodegenIntegration.h>

#include "ElectricalEnergyMeasurementCluster.h"

#include <protocols/interaction_model/StatusCode.h>

#include <app/EventLogging.h>
#include <app/reporting/reporting.h>
#include <app/util/attribute-storage.h>
#include <app/util/config.h>
#include <zap-generated/gen_config.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ElectricalEnergyMeasurement {

using namespace chip;
using namespace chip::app::Clusters::ElectricalEnergyMeasurement::Attributes;
using namespace chip::app::Clusters::ElectricalEnergyMeasurement::Structs;

MeasurementData gMeasurements[MATTER_DM_ELECTRICAL_ENERGY_MEASUREMENT_CLUSTER_SERVER_ENDPOINT_COUNT +
                              CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT];

MeasurementData * MeasurementDataForEndpoint(EndpointId endpointId)
{
    auto index = emberAfGetClusterServerEndpointIndex(endpointId, app::Clusters::ElectricalEnergyMeasurement::Id,
                                                      MATTER_DM_ELECTRICAL_ENERGY_MEASUREMENT_CLUSTER_SERVER_ENDPOINT_COUNT);

    if (index == kEmberInvalidEndpointIndex)
    {
        return nullptr;
    }

    if (index >= MATTER_ARRAY_SIZE(gMeasurements))
    {
        ChipLogError(NotSpecified, "Internal error: invalid/unexpected energy measurement index.");
        return nullptr;
    }
    return &gMeasurements[index];
}

CHIP_ERROR SetMeasurementAccuracy(EndpointId endpointId, const MeasurementAccuracyStruct::Type & accuracy)
{

    MeasurementData * data = MeasurementDataForEndpoint(endpointId);
    VerifyOrReturnError(data != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    data->measurementAccuracy = accuracy;

    MatterReportingAttributeChangeCallback(endpointId, ElectricalEnergyMeasurement::Id, Accuracy::Id);

    return CHIP_NO_ERROR;
}

CHIP_ERROR SetCumulativeReset(EndpointId endpointId, const Optional<CumulativeEnergyResetStruct::Type> & cumulativeReset)
{

    MeasurementData * data = MeasurementDataForEndpoint(endpointId);
    VerifyOrReturnError(data != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    data->cumulativeReset = cumulativeReset;

    MatterReportingAttributeChangeCallback(endpointId, ElectricalEnergyMeasurement::Id, CumulativeEnergyReset::Id);

    return CHIP_NO_ERROR;
}

bool NotifyCumulativeEnergyMeasured(EndpointId endpointId, const Optional<EnergyMeasurementStruct::Type> & energyImported,
                                    const Optional<EnergyMeasurementStruct::Type> & energyExported)
{
    MeasurementData * data = MeasurementDataForEndpoint(endpointId);
    if (data != nullptr)
    {
        data->cumulativeImported = energyImported;
        data->cumulativeExported = energyExported;
    }

    Events::CumulativeEnergyMeasured::Type event;

    event.energyImported = energyImported;
    event.energyExported = energyExported;

    EventNumber eventNumber;
    CHIP_ERROR error = app::LogEvent(event, endpointId, eventNumber);

    if (CHIP_NO_ERROR != error)
    {
        ChipLogError(Zcl, "[NotifyCumulativeEnergyMeasured] Unable to send event: %" CHIP_ERROR_FORMAT " [endpointId=%d]",
                     error.Format(), endpointId);
        return false;
    }
    ChipLogProgress(Zcl, "[NotifyCumulativeEnergyMeasured] Sent event [endpointId=%d,eventNumber=%lu]", endpointId,
                    static_cast<unsigned long>(eventNumber));
    return true;
}

bool NotifyPeriodicEnergyMeasured(EndpointId endpointId, const Optional<EnergyMeasurementStruct::Type> & energyImported,
                                  const Optional<EnergyMeasurementStruct::Type> & energyExported)
{
    MeasurementData * data = MeasurementDataForEndpoint(endpointId);
    if (data != nullptr)
    {
        data->periodicImported = energyImported;
        data->periodicExported = energyExported;
    }

    Events::PeriodicEnergyMeasured::Type event;

    event.energyImported = energyImported;
    event.energyExported = energyExported;

    EventNumber eventNumber;
    CHIP_ERROR error = app::LogEvent(event, endpointId, eventNumber);

    if (CHIP_NO_ERROR != error)
    {
        ChipLogError(Zcl, "[NotifyPeriodicEnergyMeasured] Unable to send event: %" CHIP_ERROR_FORMAT " [endpointId=%d]",
                     error.Format(), endpointId);
        return false;
    }
    ChipLogProgress(Zcl, "[NotifyPeriodicEnergyMeasured] Sent event [endpointId=%d,eventNumber=%lu]", endpointId,
                    static_cast<unsigned long>(eventNumber));
    return true;
}

} // namespace ElectricalEnergyMeasurement
} // namespace Clusters
} // namespace app
} // namespace chip
