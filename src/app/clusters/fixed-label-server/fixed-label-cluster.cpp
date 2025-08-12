/*
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

#include "fixed-label-cluster.h"
#include <app/server-cluster/AttributeListBuilder.h>
#include <clusters/FixedLabel/Metadata.h>

namespace chip::app::Clusters {

using namespace FixedLabel::Attributes;

// Register for the Fixed Label cluster on all endpoints.
FixedLabelCluster::FixedLabelCluster() : DefaultServerCluster({ 1, FixedLabel::Id }) {}

CHIP_ERROR FixedLabelCluster::ReadLabelList(EndpointId endpoint, AttributeValueEncoder & encoder)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    DeviceLayer::DeviceInfoProvider * provider = DeviceLayer::GetDeviceInfoProvider();

    if (provider)
    {
        DeviceLayer::DeviceInfoProvider::FixedLabelIterator * it = provider->IterateFixedLabel(endpoint);

        if (it)
        {
            err = encoder.EncodeList([&it](const auto & encoder) -> CHIP_ERROR {
                FixedLabel::Structs::LabelStruct::Type fixedlabel;

                while (it->Next(fixedlabel))
                {
                    ReturnErrorOnFailure(encoder.Encode(fixedlabel));
                }

                return CHIP_NO_ERROR;
            });

            it->Release();
        }
        else
        {
            err = encoder.EncodeEmptyList();
        }
    }
    else
    {
        err = encoder.EncodeEmptyList();
    }

    return err;
}

DataModel::ActionReturnStatus FixedLabelCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                               AttributeValueEncoder & encoder)
{
    switch (request.path.mAttributeId)
    {
    case LabelList::Id:
        return ReadLabelList(mPath.mEndpointId, encoder);
    case ClusterRevision::Id:
        return encoder.Encode(FixedLabel::kRevision);
    case FeatureMap::Id:
        return encoder.Encode(static_cast<uint32_t>(0));
    default:
        return Protocols::InteractionModel::Status::UnreportableAttribute;
    }
}

CHIP_ERROR FixedLabelCluster::Attributes(const ConcreteClusterPath & path,
                                         ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    AttributeListBuilder listBuilder(builder);
    return listBuilder.Append(Span(FixedLabel::Attributes::kMandatoryMetadata), {});
}

} // namespace chip::app::Clusters
