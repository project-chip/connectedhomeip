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
#include <app/server-cluster/testing/EmptyProvider.h>

namespace chip {
namespace Test {

using Protocols::InteractionModel::Status;
using namespace chip::app;
using namespace chip::app::DataModel;

CHIP_ERROR EmptyProvider::Shutdown()
{
    return CHIP_NO_ERROR;
}

CHIP_ERROR EmptyProvider::Endpoints(ListBuilder<app::DataModel::EndpointEntry> & builder)
{
    return CHIP_NO_ERROR;
}

CHIP_ERROR EmptyProvider::SemanticTags(EndpointId endpointId, ListBuilder<SemanticTag> & builder)
{
    return CHIP_IM_GLOBAL_STATUS(UnsupportedEndpoint);
}
CHIP_ERROR EmptyProvider::DeviceTypes(EndpointId endpointId, ListBuilder<app::DataModel::DeviceTypeEntry> & builder)
{
    return CHIP_IM_GLOBAL_STATUS(UnsupportedEndpoint);
}

CHIP_ERROR EmptyProvider::ClientClusters(EndpointId endpointId, ListBuilder<ClusterId> & builder)
{
    return CHIP_IM_GLOBAL_STATUS(UnsupportedEndpoint);
}
CHIP_ERROR EmptyProvider::ServerClusters(EndpointId endpointId, ListBuilder<app::DataModel::ServerClusterEntry> & builder)
{
    return CHIP_IM_GLOBAL_STATUS(UnsupportedEndpoint);
}

CHIP_ERROR EmptyProvider::Attributes(const app::ConcreteClusterPath & path, ListBuilder<app::DataModel::AttributeEntry> & builder)
{
    return CHIP_IM_GLOBAL_STATUS(UnsupportedEndpoint);
}

CHIP_ERROR EmptyProvider::GeneratedCommands(const app::ConcreteClusterPath & path, ListBuilder<CommandId> & builder)
{
    return CHIP_IM_GLOBAL_STATUS(UnsupportedEndpoint);
}

CHIP_ERROR EmptyProvider::AcceptedCommands(const app::ConcreteClusterPath & path,
                                           ListBuilder<app::DataModel::AcceptedCommandEntry> & builder)
{
    return CHIP_IM_GLOBAL_STATUS(UnsupportedEndpoint);
}

void EmptyProvider::Temporary_ReportAttributeChanged(const app::AttributePathParams & path) {}

ActionReturnStatus EmptyProvider::ReadAttribute(const app::DataModel::ReadAttributeRequest & request,
                                                app::AttributeValueEncoder & encoder)
{
    return Status::UnsupportedEndpoint;
}

ActionReturnStatus EmptyProvider::WriteAttribute(const app::DataModel::WriteAttributeRequest & request,
                                                 app::AttributeValueDecoder & decoder)
{
    return Status::UnsupportedEndpoint;
}
std::optional<ActionReturnStatus> EmptyProvider::InvokeCommand(const app::DataModel::InvokeRequest & request,
                                                               chip::TLV::TLVReader & input_arguments,
                                                               app::CommandHandler * handler)
{
    return Status::UnsupportedEndpoint;
}

} // namespace Test
} // namespace chip
