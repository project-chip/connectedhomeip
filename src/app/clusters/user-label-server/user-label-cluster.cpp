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

#include <app/clusters/user-label-server/user-label-cluster.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <app/server/Server.h>
#include <clusters/UserLabel/Metadata.h>

#include <array>

namespace chip::app::Clusters {

using namespace UserLabel;
using namespace UserLabel::Attributes;

namespace {

class AutoReleaseIterator
{
public:
    AutoReleaseIterator(DeviceLayer::DeviceInfoProvider * provider, EndpointId endpointId) :
        mIterator(provider != nullptr ? provider->IterateUserLabel(endpointId) : nullptr)
    {}
    ~AutoReleaseIterator()
    {
        if (mIterator != nullptr)
        {
            mIterator->Release();
        }
    }

    bool IsValid() const { return mIterator != nullptr; }

    DeviceLayer::DeviceInfoProvider::UserLabelIterator * operator->() { return mIterator; }

private:
    DeviceLayer::DeviceInfoProvider::UserLabelIterator * mIterator;
};

CHIP_ERROR ReadLabelList(EndpointId endpoint, AttributeValueEncoder & encoder)
{
    AutoReleaseIterator it(DeviceLayer::GetDeviceInfoProvider(), endpoint);
    VerifyOrReturnValue(it.IsValid(), encoder.EncodeEmptyList());

    return encoder.EncodeList([&it](const auto & encod) -> CHIP_ERROR {
        UserLabel::Structs::LabelStruct::Type userlabel;
        while (it->Next(userlabel))
        {
            ReturnErrorOnFailure(encod.Encode(userlabel));
        }
        return CHIP_NO_ERROR;
    });
}

/// Matches constraints on a LabelStruct.
bool IsValidLabelEntry(const Structs::LabelStruct::Type & entry)
{
    // NOTE: spec default for label and value is empty, so empty is accepted here
    return (entry.label.size() <= UserLabelCluster::kMaxLabelSize) && (entry.value.size() <= UserLabelCluster::kMaxValueSize);
}

CHIP_ERROR WriteLabelList(const ConcreteDataAttributePath & path, AttributeValueDecoder & decoder)
{
    DeviceLayer::DeviceInfoProvider * provider = DeviceLayer::GetDeviceInfoProvider();

    VerifyOrReturnError(provider != nullptr, CHIP_ERROR_NOT_IMPLEMENTED);

    EndpointId endpoint = path.mEndpointId;

    if (!path.IsListItemOperation())
    {
        size_t numLabels = 0;
        std::array<Structs::LabelStruct::Type, DeviceLayer::kMaxUserLabelListLength> labels;
        LabelList::TypeInfo::DecodableType decodablelist;

        ReturnErrorOnFailure(decoder.Decode(decodablelist));
        auto iter = decodablelist.begin();
        while (iter.Next())
        {
            auto & label = iter.GetValue();
            VerifyOrReturnError(IsValidLabelEntry(label), CHIP_IM_GLOBAL_STATUS(ConstraintError));
            VerifyOrReturnError(numLabels < labels.size(), CHIP_ERROR_NO_MEMORY);
            labels[numLabels++] = label;
        }
        ReturnErrorOnFailure(iter.GetStatus());

        return provider->SetUserLabelList(endpoint, Span(labels.data(), numLabels));
    }

    if (path.mListOp == ConcreteDataAttributePath::ListOperation::AppendItem)
    {
        Structs::LabelStruct::DecodableType entry;

        ReturnErrorOnFailure(decoder.Decode(entry));
        VerifyOrReturnError(IsValidLabelEntry(entry), CHIP_IM_GLOBAL_STATUS(ConstraintError));

        // Append the single user label entry
        CHIP_ERROR err = provider->AppendUserLabel(endpoint, entry);
        if (err == CHIP_ERROR_NO_MEMORY)
        {
            return CHIP_IM_GLOBAL_STATUS(ResourceExhausted);
        }

        return err;
    }

    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

} // namespace

UserLabelCluster::UserLabelCluster(EndpointId endpoint) : DefaultServerCluster({ endpoint, UserLabel::Id }) {}

DataModel::ActionReturnStatus UserLabelCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                              AttributeValueEncoder & encoder)
{
    switch (request.path.mAttributeId)
    {
    case LabelList::Id:
        return ReadLabelList(mPath.mEndpointId, encoder);
    case ClusterRevision::Id:
        return encoder.Encode(UserLabel::kRevision);
    case FeatureMap::Id:
        return encoder.Encode<uint32_t>(0);
    default:
        return Protocols::InteractionModel::Status::UnsupportedAttribute;
    }
}

DataModel::ActionReturnStatus UserLabelCluster::WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                               AttributeValueDecoder & decoder)
{
    switch (request.path.mAttributeId)
    {
    case LabelList::Id:
        return NotifyAttributeChangedIfSuccess(LabelList::Id, WriteLabelList(request.path, decoder));
    default:
        return Protocols::InteractionModel::Status::UnsupportedWrite;
    }
}

CHIP_ERROR UserLabelCluster::Attributes(const ConcreteClusterPath & path,
                                        ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    AttributeListBuilder listBuilder(builder);
    return listBuilder.Append(Span(UserLabel::Attributes::kMandatoryMetadata), {});
}

CHIP_ERROR UserLabelCluster::Startup(ServerClusterContext & context)
{
    ReturnErrorOnFailure(DefaultServerCluster::Startup(context));

    return Server::GetInstance().GetFabricTable().AddFabricDelegate(this);
}

void UserLabelCluster::Shutdown()
{
    Server::GetInstance().GetFabricTable().RemoveFabricDelegate(this);
    DefaultServerCluster::Shutdown();
}

void UserLabelCluster::OnFabricRemoved(const FabricTable & fabricTable, FabricIndex fabricIndex)
{
    // If the FabricIndex matches the last remaining entry in the Fabrics list, then the device SHALL delete all Matter
    // related data on the node which was created since it was commissioned.
    VerifyOrReturn(Server::GetInstance().GetFabricTable().FabricCount() == 0);

    ChipLogProgress(Zcl, "UserLabel: Last Fabric index 0x%x was removed", static_cast<unsigned>(fabricIndex));

    // Delete all user label data on the node which was added since it was commissioned.
    DeviceLayer::DeviceInfoProvider * provider = DeviceLayer::GetDeviceInfoProvider();
    VerifyOrReturn(provider != nullptr);
    // If UserLabel cluster is implemented on this endpoint
    if (CHIP_NO_ERROR != provider->ClearUserLabelList(mPath.mEndpointId))
    {
        ChipLogError(Zcl, "UserLabel: Failed to clear UserLabelList for endpoint: %d", mPath.mEndpointId);
    }
}

} // namespace chip::app::Clusters
