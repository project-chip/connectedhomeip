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

#include <app/clusters/user-label-server/user-label-cluster.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <app/server/Server.h>
#include <clusters/UserLabel/Metadata.h>

namespace chip::app::Clusters {

using namespace UserLabel;
using namespace UserLabel::Attributes;

namespace {

CHIP_ERROR ReadLabelList(EndpointId endpoint, AttributeValueEncoder & encoder)
{
    DeviceLayer::DeviceInfoProvider * provider = DeviceLayer::GetDeviceInfoProvider();

    if (provider)
    {
        DeviceLayer::DeviceInfoProvider::UserLabelIterator * it = provider->IterateUserLabel(endpoint);

        if (it)
        {
            CHIP_ERROR err = encoder.EncodeList([&it](const auto & encod) -> CHIP_ERROR {
                UserLabel::Structs::LabelStruct::Type userlabel;

                while (it->Next(userlabel))
                {
                    ReturnErrorOnFailure(encod.Encode(userlabel));
                }

                return CHIP_NO_ERROR;
            });

            it->Release();
            return err;
        }
    }

    return encoder.EncodeEmptyList();
}

/// Matches constraints on a LabelStruct.
bool IsValidLabelEntry(const Structs::LabelStruct::Type & entry)
{
    constexpr size_t kMaxLabelSize = 16;
    constexpr size_t kMaxValueSize = 16;

    // NOTE: spec default for label and value is empty, so empty is accepted here
    return (entry.label.size() <= kMaxLabelSize) && (entry.value.size() <= kMaxValueSize);
}

bool IsValidLabelEntryList(const LabelList::TypeInfo::DecodableType & list)
{
    auto iter = list.begin();
    while (iter.Next())
    {
        if (!IsValidLabelEntry(iter.GetValue()))
        {
            return false;
        }
    }
    return true;
}

CHIP_ERROR WriteLabelList(const ConcreteDataAttributePath & path, AttributeValueDecoder & decoder)
{
    DeviceLayer::DeviceInfoProvider * provider = DeviceLayer::GetDeviceInfoProvider();

    VerifyOrReturnError(provider != nullptr, CHIP_ERROR_NOT_IMPLEMENTED);

    EndpointId endpoint = path.mEndpointId;

    if (!path.IsListItemOperation())
    {
        DeviceLayer::AttributeList<Structs::LabelStruct::Type, DeviceLayer::kMaxUserLabelListLength> labelList;
        LabelList::TypeInfo::DecodableType decodablelist;

        ReturnErrorOnFailure(decoder.Decode(decodablelist));
        VerifyOrReturnError(IsValidLabelEntryList(decodablelist), CHIP_IM_GLOBAL_STATUS(ConstraintError));

        auto iter = decodablelist.begin();
        while (iter.Next())
        {
            auto & entry = iter.GetValue();
            ReturnErrorOnFailure(labelList.add(entry));
        }
        ReturnErrorOnFailure(iter.GetStatus());

        return provider->SetUserLabelList(endpoint, labelList);
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
    case LabelList::Id: {
        CHIP_ERROR err = WriteLabelList(request.path, decoder);
        VerifyOrReturnError(err == CHIP_NO_ERROR, err);
        NotifyAttributeChanged(LabelList::Id);
        return CHIP_NO_ERROR;
    }
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
    CHIP_ERROR err = DefaultServerCluster::Startup(context);
    VerifyOrReturnError(err == CHIP_NO_ERROR, err);

    err = Server::GetInstance().GetFabricTable().AddFabricDelegate(this);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "UserLabel: Unable to register Fabric table delegate");
    }

    return err;
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
