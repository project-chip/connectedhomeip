/*
 *
 *    Copyright (c) 2021-2023 Project CHIP Authors
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

#include "CHIPDeviceManager.h"

#include <app/ConcreteAttributePath.h>

#if CONFIG_DIAG_LOGS_DEMO
#include "DiagnosticLogsProviderDelegateImpl.h"
#include <app/clusters/diagnostic-logs-server/diagnostic-logs-server.h>
#endif

void MatterPostAttributeChangeCallback(const chip::app::ConcreteAttributePath & path, uint8_t type, uint16_t size, uint8_t * value)
{
    chip::DeviceManager::CHIPDeviceManagerCallbacks * cb =
        chip::DeviceManager::CHIPDeviceManager::GetInstance().GetCHIPDeviceManagerCallbacks();
    if (cb != nullptr)
    {
        // propagate event to device manager
        cb->PostAttributeChangeCallback(path.mEndpointId, path.mClusterId, path.mAttributeId, type, size, value);
    }
}

#if CONFIG_DIAG_LOGS_DEMO
/** @brief DiagnosticLogs Cluster Init
 *
 * This function is called when a specific cluster is initialized. It gives the
 * application an opportunity to take care of cluster initialization procedures.
 * It is called exactly once for each endpoint where cluster is present.
 */
void emberAfDiagnosticLogsClusterInitCallback(chip::EndpointId endpoint)
{
    auto & logProvider = chip::app::Clusters::DiagnosticLogs::LogProvider::GetInstance();
    auto & server      = chip::app::Clusters::DiagnosticLogs::DiagnosticLogsServer::Instance();
    server.SetDiagnosticLogsProviderDelegate(endpoint, &logProvider);
}
#endif
