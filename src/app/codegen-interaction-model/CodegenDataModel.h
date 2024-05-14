/*
 *    Copyright (c) 2024 Project CHIP Authors
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

#include <app/interaction-model/Model.h>

namespace chip {
namespace app {

class CodegenDataModel : public chip::app::InteractionModel::Model
{
public:
    /// Generic model implementations
    CHIP_ERROR Shutdown() override { return CHIP_NO_ERROR; }

    CHIP_ERROR ReadAttribute(const InteractionModel::ReadAttributeRequest & request, InteractionModel::ReadState & state,
                             AttributeValueEncoder & encoder) override;
    CHIP_ERROR WriteAttribute(const InteractionModel::WriteAttributeRequest & request, AttributeValueDecoder & decoder) override;
    CHIP_ERROR Invoke(const InteractionModel::InvokeRequest & request, chip::TLV::TLVReader & input_arguments,
                      InteractionModel::InvokeReply & reply) override;

    /// attribute tree iteration
    EndpointId FirstEndpoint() override;
    EndpointId NextEndpoint(EndpointId before) override;

    InteractionModel::ClusterEntry FirstCluster(EndpointId endpoint) override;
    InteractionModel::ClusterEntry NextCluster(const ConcreteClusterPath & before) override;
    std::optional<InteractionModel::ClusterInfo> GetClusterInfo(const ConcreteClusterPath & path) override;

    InteractionModel::AttributeEntry FirstAttribute(const ConcreteClusterPath & cluster) override;
    InteractionModel::AttributeEntry NextAttribute(const ConcreteAttributePath & before) override;
    std::optional<InteractionModel::AttributeInfo> GetAttributeInfo(const ConcreteAttributePath & path) override;
};

} // namespace app
} // namespace chip
