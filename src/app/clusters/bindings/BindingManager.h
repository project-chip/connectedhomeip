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

/**
 * Application callback function when a cluster associated with a bound peer changes.
 *
 * The connection is managed by the stack and peer_device is guaranteed to be available.
 * The application shall decide the content to be sent to the peer.
 *
 * E.g. The application will send on/off commands to peer for the OnOff cluster.
 *
 */
using BoundDeviceChangedHandler = void (*)(EndpointId localEndpoint, EndpointId remoteEndpoint, ClusterId clusterId,
                                           OperationalDeviceProxy * peer_device);

/**
 *
 * The BindingManager class manages the CASEConnection and attribute writes to bound peers.
 *
 * The binding cluster will tell the device about the peer to connect to and the cluster to watch.
 * A CASE connection will be triggered when:
 *  - The binding cluster adds an entry to the binding table.
 *  - A watched cluster changes but we cannot find an active connection to the peer.
 *
 * The class uses an LRU mechanism to choose the connection to eliminate when there is no space for a new connection.
 * The BindingManager class will not actively re-establish connection and will connect on-demand (when binding cluster
 * or watched cluster is changed).
 *
 */
class BindingManager
{
public:
    BindingManager() :
        mOnConnectedCallback(HandleDeviceConnected, this), mOnConnectionFailureCallback(HandleDeviceConnectionFailure, this)
    {}

    void RegisterBoundDeviceChangedHandler(BoundDeviceChangedHandler handler) { mBoundDeviceChangedHandler = handler; }

    void SetAppServer(Server * appServer) { mAppServer = appServer; }

    /*
     * Adds a binding entry. Can be called for adding a new cluster to a node already connected.
     *
     */
    CHIP_ERROR CreateBinding(FabricIndex fabric, NodeId node, EndpointId localEndpoint, ClusterId cluster);

    /*
     * Totally disconnect a device. Call this function only when no binding table entry is associated with the node.
     *
     */
    CHIP_ERROR DisconnectDevice(FabricIndex fabric, NodeId node);

    /*
     * Notify a cluster change to **all** bound devices associated with the cluster.
     *
     */
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

    // A pending command to be sent to a bound peer waiting for the CASE session to be established.
    class PendingClusterEntry
    {
    public:
        PendingClusterEntry(PeerId peerId) : mPeerId(peerId) {}

        PeerId GetPeerId() { return mPeerId; }

        System::Clock::Timestamp GetLastUpdateTime() { return mLastUpdateTime; }
        void Touch() { mLastUpdateTime = System::SystemClock().GetMonotonicTimestamp(); }

        ClusterPath * begin() { return &mPendingClusters[0]; }
        ClusterPath * end() { return &mPendingClusters[mNumPendingClusters]; }

        void AddPendingCluster(EndpointId endpoint, ClusterId cluster)
        {
            if (mNumPendingClusters < kMaxPendingClusters)
            {
                mPendingClusters[mNumPendingClusters++] = { endpoint, cluster };
            }
            else
            {
                mPendingClusters[mNextToOverride] = { endpoint, cluster };
                mNextToOverride++;
                mNextToOverride %= kMaxPendingClusters;
            }
        }

    private:
        uint8_t mNumPendingClusters = 0;
        uint8_t mNextToOverride     = 0;

        PeerId mPeerId;
        ClusterPath mPendingClusters[kMaxPendingClusters];
        System::Clock::Timestamp mLastUpdateTime;
    };

    // The pool for all the pending comands.
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

    static void HandleDeviceConnectionFailure(void * context, PeerId peerId, CHIP_ERROR error);
    void HandleDeviceConnectionFailure(PeerId peerId, CHIP_ERROR error);

    // Called when CASE session is established to a peer device. Will send all the pending commands to the peer.
    void SyncPendingClustersToPeer(OperationalDeviceProxy * device, PendingClusterEntry * pendingClusters);

    // Called when CASE session is not established to a bound peer. Will enqueue the command and initialize connection.
    CHIP_ERROR EnqueueClusterAndConnect(FabricIndex fabric, NodeId node, EndpointId endpoint, ClusterId cluster);

    PendingClusterMap mPendingClusterMap;
    BoundDeviceChangedHandler mBoundDeviceChangedHandler;
    Server * mAppServer;

    Callback::Callback<OnDeviceConnected> mOnConnectedCallback;
    Callback::Callback<OnDeviceConnectionFailure> mOnConnectionFailureCallback;
};

} // namespace chip
