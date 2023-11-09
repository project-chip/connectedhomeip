#pragma once

#include <app-common/zap-generated/cluster-objects.h>
#include <app/CommandHandler.h>
#include <app/ConcreteCommandPath.h>
#include <app/util/af.h>
#include <app/util/config.h>
#include <platform/CHIPDeviceConfig.h>
#include <protocols/interaction_model/StatusCode.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ElectricalEnergyMeasurement {
namespace Server {

bool NotifyCumulativeEnergyImported(chip::EndpointId endpointId, uint32_t aImportedTimeStampS, uint64_t aEnergyImported);
bool NotifyCumulativeEnergyExported(chip::EndpointId endpointId, uint32_t aImportedTimeStampS, uint64_t aEnergyExported);

bool NotifyPeriodicEnergyImported(chip::EndpointId endpointId, uint32_t aPeriodStartS, uint32_t aPeriodEndS, uint64_t aEnergyImported);
bool NotifyPeriodicEnergyExported(chip::EndpointId endpointId, uint32_t aPeriodStartS, uint32_t aPeriodEndS, uint64_t aEnergyExported);

bool NotifyEphemeralEnergyImported(chip::EndpointId endpointId, uint32_t aPeriodStartS, uint32_t aPeriodEndS, uint64_t aEnergyImported);
bool NotifyEphemeralEnergyExported(chip::EndpointId endpointId, uint32_t aPeriodStartS, uint32_t aPeriodEndS, uint64_t aEnergyExported);

}}}}}