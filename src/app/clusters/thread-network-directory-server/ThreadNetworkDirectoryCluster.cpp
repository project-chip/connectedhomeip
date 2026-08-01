/**
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include <app/clusters/thread-network-directory-server/ThreadNetworkDirectoryCluster.h>

#include <app/MessageDef/StatusIB.h>
#include <app/SafeAttributePersistenceProvider.h>
#include <app/data-model/Nullable.h>
#include <app/reporting/reporting.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <clusters/ThreadNetworkDirectory/Metadata.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/ThreadOperationalDataset.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ThreadNetworkDirectory::Attributes;
using namespace chip::app::Clusters::ThreadNetworkDirectory::Commands;
using namespace chip::app::Clusters::ThreadNetworkDirectory::Structs;
using namespace chip::Thread;
using IMStatus = chip::Protocols::InteractionModel::Status;

namespace chip {
namespace app {
namespace Clusters {

ThreadNetworkDirectoryCluster::ThreadNetworkDirectoryCluster(EndpointId endpointId, ThreadNetworkDirectoryStorage & storage) :
    DefaultServerCluster({ endpointId, ThreadNetworkDirectory::Id }), mStorage(storage)
{}

CHIP_ERROR ThreadNetworkDirectoryCluster::Attributes(const ConcreteClusterPath & path,
                                                     ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    // This cluster only has Mandatory attributes
    AttributeListBuilder listBuilder(builder);
    return listBuilder.Append(Span(ThreadNetworkDirectory::Attributes::kMandatoryMetadata), {});
}

CHIP_ERROR ThreadNetworkDirectoryCluster::AcceptedCommands(const ConcreteClusterPath & path,
                                                           ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder)
{
    static constexpr DataModel::AcceptedCommandEntry kCommands[] = {
        ThreadNetworkDirectory::Commands::AddNetwork::kMetadataEntry,
        ThreadNetworkDirectory::Commands::RemoveNetwork::kMetadataEntry,
        ThreadNetworkDirectory::Commands::GetOperationalDataset::kMetadataEntry,
    };
    return builder.ReferenceExisting(Span(kCommands));
}

CHIP_ERROR ThreadNetworkDirectoryCluster::GeneratedCommands(const ConcreteClusterPath & path,
                                                            ReadOnlyBufferBuilder<CommandId> & builder)
{
    static constexpr CommandId kResponses[] = {
        ThreadNetworkDirectory::Commands::OperationalDatasetResponse::Id,
    };
    return builder.ReferenceExisting(Span(kResponses));
}

DataModel::ActionReturnStatus ThreadNetworkDirectoryCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                                           AttributeValueEncoder & encoder)
{
    switch (request.path.mAttributeId)
    {

    case PreferredExtendedPanID::Id:
        return ReadPreferredExtendedPanId(request.path, encoder);

    case ThreadNetworks::Id:
        return ReadThreadNetworks(request.path, encoder);

    case ThreadNetworkTableSize::Id:
        return encoder.Encode(mStorage.Capacity());

    case ClusterRevision::Id:
        return encoder.Encode(ThreadNetworkDirectory::kRevision);

    case FeatureMap::Id:
        return encoder.Encode(static_cast<uint32_t>(0));

    default:
        return IMStatus::UnsupportedAttribute;
    }
}

DataModel::ActionReturnStatus ThreadNetworkDirectoryCluster::WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                                            AttributeValueDecoder & decoder)
{
    if (request.path.mAttributeId == PreferredExtendedPanID::Id)
    {
        return WritePreferredExtendedPanId(request.path, decoder);
    }

    return IMStatus::UnsupportedAttribute;
}

CHIP_ERROR ThreadNetworkDirectoryCluster::ReadExtendedPanId(const ConcreteDataAttributePath & aPath,
                                                            std::optional<ExtendedPanId> & outExPanId)
{
    MutableByteSpan value(outExPanId.emplace().bytes);
    CHIP_ERROR err = GetSafeAttributePersistenceProvider()->SafeReadValue(aPath, value);
    if (err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
    {
        outExPanId.reset(); // default to empty
        return CHIP_NO_ERROR;
    }
    ReturnErrorOnFailure(err);

    if (value.size() == 0)
    {
        outExPanId.reset();
        return CHIP_NO_ERROR;
    }

    VerifyOrReturnError(value.size() == ExtendedPanId::size(), CHIP_ERROR_INTERNAL);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ThreadNetworkDirectoryCluster::ReadPreferredExtendedPanId(const ConcreteDataAttributePath & aPath,
                                                                     AttributeValueEncoder & aEncoder)
{
    std::optional<ExtendedPanId> value;
    ReturnErrorOnFailure(ReadExtendedPanId(aPath, value));
    return (value.has_value()) ? aEncoder.Encode(value.value().AsSpan()) : aEncoder.EncodeNull();
}

CHIP_ERROR ThreadNetworkDirectoryCluster::WritePreferredExtendedPanId(const ConcreteDataAttributePath & aPath,
                                                                      AttributeValueDecoder & aDecoder)
{
    DataModel::Nullable<ByteSpan> nullableValue;
    ReturnErrorOnFailure(aDecoder.Decode(nullableValue));

    // "A zero-length value SHALL be allowed for nullable values ... and SHALL have the same semantics as the null value."
    ByteSpan value = nullableValue.ValueOr(ByteSpan());
    // Ensure the provided value is valid (correct size) and refers to PAN from the list.
    VerifyOrReturnError(value.empty() || (value.size() == ExtendedPanId::size() && mStorage.ContainsNetwork(ExtendedPanId(value))),
                        StatusIB(IMStatus::ConstraintError).ToChipError());

    return GetSafeAttributePersistenceProvider()->SafeWriteValue(aPath, value);
}

CHIP_ERROR ThreadNetworkDirectoryCluster::ReadThreadNetworks(const ConcreteDataAttributePath &, AttributeValueEncoder & aEncoder)
{
    return aEncoder.EncodeList([this](const auto & encoder) -> CHIP_ERROR {
        CHIP_ERROR err = CHIP_NO_ERROR;
        ExtendedPanId exPanId;
        auto * iterator = mStorage.IterateNetworkIds();
        VerifyOrReturnError(iterator != nullptr, CHIP_ERROR_NO_MEMORY);
        while (iterator->Next(exPanId))
        {
            uint8_t datasetBuffer[kSizeOperationalDataset];
            MutableByteSpan datasetSpan(datasetBuffer);
            SuccessOrExit(err = mStorage.GetNetworkDataset(exPanId, datasetSpan));

            OperationalDataset dataset;
            char networkName[kSizeNetworkName + 1];
            ThreadNetworkStruct::Type network;

            SuccessOrExit(err = dataset.Init(datasetSpan));
            SuccessOrExit(err = dataset.GetExtendedPanIdAsByteSpan(network.extendedPanID));
            SuccessOrExit(err = dataset.GetNetworkName(networkName));
            network.networkName = CharSpan::fromCharString(networkName);
            SuccessOrExit(err = dataset.GetChannel(network.channel));
            SuccessOrExit(err = dataset.GetActiveTimestamp(network.activeTimestamp));

            SuccessOrExit(err = encoder.Encode(network));
        }
    exit:
        iterator->Release();
        return err;
    });
}

std::optional<DataModel::ActionReturnStatus> ThreadNetworkDirectoryCluster::InvokeCommand(const DataModel::InvokeRequest & request,
                                                                                          chip::TLV::TLVReader & input_arguments,
                                                                                          CommandHandler * handler)
{
    VerifyOrReturnError(handler != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    switch (request.path.mCommandId)
    {
    case AddNetwork::Id: {
        ThreadNetworkDirectory::Commands::AddNetwork::DecodableType addNetworkReq;
        ReturnErrorOnFailure(addNetworkReq.Decode(input_arguments));
        return HandleAddNetworkRequest(addNetworkReq);
    }
    case RemoveNetwork::Id: {
        ThreadNetworkDirectory::Commands::RemoveNetwork::DecodableType removeNetworkReq;
        ReturnErrorOnFailure(removeNetworkReq.Decode(input_arguments));
        return HandleRemoveNetworkRequest(removeNetworkReq);
    }
    case GetOperationalDataset::Id: {
        ThreadNetworkDirectory::Commands::GetOperationalDataset::DecodableType getOpDataset;
        ReturnErrorOnFailure(getOpDataset.Decode(input_arguments));
        return HandleOperationalDatasetRequest(*handler, getOpDataset, request.path);
    }
    default:
        return IMStatus::UnsupportedCommand;
    }
}

DataModel::ActionReturnStatus
ThreadNetworkDirectoryCluster::HandleAddNetworkRequest(const ThreadNetworkDirectory::Commands::AddNetwork::DecodableType & req)
{
    OperationalDatasetView dataset;
    ByteSpan extendedPanIdSpan;
    uint64_t activeTimestamp;

    CHIP_ERROR err;
    auto status = IMStatus::ConstraintError;
    SuccessOrExit(err = ValidateDatasetForDirectory(req.operationalDataset, extendedPanIdSpan, activeTimestamp));

    status = IMStatus::Failure;

    // "If the received dataset has an Active Timestamp that is less than or equal to that of the existing entry,
    // then the update SHALL be rejected with a status of INVALID_IN_STATE."
    {
        uint8_t datasetBuffer[kSizeOperationalDataset];
        MutableByteSpan datasetSpan(datasetBuffer);
        err = mStorage.GetNetworkDataset(ExtendedPanId(extendedPanIdSpan), datasetSpan);
        if (err != CHIP_ERROR_NOT_FOUND)
        {
            SuccessOrExit(err);
            SuccessOrExit(err = dataset.Init(datasetSpan));
            uint64_t existingActiveTimestamp;
            SuccessOrExit(err = dataset.GetActiveTimestamp(existingActiveTimestamp));
            if (activeTimestamp <= existingActiveTimestamp)
            {
                return IMStatus::InvalidInState;
            }
        }
    }

    SuccessOrExit(err = mStorage.AddOrUpdateNetwork(ExtendedPanId(extendedPanIdSpan), req.operationalDataset));

    NotifyAttributeChanged(ThreadNetworks::Id);
    return IMStatus::Success;

exit:
    ChipLogError(Zcl, "AddNetwork: %" CHIP_ERROR_FORMAT, err.Format());
    return (status == IMStatus::Failure && err == CHIP_ERROR_NO_MEMORY) ? IMStatus::ResourceExhausted : status;
}

ConcreteDataAttributePath ThreadNetworkDirectoryCluster::PreferredExtendedPanIdPath() const
{
    return ConcreteDataAttributePath(mPath.mEndpointId, ThreadNetworkDirectory::Id,
                                     ThreadNetworkDirectory::Attributes::PreferredExtendedPanID::Id);
}

// "It SHALL contain at least the following sub-TLVs: Active Timestamp, Channel, Channel Mask,
// Extended PAN ID, Network Key, Network Mesh-Local Prefix, Network Name, PAN ID, PKSc, and Security Policy."
CHIP_ERROR ThreadNetworkDirectoryCluster::ValidateDatasetForDirectory(ByteSpan dataset, ByteSpan & outExtendedPanId,
                                                                      uint64_t & outActiveTimestamp)
{
    OperationalDatasetView view;
    union
    {
        uint16_t channel;
        uint8_t masterKey[kSizeMasterKey];
        uint8_t meshLocalPrefix[kSizeMeshLocalPrefix];
        char networkName[kSizeNetworkName + 1];
        uint16_t panId;
        uint8_t pksc[kSizePSKc];
        uint32_t securityPolicy;
    } unused;
    ByteSpan unusedSpan;

    ReturnErrorOnFailure(view.Init(dataset));
    ReturnErrorOnFailure(view.GetExtendedPanIdAsByteSpan(outExtendedPanId));
    ReturnErrorOnFailure(view.GetActiveTimestamp(outActiveTimestamp));
    ReturnErrorOnFailure(view.GetChannel(unused.channel));
    ReturnErrorOnFailure(view.GetChannelMask(unusedSpan));
    ReturnErrorOnFailure(view.GetMasterKey(unused.masterKey));
    ReturnErrorOnFailure(view.GetMeshLocalPrefix(unused.meshLocalPrefix));
    ReturnErrorOnFailure(view.GetNetworkName(unused.networkName));
    ReturnErrorOnFailure(view.GetPanId(unused.panId));
    ReturnErrorOnFailure(view.GetPSKc(unused.pksc));
    ReturnErrorOnFailure(view.GetSecurityPolicy(unused.securityPolicy));
    return CHIP_NO_ERROR;
}

CHIP_ERROR ThreadNetworkDirectoryCluster::AddOrUpdateNetwork(const ThreadNetworkDirectoryStorage::ExtendedPanId & extendedPanId,
                                                             ByteSpan dataset)
{
    // An entry an application records has to satisfy the same constraints as
    // one a client adds, or a controller reading ThreadNetworks gets something
    // the cluster's own command would have rejected.
    ByteSpan datasetExtendedPanId;
    uint64_t activeTimestamp;
    ReturnErrorOnFailure(ValidateDatasetForDirectory(dataset, datasetExtendedPanId, activeTimestamp));
    VerifyOrReturnError(ExtendedPanId(datasetExtendedPanId) == extendedPanId, CHIP_ERROR_INVALID_ARGUMENT);

    // The increasing Active Timestamp rule AddNetwork enforces applies here too. A dataset that is
    // not newer than the stored one is not what the mesh converges on -- Thread orders datasets by
    // that timestamp -- so recording it would publish credentials no device on the network adopts,
    // whoever the writer is. Re-recording the stored dataset unchanged stays a no-op rather than an
    // error, so an application can report its border router's network without tracking what it has
    // already recorded.
    {
        uint8_t storedBuffer[kSizeOperationalDataset];
        MutableByteSpan stored(storedBuffer);
        CHIP_ERROR err = mStorage.GetNetworkDataset(extendedPanId, stored);
        if (err != CHIP_ERROR_NOT_FOUND)
        {
            ReturnErrorOnFailure(err);
            VerifyOrReturnValue(!stored.data_equal(dataset), CHIP_NO_ERROR);

            // Only the timestamp is read back, the way AddNetwork reads it: the stored dataset was
            // validated when it was recorded, and re-checking it here would let one that somehow
            // failed to parse block every later update to that network.
            OperationalDatasetView storedView;
            uint64_t storedActiveTimestamp;
            ReturnErrorOnFailure(storedView.Init(stored));
            ReturnErrorOnFailure(storedView.GetActiveTimestamp(storedActiveTimestamp));
            VerifyOrReturnError(activeTimestamp > storedActiveTimestamp, CHIP_ERROR_INCORRECT_STATE);
        }
    }

    ReturnErrorOnFailure(mStorage.AddOrUpdateNetwork(extendedPanId, dataset));
    NotifyAttributeChanged(ThreadNetworks::Id);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ThreadNetworkDirectoryCluster::ForgetNetwork(const ThreadNetworkDirectoryStorage::ExtendedPanId & extendedPanId)
{
    // "If not null, the value ... SHALL match the ExtendedPanID of a network in
    // the ThreadNetworks attribute". Clear the preference before the removal
    // rather than after: if the removal then fails, a null preference is still
    // a legal state, whereas one naming a network that is gone is not.
    std::optional<ExtendedPanId> preferred;
    ReturnErrorOnFailure(GetPreferredNetwork(preferred));
    const bool clearedPreference = preferred.has_value() && preferred.value() == extendedPanId;
    if (clearedPreference)
    {
        ReturnErrorOnFailure(SetPreferredNetwork(nullptr));
    }

    CHIP_ERROR err = mStorage.RemoveNetwork(extendedPanId);
    if (err != CHIP_NO_ERROR)
    {
        // The network is still listed, so the preference it carried is still
        // legal: put it back. If that write fails too, the cleared preference
        // is the legal remnant of the double fault, and the removal error is
        // the one worth reporting either way.
        if (clearedPreference)
        {
            (void) SetPreferredNetwork(&extendedPanId);
        }
        return err;
    }
    NotifyAttributeChanged(ThreadNetworks::Id);
    return CHIP_NO_ERROR;
}

CHIP_ERROR
ThreadNetworkDirectoryCluster::GetPreferredNetwork(std::optional<ThreadNetworkDirectoryStorage::ExtendedPanId> & extendedPanId)
{
    return ReadExtendedPanId(PreferredExtendedPanIdPath(), extendedPanId);
}

CHIP_ERROR ThreadNetworkDirectoryCluster::SetPreferredNetwork(const ThreadNetworkDirectoryStorage::ExtendedPanId * extendedPanId)
{
    // Same constraint the attribute write enforces: a preference has to name a
    // network that is actually in the list.
    VerifyOrReturnError(extendedPanId == nullptr || mStorage.ContainsNetwork(*extendedPanId), CHIP_ERROR_INVALID_ARGUMENT);

    const ByteSpan value = (extendedPanId != nullptr) ? ByteSpan(extendedPanId->bytes) : ByteSpan();
    ReturnErrorOnFailure(GetSafeAttributePersistenceProvider()->SafeWriteValue(PreferredExtendedPanIdPath(), value));
    NotifyAttributeChanged(ThreadNetworkDirectory::Attributes::PreferredExtendedPanID::Id);
    return CHIP_NO_ERROR;
}

DataModel::ActionReturnStatus ThreadNetworkDirectoryCluster::HandleRemoveNetworkRequest(
    const ThreadNetworkDirectory::Commands::RemoveNetwork::DecodableType & req)
{
    CHIP_ERROR err;

    if (req.extendedPanID.size() != ExtendedPanId::size())
    {
        return IMStatus::ConstraintError;
    }
    ExtendedPanId exPanId(req.extendedPanID);

    std::optional<ExtendedPanId> preferredExPanId;
    ConcreteReadAttributePath preferredExPanIdPath(mPath.mEndpointId, ThreadNetworkDirectory::Id,
                                                   ThreadNetworkDirectory::Attributes::PreferredExtendedPanID::Id);
    SuccessOrExit(err = ReadExtendedPanId(preferredExPanIdPath, preferredExPanId));
    if (preferredExPanId.has_value() && preferredExPanId.value() == exPanId)
    {
        ChipLogError(Zcl, "RemoveNetwork: Rejecting removal of preferred PAN");
        return IMStatus::ConstraintError;
    }

    SuccessOrExit(err = mStorage.RemoveNetwork(exPanId));

    NotifyAttributeChanged(ThreadNetworks::Id);
    return IMStatus::Success;

exit:
    ChipLogError(Zcl, "RemoveNetwork: %" CHIP_ERROR_FORMAT, err.Format());
    return (err == CHIP_ERROR_NOT_FOUND) ? IMStatus::NotFound : IMStatus::Failure;
}

std::optional<DataModel::ActionReturnStatus> ThreadNetworkDirectoryCluster::HandleOperationalDatasetRequest(
    CommandHandler & handler, const ThreadNetworkDirectory::Commands::GetOperationalDataset::DecodableType & req,
    const chip::app::ConcreteCommandPath & commandPath)
{
    CHIP_ERROR err;

    if (handler.GetSubjectDescriptor().authMode != Access::AuthMode::kCase)
    {
        return IMStatus::UnsupportedAccess;
    }

    if (req.extendedPanID.size() != ExtendedPanId::size())
    {
        return IMStatus::ConstraintError;
    }

    uint8_t datasetBuffer[kSizeOperationalDataset];
    MutableByteSpan datasetSpan(datasetBuffer);
    OperationalDatasetResponse::Type response;
    SuccessOrExit(err = mStorage.GetNetworkDataset(ExtendedPanId(req.extendedPanID), datasetSpan));
    response.operationalDataset = datasetSpan;
    handler.AddResponse(commandPath, response);
    return std::nullopt;
exit:
    ChipLogError(Zcl, "GetOperationalDataset: %" CHIP_ERROR_FORMAT, err.Format());
    return (err == CHIP_ERROR_NOT_FOUND) ? IMStatus::NotFound : IMStatus::Failure;
}

} // namespace Clusters
} // namespace app
} // namespace chip
