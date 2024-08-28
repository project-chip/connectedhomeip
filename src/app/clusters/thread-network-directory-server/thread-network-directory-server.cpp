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

#include "thread-network-directory-server.h"

#include <app/AttributeAccessInterfaceRegistry.h>
#include <app/CommandHandlerInterfaceRegistry.h>
#include <app/InteractionModelEngine.h>
#include <app/MessageDef/StatusIB.h>
#include <app/SafeAttributePersistenceProvider.h>
#include <app/data-model/Nullable.h>
#include <app/reporting/reporting.h>
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

ThreadNetworkDirectoryServer::ThreadNetworkDirectoryServer(EndpointId endpoint, ThreadNetworkDirectoryStorage & storage) :
    AttributeAccessInterface(MakeOptional(endpoint), ThreadNetworkDirectory::Id),
    CommandHandlerInterface(MakeOptional(endpoint), ThreadNetworkDirectory::Id), mStorage(storage)
{}

ThreadNetworkDirectoryServer::~ThreadNetworkDirectoryServer()
{
    AttributeAccessInterfaceRegistry::Instance().Unregister(this);
    CommandHandlerInterfaceRegistry::Instance().UnregisterCommandHandler(this);
}

CHIP_ERROR ThreadNetworkDirectoryServer::Init()
{
    VerifyOrReturnError(AttributeAccessInterfaceRegistry::Instance().Register(this), CHIP_ERROR_INTERNAL);
    ReturnErrorOnFailure(CommandHandlerInterfaceRegistry::Instance().RegisterCommandHandler(this));
    return CHIP_NO_ERROR;
}

