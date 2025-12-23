/*
 *
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

/****************************************************************************'
 * @file
 * @brief Implementation for the TlsClientManagement Server Cluster
 ***************************************************************************/

#include "TlsClientManagementCluster.h"

#include <app/ConcreteAttributePath.h>
#include <app/ConcreteCommandPath.h>
#include <app/SafeAttributePersistenceProvider.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <app/server/Server.h>
#include <clusters/TlsClientManagement/Attributes.h>
#include <clusters/TlsClientManagement/Commands.h>
#include <clusters/TlsClientManagement/Metadata.h>
#include <clusters/TlsClientManagement/Structs.h>
#include <protocols/interaction_model/StatusCode.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::Tls;
using namespace chip::app::Clusters::TlsClientManagement;
using namespace chip::app::Clusters::TlsClientManagement::Structs;
using namespace chip::app::Clusters::TlsClientManagement::Attributes;
using namespace Protocols::InteractionModel;

static constexpr uint16_t kMaxTlsEndpointId = 65534;
// Minimum hostname length of 4 allows for shortest valid format: single char + dot + 2-letter TLD (e.g., "a.bc")
static constexpr size_t kMinHostnameLength = 4;

constexpr DataModel::AcceptedCommandEntry kAcceptedCommands[] = {
    Commands::ProvisionEndpoint::kMetadataEntry,
    Commands::FindEndpoint::kMetadataEntry,
    Commands::RemoveEndpoint::kMetadataEntry,
};

TlsClientManagementCluster::TlsClientManagementCluster(EndpointId endpointId, TlsClientManagementDelegate & delegate,
                                                       CertificateTable & certificateTable, uint8_t maxProvisioned) :
    DefaultServerCluster({ endpointId, TlsClientManagement::Id }),
    mDelegate(delegate), mCertificateTable(certificateTable), mMaxProvisioned(maxProvisioned)
{
    VerifyOrDieWithMsg(mMaxProvisioned >= 5, NotSpecified, "Spec requires MaxProvisioned be >= 5");
    VerifyOrDieWithMsg(mMaxProvisioned <= 254, NotSpecified, "Spec requires MaxProvisioned be <= 254");
    mDelegate.SetTlsClientManagementCluster(this);
}

TlsClientManagementCluster::~TlsClientManagementCluster()
{
    // null out the ref to us on the delegate
    mDelegate.SetTlsClientManagementCluster(nullptr);
}

CHIP_ERROR TlsClientManagementCluster::Startup(ServerClusterContext & context)
{
    ChipLogProgress(DataManagement, "TlsClientManagementCluster: initializing");
    ReturnErrorOnFailure(DefaultServerCluster::Startup(context));

    ReturnErrorOnFailure(mCertificateTable.Init(context.storage));
    ReturnErrorOnFailure(mDelegate.Init(context.storage));

    return Server::GetInstance().GetFabricTable().AddFabricDelegate(this);
}

void TlsClientManagementCluster::Shutdown(ClusterShutdownType)
{
    ChipLogProgress(DataManagement, "TlsClientManagementCluster: shutdown");

    mCertificateTable.Finish();
    Server::GetInstance().GetFabricTable().RemoveFabricDelegate(this);

    DefaultServerCluster::Shutdown(ClusterShutdownType::kClusterShutdown);
}

DataModel::ActionReturnStatus TlsClientManagementCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                                        AttributeValueEncoder & encoder)
{
    VerifyOrDie(request.path.mClusterId == TlsClientManagement::Id);

    switch (request.path.mAttributeId)
    {
    case MaxProvisioned::Id:
        return encoder.Encode(mMaxProvisioned);
    case ProvisionedEndpoints::Id: {
        TlsClientManagementCluster * server = this;
        auto matterEndpoint                 = request.path.mEndpointId;
        auto fabric                         = request.GetAccessingFabricIndex();
        return encoder.EncodeList([server, matterEndpoint, fabric](const auto & listEncoder) -> CHIP_ERROR {
            return server->EncodeProvisionedEndpoints(matterEndpoint, fabric, listEncoder);
        });
    }
    case ClusterRevision::Id:
        return encoder.Encode(kRevision);
    case FeatureMap::Id:
        // TODO: Allow delegate to specify supported features
        // Currently hardcoded to 0 (no features supported)
        // METADATA feature (bit 0) should be configurable based on delegate capabilities
        return encoder.Encode<uint32_t>(0);
    default:
        return Status::UnsupportedAttribute;
    }
}

CHIP_ERROR TlsClientManagementCluster::Attributes(const ConcreteClusterPath & path,
                                                  ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    AttributeListBuilder listBuilder(builder);
    // TlsClientManagement does not have optional attributes implemented yet,
    // so we just return mandatory ones.
    return listBuilder.Append(Span(kMandatoryMetadata), {});
}

CHIP_ERROR TlsClientManagementCluster::AcceptedCommands(const ConcreteClusterPath & path,
                                                        ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder)
{
    return builder.ReferenceExisting(kAcceptedCommands);
}

uint8_t TlsClientManagementCluster::GetMaxProvisioned() const
{
    return mMaxProvisioned;
}

// helper method to get the TlsClientManagement provisioned endpoints encoded into a list
CHIP_ERROR
TlsClientManagementCluster::EncodeProvisionedEndpoints(EndpointId matterEndpoint, FabricIndex fabric,
                                                       const AttributeValueEncoder::ListEncodeHelper & encoder)
{
    return mDelegate.ForEachEndpoint(matterEndpoint, fabric,
                                     [&](auto & endpoint) -> CHIP_ERROR { return encoder.Encode(endpoint); });
}

