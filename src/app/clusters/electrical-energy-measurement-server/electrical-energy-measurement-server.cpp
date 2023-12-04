/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
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
#include <app/util/attribute-storage.h>

using chip::Protocols::InteractionModel::Status;
using namespace chip::app::Clusters::ElectricalEnergyMeasurement::Structs;
using namespace chip::app::Clusters::ElectricalEnergyMeasurement::Attributes;

namespace {

using namespace chip;

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

    switch (aPath.mAttributeId)
    {
    case CumulativeEnergyImported::Id:
    case CumulativeEnergyExported::Id:
    case PeriodicEnergyImported::Id:
    case PeriodicEnergyExported::Id:
        // TODO: this needs implementation
        aEncoder.EncodeNull();
        break;
    }

    return CHIP_NO_ERROR;
}

ElectricalEnergyMeasurementAttrAccess gAttrAccess;

} // namespace

namespace chip {
namespace app {
namespace Clusters {
namespace ElectricalEnergyMeasurement {

bool NotifyCumulativeEnergyMeasured(EndpointId endpointId, const Optional<EnergyMeasurementStruct::Type> & energyImported,
                                    const Optional<EnergyMeasurementStruct::Type> & energyExported)
{
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
