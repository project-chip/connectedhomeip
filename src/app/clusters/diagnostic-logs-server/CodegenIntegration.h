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
#pragma once

#include <app/clusters/diagnostic-logs-server/DiagnosticLogsProviderDelegate.h>

namespace chip {
namespace app {
namespace Clusters {
namespace DiagnosticLogs {

class DiagnosticLogsServer
{
public:
    static DiagnosticLogsServer & Instance()
    {
        static DiagnosticLogsServer instance;
        return instance;
    }

    /**
     * Set the default delegate of the diagnostic logs cluster.
     *
     * @note The `endpoint` parameter is ignored,
     * as diagnostic logs are a node-wide utility and not specific to any endpoint.
     *
     * @param endpoint Ignored. Was intended to be the ID of the endpoint.
     * @param delegate The log provider delegate.
     */
    void SetDiagnosticLogsProviderDelegate(EndpointId endpoint, DiagnosticLogsProviderDelegate * delegate);
};

} // namespace DiagnosticLogs
} // namespace Clusters
} // namespace app
} // namespace chip
