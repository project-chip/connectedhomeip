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

using namespace chip;
using namespace chip::app;
using namespace chip::app::DataModel;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::Tls;
using namespace chip::app::Clusters::TlsClientManagement;
using namespace Protocols::InteractionModel;

CHIP_ERROR TlsClientManagementCommandDelegate::GetProvisionedEndpointByIndex(EndpointId matterEndpoint, FabricIndex fabric,
                                                                             size_t index, EndpointStructType & endpoint)
{
    VerifyOrReturnError(matterEndpoint == EndpointId(1), CHIP_ERROR_INTERNAL);

    size_t fabricI = 0;
    for (auto i = mProvisioned.begin(); i != mProvisioned.end(); i++)
    {
        if (i->fabric == fabric)
        {
            if (fabricI++ == index)
            {
                endpoint = i->payload;
                return CHIP_NO_ERROR;
            }
        }
    }

    return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
}

Status TlsClientManagementCommandDelegate::ProvisionEndpoint(
    EndpointId matterEndpoint, FabricIndex fabric,
    const TlsClientManagement::Commands::ProvisionEndpoint::DecodableType & provisionReq, uint16_t & endpointID)
{
    VerifyOrReturnError(matterEndpoint == EndpointId(1), Status::ConstraintError);

    if (mCertificateTable.HasRootCertificateEntry(fabric, provisionReq.caid) != CHIP_NO_ERROR)
    {
        return Status::ConstraintError;
    }
    if (!provisionReq.ccdid.IsNull() &&
        mCertificateTable.HasClientCertificateEntry(fabric, provisionReq.ccdid.Value()) != CHIP_NO_ERROR)
    {
        return Status::ConstraintError;
    }
    Provisioned * provisioned;
    if (!provisionReq.endpointID.IsNull())
    {
        // Updating existing value
        auto i = mProvisioned.begin();
        for (; i != mProvisioned.end(); i++)
        {
            if (i->payload.endpointID == provisionReq.endpointID.Value())
            {
                break;
            }
        }
        if (i == mProvisioned.end())
        {
            return Status::NotFound;
        }
        provisioned = &*i;
        if (provisioned->fabric != fabric)
        {
            return Status::ConstraintError;
        }
    }
    else
    {
        provisioned           = &mProvisioned.emplace_back();
        auto & endpointStruct = provisioned->payload;

        endpointStruct.endpointID = mNextId++;
    }
    provisioned->fabric   = fabric;
    auto & endpointStruct = provisioned->payload;

    endpointStruct.hostname = provisionReq.hostname;
    endpointStruct.port     = provisionReq.port;
    endpointStruct.caid     = provisionReq.caid;
    endpointStruct.ccdid    = provisionReq.ccdid;

    return Status::Success;
}

Status TlsClientManagementCommandDelegate::FindProvisionedEndpointByID(EndpointId matterEndpoint, FabricIndex fabric,
                                                                       uint16_t endpointID, EndpointStructType & endpoint)
{
    VerifyOrReturnError(matterEndpoint == EndpointId(1), Status::ConstraintError);

    for (auto i = mProvisioned.begin(); i != mProvisioned.end(); i++)
    {
        if (i->payload.endpointID == endpointID && i->fabric == fabric)
        {
            endpoint = i->payload;
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
    if (i == mProvisioned.end() || i->fabric != fabric)
    {
        return Status::NotFound;
    }
    mProvisioned.erase(i);

    return Status::Success;
}

static CertificateTableImpl gCertificateTableInstance;
TlsClientManagementCommandDelegate TlsClientManagementCommandDelegate::instance(gCertificateTableInstance);
static TlsClientManagementServer gTlsClientManagementClusterServerInstance =
    TlsClientManagementServer(EndpointId(1), TlsClientManagementCommandDelegate::getInstance(), gCertificateTableInstance);

void emberAfTlsClientManagementClusterInitCallback(EndpointId matterEndpoint)
{
    gCertificateTableInstance.SetEndpoint(EndpointId(1));
    gTlsClientManagementClusterServerInstance.Init();
}

void emberAfTlsClientManagementClusterShutdownCallback(EndpointId matterEndpoint)
{
    gTlsClientManagementClusterServerInstance.Finish();
}
