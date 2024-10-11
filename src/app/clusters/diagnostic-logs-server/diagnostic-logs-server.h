/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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
#include <app/CommandHandlerInterface.h>
#include <app/clusters/diagnostic-logs-server/DiagnosticLogsProviderDelegate.h>

namespace chip {
namespace app {
namespace Clusters {
namespace DiagnosticLogs {

/// A reference implementation for DiagnosticLogs source.
class DiagnosticLogsServer
{
public:
    static DiagnosticLogsServer & Instance();

    /**
     * Set the default delegate of the diagnostic logs cluster for the specified endpoint
     *
     * @param endpoint ID of the endpoint
     *
     * @param delegate The log provider delegate at the endpoint
     */
    void SetDiagnosticLogsProviderDelegate(EndpointId endpoint, DiagnosticLogsProviderDelegate * delegate);

    /**
     * Handles the request to download diagnostic logs of type specified in the intent argument for protocol type ResponsePayload
     * This should return whatever fits in the logContent field of the RetrieveLogsResponse command
     *
     * @param commandObj  The command handler object from the RetrieveLogsRequest command
     * @param path        The command path from the RetrieveLogsRequest command
     * @param intent      The log type requested in the RetrieveLogsRequest command
     * @param status      The status to be returned on success
     *
     */
    void HandleLogRequestForResponsePayload(CommandHandler * commandObj, const ConcreteCommandPath & path, IntentEnum intent,
                                            StatusEnum status = StatusEnum::kSuccess);

    void HandleLogRequestForBdx(CommandHandler * commandObj, const ConcreteCommandPath & path, IntentEnum intent,
                                Optional<CharSpan> transferFileDesignator);

private:
    static DiagnosticLogsServer sInstance;
};

} // namespace DiagnosticLogs
} // namespace Clusters
} // namespace app
} // namespace chip
