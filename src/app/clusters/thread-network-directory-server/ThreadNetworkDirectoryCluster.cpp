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

    // "It SHALL contain at least the following sub-TLVs: Active Timestamp, Channel, Channel Mask,
    // Extended PAN ID, Network Key, Network Mesh-Local Prefix, Network Name, PAN ID, PKSc, and Security Policy."
    CHIP_ERROR err;
    auto status = IMStatus::ConstraintError;
    SuccessOrExit(err = dataset.Init(req.operationalDataset));
    SuccessOrExit(err = dataset.GetExtendedPanIdAsByteSpan(extendedPanIdSpan));
    SuccessOrExit(err = dataset.GetActiveTimestamp(activeTimestamp));
    SuccessOrExit(err = dataset.GetChannel(unused.channel));
    SuccessOrExit(err = dataset.GetChannelMask(unusedSpan));
    SuccessOrExit(err = dataset.GetMasterKey(unused.masterKey));
    SuccessOrExit(err = dataset.GetMeshLocalPrefix(unused.meshLocalPrefix));
    SuccessOrExit(err = dataset.GetNetworkName(unused.networkName));
    SuccessOrExit(err = dataset.GetPanId(unused.panId));
    SuccessOrExit(err = dataset.GetPSKc(unused.pksc));
    SuccessOrExit(err = dataset.GetSecurityPolicy(unused.securityPolicy));

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
