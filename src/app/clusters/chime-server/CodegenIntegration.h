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

#pragma once

#include <app/clusters/chime-server/ChimeCluster.h>
#include <app/server-cluster/ServerClusterInterfaceRegistry.h>

namespace chip {
namespace app {
namespace Clusters {

class ChimeServer
{
public:
    /**
     * Creates a chime server instance. This is just a backwards compatibility wrapper around the ChimeCluster.
     * @param aEndpointId The endpoint on which this cluster exists. This must match the zap configuration.
     * @param aDelegate A reference to the delegate to be used by this server.
     * Note: the caller must ensure that the delegate lives throughout the instance's lifetime.
     */
    ChimeServer(EndpointId endpointId, ChimeDelegate & delegate);
    ~ChimeServer();

    /**
     * Register the chime cluster instance with the codegen data model provider.
     * @return Returns an error if registration fails.
     */
    CHIP_ERROR Init();

    // Attribute Setters
    /**
     * Sets the SelectedChime attribute. Note, this also handles writing the new value into non-volatile storage.
     * @param chimeSoundID The value to which the SelectedChime  is to be set.
     * @return Returns a ConstraintError if the chimeSoundID value is not valid. Returns Success otherwise.
     */
    Protocols::InteractionModel::Status SetSelectedChime(uint8_t chimeSoundID);

    /**
     * Sets the Enabled attribute. Note, this also handles writing the new value into non-volatile storage.
     * @param Enabled The value to which the Enabled  is to be set.
     */
    Protocols::InteractionModel::Status SetEnabled(bool Enabled);

    // Attribute Getters
    /**
     * @return The Current SelectedChime.
     */
    uint8_t GetSelectedChime() const;

    /**
     * @return The Enabled attribute..
     */
    bool GetEnabled() const;

    /**
     * @return The endpoint ID.
     */
    EndpointId GetEndpointId() { return mCluster.Cluster().GetPaths()[0].mEndpointId; }

    // Cluster constants from the spec
    static constexpr uint8_t kMaxChimeSoundNameSize = ChimeCluster::kMaxChimeSoundNameSize;

    // The Code Driven ChimeCluster instance
    chip::app::RegisteredServerCluster<ChimeCluster> mCluster;
};

} // namespace Clusters
} // namespace app
} // namespace chip
