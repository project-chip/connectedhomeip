/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <app/CASESessionManager.h>
#include <app/server/Server.h>
#include <app/util/binding-table.h>

namespace chip {

using BoundDeviceChangedHandler = void (*)(EndpointId localEndpoint, EndpointId remoteEndpoint, ClusterId clusterId,
                                           OperationalDeviceProxy * peer_device);

class BindingManager
{
public:
    BindingManager() :
        mOnConnectedCallback(HandleDeviceConnected, this), mOnConnectionFailureCallback(HandleDeviceConnectionFailure, this)
    {}

    void RegisterBoundDeviceChangedHandler(BoundDeviceChangedHandler handler) { mBoundDeviceChangedHandler = handler; }

    void SetAppServer(Server * appServer) { mAppServer = appServer; }

    CHIP_ERROR CreateBinding(FabricIndex fabric, NodeId node, EndpointId localEndpoint, ClusterId cluster);

    CHIP_ERROR DisconnectDevice(FabricIndex fabric, NodeId node);

    CHIP_ERROR NotifyBoundClusterChanged(EndpointId endpoint, ClusterId cluster);

    static BindingManager & GetInstance() { return sBindingManager; }

private:
    static BindingManager sBindingManager;

    static constexpr uint8_t kMaxPendingClusters = 5;

    struct ClusterPath
    {
        EndpointId endpoint;
        ClusterId cluster;
    };

    class PendingClusterEntry
    {
    public:
        PeerId mPeerId;

        ClusterPath mPendingClusters[kMaxPendingClusters];
        uint8_t mNumPendingClusters = 0;

        System::Clock::Timestamp mLastUpdateTime;

        void AddPendingCluster(EndpointId endpoint, ClusterId cluster)
        {
            if (mNumPendingClusters < kMaxPendingClusters)
            {
                mPendingClusters[mNumPendingClusters++] = { endpoint, cluster };
            }
            else
            {
                mPendingClusters[mNumPendingClusters++] = { endpoint, cluster };
                mNextToOverride++;
                mNextToOverride %= kMaxPendingClusters;
            }
        }

    private:
        uint8_t mNextToOverride = 0;
    };

    class PendingClusterMap
    {
    public:
        PendingClusterEntry * FindLRUEntry();

        PendingClusterEntry * FindEntry(PeerId peerId);

        CHIP_ERROR AddPendingCluster(PeerId peer, EndpointId endpoint, ClusterId cluster);

        void RemoveEntry(PendingClusterEntry * entry) { mPendingClusterMap.ReleaseObject(entry); }

        template <typename Function>
        Loop ForEachActiveObject(Function && function)
        {
            return mPendingClusterMap.ForEachActiveObject(std::forward<Function>(function));
        }

    private:
        BitMapObjectPool<PendingClusterEntry, EMBER_BINDING_TABLE_SIZE> mPendingClusterMap;
    };

    static void HandleDeviceConnected(void * context, OperationalDeviceProxy * device);
    void HandleDeviceConnected(OperationalDeviceProxy * device);

    static void HandleDeviceConnectionFailure(void * context, NodeId nodeId, CHIP_ERROR error);
    void HandleDeviceConnectionFailure(NodeId nodeId, CHIP_ERROR error);

    void SyncPendingClustersToPeer(OperationalDeviceProxy * device, PendingClusterEntry * pendingClusters);

    CHIP_ERROR EnqueueClusterAndConnect(FabricIndex fabric, NodeId node, EndpointId endpoint, ClusterId cluster);

    PendingClusterMap mPendingClusterMap;
    BoundDeviceChangedHandler mBoundDeviceChangedHandler;
    Server * mAppServer;

    Callback::Callback<OnDeviceConnected> mOnConnectedCallback;
    Callback::Callback<OnDeviceConnectionFailure> mOnConnectionFailureCallback;
};

} // namespace chip
