/*
 *
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
#include <app/clusters/power-topology-server/PowerTopologyCluster.h>

#include <app/AttributeValueDecoder.h>
#include <app/persistence/AttributePersistenceProvider.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <app/server-cluster/OptionalAttributeSet.h>
#include <clusters/PowerTopology/Metadata.h>
#include <lib/core/TLV.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/ScopedMemoryBuffer.h>

#include <cstring>

using namespace chip;
using namespace chip::app;
using namespace chip::app::DataModel;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::PowerTopology;
using namespace chip::app::Clusters::PowerTopology::Attributes;

using chip::Protocols::InteractionModel::Status;

namespace chip {
namespace app {
namespace Clusters {
namespace PowerTopology {

Structs::CircuitNodeStruct::Type PowerTopologyCluster::StoredCircuitNode::AsStruct() const
{
    Structs::CircuitNodeStruct::Type value;
    value.node        = node;
    value.endpoint    = endpoint;
    value.fabricIndex = fabricIndex;
    if (hasLabel)
    {
        value.label.SetValue(CharSpan(label, labelLength));
    }
    return value;
}

CHIP_ERROR PowerTopologyCluster::GetAvailableEndpoints(AttributeValueEncoder & aEncoder) const
{
    return aEncoder.EncodeList([this](const auto & encoder) -> CHIP_ERROR {
        for (uint8_t i = 0; true; i++)
        {
            EndpointId endpointId;
            auto err = mDelegate.GetAvailableEndpointAtIndex(i, endpointId);

            VerifyOrReturnError(err != CHIP_ERROR_PROVIDER_LIST_EXHAUSTED, CHIP_NO_ERROR);

            ReturnErrorOnFailure(err);
            ReturnErrorOnFailure(encoder.Encode(endpointId));
        }
    });
}

CHIP_ERROR PowerTopologyCluster::GetActiveEndpoints(AttributeValueEncoder & aEncoder) const
{
    return aEncoder.EncodeList([this](const auto & encoder) -> CHIP_ERROR {
        for (uint8_t i = 0; true; i++)
        {
            EndpointId endpointId;
            auto err = mDelegate.GetActiveEndpointAtIndex(i, endpointId);

            VerifyOrReturnError(err != CHIP_ERROR_PROVIDER_LIST_EXHAUSTED, CHIP_NO_ERROR);

            ReturnErrorOnFailure(err);
            ReturnErrorOnFailure(encoder.Encode(endpointId));
        }
    });
}

CHIP_ERROR PowerTopologyCluster::GetElectricalCircuitNodes(AttributeValueEncoder & aEncoder) const
{
    // The item type is fabric-scoped, so the encoder transparently filters to the accessing fabric
    // on a fabric-filtered read; the cluster encodes every stored node unconditionally.
    return aEncoder.EncodeList([this](const auto & encoder) -> CHIP_ERROR {
        for (size_t i = 0; i < mCircuitNodeCount; i++)
        {
            ReturnErrorOnFailure(encoder.Encode(mCircuitNodes[i].AsStruct()));
        }
        return CHIP_NO_ERROR;
    });
}

size_t PowerTopologyCluster::CountNodesForFabric(FabricIndex fabricIndex) const
{
    size_t count = 0;
    for (size_t i = 0; i < mCircuitNodeCount; i++)
    {
        if (mCircuitNodes[i].fabricIndex == fabricIndex)
        {
            count++;
        }
    }
    return count;
}

void PowerTopologyCluster::RemoveNodesForFabric(FabricIndex fabricIndex)
{
    size_t kept = 0;
    for (size_t i = 0; i < mCircuitNodeCount; i++)
    {
        if (mCircuitNodes[i].fabricIndex != fabricIndex)
        {
            if (kept != i)
            {
                mCircuitNodes[kept] = mCircuitNodes[i];
            }
            kept++;
        }
    }
    mCircuitNodeCount = kept;
}

bool PowerTopologyCluster::DecodeCircuitNode(const Structs::CircuitNodeStruct::DecodableType & decoded, FabricIndex fabricIndex,
                                             StoredCircuitNode & out) const
{
    out          = StoredCircuitNode{};
    out.node     = decoded.node;
    out.endpoint = decoded.endpoint;
    // The accessing fabric is authoritative for a fabric-scoped write.
    out.fabricIndex = fabricIndex;
    if (decoded.label.HasValue())
    {
        CharSpan label = decoded.label.Value();
        VerifyOrReturnValue(label.size() <= kMaxNodeLabelLength, false);
        memcpy(out.label, label.data(), label.size());
        out.labelLength = label.size();
        out.hasLabel    = true;
    }
    return true;
}

CHIP_ERROR PowerTopologyCluster::Startup(ServerClusterContext & context)
{

    VerifyOrReturnError(
        mFeatureFlags.Has(Feature::kSetTopology) || !mFeatureFlags.Has(Feature::kDynamicPowerFlow), CHIP_ERROR_INCORRECT_STATE,
        ChipLogError(Zcl, "Power Topology Cluster: DynamicPowerFlow feature requires SetTopology feature to be enabled"));

    ReturnErrorOnFailure(DefaultServerCluster::Startup(context));

    if (mFeatureFlags.Has(Feature::kElectricalCircuit))
    {
        // Restore the persisted (non-volatile) ElectricalCircuitNodes list. A missing value is normal
        // on first boot and must not fail startup.
        CHIP_ERROR err = LoadCircuitNodes();
        if (err != CHIP_NO_ERROR && err != CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
        {
            ChipLogError(Zcl, "PowerTopology: failed to load ElectricalCircuitNodes: %" CHIP_ERROR_FORMAT, err.Format());
        }

        // Register for fabric removal so a removed fabric's fabric-scoped nodes are purged.
        if (mFabricTable != nullptr)
        {
            ReturnErrorOnFailure(mFabricTable->AddFabricDelegate(this));
        }
    }
    return CHIP_NO_ERROR;
}

void PowerTopologyCluster::Shutdown(ClusterShutdownType shutdownType)
{
    if (mFabricTable != nullptr)
    {
        mFabricTable->RemoveFabricDelegate(this);
    }
    DefaultServerCluster::Shutdown(shutdownType);
}

void PowerTopologyCluster::OnFabricRemoved(const FabricTable & fabricTable, FabricIndex fabricIndex)
{
    if (CountNodesForFabric(fabricIndex) == 0)
    {
        return;
    }
    RemoveNodesForFabric(fabricIndex);
    CHIP_ERROR err = SaveCircuitNodes();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "PowerTopology: failed to persist ElectricalCircuitNodes after fabric 0x%x removal: %" CHIP_ERROR_FORMAT,
                     fabricIndex, err.Format());
    }
    NotifyAttributeChanged(ElectricalCircuitNodes::Id);
}

DataModel::ActionReturnStatus PowerTopologyCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                                  AttributeValueEncoder & encoder)
{
    switch (request.path.mAttributeId)
    {
    case FeatureMap::Id:
        return encoder.Encode(mFeatureFlags);

    case ClusterRevision::Id:
        return encoder.Encode(kRevision);

    case AvailableEndpoints::Id:
        return GetAvailableEndpoints(encoder);

    case ActiveEndpoints::Id:
        return GetActiveEndpoints(encoder);

    case ElectricalCircuitNodes::Id:
        return GetElectricalCircuitNodes(encoder);

    default:
        return Protocols::InteractionModel::Status::UnsupportedAttribute;
    }
}

DataModel::ActionReturnStatus PowerTopologyCluster::WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                                   AttributeValueDecoder & decoder)
{
    switch (request.path.mAttributeId)
    {
    case ElectricalCircuitNodes::Id:
        return WriteElectricalCircuitNodes(request, decoder);

    default:
        return Protocols::InteractionModel::Status::UnsupportedWrite;
    }
}

DataModel::ActionReturnStatus PowerTopologyCluster::WriteElectricalCircuitNodes(const DataModel::WriteAttributeRequest & request,
                                                                                AttributeValueDecoder & decoder)
{
    VerifyOrReturnValue(mFeatureFlags.Has(Feature::kElectricalCircuit), Status::UnsupportedAttribute);

    const FabricIndex fabricIndex          = decoder.AccessingFabricIndex();
    const ConcreteDataAttributePath & path = request.path;
    const size_t otherFabricNodeCount      = mCircuitNodeCount - CountNodesForFabric(fabricIndex);

    if (!path.IsListItemOperation())
    {
        // ReplaceAll: replace ONLY the accessing fabric's slice; other fabrics' nodes are untouched.
        ElectricalCircuitNodes::TypeInfo::DecodableType list;
        ReturnErrorOnFailure(decoder.Decode(list));
        size_t newCount = 0;
        ReturnErrorOnFailure(list.ComputeSize(&newCount));
        VerifyOrReturnValue(otherFabricNodeCount + newCount <= kMaxCircuitNodes, Status::ResourceExhausted);

        // Stage + validate the whole incoming list before mutating state (no partial application).
        StoredCircuitNode staging[kMaxCircuitNodes];
        size_t stagingCount = 0;
        auto iter           = list.begin();
        while (iter.Next())
        {
            VerifyOrReturnValue(DecodeCircuitNode(iter.GetValue(), fabricIndex, staging[stagingCount]), Status::ConstraintError);
            stagingCount++;
        }
        ReturnErrorOnFailure(iter.GetStatus());

        RemoveNodesForFabric(fabricIndex);
        for (size_t i = 0; i < stagingCount; i++)
        {
            mCircuitNodes[mCircuitNodeCount++] = staging[i];
        }
    }
    else if (path.mListOp == ConcreteDataAttributePath::ListOperation::AppendItem)
    {
        Structs::CircuitNodeStruct::DecodableType value;
        ReturnErrorOnFailure(decoder.Decode(value));
        VerifyOrReturnValue(mCircuitNodeCount < kMaxCircuitNodes, Status::ResourceExhausted);
        VerifyOrReturnValue(DecodeCircuitNode(value, fabricIndex, mCircuitNodes[mCircuitNodeCount]), Status::ConstraintError);
        mCircuitNodeCount++;
    }
    else
    {
        return Status::UnsupportedWrite;
    }

    ReturnErrorOnFailure(SaveCircuitNodes());
    NotifyAttributeChanged(ElectricalCircuitNodes::Id);
    return CHIP_NO_ERROR;
}

namespace {
// TLV tags for the persisted ElectricalCircuitNodes blob (a private storage format; not the wire
// encoding). The CircuitNodeStruct wire codec is fabric-aware and omits the fabric index on write,
// so persistence uses this explicit format that retains every field including fabricIndex.
constexpr TLV::Tag kTagFabricIndex = TLV::ContextTag(0);
constexpr TLV::Tag kTagNode        = TLV::ContextTag(1);
constexpr TLV::Tag kTagEndpoint    = TLV::ContextTag(2);
constexpr TLV::Tag kTagLabel       = TLV::ContextTag(3);

// Worst case ~ kMaxCircuitNodes * (node + endpoint + label + fabricIndex + TLV overhead).
constexpr size_t kCircuitNodesBlobSize =
    PowerTopologyCluster::kMaxCircuitNodes * (PowerTopologyCluster::kMaxNodeLabelLength + 32) + 8;
} // namespace

CHIP_ERROR PowerTopologyCluster::SaveCircuitNodes()
{
    VerifyOrReturnError(mContext != nullptr, CHIP_ERROR_INCORRECT_STATE);

    Platform::ScopedMemoryBuffer<uint8_t> buffer;
    VerifyOrReturnError(buffer.Calloc(kCircuitNodesBlobSize), CHIP_ERROR_NO_MEMORY);

    TLV::TLVWriter writer;
    writer.Init(buffer.Get(), kCircuitNodesBlobSize);
    TLV::TLVType arrayContainer;
    ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Array, arrayContainer));
    for (size_t i = 0; i < mCircuitNodeCount; i++)
    {
        const StoredCircuitNode & node = mCircuitNodes[i];
        TLV::TLVType nodeContainer;
        ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, nodeContainer));
        ReturnErrorOnFailure(writer.Put(kTagFabricIndex, node.fabricIndex));
        ReturnErrorOnFailure(writer.Put(kTagNode, node.node));
        if (node.endpoint.HasValue())
        {
            ReturnErrorOnFailure(writer.Put(kTagEndpoint, node.endpoint.Value()));
        }
        if (node.hasLabel)
        {
            ReturnErrorOnFailure(writer.PutString(kTagLabel, CharSpan(node.label, node.labelLength)));
        }
        ReturnErrorOnFailure(writer.EndContainer(nodeContainer));
    }
    ReturnErrorOnFailure(writer.EndContainer(arrayContainer));
    ReturnErrorOnFailure(writer.Finalize());

    return mContext->attributeStorage.WriteValue(
        ConcreteAttributePath(mPath.mEndpointId, PowerTopology::Id, ElectricalCircuitNodes::Id),
        ByteSpan(buffer.Get(), writer.GetLengthWritten()));
}

CHIP_ERROR PowerTopologyCluster::LoadCircuitNodes()
{
    VerifyOrReturnError(mContext != nullptr, CHIP_ERROR_INCORRECT_STATE);

    Platform::ScopedMemoryBuffer<uint8_t> buffer;
    VerifyOrReturnError(buffer.Calloc(kCircuitNodesBlobSize), CHIP_ERROR_NO_MEMORY);

    MutableByteSpan span(buffer.Get(), kCircuitNodesBlobSize);
    ReturnErrorOnFailure(mContext->attributeStorage.ReadValue(
        ConcreteAttributePath(mPath.mEndpointId, PowerTopology::Id, ElectricalCircuitNodes::Id), span));

    TLV::TLVReader reader;
    reader.Init(span);
    ReturnErrorOnFailure(reader.Next(TLV::kTLVType_Array, TLV::AnonymousTag()));
    TLV::TLVType arrayContainer;
    ReturnErrorOnFailure(reader.EnterContainer(arrayContainer));

    mCircuitNodeCount = 0;
    CHIP_ERROR err;
    while ((err = reader.Next(TLV::kTLVType_Structure, TLV::AnonymousTag())) == CHIP_NO_ERROR)
    {
        if (mCircuitNodeCount >= kMaxCircuitNodes)
        {
            break;
        }
        StoredCircuitNode & node = mCircuitNodes[mCircuitNodeCount];
        node                     = StoredCircuitNode{};

        TLV::TLVType nodeContainer;
        ReturnErrorOnFailure(reader.EnterContainer(nodeContainer));
        CHIP_ERROR fieldErr;
        while ((fieldErr = reader.Next()) == CHIP_NO_ERROR)
        {
            if (reader.GetTag() == kTagFabricIndex)
            {
                ReturnErrorOnFailure(reader.Get(node.fabricIndex));
            }
            else if (reader.GetTag() == kTagNode)
            {
                ReturnErrorOnFailure(reader.Get(node.node));
            }
            else if (reader.GetTag() == kTagEndpoint)
            {
                EndpointId endpoint;
                ReturnErrorOnFailure(reader.Get(endpoint));
                node.endpoint.SetValue(endpoint);
            }
            else if (reader.GetTag() == kTagLabel)
            {
                CharSpan label;
                ReturnErrorOnFailure(reader.Get(label));
                if (label.size() <= kMaxNodeLabelLength)
                {
                    memcpy(node.label, label.data(), label.size());
                    node.labelLength = label.size();
                    node.hasLabel    = true;
                }
            }
        }
        VerifyOrReturnError(fieldErr == CHIP_END_OF_TLV, fieldErr);
        ReturnErrorOnFailure(reader.ExitContainer(nodeContainer));
        mCircuitNodeCount++;
    }
    VerifyOrReturnError(err == CHIP_END_OF_TLV, err);
    return reader.ExitContainer(arrayContainer);
}

CHIP_ERROR PowerTopologyCluster::Attributes(const ConcreteClusterPath & path,
                                            ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    DataModel::AttributeEntry optionalAttributes[] = {
        AvailableEndpoints::kMetadataEntry,     //
        ActiveEndpoints::kMetadataEntry,        //
        ElectricalCircuitNodes::kMetadataEntry, //
    };

    OptionalAttributeSet<                      //
        Attributes::AvailableEndpoints::Id,    //
        Attributes::ActiveEndpoints::Id,       //
        Attributes::ElectricalCircuitNodes::Id //
        >
        enabledOptionalAttributeSet;

    enabledOptionalAttributeSet.Set<Attributes::AvailableEndpoints::Id>(mFeatureFlags.Has(Feature::kSetTopology));
    enabledOptionalAttributeSet.Set<Attributes::ActiveEndpoints::Id>(mFeatureFlags.Has(Feature::kDynamicPowerFlow));
    enabledOptionalAttributeSet.Set<Attributes::ElectricalCircuitNodes::Id>(mFeatureFlags.Has(Feature::kElectricalCircuit));

    AttributeListBuilder listBuilder(builder);
    return listBuilder.Append(Span(kMandatoryMetadata), Span(optionalAttributes), enabledOptionalAttributeSet);
}

} // namespace PowerTopology
} // namespace Clusters
} // namespace app
} // namespace chip