CHIP_ERROR ThreadNetworkDirectoryServer::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    switch (aPath.mAttributeId)
    {
    case PreferredExtendedPanID::Id:
        return ReadPreferredExtendedPanId(aPath, aEncoder);
    case ThreadNetworks::Id:
        return ReadThreadNetworks(aPath, aEncoder);
    case ThreadNetworkTableSize::Id:
        return aEncoder.Encode(mStorage.Capacity());
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR ThreadNetworkDirectoryServer::Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder)
{
    switch (aPath.mAttributeId)
    {
    case PreferredExtendedPanID::Id:
        return WritePreferredExtendedPanId(aPath, aDecoder);
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR ThreadNetworkDirectoryServer::ReadExtendedPanId(const ConcreteDataAttributePath & aPath,
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

CHIP_ERROR ThreadNetworkDirectoryServer::ReadPreferredExtendedPanId(const ConcreteDataAttributePath & aPath,
                                                                    AttributeValueEncoder & aEncoder)
{
    std::optional<ExtendedPanId> value;
    ReturnErrorOnFailure(ReadExtendedPanId(aPath, value));
    return (value.has_value()) ? aEncoder.Encode(value.value().AsSpan()) : aEncoder.EncodeNull();
}

CHIP_ERROR ThreadNetworkDirectoryServer::WritePreferredExtendedPanId(const ConcreteDataAttributePath & aPath,
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

CHIP_ERROR ThreadNetworkDirectoryServer::ReadThreadNetworks(const ConcreteDataAttributePath &, AttributeValueEncoder & aEncoder)
{
    return aEncoder.EncodeList([this](const auto & encoder) -> CHIP_ERROR {
        CHIP_ERROR err = CHIP_NO_ERROR;
        ExtendedPanId exPanId;
        auto * iterator = mStorage.IterateNetworkIds();
        while (iterator->Next(exPanId))
        {
            uint8_t datasetBuffer[kSizeOperationalDataset];
            MutableByteSpan datasetSpan(datasetBuffer);
            SuccessOrExit(err = mStorage.GetNetworkDataset(exPanId, datasetSpan));

            OperationalDataset dataset;
            char networkName[kSizeNetworkName + 1];
            ThreadNetworkStruct::Type network;

            dataset.Init(datasetSpan);
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

void ThreadNetworkDirectoryServer::InvokeCommand(HandlerContext & ctx)
{
    switch (ctx.mRequestPath.mCommandId)
    {
    case AddNetwork::Id:
        HandleCommand<AddNetwork::DecodableType>(
            ctx, [this](HandlerContext & aCtx, const auto & req) { HandleAddNetworkRequest(aCtx, req); });
        return;
    case RemoveNetwork::Id:
        HandleCommand<RemoveNetwork::DecodableType>(
            ctx, [this](HandlerContext & aCtx, const auto & req) { HandleRemoveNetworkRequest(aCtx, req); });
        return;
    case GetOperationalDataset::Id:
        HandleCommand<GetOperationalDataset::DecodableType>(
            ctx, [this](HandlerContext & aCtx, const auto & req) { HandleOperationalDatasetRequest(aCtx, req); });
        return;
    }
}

void ThreadNetworkDirectoryServer::HandleAddNetworkRequest(HandlerContext & ctx,
                                                           const ThreadNetworkDirectory::Commands::AddNetwork::DecodableType & req)
{
    OperationalDataset dataset;
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
    auto status          = IMStatus::ConstraintError;
    const char * context = nullptr;
    // TODO: An immutable OperationalDatasetView on top of a ByteSpan (without copying) would be useful here.
    SuccessOrExitAction(err = dataset.Init(req.operationalDataset), context = "OperationalDataset");
    SuccessOrExitAction(err = dataset.GetExtendedPanIdAsByteSpan(extendedPanIdSpan), context = "ExtendedPanID");
    SuccessOrExitAction(err = dataset.GetActiveTimestamp(activeTimestamp), context = "ActiveTimestamp");
    SuccessOrExitAction(err = dataset.GetChannel(unused.channel), context = "Channel");
    SuccessOrExitAction(err = dataset.GetChannelMask(unusedSpan), context = "ChannelMask");
    SuccessOrExitAction(err = dataset.GetMasterKey(unused.masterKey), context = "NetworkKey");
    SuccessOrExitAction(err = dataset.GetMeshLocalPrefix(unused.meshLocalPrefix), context = "MeshLocalPrefix");
    SuccessOrExitAction(err = dataset.GetNetworkName(unused.networkName), context = "NetworkName");
    SuccessOrExitAction(err = dataset.GetPanId(unused.panId), context = "PanID");
    SuccessOrExitAction(err = dataset.GetPSKc(unused.pksc), context = "PKSc");
    SuccessOrExitAction(err = dataset.GetSecurityPolicy(unused.securityPolicy), context = "SecurityContext");

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
                ctx.mCommandHandler.AddStatus(ctx.mRequestPath, IMStatus::InvalidInState, "ActiveTimestamp");
                return;
            }
        }
    }

    SuccessOrExit(err = mStorage.AddOrUpdateNetwork(ExtendedPanId(extendedPanIdSpan), req.operationalDataset));

    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, IMStatus::Success);
    MatterReportingAttributeChangeCallback(GetEndpointId(), ThreadNetworkDirectory::Id, ThreadNetworks::Id);
    return;

exit:
    ChipLogError(Zcl, "AddNetwork: %" CHIP_ERROR_FORMAT, err.Format());
    ctx.mCommandHandler.AddStatus(
        ctx.mRequestPath, (status == IMStatus::Failure && err == CHIP_ERROR_NO_MEMORY) ? IMStatus::ResourceExhausted : status,
        context);
}

void ThreadNetworkDirectoryServer::HandleRemoveNetworkRequest(
    HandlerContext & ctx, const ThreadNetworkDirectory::Commands::RemoveNetwork::DecodableType & req)
{
    CHIP_ERROR err;

    if (req.extendedPanID.size() != ExtendedPanId::size())
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, IMStatus::ConstraintError);
        return;
    }
    ExtendedPanId exPanId(req.extendedPanID);

    std::optional<ExtendedPanId> preferredExPanId;
    ConcreteReadAttributePath preferredExPanIdPath(GetEndpointId(), ThreadNetworkDirectory::Id,
                                                   ThreadNetworkDirectory::Attributes::PreferredExtendedPanID::Id);
    SuccessOrExit(err = ReadExtendedPanId(preferredExPanIdPath, preferredExPanId));
    if (preferredExPanId.has_value() && preferredExPanId.value() == exPanId)
    {
        ChipLogError(Zcl, "RemoveNetwork: Rejecting removal of preferred PAN");
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, IMStatus::ConstraintError);
        return;
    }

    SuccessOrExit(err = mStorage.RemoveNetwork(exPanId));

    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, IMStatus::Success);
    MatterReportingAttributeChangeCallback(GetEndpointId(), ThreadNetworkDirectory::Id, ThreadNetworks::Id);
    return;

exit:
    ChipLogError(Zcl, "RemoveNetwork: %" CHIP_ERROR_FORMAT, err.Format());
    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, (err == CHIP_ERROR_NOT_FOUND) ? IMStatus::NotFound : IMStatus::Failure);
}

void ThreadNetworkDirectoryServer::HandleOperationalDatasetRequest(
    HandlerContext & ctx, const ThreadNetworkDirectory::Commands::GetOperationalDataset::DecodableType & req)
{
    CHIP_ERROR err;

    if (ctx.mCommandHandler.GetSubjectDescriptor().authMode != Access::AuthMode::kCase)
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, IMStatus::UnsupportedAccess);
        return;
    }

    if (req.extendedPanID.size() != ExtendedPanId::size())
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, IMStatus::ConstraintError);
        return;
    }

    uint8_t datasetBuffer[kSizeOperationalDataset];
    MutableByteSpan datasetSpan(datasetBuffer);
    OperationalDatasetResponse::Type response;
    SuccessOrExit(err = mStorage.GetNetworkDataset(ExtendedPanId(req.extendedPanID), datasetSpan));
    response.operationalDataset = datasetSpan;
    ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
    return;
exit:
    ChipLogError(Zcl, "GetOperationalDataset: %" CHIP_ERROR_FORMAT, err.Format());
    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, (err == CHIP_ERROR_NOT_FOUND) ? IMStatus::NotFound : IMStatus::Failure);
}

} // namespace Clusters
} // namespace app
} // namespace chip

void MatterThreadNetworkDirectoryPluginServerInitCallback() {}
