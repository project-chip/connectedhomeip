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
#include <clusters/shared/Structs.h>
#include <lib/support/BitFlags.h>
#include <lib/support/Span.h>

namespace chip::app::Clusters {

class DescriptorCluster : public DefaultServerCluster
{
public:
    using SemanticTag           = Globals::Structs::SemanticTagStruct::Type;
    using OptionalAttributesSet = OptionalAttributeSet<Descriptor::Attributes::EndpointUniqueID::Id>;

    /*
     * The caller who provides the Span of semantic tags MUST ensure that the underlying data remains
     * valid for the lifetime of the DescriptorCluster instance. This is a non-owning view of the
     * semantic tag data.
     */
    DescriptorCluster(EndpointId endpointId, OptionalAttributesSet optionalAttributeSet, Span<const SemanticTag> semanticTags) :
        DefaultServerCluster({ endpointId, Descriptor::Id }), mEnabledOptionalAttributes(optionalAttributeSet),
        mSemanticTags(semanticTags)
    {}

    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override;
    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override;

protected:
    OptionalAttributesSet mEnabledOptionalAttributes;
    Span<const SemanticTag> mSemanticTags;
};

} // namespace chip::app::Clusters
