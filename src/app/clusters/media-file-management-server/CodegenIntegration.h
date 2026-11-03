/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include <app/clusters/media-file-management-server/MediaFileManagementCluster.h>
#include <app/server-cluster/ServerClusterInterfaceRegistry.h>

namespace chip::app::Clusters::MediaFileManagement {

/**
 * Wrapper around MediaFileManagementCluster that integrates with the ZAP/Ember
 * generated configuration for applications that use the codegen data model.
 *
 * The feature map is read from the ZAP attribute store at construction time.
 * Applications instantiate one MediaFileManagementServer per endpoint that hosts
 * the cluster, then call Init() to register it with the data model provider.
 *
 * NEW CODE that does not use ZAP should use MediaFileManagementCluster directly.
 */
class MediaFileManagementServer
{
public:
    MediaFileManagementServer(EndpointId endpointId, Delegate & delegate);
    ~MediaFileManagementServer();

    /// Register the cluster instance with the codegen data model provider.
    CHIP_ERROR Init();

    /// Unregister the cluster instance from the data model provider.
    void Shutdown();

    MediaFileManagementCluster & Cluster() { return mCluster.Cluster(); }

private:
    bool mRegistered = false;
    RegisteredServerCluster<MediaFileManagementCluster> mCluster;
};

} // namespace chip::app::Clusters::MediaFileManagement
