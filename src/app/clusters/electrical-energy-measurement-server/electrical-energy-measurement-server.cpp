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

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app-common/zap-generated/ids/Commands.h>
#include <app-common/zap-generated/ids/Events.h>
#include <app/CommandHandler.h>
#include <app/ConcreteCommandPath.h>
#include <app/EventLogging.h>

#include <app/util/af.h>
#include <app/util/common.h>
#include <app/util/error-mapping.h>
#include <array>
#include <lib/support/CodeUtils.h>
#include <platform/CHIPDeviceLayer.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters::ElectricalEnergyMeasurement;
using chip::Protocols::InteractionModel::Status;

void MatterElectricalEnergyMeasurementPluginServerInitCallback() {}

namespace chip {
namespace app {
namespace Clusters {
namespace ElectricalEnergyMeasurement {

bool NotifyCumulativeEnergyImported(EndpointId endpointId, uint32_t aImportedTimeStampS, uint64_t aEnergyImported)
{
    Events::CumulativeEnergyImported::Type event;
    event.importedTimestamp = aImportedTimeStampS;
    event.energyImported    = aEnergyImported;

    EventNumber eventNumber;
    CHIP_ERROR error = app::LogEvent(event, endpointId, eventNumber);

    if (CHIP_NO_ERROR != error)
    {
        ChipLogError(Zcl, "[NotifyCumulativeEnergyImported] Unable to send event: %" CHIP_ERROR_FORMAT " [endpointId=%d]",
                     error.Format(), endpointId);
        return false;
    }
    ChipLogProgress(Zcl,
                    "[NotifyCumulativeEnergyImported] Sent event [endpointId=%d,eventNumber=%" PRIu64
                    ",importedTimestamp=%u,energyImported=%" PRIu64 "]",
                    endpointId, eventNumber, aImportedTimeStampS, aEnergyImported);
    return true;
}

bool NotifyCumulativeEnergyExported(EndpointId endpointId, uint32_t aImportedTimeStampS, uint64_t aEnergyExported)
{
    Events::CumulativeEnergyExported::Type event;
    event.importedTimestamp = aImportedTimeStampS;
    event.energyExported    = aEnergyExported;

    EventNumber eventNumber;
    CHIP_ERROR error = app::LogEvent(event, endpointId, eventNumber);

    if (CHIP_NO_ERROR != error)
    {
        ChipLogError(Zcl, "[NotifyCumulativeEnergyExported] Unable to send event: %" CHIP_ERROR_FORMAT " [endpointId=%d]",
                     error.Format(), endpointId);
        return false;
    }
    ChipLogProgress(Zcl,
                    "[NotifyCumulativeEnergyExported] Sent event [endpointId=%d,eventNumber=%" PRIu64
                    ",importedTimestamp=%u,energyExported=%" PRIu64 "]",
                    endpointId, eventNumber, aImportedTimeStampS, aEnergyExported);
    return true;
}

bool NotifyPeriodicEnergyImported(EndpointId endpointId, uint32_t aPeriodStartS, uint32_t aPeriodEndS, uint64_t aEnergyImported)
{
    Events::PeriodicEnergyImported::Type event;
    event.periodStart    = aPeriodStartS;
    event.periodEnd      = aPeriodEndS;
    event.energyImported = aEnergyImported;

    EventNumber eventNumber;
    CHIP_ERROR error = app::LogEvent(event, endpointId, eventNumber);

    if (CHIP_NO_ERROR != error)
    {
        ChipLogError(Zcl, "[NotifyPeriodicEnergyImported] Unable to send event: %" CHIP_ERROR_FORMAT " [endpointId=%d]",
                     error.Format(), endpointId);
        return false;
    }
    ChipLogProgress(Zcl,
                    "[NotifyPeriodicEnergyImported] Sent event [endpointId=%d,eventNumber=%" PRIu64
                    ",periodStart=%u,periodEnd=%u,energyImported=%" PRIu64 "]",
                    endpointId, eventNumber, aPeriodStartS, aPeriodEndS, aEnergyImported);
    return true;
}

bool NotifyPeriodicEnergyExported(EndpointId endpointId, uint32_t aPeriodStartS, uint32_t aPeriodEndS, uint64_t aEnergyExported)
{
    Events::PeriodicEnergyExported::Type event;
    event.periodStart    = aPeriodStartS;
    event.periodEnd      = aPeriodEndS;
    event.energyExported = aEnergyExported;

    EventNumber eventNumber;
    CHIP_ERROR error = app::LogEvent(event, endpointId, eventNumber);

    if (CHIP_NO_ERROR != error)
    {
        ChipLogError(Zcl, "[NotifyPeriodicEnergyExported] Unable to send event: %" CHIP_ERROR_FORMAT " [endpointId=%d]",
                     error.Format(), endpointId);
        return false;
    }
    ChipLogProgress(Zcl,
                    "[NotifyPeriodicEnergyExported] Sent event [endpointId=%d,eventNumber=%" PRIu64
                    ",periodStart=%u,periodEnd=%u,energyExported=%" PRIu64 "]",
                    endpointId, eventNumber, aPeriodStartS, aPeriodEndS, aEnergyExported);
    return true;
}

bool NotifyEphemeralEnergyImported(EndpointId endpointId, uint32_t aPeriodStartS, uint32_t aPeriodEndS, uint64_t aEnergyImported)
{
    Events::EphemeralEnergyImported::Type event;
    event.periodStart    = aPeriodStartS;
    event.periodEnd      = aPeriodEndS;
    event.energyImported = aEnergyImported;

    EventNumber eventNumber;
    CHIP_ERROR error = app::LogEvent(event, endpointId, eventNumber);

    if (CHIP_NO_ERROR != error)
    {
        ChipLogError(Zcl, "[NotifyEphemeralEnergyImported] Unable to send event: %" CHIP_ERROR_FORMAT " [endpointId=%d]",
                     error.Format(), endpointId);
        return false;
    }
    ChipLogProgress(Zcl,
                    "[NotifyEphemeralEnergyImported] Sent event [endpointId=%d,eventNumber=%" PRIu64
                    ",periodStart=%u,periodEnd=%u,energyImported=%" PRIu64 "]",
                    endpointId, eventNumber, aPeriodStartS, aPeriodEndS, aEnergyImported);
    return true;
}

bool NotifyEphemeralEnergyExported(EndpointId endpointId, uint32_t aPeriodStartS, uint32_t aPeriodEndS, uint64_t aEnergyExported)
{
    Events::EphemeralEnergyExported::Type event;
    event.periodStart    = aPeriodStartS;
    event.periodEnd      = aPeriodEndS;
    event.energyExported = aEnergyExported;

    EventNumber eventNumber;
    CHIP_ERROR error = app::LogEvent(event, endpointId, eventNumber);

    if (CHIP_NO_ERROR != error)
    {
        ChipLogError(Zcl, "[NotifyEphemeralEnergyExported] Unable to send event: %" CHIP_ERROR_FORMAT " [endpointId=%d]",
                     error.Format(), endpointId);
        return false;
    }
    ChipLogProgress(Zcl,
                    "[NotifyEphemeralEnergyExported] Sent event [endpointId=%d,eventNumber=%" PRIu64
                    ",periodStart=%u,periodEnd=%u,energyExported=%" PRIu64 "]",
                    endpointId, eventNumber, aPeriodStartS, aPeriodEndS, aEnergyExported);
    return true;
}

} // namespace ElectricalEnergyMeasurement
} // namespace Clusters
} // namespace app
} // namespace chip
