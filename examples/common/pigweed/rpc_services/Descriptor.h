/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "descriptor_service/descriptor_service.rpc.pb.h"

#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
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
            chip_rpc_DeviceType out{ .device_type = deviceTypeList.data()[0].deviceId };
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
