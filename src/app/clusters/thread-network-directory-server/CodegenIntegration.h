/**
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <app/clusters/thread-network-directory-server/DefaultThreadNetworkDirectoryStorage.h>
#include <app/clusters/thread-network-directory-server/ThreadNetworkDirectoryCluster.h>
#include <app/clusters/thread-network-directory-server/ThreadNetworkDirectoryStorage.h>
#include <app/server-cluster/ServerClusterInterfaceRegistry.h>
#include <app/server/Server.h>
#include <lib/core/CHIPError.h>

#include <optional>

namespace chip {
namespace app {
namespace Clusters {

/**
 * A ThreadNetworkDirectoryServer using DefaultThreadNetworkDirectoryStorage.
 */
class DefaultThreadNetworkDirectoryServer
{
public:
    DefaultThreadNetworkDirectoryServer(EndpointId endpoint,
                                        PersistentStorageDelegate & storage = Server::GetInstance().GetPersistentStorage()) :
        mStorage(storage),
        mCluster(endpoint, mStorage)
    {}

    ~DefaultThreadNetworkDirectoryServer();

    CHIP_ERROR Init();

private:
    DefaultThreadNetworkDirectoryStorage mStorage;
    RegisteredServerCluster<ThreadNetworkDirectoryCluster> mCluster;
};

} // namespace Clusters
} // namespace app
} // namespace chip
