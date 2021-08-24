/**
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/CommandHandler.h>
#include <app/util/af.h>

using namespace chip;
using namespace chip::app::Clusters;

bool emberAfSoftwareDiagnosticsClusterResetWatermarksCallback(EndpointId endpoint, app::CommandHandler * commandObj)
{
    uint64_t currentHeapUsed;

    EmberAfStatus status = SoftwareDiagnostics::Attributes::GetCurrentHeapUsed(endpoint, &currentHeapUsed);
    VerifyOrExit(status == EMBER_ZCL_STATUS_SUCCESS, ChipLogError(Zcl, "Failed to get the value of the CurrentHeapUsed attribute"));

    status = SoftwareDiagnostics::Attributes::SetCurrentHeapHighWatermark(endpoint, currentHeapUsed);
    VerifyOrExit(
        status == EMBER_ZCL_STATUS_SUCCESS,
        ChipLogError(
            Zcl,
            "Failed to reset the value of the CurrentHeapHighWaterMark attribute to the value of the CurrentHeapUsed attribute"));

exit:
    emberAfSendImmediateDefaultResponse(status);

    return true;
}
