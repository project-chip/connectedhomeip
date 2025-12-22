/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <app/util/af-types.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/logging/CHIPLogging.h>

// MatterTlsClientManagementPluginServerInitCallback is called once during application startup.
// MatterTlsClientManagementClusterInitCallback and MatterTlsClientManagementClusterShutdownCallback
// are called per-endpoint and must be implemented by the application to create/destroy cluster
// instances with application-provided dependencies (TlsClientManagementDelegate, CertificateTable).
// See examples/all-clusters-app for reference implementation.

void MatterTlsClientManagementPluginServerInitCallback()
{
    ChipLogProgress(Zcl, "Initializing TLS Client Management cluster.");
}
