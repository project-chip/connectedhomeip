/*
 *    Copyright (c) 2021-2025 Project CHIP Authors
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

#include <app/clusters/fixed-label-server/fixed-label-cluster.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <clusters/FixedLabel/Metadata.h>

namespace chip::app::Clusters {

using namespace FixedLabel::Attributes;

namespace {

CHIP_ERROR ReadLabelList(EndpointId endpoint, AttributeValueEncoder & encoder)
{
    DeviceLayer::DeviceInfoProvider * provider = DeviceLayer::GetDeviceInfoProvider();

    if (provider)
    {
        DeviceLayer::DeviceInfoProvider::FixedLabelIterator * it = provider->IterateFixedLabel(endpoint);

        if (it)
        {
            CHIP_ERROR err = encoder.EncodeList([&it](const auto & encod) -> CHIP_ERROR {
                FixedLabel::Structs::LabelStruct::Type fixedlabel;

                while (it->Next(fixedlabel))
                {
                    ReturnErrorOnFailure(encod.Encode(fixedlabel));
                }

                return CHIP_NO_ERROR;
            });

            it->Release();
            return err;
        }
    }

    return encoder.EncodeEmptyList();
}

} // namespace

FixedLabelCluster::FixedLabelCluster(EndpointId endpoint) : DefaultServerCluster({ endpoint, FixedLabel::Id }) {}

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
        return encoder.Encode<uint32_t>(0);
    default:
        return Protocols::InteractionModel::Status::UnsupportedAttribute;
    }
}

CHIP_ERROR FixedLabelCluster::Attributes(const ConcreteClusterPath & path,
                                         ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    AttributeListBuilder listBuilder(builder);
    return listBuilder.Append(Span(FixedLabel::Attributes::kMandatoryMetadata), {});
}

} // namespace chip::app::Clusters
