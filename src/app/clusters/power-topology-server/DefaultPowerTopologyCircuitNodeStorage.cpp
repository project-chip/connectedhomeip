/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <app/clusters/power-topology-server/DefaultPowerTopologyCircuitNodeStorage.h>

#include <clusters/PowerTopology/AttributeIds.h>
#include <clusters/PowerTopology/ClusterId.h>
#include <lib/core/TLV.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

#include <cstring>

namespace chip {
namespace app {
namespace Clusters {
namespace PowerTopology {

namespace {

// TLV tags for the persisted ElectricalCircuitNodes blob. This is a private storage format, not the
// wire encoding: the CircuitNodeStruct wire codec is fabric-aware and omits the fabric index on
// write, so persistence uses an explicit format that retains every field including fabricIndex.
constexpr TLV::Tag kTagFabricIndex = TLV::ContextTag(0);
constexpr TLV::Tag kTagNode        = TLV::ContextTag(1);
constexpr TLV::Tag kTagEndpoint    = TLV::ContextTag(2);
constexpr TLV::Tag kTagLabel       = TLV::ContextTag(3);

// Worst case ~ kMaxCircuitNodes * (node + endpoint + label + fabricIndex + TLV overhead).
constexpr size_t kCircuitNodesBlobSize = CircuitNodeStorage::kMaxCircuitNodes * (CircuitNodeStorage::kMaxNodeLabelLength + 32) + 8;

ConcreteAttributePath CircuitNodesPath(EndpointId endpointId)
{
    return ConcreteAttributePath(endpointId, PowerTopology::Id, Attributes::ElectricalCircuitNodes::Id);
}

} // namespace

CHIP_ERROR DefaultCircuitNodeStorage::Init(AttributePersistenceProvider & attributeStorage, EndpointId endpointId)
{
    VerifyOrReturnError(mNodes.Alloc(kMaxCircuitNodes), CHIP_ERROR_NO_MEMORY);

    mAttributeStorage = &attributeStorage;
    mEndpointId       = endpointId;
    mCount            = 0;

    // A missing value is normal on first boot and must not fail startup.
    CHIP_ERROR err = Load();
    if (err != CHIP_NO_ERROR && err != CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
    {
        ChipLogError(Zcl, "PowerTopology: failed to load ElectricalCircuitNodes: %" CHIP_ERROR_FORMAT, err.Format());
    }
    return CHIP_NO_ERROR;
}

size_t DefaultCircuitNodeStorage::CountForFabric(FabricIndex fabricIndex) const
{
    size_t count = 0;
    for (size_t i = 0; i < mCount; i++)
    {
        if (mNodes[i].fabricIndex == fabricIndex)
        {
            count++;
        }
    }
    return count;
}

CHIP_ERROR DefaultCircuitNodeStorage::GetNodeAtIndex(size_t index, Node & outNode) const
{
    VerifyOrReturnError(index < mCount, CHIP_ERROR_INVALID_ARGUMENT);
    outNode = mNodes[index];
    return CHIP_NO_ERROR;
}

size_t DefaultCircuitNodeStorage::EraseFabric(FabricIndex fabricIndex)
{
    size_t kept = 0;
    for (size_t i = 0; i < mCount; i++)
    {
        if (mNodes[i].fabricIndex != fabricIndex)
        {
            if (kept != i)
            {
                mNodes[kept] = mNodes[i];
            }
            kept++;
        }
    }
    const size_t removed = mCount - kept;
    mCount               = kept;
    return removed;
}

CHIP_ERROR DefaultCircuitNodeStorage::ReplaceNodesForFabric(FabricIndex fabricIndex, const Node * nodes, size_t count)
{
    VerifyOrReturnError(mAttributeStorage != nullptr, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(count == 0 || nodes != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    // Check capacity before mutating anything: on failure the stored contents must be unchanged.
    const size_t otherFabricCount = mCount - CountForFabric(fabricIndex);
    VerifyOrReturnError(otherFabricCount + count <= Capacity(), CHIP_ERROR_NO_MEMORY);

    EraseFabric(fabricIndex);
    for (size_t i = 0; i < count; i++)
    {
        mNodes[mCount++] = nodes[i];
    }
    return Save();
}

CHIP_ERROR DefaultCircuitNodeStorage::AppendNode(const Node & node)
{
    VerifyOrReturnError(mAttributeStorage != nullptr, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mCount < Capacity(), CHIP_ERROR_NO_MEMORY);

    mNodes[mCount++] = node;
    return Save();
}

CHIP_ERROR DefaultCircuitNodeStorage::RemoveNodesForFabric(FabricIndex fabricIndex)
{
    VerifyOrReturnError(mAttributeStorage != nullptr, CHIP_ERROR_INCORRECT_STATE);

    // Removing a fabric with no entries succeeds without a persist.
    VerifyOrReturnValue(EraseFabric(fabricIndex) > 0, CHIP_NO_ERROR);
    return Save();
}

CHIP_ERROR DefaultCircuitNodeStorage::Save() const
{
    VerifyOrReturnError(mAttributeStorage != nullptr, CHIP_ERROR_INCORRECT_STATE);

    Platform::ScopedMemoryBuffer<uint8_t> buffer;
    VerifyOrReturnError(buffer.Calloc(kCircuitNodesBlobSize), CHIP_ERROR_NO_MEMORY);

    TLV::TLVWriter writer;
    writer.Init(buffer.Get(), kCircuitNodesBlobSize);
    TLV::TLVType arrayContainer;
    ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Array, arrayContainer));
    for (size_t i = 0; i < mCount; i++)
    {
        const Node & node = mNodes[i];
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

    return mAttributeStorage->WriteValue(CircuitNodesPath(mEndpointId), ByteSpan(buffer.Get(), writer.GetLengthWritten()));
}

CHIP_ERROR DefaultCircuitNodeStorage::Load()
{
    VerifyOrReturnError(mAttributeStorage != nullptr, CHIP_ERROR_INCORRECT_STATE);

    Platform::ScopedMemoryBuffer<uint8_t> buffer;
    VerifyOrReturnError(buffer.Calloc(kCircuitNodesBlobSize), CHIP_ERROR_NO_MEMORY);

    MutableByteSpan span(buffer.Get(), kCircuitNodesBlobSize);
    ReturnErrorOnFailure(mAttributeStorage->ReadValue(CircuitNodesPath(mEndpointId), span));

    TLV::TLVReader reader;
    reader.Init(span);
    ReturnErrorOnFailure(reader.Next(TLV::kTLVType_Array, TLV::AnonymousTag()));
    TLV::TLVType arrayContainer;
    ReturnErrorOnFailure(reader.EnterContainer(arrayContainer));

    mCount = 0;
    CHIP_ERROR err;
    while ((err = reader.Next(TLV::kTLVType_Structure, TLV::AnonymousTag())) == CHIP_NO_ERROR)
    {
        // Silently stop at capacity rather than fail: a shrunk Capacity() must not brick startup.
        VerifyOrReturnError(mCount < Capacity(), CHIP_NO_ERROR);

        TLV::TLVType nodeContainer;
        ReturnErrorOnFailure(reader.EnterContainer(nodeContainer));

        Node node;
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
                VerifyOrReturnError(label.size() <= kMaxNodeLabelLength, CHIP_ERROR_INVALID_TLV_ELEMENT);
                memcpy(node.label, label.data(), label.size());
                node.labelLength = label.size();
                node.hasLabel    = true;
            }
        }
        VerifyOrReturnError(fieldErr == CHIP_END_OF_TLV, fieldErr);
        ReturnErrorOnFailure(reader.ExitContainer(nodeContainer));

        mNodes[mCount++] = node;
    }
    VerifyOrReturnError(err == CHIP_END_OF_TLV, err);
    return reader.ExitContainer(arrayContainer);
}

} // namespace PowerTopology
} // namespace Clusters
} // namespace app
} // namespace chip
