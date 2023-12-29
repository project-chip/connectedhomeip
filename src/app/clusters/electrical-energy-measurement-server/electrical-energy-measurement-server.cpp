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
#include "electrical-energy-measurement-server.h"

#include <protocols/interaction_model/StatusCode.h>

#include <app/AttributeAccessInterface.h>
#include <app/EventLogging.h>
#include <app/reporting/reporting.h>
#include <app/util/attribute-storage.h>

using chip::Protocols::InteractionModel::Status;

namespace {

using namespace chip;
using namespace chip::app::Clusters::ElectricalEnergyMeasurement;
using namespace chip::app::Clusters::ElectricalEnergyMeasurement::Attributes;
using namespace chip::app::Clusters::ElectricalEnergyMeasurement::Structs;

struct MeasurementData
{
    MeasurementAccuracyStruct::Type measurementAccuracy;
    Optional<EnergyMeasurementStruct::Type> cumulativeImported;
    Optional<EnergyMeasurementStruct::Type> cumulativeExported;
    Optional<EnergyMeasurementStruct::Type> periodicImported;
    Optional<EnergyMeasurementStruct::Type> periodicExported;
};

MeasurementData
    gMeasurements[EMBER_AF_ELECTRICAL_ENERGY_MEASUREMENT_CLUSTER_SERVER_ENDPOINT_COUNT + CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT];

MeasurementData * MeasurementDataForEndpoint(EndpointId endpointId)
{
    auto index = emberAfGetClusterServerEndpointIndex(endpointId, app::Clusters::ElectricalEnergyMeasurement::Id,
                                                      EMBER_AF_ELECTRICAL_ENERGY_MEASUREMENT_CLUSTER_SERVER_ENDPOINT_COUNT);

    if (index == kEmberInvalidEndpointIndex)
    {
        return nullptr;
    }

    if (index >= EMBER_AF_ELECTRICAL_ENERGY_MEASUREMENT_CLUSTER_SERVER_ENDPOINT_COUNT)
    {
        ChipLogError(NotSpecified, "Internal error: invalid/unexpected energy measurement index.");
        return nullptr;
    }
    return &gMeasurements[index];
}

class ElectricalEnergyMeasurementAttrAccess : public app::AttributeAccessInterface
{
public:
    ElectricalEnergyMeasurementAttrAccess() :
        app::AttributeAccessInterface(Optional<EndpointId>::Missing(), app::Clusters::ElectricalEnergyMeasurement::Id)
    {}

    CHIP_ERROR Read(const app::ConcreteReadAttributePath & aPath, app::AttributeValueEncoder & aEncoder) override;
};

CHIP_ERROR ElectricalEnergyMeasurementAttrAccess::Read(const app::ConcreteReadAttributePath & aPath,
                                                       app::AttributeValueEncoder & aEncoder)
{
    VerifyOrDie(aPath.mClusterId == app::Clusters::ElectricalEnergyMeasurement::Id);

    MeasurementData * data = MeasurementDataForEndpoint(aPath.mEndpointId);

    switch (aPath.mAttributeId)
    {
    case Accuracy::Id:
        if (data == nullptr)
        {
            return CHIP_ERROR_NOT_FOUND;
        }
        return aEncoder.Encode(data->measurementAccuracy);
    case CumulativeEnergyImported::Id:
        if ((data == nullptr) || !data->cumulativeImported.HasValue())
        {
            return aEncoder.EncodeNull();
        }
        return aEncoder.Encode(data->cumulativeImported.Value());
    case CumulativeEnergyExported::Id:
        if ((data == nullptr) || !data->cumulativeExported.HasValue())
        {
            return aEncoder.EncodeNull();
        }
        return aEncoder.Encode(data->cumulativeExported.Value());
    case PeriodicEnergyImported::Id:
        if ((data == nullptr) || !data->periodicImported.HasValue())
        {
            return aEncoder.EncodeNull();
        }
        return aEncoder.Encode(data->periodicImported.Value());
    case PeriodicEnergyExported::Id:
        if ((data == nullptr) || !data->periodicExported.HasValue())
        {
            return aEncoder.EncodeNull();
        }
        return aEncoder.Encode(data->periodicExported.Value());
    }

    return CHIP_NO_ERROR;
}

ElectricalEnergyMeasurementAttrAccess gAttrAccess;

} // namespace

namespace chip {
namespace app {
namespace Clusters {
namespace ElectricalEnergyMeasurement {

CHIP_ERROR SetMeasurementAccuracy(EndpointId endpointId, const MeasurementAccuracyStruct::Type & accuracy)
{

    MeasurementData * data = MeasurementDataForEndpoint(endpointId);
    VerifyOrReturnError(data != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    data->measurementAccuracy = accuracy;

    MatterReportingAttributeChangeCallback(endpointId, ElectricalEnergyMeasurement::Id, Accuracy::Id);

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

void MatterElectricalEnergyMeasurementPluginServerInitCallback()
{
    registerAttributeAccessOverride(&gAttrAccess);
}
