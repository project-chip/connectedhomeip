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

#include <app/server-cluster/DefaultServerCluster.h>
#include <platform/DeviceInfoProvider.h>

namespace chip::app::Clusters {

class BooleanStateCluster : public DefaultServerCluster
{
public:
    BooleanStateCluster(EndpointId endpointId);

    // Server cluster implementation
    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override;
    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override;

    // Set a boolean value.
    // If the boolean value was actually modified, an event will be generated.
    // On success, the return value is an optional containing the EventNumber of the event that was generated.
    // On error, the return value is nullopt.
    std::optional<EventNumber> SetStateValue(bool stateValue);

    bool GetStateValue() const { return mStateValue; }

protected:
    bool mStateValue;
};

} // namespace chip::app::Clusters
