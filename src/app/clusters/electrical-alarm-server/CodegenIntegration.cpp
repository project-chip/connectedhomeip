/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <app/clusters/electrical-alarm-server/CodegenIntegration.h>

#include <data-model-providers/codegen/CodegenDataModelProvider.h>
#include <data-model-providers/codegen/CodegenProcessingConfig.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ElectricalAlarm {

CHIP_ERROR Instance::Init()
{
    VerifyOrReturnError(!mRegistered, CHIP_ERROR_INCORRECT_STATE);
    ReturnErrorOnFailure(CodegenDataModelProvider::Instance().Registry().Register(mCluster.Registration()));
    mRegistered = true;
    return CHIP_NO_ERROR;
}

void Instance::Shutdown()
{
    VerifyOrReturn(mRegistered);
    mRegistered = false;

    CHIP_ERROR err = CodegenDataModelProvider::Instance().Registry().Unregister(&mCluster.Cluster());
    if (err != CHIP_NO_ERROR)
    {
#if CHIP_CODEGEN_CONFIG_ENABLE_CODEGEN_INTEGRATION_LOOKUP_ERRORS
        ChipLogError(AppServer, "Failed to unregister Electrical Alarm cluster: %" CHIP_ERROR_FORMAT, err.Format());
#endif // CHIP_CODEGEN_CONFIG_ENABLE_CODEGEN_INTEGRATION_LOOKUP_ERRORS
    }
}

} // namespace ElectricalAlarm
} // namespace Clusters
} // namespace app
} // namespace chip

// Weak stubs — an app that registers ESALM imperatively provides strong overrides.
__attribute__((weak)) void MatterElectricalAlarmPluginServerInitCallback() {}
__attribute__((weak)) void MatterElectricalAlarmPluginServerShutdownCallback() {}
__attribute__((weak)) void MatterElectricalAlarmClusterInitCallback(chip::EndpointId) {}
__attribute__((weak)) void MatterElectricalAlarmClusterShutdownCallback(chip::EndpointId, MatterClusterShutdownType) {}
