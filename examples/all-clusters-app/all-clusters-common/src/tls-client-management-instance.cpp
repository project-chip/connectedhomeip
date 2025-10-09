/*
 *
 *    Copyright (c) 2025 Matter Authors
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

#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/clusters/tls-certificate-management-server/CertificateTableImpl.h>
#include <app/clusters/tls-client-management-server/tls-client-management-server.h>
#include <clusters/TlsClientManagement/Commands.h>
#include <tls-client-management-instance.h>
#include <vector>

static constexpr uint8_t kMaxProvisioned = 254;

using namespace chip;
using namespace chip::app;
using namespace chip::app::DataModel;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::Tls;
using namespace chip::app::Clusters::TlsClientManagement;
using namespace Protocols::InteractionModel;

CHIP_ERROR TlsClientManagementCommandDelegate::GetProvisionedEndpointByIndex(EndpointId matterEndpoint, FabricIndex fabric,
                                                                             size_t index, EndpointStructType & outEndpoint) const
{
    VerifyOrReturnError(matterEndpoint == EndpointId(1), CHIP_ERROR_INTERNAL);

    size_t fabricI = 0;
    for (const auto & item : mProvisioned)
    {
        if (item.fabric == fabric)
        {
            if (fabricI++ == index)
            {
                outEndpoint = item.payload;
                return CHIP_NO_ERROR;
            }
        }
    }

    return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
}

uint16_t TlsClientManagementCommandDelegate::GetEndpointId(Provisioned * provisioned)
{
    uint16_t ret      = 0;
    uint16_t totalIds = 0;
    while (totalIds < UINT16_MAX)
    {
        bool idInUse = false;
        for (const auto & item : mProvisioned)
        {
            if (item.payload.endpointID == mNextId)
            {
                idInUse = true;
                totalIds++;
                if (totalIds == UINT16_MAX)
                {
                    break;
                }
                mNextId++;
                if (mNextId == 0)
                {
                    mNextId = 1;
                }
                break;
            }
        }
        if (!idInUse)
        {
            break;
        }
    }
    ret = (totalIds == UINT16_MAX) ? 0 : mNextId;

    return ret;
}

ClusterStatusCode TlsClientManagementCommandDelegate::ProvisionEndpoint(
    EndpointId matterEndpoint, FabricIndex fabric,
    const TlsClientManagement::Commands::ProvisionEndpoint::DecodableType & provisionReq, uint16_t & endpointID)
{
    VerifyOrReturnError(matterEndpoint == EndpointId(1), ClusterStatusCode(Status::ConstraintError));

    // Find existing value to update & check for port/name collisions
    Provisioned * provisioned = nullptr;
    uint16_t numInFabric      = 0;
    for (auto & item : mProvisioned)
    {
        const auto & endpointStruct = item.payload;
        if (!provisionReq.endpointID.IsNull() && endpointStruct.endpointID == provisionReq.endpointID.Value())
        {
            provisioned = &item;
            continue;
        }
        if (item.fabric == fabric)
        {
            numInFabric++;
            if (endpointStruct.hostname.data_equal(provisionReq.hostname) && (endpointStruct.port == provisionReq.port))
            {
                return ClusterStatusCode::ClusterSpecificFailure(StatusCodeEnum::kEndpointAlreadyInstalled);
            }
        }
    }

    if (provisionReq.endpointID.IsNull())
    {
        VerifyOrReturnError(numInFabric < mTlsClientManagementServer->GetMaxProvisioned(),
                            ClusterStatusCode(Status::ResourceExhausted));

        provisioned           = &mProvisioned.emplace_back();
        auto & endpointStruct = provisioned->payload;

        uint16_t nextId = GetEndpointId(provisioned);
        if (nextId == 0)
        {
            return ClusterStatusCode(Status::ResourceExhausted);
        }
        endpointStruct.endpointID = nextId;
        provisioned->fabric       = fabric;
        endpointID                = endpointStruct.endpointID;
    }
    // Updating existing value
    else if (provisioned == nullptr || provisioned->fabric != fabric)
    {
        return ClusterStatusCode(Status::NotFound);
    }
    else
    {
        endpointID = provisionReq.endpointID.Value();
    }

    auto & endpointStruct = provisioned->payload;
    provisioned->fabric   = fabric;
    ReturnValueOnFailure(endpointStruct.CopyHostnameFrom(provisionReq.hostname), ClusterStatusCode(Status::ConstraintError));
    endpointStruct.port           = provisionReq.port;
    endpointStruct.caid           = provisionReq.caid;
    endpointStruct.ccdid          = provisionReq.ccdid;
    endpointStruct.referenceCount = 0;
    endpointStruct.SetFabricIndex(fabric);

    return ClusterStatusCode(Status::Success);
}

Status TlsClientManagementCommandDelegate::FindProvisionedEndpointByID(EndpointId matterEndpoint, FabricIndex fabric,
                                                                       uint16_t endpointID, EndpointStructType & outEndpoint) const
{
    VerifyOrReturnError(matterEndpoint == EndpointId(1), Status::ConstraintError);

    for (auto i = mProvisioned.begin(); i != mProvisioned.end(); i++)
    {
        if (i->payload.endpointID == endpointID && i->fabric == fabric)
        {
            outEndpoint = i->payload;
            return Status::Success;
        }
    }

    return Status::NotFound;
}

Status TlsClientManagementCommandDelegate::RemoveProvisionedEndpointByID(EndpointId matterEndpoint, FabricIndex fabric,
                                                                         uint16_t endpointID)
{
    VerifyOrReturnError(matterEndpoint == EndpointId(1), Status::ConstraintError);

    auto i = mProvisioned.begin();
    for (; i != mProvisioned.end(); i++)
    {
        if (i->payload.endpointID == endpointID)
        {
            break;
        }
    }
    VerifyOrReturnError(i != mProvisioned.end() && i->fabric == fabric, Status::NotFound);
    VerifyOrReturnError(i->payload.referenceCount == 0, Status::InvalidInState);

    mProvisioned.erase(i);

    return Status::Success;
}

CHIP_ERROR TlsClientManagementCommandDelegate::RootCertCanBeRemoved(EndpointId matterEndpoint, FabricIndex fabric, Tls::TLSCAID id)
{
    auto i = mProvisioned.begin();
    for (; i != mProvisioned.end(); i++)
    {
        if (i->payload.caid == id)
        {
            return CHIP_ERROR_BAD_REQUEST;
        }
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR TlsClientManagementCommandDelegate::ClientCertCanBeRemoved(EndpointId matterEndpoint, FabricIndex fabric,
                                                                      Tls::TLSCCDID id)
{
    auto i = mProvisioned.begin();
    for (; i != mProvisioned.end(); i++)
    {
        if (i->payload.ccdid == id)
        {
            return CHIP_ERROR_BAD_REQUEST;
        }
    }
    return CHIP_NO_ERROR;
}

void TlsClientManagementCommandDelegate::RemoveFabric(FabricIndex fabric)
{
    for (auto i = mProvisioned.begin(); i != mProvisioned.end();)
    {
        if (i->fabric == fabric)
        {
            i = mProvisioned.erase(i);
        }
        else
        {
            ++i;
        }
    }
}

static CertificateTableImpl gCertificateTableInstance;
TlsClientManagementCommandDelegate TlsClientManagementCommandDelegate::instance;
static TlsClientManagementServer gTlsClientManagementClusterServerInstance = TlsClientManagementServer(
    EndpointId(1), TlsClientManagementCommandDelegate::GetInstance(), gCertificateTableInstance, kMaxProvisioned);

void emberAfTlsClientManagementClusterInitCallback(EndpointId matterEndpoint)
{
    gCertificateTableInstance.SetEndpoint(EndpointId(1));
    gTlsClientManagementClusterServerInstance.Init();
}

void emberAfTlsClientManagementClusterShutdownCallback(EndpointId matterEndpoint)
{
    gTlsClientManagementClusterServerInstance.Finish();
}
