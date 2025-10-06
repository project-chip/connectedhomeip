/**
 *
 *    Copyright (c) 2025 Project CHIP Authors
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
#include <app/server-cluster/OptionalAttributeSet.h>
#include <clusters/Descriptor/AttributeIds.h>
#include <clusters/Descriptor/ClusterId.h>
#include <clusters/Descriptor/Structs.h>

#include <initializer_list>

namespace chip::app::Clusters {

/// Exposes mandatory attributes for the descriptor clusters (it does NOT implement
/// optional bits through optional attribute set, currently using ifdef to maintain previous functionality.)
class DescriptorCluster : public DefaultServerCluster
{
public:
    using OptionalAttributesSet =
        chip::app::OptionalAttributeSet<Descriptor::Attributes::TagList::Id, Descriptor::Attributes::EndpointUniqueID::Id>;

    DescriptorCluster(EndpointId endpointId, OptionalAttributesSet optionalAttributeSet) :
        DefaultServerCluster({ endpointId, Descriptor::Id }), mEnabledOptionalAttributes(optionalAttributeSet)
    {}

    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override;
    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override;

private:
    OptionalAttributesSet mEnabledOptionalAttributes;
};

} // namespace chip::app::Clusters
