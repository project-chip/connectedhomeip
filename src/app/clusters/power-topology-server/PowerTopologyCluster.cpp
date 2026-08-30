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
    VerifyOrReturnError(mCircuitNodeStorage != nullptr, CHIP_ERROR_INCORRECT_STATE);
    return aEncoder.EncodeList([this](const auto & encoder) -> CHIP_ERROR {
        const size_t count = mCircuitNodeStorage->Count();
        for (size_t i = 0; i < count; i++)
        {
            CircuitNodeStorage::Node node;
            ReturnErrorOnFailure(mCircuitNodeStorage->GetNodeAtIndex(i, node));
            ReturnErrorOnFailure(encoder.Encode(node.AsStruct()));
        }
        return CHIP_NO_ERROR;
    });
}

bool PowerTopologyCluster::DecodeCircuitNode(const Structs::CircuitNodeStruct::DecodableType & decoded, FabricIndex fabricIndex,
                                             CircuitNodeStorage::Node & out) const
{
    out          = CircuitNodeStorage::Node{};
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
        // The application owns ElectricalCircuitNodes storage; the cluster allocates nothing.
        VerifyOrReturnError(mCircuitNodeStorage != nullptr, CHIP_ERROR_INCORRECT_STATE,
                            ChipLogError(Zcl,
                                         "Power Topology Cluster: the ElectricalCircuit feature requires "
                                         "Config::circuitNodeStorage to be set"));
        ReturnErrorOnFailure(mCircuitNodeStorage->Init(context.attributeStorage, mPath.mEndpointId));

        // ElectricalCircuitNodes is fabric-scoped, so a removed fabric's entries must be purged.
        // Without the fabric table there is no OnFabricRemoved callback and they would leak, so
        // this is a hard requirement rather than a best effort.
        VerifyOrReturnError(mFabricTable != nullptr, CHIP_ERROR_INCORRECT_STATE,
                            ChipLogError(Zcl,
                                         "Power Topology Cluster: the ElectricalCircuit feature requires "
                                         "Config::fabricTable to be set"));
        ReturnErrorOnFailure(mFabricTable->AddFabricDelegate(this));
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
    VerifyOrReturn(mCircuitNodeStorage != nullptr);
    if (mCircuitNodeStorage->CountForFabric(fabricIndex) == 0)
    {
        return;
    }
    CHIP_ERROR err = mCircuitNodeStorage->RemoveNodesForFabric(fabricIndex);
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
    VerifyOrReturnError(mCircuitNodeStorage != nullptr, CHIP_ERROR_INCORRECT_STATE);

    const FabricIndex fabricIndex          = decoder.AccessingFabricIndex();
    const ConcreteDataAttributePath & path = request.path;

    if (!path.IsListItemOperation())
    {
        // ReplaceAll: replace ONLY the accessing fabric's slice; other fabrics' nodes are untouched.
        ElectricalCircuitNodes::TypeInfo::DecodableType list;
        ReturnErrorOnFailure(decoder.Decode(list));
        size_t newCount = 0;
        ReturnErrorOnFailure(list.ComputeSize(&newCount));

        const size_t otherFabricNodeCount = mCircuitNodeStorage->Count() - mCircuitNodeStorage->CountForFabric(fabricIndex);
        VerifyOrReturnValue(otherFabricNodeCount + newCount <= mCircuitNodeStorage->Capacity(), Status::ResourceExhausted);

        // Stage and validate the whole incoming list before touching storage, so a malformed entry
        // part-way through cannot leave the attribute partially written. Sized to the request rather
        // than to the list maximum.
        Platform::ScopedMemoryBuffer<CircuitNodeStorage::Node> staging;
        if (newCount > 0)
        {
            VerifyOrReturnValue(staging.Calloc(newCount), CHIP_ERROR_NO_MEMORY);
        }
        size_t stagingCount = 0;
        auto iter           = list.begin();
        while (iter.Next())
        {
            // ComputeSize() sized the buffer, so the iterator should not outrun it, and when
            // newCount is 0 the buffer was never allocated. Do not rely on the decoder agreeing
            // with itself for memory safety.
            VerifyOrReturnValue(stagingCount < newCount, Status::ConstraintError);
            VerifyOrReturnValue(DecodeCircuitNode(iter.GetValue(), fabricIndex, staging[stagingCount]), Status::ConstraintError);
            stagingCount++;
        }
        ReturnErrorOnFailure(iter.GetStatus());

        ReturnErrorOnFailure(mCircuitNodeStorage->ReplaceNodesForFabric(fabricIndex, staging.Get(), stagingCount));
    }
    else if (path.mListOp == ConcreteDataAttributePath::ListOperation::AppendItem)
    {
        Structs::CircuitNodeStruct::DecodableType value;
        ReturnErrorOnFailure(decoder.Decode(value));
        VerifyOrReturnValue(mCircuitNodeStorage->Count() < mCircuitNodeStorage->Capacity(), Status::ResourceExhausted);

        CircuitNodeStorage::Node node;
        VerifyOrReturnValue(DecodeCircuitNode(value, fabricIndex, node), Status::ConstraintError);
        ReturnErrorOnFailure(mCircuitNodeStorage->AppendNode(node));
    }
    else
    {
        return Status::UnsupportedWrite;
    }

    NotifyAttributeChanged(ElectricalCircuitNodes::Id);
    return CHIP_NO_ERROR;
}

CHIP_ERROR PowerTopologyCluster::Attributes(const ConcreteClusterPath & path,
                                            ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    AttributeListBuilder::OptionalAttributeEntry optionalAttributes[] = {
        { mFeatureFlags.Has(Feature::kSetTopology), AvailableEndpoints::kMetadataEntry },
        { mFeatureFlags.Has(Feature::kDynamicPowerFlow), ActiveEndpoints::kMetadataEntry },
        { mFeatureFlags.Has(Feature::kElectricalCircuit), ElectricalCircuitNodes::kMetadataEntry },
    };

    AttributeListBuilder listBuilder(builder);
    return listBuilder.Append(Span(kMandatoryMetadata), Span(optionalAttributes));
}

} // namespace PowerTopology
} // namespace Clusters
} // namespace app
} // namespace chip
