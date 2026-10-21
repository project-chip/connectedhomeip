/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#pragma once

#include "descriptor_service/descriptor_service.rpc.pb.h"

#include <app/InteractionModelEngine.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <app/data-model-provider/Provider.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/ReadOnlyBuffer.h>
#include <lib/support/Span.h>
#include <platform/PlatformManager.h>

namespace chip {
namespace rpc {

class Descriptor : public pw_rpc::nanopb::Descriptor::Service<Descriptor>
{
public:
    virtual ~Descriptor() = default;

    virtual void DeviceTypeList(const ::chip_rpc_Endpoint & request, ServerWriter<::chip_rpc_DeviceType> & writer)
    {
        EndpointId endpointId = kInvalidEndpointId;
        VerifyOrReturn(TryGetEndpointId(request, endpointId), writer.Finish(pw::Status::InvalidArgument()));

        DeviceLayer::StackLock lock;
        app::DataModel::Provider * provider = app::InteractionModelEngine::GetInstance()->GetDataModelProvider();
        VerifyOrReturn(provider != nullptr, writer.Finish(pw::Status::Internal()));

        ReadOnlyBufferBuilder<app::DataModel::DeviceTypeEntry> builder;
        if (provider->DeviceTypes(endpointId, builder) != CHIP_NO_ERROR)
        {
            writer.Finish(pw::Status::InvalidArgument());
            return;
        }

        for (const auto & deviceType : builder.TakeBuffer())
        {
            chip_rpc_DeviceType out{ .device_type = deviceType.deviceTypeId };
            writer.Write(out);
        }

        writer.Finish();
    }

    void ServerList(const ::chip_rpc_Endpoint & request, ServerWriter<::chip_rpc_Cluster> & writer)
    {
        EndpointId endpointId = kInvalidEndpointId;
        VerifyOrReturn(TryGetEndpointId(request, endpointId), writer.Finish(pw::Status::InvalidArgument()));

        DeviceLayer::StackLock lock;
        app::DataModel::Provider * provider = app::InteractionModelEngine::GetInstance()->GetDataModelProvider();
        VerifyOrReturn(provider != nullptr, writer.Finish(pw::Status::Internal()));

        ReadOnlyBufferBuilder<app::DataModel::ServerClusterEntry> builder;
        if (provider->ServerClusters(endpointId, builder) != CHIP_NO_ERROR)
        {
            writer.Finish(pw::Status::InvalidArgument());
            return;
        }

        for (const auto & cluster : builder.TakeBuffer())
        {
            chip_rpc_Cluster out{ .cluster_id = cluster.clusterId };
            writer.Write(out);
        }
        writer.Finish();
    }

    void ClientList(const ::chip_rpc_Endpoint & request, ServerWriter<::chip_rpc_Cluster> & writer)
    {
        EndpointId endpointId = kInvalidEndpointId;
        VerifyOrReturn(TryGetEndpointId(request, endpointId), writer.Finish(pw::Status::InvalidArgument()));

        DeviceLayer::StackLock lock;
        app::DataModel::Provider * provider = app::InteractionModelEngine::GetInstance()->GetDataModelProvider();
        VerifyOrReturn(provider != nullptr, writer.Finish(pw::Status::Internal()));

        ReadOnlyBufferBuilder<ClusterId> builder;
        if (provider->ClientClusters(endpointId, builder) != CHIP_NO_ERROR)
        {
            writer.Finish(pw::Status::InvalidArgument());
            return;
        }

        for (const auto & clusterId : builder.TakeBuffer())
        {
            chip_rpc_Cluster out{ .cluster_id = clusterId };
            writer.Write(out);
        }
        writer.Finish();
    }

    void PartsList(const ::chip_rpc_Endpoint & request, ServerWriter<::chip_rpc_Endpoint> & writer)
    {
        EndpointId endpointId = kInvalidEndpointId;
        VerifyOrReturn(TryGetEndpointId(request, endpointId), writer.Finish(pw::Status::InvalidArgument()));

        DeviceLayer::StackLock lock;
        app::DataModel::Provider * provider = app::InteractionModelEngine::GetInstance()->GetDataModelProvider();
        VerifyOrReturn(provider != nullptr, writer.Finish(pw::Status::Internal()));

        ReadOnlyBufferBuilder<app::DataModel::EndpointEntry> endpointsBuilder;
        if (provider->Endpoints(endpointsBuilder) != CHIP_NO_ERROR)
        {
            writer.Finish(pw::Status::Internal());
            return;
        }
        auto endpoints = endpointsBuilder.TakeBuffer();

        if (endpointId == kRootEndpointId)
        {
            for (const auto & ep : endpoints)
            {
                if (ep.id != kRootEndpointId)
                {
                    chip_rpc_Endpoint out{ .endpoint = ep.id };
                    writer.Write(out);
                }
            }
            writer.Finish();
            return;
        }

        const app::DataModel::EndpointEntry * targetEndpoint = nullptr;
        for (const auto & ep : endpoints)
        {
            if (ep.id == endpointId)
            {
                targetEndpoint = &ep;
                break;
            }
        }

        if (targetEndpoint == nullptr)
        {
            writer.Finish(pw::Status::InvalidArgument());
            return;
        }

        switch (targetEndpoint->compositionPattern)
        {
        case app::DataModel::EndpointCompositionPattern::kTree:
            for (const auto & ep : endpoints)
            {
                if (ep.parentId == endpointId)
                {
                    chip_rpc_Endpoint out{ .endpoint = ep.id };
                    writer.Write(out);
                }
            }
            break;
        case app::DataModel::EndpointCompositionPattern::kFullFamily:
            for (const auto & ep : endpoints)
            {
                if (IsDescendantOf(&ep, endpointId, endpoints))
                {
                    chip_rpc_Endpoint out{ .endpoint = ep.id };
                    writer.Write(out);
                }
            }
            break;
        }
        writer.Finish();
    }

private:
    static constexpr bool TryGetEndpointId(const ::chip_rpc_Endpoint & request, EndpointId & outEndpointId)
    {
        if (request.endpoint >= kInvalidEndpointId)
        {
            return false;
        }
        outEndpointId = static_cast<EndpointId>(request.endpoint);
        return true;
    }

    static bool IsDescendantOf(const app::DataModel::EndpointEntry * childEndpoint, const EndpointId parentId,
                               Span<const app::DataModel::EndpointEntry> allEndpoints)
    {
        size_t depth = 0;
        while (depth < allEndpoints.size())
        {
            depth++;
            VerifyOrReturnValue(childEndpoint != nullptr, false);
            VerifyOrReturnValue(childEndpoint->parentId != parentId, true);
            VerifyOrReturnValue(childEndpoint->parentId != kInvalidEndpointId, false);

            const auto lookupId = childEndpoint->parentId;
            childEndpoint       = nullptr;

            for (const auto & ep : allEndpoints)
            {
                if (ep.id == lookupId)
                {
                    childEndpoint = &ep;
                    break;
                }
            }
        }
        return false;
    }
};

} // namespace rpc
} // namespace chip
