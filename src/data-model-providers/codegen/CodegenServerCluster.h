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

#include <app/ConcreteClusterPath.h>
#include <app/server-cluster/ServerClusterInterface.h>

namespace chip {
namespace app {

class CodegenServerCluster : public ServerClusterInterface
{
public:
    CodegenServerCluster() = default;

    CHIP_ERROR Startup(ServerClusterContext & context) override;
    void Shutdown() override;

    DataVersion GetDataVersion(const ConcreteClusterPath & path) const override;
    BitFlags<DataModel::ClusterQualityFlags> GetClusterFlags(const ConcreteClusterPath &) const override
    {
        return {};
    } // TODO(sergiosoares): implement this
    CHIP_ERROR Attributes(const ConcreteClusterPath & path, DataModel::ListBuilder<DataModel::AttributeEntry> & builder) override;

    Span<const ConcreteClusterPath> GetPaths() const override { return {}; };

    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override;

    DataModel::ActionReturnStatus WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                 AttributeValueDecoder & decoder) override;

    std::optional<DataModel::ActionReturnStatus> InvokeCommand(const DataModel::InvokeRequest & request,
                                                               chip::TLV::TLVReader & input_arguments,
                                                               CommandHandler * handler) override;

    CHIP_ERROR AcceptedCommands(const ConcreteClusterPath & path,
                                DataModel::ListBuilder<DataModel::AcceptedCommandEntry> & builder) override;

    CHIP_ERROR GeneratedCommands(const ConcreteClusterPath & path, DataModel::ListBuilder<CommandId> & builder) override;

    void ListAttributeWriteNotification(const ConcreteAttributePath & aPath, DataModel::ListWriteOperation opType) override;

private:
    ServerClusterContext * mContext = nullptr;
};

} // namespace app
} // namespace chip
