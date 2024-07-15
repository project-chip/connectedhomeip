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

#include <app-common/zap-generated/callback.h>
#include <app/clusters/thread-network-directory-server/DefaultThreadNetworkDirectoryStorage.h>
#include <app/clusters/thread-network-directory-server/thread-network-directory-server.h>
#include <app/server/Server.h>
#include <app/util/config.h>

#include <optional>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;

#if defined(MATTER_DM_WIFI_NETWORK_MANAGEMENT_CLUSTER_SERVER_ENDPOINT_COUNT) &&                                                    \
    MATTER_DM_WIFI_NETWORK_MANAGEMENT_CLUSTER_SERVER_ENDPOINT_COUNT == 1

namespace {
struct ThreadNetworkDirectoryServerDefaultInstance final
{
    ThreadNetworkDirectoryServerDefaultInstance(EndpointId endpoint, PersistentStorageDelegate & storageDelegate) :
        storage(storageDelegate), server(endpoint, storage)
    {}

    DefaultThreadNetworkDirectoryStorage storage;
    ThreadNetworkDirectoryServer server;
};
std::optional<ThreadNetworkDirectoryServerDefaultInstance> gThreadNetworkDirectoryServerInstance;
} // namespace

namespace chip {
namespace app {
ThreadNetworkDirectoryServer & ThreadNetworkDirectoryServer::DefaultInstance()
{
    return gThreadNetworkDirectoryServerInstance.value().server;
}
} // namespace app
} // namespace chip

__attribute__((weak)) // application can override to configure the cluster manually
void emberAfThreadNetworkDirectoryClusterServerInitCallback(chip::EndpointId endpoint)
{
    LogErrorOnFailure(
        gThreadNetworkDirectoryServerInstance.emplace(endpoint, Server::GetInstance().GetPersistentStorage()).server.Init());
}

#endif
