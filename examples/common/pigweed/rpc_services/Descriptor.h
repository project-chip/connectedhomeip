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

#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/util/endpoint-config-api.h>
#include <platform/PlatformManager.h>

namespace chip {
namespace rpc {

class Descriptor : public pw_rpc::nanopb::Descriptor::Service<Descriptor>
{
public:
    virtual ~Descriptor() = default;

    virtual void DeviceTypeList(const ::chip_rpc_Endpoint & request, ServerWriter<::chip_rpc_DeviceType> & writer)
    {
        DeviceLayer::StackLock lock;
        CHIP_ERROR err;

        auto deviceTypeList = emberAfDeviceTypeListFromEndpoint(request.endpoint, err);
        if (err != CHIP_NO_ERROR)
        {
            writer.Finish(pw::Status::InvalidArgument());
            return;
        }

        if (deviceTypeList.size())
        {
            //
            // TODO: Need to update the Pigweed proto definition to actually represent this
            //       as a list of device types.
            //
            chip_rpc_DeviceType out{ .device_type = deviceTypeList.data()[0].deviceTypeId };
            writer.Write(out);
        }

        writer.Finish();
    }

    void ServerList(const ::chip_rpc_Endpoint & request, ServerWriter<::chip_rpc_Cluster> & writer)
    {
        DeviceLayer::StackLock lock;
        ClusterList(request.endpoint, true /*server*/, writer);
    }

    void ClientList(const ::chip_rpc_Endpoint & request, ServerWriter<::chip_rpc_Cluster> & writer)
    {
        DeviceLayer::StackLock lock;
        ClusterList(request.endpoint, false /*server*/, writer);
    }

    void PartsList(const ::chip_rpc_Endpoint & request, ServerWriter<::chip_rpc_Endpoint> & writer)
    {
        DeviceLayer::StackLock lock;
        if (request.endpoint == 0x00)
        {
            for (uint16_t index = 0; index < emberAfEndpointCount(); index++)
            {
                if (emberAfEndpointIndexIsEnabled(index))
                {
                    EndpointId endpoint_id = emberAfEndpointFromIndex(index);
                    if (endpoint_id == 0)
                        continue;
                    chip_rpc_Endpoint out{ .endpoint = endpoint_id };
                    writer.Write(out);
                }
            }
        }
        writer.Finish();
    }

private:
    void ClusterList(EndpointId endpoint, bool server, ServerWriter<::chip_rpc_Cluster> & writer)
    {
        uint8_t cluster_count = emberAfClusterCount(endpoint, server);

        for (uint8_t cluster_index = 0; cluster_index < cluster_count; cluster_index++)
        {
            const EmberAfCluster * cluster = emberAfGetNthCluster(endpoint, cluster_index, server);
            chip_rpc_Cluster out{ .cluster_id = cluster->clusterId };
            writer.Write(out);
        }
        writer.Finish();
    }
};

} // namespace rpc
} // namespace chip
