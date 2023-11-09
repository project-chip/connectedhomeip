#include "electrical-energy-measurement-server.h"
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Events.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app-common/zap-generated/ids/Commands.h>
#include <app/EventLogging.h>
#include <app/CommandHandler.h>
#include <app/ConcreteCommandPath.h>

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

void MatterElectricalEnergyMeasurementPluginServerInitCallback() 
{

}

namespace chip {
namespace app {
namespace Clusters {
namespace ElectricalEnergyMeasurement {
namespace Server {

bool NotifyCumulativeEnergyImported(chip::EndpointId endpointId, uint32_t aImportedTimeStampS, uint64_t aEnergyImported)
{
	Events::CumulativeEnergyImported::Type event;
	event.importedTimestamp = aImportedTimeStampS;
	event.energyImported = aEnergyImported;

    EventNumber eventNumber;
    CHIP_ERROR error = app::LogEvent(event, endpointId, eventNumber);

    if (CHIP_NO_ERROR != error)
    {
        ChipLogError(Zcl, "[NotifyCumulativeEnergyImported] Unable to send event: %s [endpointId=%d]", error.AsString(), endpointId);
        return false;
    }
    ChipLogProgress(Zcl, "[NotifyCumulativeEnergyImported] Sent event [endpointId=%d,eventNumber=%" PRIu64 ",importedTimestamp=%u,energyImported=%" PRIu64 "]",
                    endpointId, eventNumber, aImportedTimeStampS, aEnergyImported);
    return true;
}

bool NotifyCumulativeEnergyExported(chip::EndpointId endpointId, uint32_t aImportedTimeStampS, uint64_t aEnergyExported)
{
	Events::CumulativeEnergyExported::Type event;
	event.importedTimestamp = aImportedTimeStampS;
	event.energyExported = aEnergyExported;

    EventNumber eventNumber;
    CHIP_ERROR error = app::LogEvent(event, endpointId, eventNumber);

    if (CHIP_NO_ERROR != error)
    {
        ChipLogError(Zcl, "[NotifyCumulativeEnergyExported] Unable to send event: %s [endpointId=%d]", error.AsString(), endpointId);
        return false;
    }
    ChipLogProgress(Zcl, "[NotifyCumulativeEnergyExported] Sent event [endpointId=%d,eventNumber=%" PRIu64 ",importedTimestamp=%u,energyExported=%" PRIu64 "]",
                    endpointId, eventNumber, aImportedTimeStampS, aEnergyExported);
    return true;
}

bool NotifyPeriodicEnergyImported(chip::EndpointId endpointId, uint32_t aPeriodStartS, uint32_t aPeriodEndS, uint64_t aEnergyImported)
{
	Events::PeriodicEnergyImported::Type event;
	event.periodStart = aPeriodStartS;
	event.periodEnd = aPeriodEndS;
	event.energyImported = aEnergyImported;

    EventNumber eventNumber;
    CHIP_ERROR error = app::LogEvent(event, endpointId, eventNumber);

    if (CHIP_NO_ERROR != error)
    {
        ChipLogError(Zcl, "[NotifyPeriodicEnergyImported] Unable to send event: %s [endpointId=%d]", error.AsString(), endpointId);
        return false;
    }
    ChipLogProgress(Zcl, "[NotifyPeriodicEnergyImported] Sent event [endpointId=%d,eventNumber=%" PRIu64 ",periodStart=%u,periodEnd=%u,energyImported=%" PRIu64 "]",
                    endpointId, eventNumber, aPeriodStartS, aPeriodEndS, aEnergyImported);
    return true;
}

bool NotifyPeriodicEnergyExported(chip::EndpointId endpointId, uint32_t aPeriodStartS, uint32_t aPeriodEndS, uint64_t aEnergyExported)
{
	Events::PeriodicEnergyExported::Type event;
	event.periodStart = aPeriodStartS;
	event.periodEnd = aPeriodEndS;
	event.energyExported = aEnergyExported;

    EventNumber eventNumber;
    CHIP_ERROR error = app::LogEvent(event, endpointId, eventNumber);

    if (CHIP_NO_ERROR != error)
    {
        ChipLogError(Zcl, "[NotifyPeriodicEnergyExported] Unable to send event: %s [endpointId=%d]", error.AsString(), endpointId);
        return false;
    }
    ChipLogProgress(Zcl, "[NotifyPeriodicEnergyExported] Sent event [endpointId=%d,eventNumber=%" PRIu64 ",periodStart=%u,periodEnd=%u,energyExported=%" PRIu64 "]",
                    endpointId, eventNumber, aPeriodStartS, aPeriodEndS, aEnergyExported);
    return true;
}

bool NotifyEphemeralEnergyImported(chip::EndpointId endpointId, uint32_t aPeriodStartS, uint32_t aPeriodEndS, uint64_t aEnergyImported)
{
	Events::EphemeralEnergyImported::Type event;
	event.periodStart = aPeriodStartS;
	event.periodEnd = aPeriodEndS;
	event.energyImported = aEnergyImported;

    EventNumber eventNumber;
    CHIP_ERROR error = app::LogEvent(event, endpointId, eventNumber);

    if (CHIP_NO_ERROR != error)
    {
        ChipLogError(Zcl, "[NotifyEphemeralEnergyImported] Unable to send event: %s [endpointId=%d]", error.AsString(), endpointId);
        return false;
    }
    ChipLogProgress(Zcl, "[NotifyEphemeralEnergyImported] Sent event [endpointId=%d,eventNumber=%" PRIu64 ",periodStart=%u,periodEnd=%u,energyImported=%" PRIu64 "]",
                    endpointId, eventNumber, aPeriodStartS, aPeriodEndS, aEnergyImported);
    return true;
}

bool NotifyEphemeralEnergyExported(chip::EndpointId endpointId, uint32_t aPeriodStartS, uint32_t aPeriodEndS, uint64_t aEnergyExported)
{
	Events::EphemeralEnergyExported::Type event;
	event.periodStart = aPeriodStartS;
	event.periodEnd = aPeriodEndS;
	event.energyExported = aEnergyExported;

    EventNumber eventNumber;
    CHIP_ERROR error = app::LogEvent(event, endpointId, eventNumber);

    if (CHIP_NO_ERROR != error)
    {
        ChipLogError(Zcl, "[NotifyEphemeralEnergyExported] Unable to send event: %s [endpointId=%d]", error.AsString(), endpointId);
        return false;
    }
    ChipLogProgress(Zcl, "[NotifyEphemeralEnergyExported] Sent event [endpointId=%d,eventNumber=%" PRIu64 ",periodStart=%u,periodEnd=%u,energyExported=%" PRIu64 "]",
                    endpointId, eventNumber, aPeriodStartS, aPeriodEndS, aEnergyExported);
    return true;
}

}}}}}