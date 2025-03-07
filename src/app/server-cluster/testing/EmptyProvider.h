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

#include <app/data-model-provider/Provider.h>
#include <protocols/interaction_model/StatusCode.h>

namespace chip {
namespace Test {

/// A provider that is emtpy - it contains no endpoints and most
/// calls fail with `Unsupported Endpoint`
///
/// This is a bare minimum implentation to have somethign that claims to be a `Provider`
/// however it has no real implementation that is useful. Over time this should be replaced
/// with some code-generated/controllable provider to allow for better testing.
class EmptyProvider : public app::DataModel::Provider
{
public:
    using ActionReturnStatus = app::DataModel::ActionReturnStatus;
    template <typename T>
    using ListBuilder = app::DataModel::ListBuilder<T>;

    CHIP_ERROR Shutdown() override;
    CHIP_ERROR Endpoints(ListBuilder<app::DataModel::EndpointEntry> & builder) override;

    CHIP_ERROR SemanticTags(EndpointId endpointId, ListBuilder<SemanticTag> & builder) override;
    CHIP_ERROR DeviceTypes(EndpointId endpointId, ListBuilder<app::DataModel::DeviceTypeEntry> & builder) override;
    CHIP_ERROR ClientClusters(EndpointId endpointId, ListBuilder<ClusterId> & builder) override;
    CHIP_ERROR ServerClusters(EndpointId endpointId, ListBuilder<app::DataModel::ServerClusterEntry> & builder) override;
    CHIP_ERROR Attributes(const app::ConcreteClusterPath & path, ListBuilder<app::DataModel::AttributeEntry> & builder) override;
    CHIP_ERROR GeneratedCommands(const app::ConcreteClusterPath & path, ListBuilder<CommandId> & builder) override;
    CHIP_ERROR AcceptedCommands(const app::ConcreteClusterPath & path,
                                ListBuilder<app::DataModel::AcceptedCommandEntry> & builder) override;
    void ListAttributeWriteNotification(const app::ConcreteAttributePath & aPath,
                                        app::DataModel::ListWriteOperation opType) override
    {}

    void Temporary_ReportAttributeChanged(const app::AttributePathParams & path) override;

    ActionReturnStatus ReadAttribute(const app::DataModel::ReadAttributeRequest & request,
                                     app::AttributeValueEncoder & encoder) override;
    ActionReturnStatus WriteAttribute(const app::DataModel::WriteAttributeRequest & request,
                                      app::AttributeValueDecoder & decoder) override;
    std::optional<ActionReturnStatus> InvokeCommand(const app::DataModel::InvokeRequest & request,
                                                    chip::TLV::TLVReader & input_arguments, app::CommandHandler * handler) override;
};

} // namespace Test
} // namespace chip
