/*
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

#include <app/clusters/ota-provider/ota-provider-delegate.h>

#include <app/server-cluster/DefaultServerCluster.h>
#include <clusters/OtaSoftwareUpdateProvider/ClusterId.h>
#include <clusters/OtaSoftwareUpdateProvider/Commands.h>
#include <lib/support/Span.h>

namespace chip {
namespace app {
namespace Clusters {

/// Type-safe implementation for callbacks for the OTA Provider server
class OtaProviderLogic
{
public:
    void SetDelegate(OTAProviderDelegate * delegate) { mDelegate = delegate; }

    std::optional<DataModel::ActionReturnStatus>
    ApplyUpdateRequest(const ConcreteCommandPath & commandPath,
                       const OtaSoftwareUpdateProvider::Commands::ApplyUpdateRequest::DecodableType & commandData,
                       app::CommandHandler * handler);

    std::optional<DataModel::ActionReturnStatus>
    NotifyUpdateApplied(const ConcreteCommandPath & commandPath,
                        const OtaSoftwareUpdateProvider::Commands::NotifyUpdateApplied::DecodableType & commandData,
                        app::CommandHandler * handler);

    std::optional<DataModel::ActionReturnStatus>
    QueryImage(const ConcreteCommandPath & commandPath,
               const OtaSoftwareUpdateProvider::Commands::QueryImage::DecodableType & commandData, app::CommandHandler * handler);

private:
    /// Convenience method that returns if the internal delegate is null and will log
    /// an error if the check returns true
    bool IsNullDelegateWithLogging(EndpointId endpointIdForLogging);

    /// convenience method that checks an update token for validity and logs
    /// on error (when returning false).
    static bool IsValidUpdateTokenWithLogging(ByteSpan updateToken);

    OTAProviderDelegate * mDelegate = nullptr;
};

/// Integration of OTA provider logic within the matter data model
///
/// Translates between matter calls and OTA logic
class OtaProviderServer : public DefaultServerCluster, private OtaProviderLogic
{
public:
    OtaProviderServer(EndpointId endpointId) : DefaultServerCluster({ endpointId, OtaSoftwareUpdateProvider::Id }) {}

    using OtaProviderLogic::SetDelegate;

    // Server cluster implementation

    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override;
    CHIP_ERROR AcceptedCommands(const ConcreteClusterPath & path,
                                ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder) override;
    CHIP_ERROR GeneratedCommands(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<CommandId> & builder) override;

    std::optional<DataModel::ActionReturnStatus> InvokeCommand(const DataModel::InvokeRequest & request,
                                                               chip::TLV::TLVReader & input_arguments,
                                                               CommandHandler * handler) override;
};

} // namespace Clusters
} // namespace app
} // namespace chip
