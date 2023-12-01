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

bool NotifyCumulativeEnergyImported(EndpointId endpointId, uint32_t aImportedTimeStampS, uint64_t aEnergyImported);
bool NotifyCumulativeEnergyExported(EndpointId endpointId, uint32_t aImportedTimeStampS, uint64_t aEnergyExported);

bool NotifyPeriodicEnergyImported(EndpointId endpointId, uint32_t aPeriodStartS, uint32_t aPeriodEndS, uint64_t aEnergyImported);
bool NotifyPeriodicEnergyExported(EndpointId endpointId, uint32_t aPeriodStartS, uint32_t aPeriodEndS, uint64_t aEnergyExported);

bool NotifyEphemeralEnergyImported(EndpointId endpointId, uint32_t aPeriodStartS, uint32_t aPeriodEndS, uint64_t aEnergyImported);
bool NotifyEphemeralEnergyExported(EndpointId endpointId, uint32_t aPeriodStartS, uint32_t aPeriodEndS, uint64_t aEnergyExported);

} // namespace Server
} // namespace ElectricalEnergyMeasurement
} // namespace Clusters
} // namespace app
} // namespace chip