std::optional<DataModel::ActionReturnStatus> TlsClientManagementCluster::InvokeCommand(const DataModel::InvokeRequest & request,
                                                                                       chip::TLV::TLVReader & input_arguments,
                                                                                       CommandHandler * handler)
{
    FabricIndex accessingFabricIndex = request.GetAccessingFabricIndex();

    switch (request.path.mCommandId)
    {
    case Commands::ProvisionEndpoint::Id: {
        Commands::ProvisionEndpoint::DecodableType req;
        ReturnErrorOnFailure(req.Decode(input_arguments, accessingFabricIndex));
        return HandleProvisionEndpoint(*handler, req);
    }
    case Commands::FindEndpoint::Id: {
        Commands::FindEndpoint::DecodableType req;
        ReturnErrorOnFailure(req.Decode(input_arguments, accessingFabricIndex));
        return HandleFindEndpoint(*handler, req);
    }
    case Commands::RemoveEndpoint::Id: {
        Commands::RemoveEndpoint::DecodableType req;
        ReturnErrorOnFailure(req.Decode(input_arguments, accessingFabricIndex));
        return HandleRemoveEndpoint(*handler, req);
    }
    default:
        return Status::UnsupportedCommand;
    }
}

std::optional<DataModel::ActionReturnStatus>
TlsClientManagementCluster::HandleProvisionEndpoint(CommandHandler & commandHandler,
                                                    const Commands::ProvisionEndpoint::DecodableType & req)
{
    ChipLogDetail(Zcl, "TlsClientManagement: ProvisionEndpoint");

    if (req.hostname.size() < kMinHostnameLength || req.hostname.size() > kSpecMaxHostname)
    {
        return Status::ConstraintError;
    }

    if (req.caid > kMaxRootCertId)
    {
        return Status::ConstraintError;
    }

    auto fabric = commandHandler.GetAccessingFabricIndex();
    if (mCertificateTable.HasRootCertificateEntry(fabric, req.caid) != CHIP_NO_ERROR)
    {
        return ClusterStatusCode::ClusterSpecificFailure(StatusCodeEnum::kRootCertificateNotFound);
    }
    if (!req.ccdid.IsNull() && mCertificateTable.HasClientCertificateEntry(fabric, req.ccdid.Value()) != CHIP_NO_ERROR)
    {
        return ClusterStatusCode::ClusterSpecificFailure(StatusCodeEnum::kClientCertificateNotFound);
    }

    Commands::ProvisionEndpointResponse::Type response;
    auto status = mDelegate.ProvisionEndpoint(mPath.mEndpointId, fabric, req, response.endpointID);

    if (status.IsSuccess())
    {
        ConcreteCommandPath responsePath(mPath.mEndpointId, TlsClientManagement::Id, Commands::ProvisionEndpointResponse::Id);
        commandHandler.AddResponse(responsePath, response);
        NotifyAttributeChanged(TlsClientManagement::Attributes::ProvisionedEndpoints::Id);
        return std::nullopt;
    }

    return status;
}

std::optional<DataModel::ActionReturnStatus>
TlsClientManagementCluster::HandleFindEndpoint(CommandHandler & commandHandler, const Commands::FindEndpoint::DecodableType & req)
{
    ChipLogDetail(Zcl, "TlsClientManagement: FindEndpoint");

    VerifyOrReturnError(req.endpointID <= kMaxTlsEndpointId, Status::ConstraintError);

    auto fabric     = commandHandler.GetAccessingFabricIndex();
    auto endpointId = mPath.mEndpointId;

    CHIP_ERROR result =
        mDelegate.FindProvisionedEndpointByID(endpointId, fabric, req.endpointID, [&](auto & endpoint) -> CHIP_ERROR {
            Commands::FindEndpointResponse::Type response;
            response.endpoint = endpoint;
            ConcreteCommandPath responsePath(mPath.mEndpointId, TlsClientManagement::Id, Commands::FindEndpointResponse::Id);
            commandHandler.AddResponse(responsePath, response);
            return CHIP_NO_ERROR;
        });

    if (result == CHIP_ERROR_NOT_FOUND)
    {
        return Status::NotFound;
    }

    if (result != CHIP_NO_ERROR)
    {
        return Status::Failure;
    }

    return std::nullopt;
}

std::optional<DataModel::ActionReturnStatus>
TlsClientManagementCluster::HandleRemoveEndpoint(CommandHandler & commandHandler,
                                                 const Commands::RemoveEndpoint::DecodableType & req)
{
    ChipLogDetail(Zcl, "TlsClientManagement: RemoveEndpoint");

    if (req.endpointID > kMaxTlsEndpointId)
    {
        return Status::ConstraintError;
    }

    auto fabric     = commandHandler.GetAccessingFabricIndex();
    auto endpointId = mPath.mEndpointId;

    auto status = mDelegate.RemoveProvisionedEndpointByID(endpointId, fabric, req.endpointID);

    if (status == Status::Success)
    {
        NotifyAttributeChanged(TlsClientManagement::Attributes::ProvisionedEndpoints::Id);
    }

    return status;
}

void TlsClientManagementCluster::OnFabricRemoved(const FabricTable & fabricTable, FabricIndex fabricIndex)
{
    TEMPORARY_RETURN_IGNORED mDelegate.RemoveFabric(fabricIndex);
}
