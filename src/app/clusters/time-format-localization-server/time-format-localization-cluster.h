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

#include <app/clusters/time-format-localization-server/time-format-localization-logic.h>

#include <app/server-cluster/DefaultServerCluster.h>
#include <clusters/TimeFormatLocalization/ClusterId.h>

namespace chip {
namespace app {
namespace Clusters {

class TimeFormatLocalizationCluster : public DefaultServerCluster
{
public:
    TimeFormatLocalizationCluster(EndpointId endpointId, BitFlags<TimeFormatLocalization::Feature> _unused_features, TimeFormatLocalizationEnabledAttributes attr);

    
    // Server cluster implementation
    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request, AttributeValueEncoder & encoder) override;
    DataModel::ActionReturnStatus WriteAttribute(const DataModel::WriteAttributeRequest & request, AttributeValueDecoder & decoder) override;
    std::optional<DataModel::ActionReturnStatus> InvokeCommand(const DataModel::InvokeRequest & request, TLV::TLVReader & input_arguments, CommandHandler * handler) override;
    CHIP_ERROR AcceptedCommands(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder) override;
    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override;

protected:
    //TODO Set this to a specific type of "TimeFormatLocalizationLogic" (The DeviceLayer one) just to compile while UT are developed
    // Will have to change this approach to avoid tying it to it.
    DeviceLayerTimeFormatLocalizationLogic mLogic;

};

} // namespace Clusters
} // namespace app
} // namespace chip