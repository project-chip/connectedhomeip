/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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
 * Application callback function when a cluster associated with a binding changes.
 *
 * The connection is managed by the stack and peer_device is guaranteed to be available.
 * The application shall decide the content to be sent to the peer.
 *
 * For unicast bindings peer_device will be a connected peer and group will be empty.
 * For multicast bindings peer_device will be nullptr.
 *
 * E.g. The application will send on/off commands to peer for the OnOff cluster.
 *
 */
using BoundDeviceChangedHandler = void (*)(const EmberBindingTableEntry * binding, DeviceProxy * peer_device, void * context);

/**
 *
 * The BindingManager class manages the connections for unicast bindings and notifies the application
 * when a binding is ready to be communicated with.
 *
 * A CASE connection will be triggered when:
 *  - The binding cluster adds a unicast entry to the binding table.
 *  - A watched cluster changes with a unicast binding but we cannot find an active connection to the peer.
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
     * Notifies the BindingManager that a new unicast binding is created.
     *
     */
    CHIP_ERROR UnicastBindingCreated(FabricIndex fabric, NodeId node) { return EstablishConnection(fabric, node); }

    /*
     * Notfies the BindingManager that the **last** unicast binding to a device has been removed.
     *
     */
    CHIP_ERROR LastUnicastBindingRemoved(FabricIndex fabric, NodeId node);

    /*
     * Notify a cluster change to **all** bound devices associated with the (endpoint, cluster) tuple.
     *
     * For unicast bindings with an active session and multicast bindings, the BoundDeviceChangedHandler
     * will be called before the function returns.
     *
     * For unicast bindings without an active session, the notification will be queued and a new session will
     * be initiated. The BoundDeviceChangedHandler will be called once the session is established.
     *
     */
    CHIP_ERROR NotifyBoundClusterChanged(EndpointId endpoint, ClusterId cluster, void * context);

    static BindingManager & GetInstance() { return sBindingManager; }

private:
    static BindingManager sBindingManager;

    static constexpr uint8_t kMaxPendingNotifications = 3;

    struct ClusterPath
    {
        void * context;
        ClusterId cluster;
        EndpointId endpoint;
    };

    // A pending notification to be sent to a binding waiting for the CASE session to be established.
    class PendingNotificationEntry
    {
    public:
        PendingNotificationEntry(PeerId peerId) : mPeerId(peerId) {}

        PeerId GetPeerId() { return mPeerId; }

        System::Clock::Timestamp GetLastUpdateTime() { return mLastUpdateTime; }
        void Touch() { mLastUpdateTime = System::SystemClock().GetMonotonicTimestamp(); }

        ClusterPath * begin() { return &mPendingNotifications[0]; }
        ClusterPath * end() { return &mPendingNotifications[mNumPendingNotifications]; }

        void AddPendingNotification(EndpointId endpoint, ClusterId cluster, void * context)
        {
            for (ClusterPath & path : *this)
            {
                // New notifications for the same (endpoint, cluster) shall
                // simply overrride the old ones
                if (path.cluster == cluster && path.endpoint == endpoint)
                {
                    path.context = context;
                    return;
                }
            }
            if (mNumPendingNotifications < kMaxPendingNotifications)
            {
                mPendingNotifications[mNumPendingNotifications++] = { context, cluster, endpoint };
            }
            else
            {
                mPendingNotifications[mNextToOverride] = { context, cluster, endpoint };
                mNextToOverride++;
                mNextToOverride %= kMaxPendingNotifications;
            }
        }

    private:
        PeerId mPeerId;
        System::Clock::Timestamp mLastUpdateTime;
        // TODO: Make the pending notifications list of binding table indecies and list of contexts
        ClusterPath mPendingNotifications[kMaxPendingNotifications];

        uint8_t mNumPendingNotifications = 0;
        uint8_t mNextToOverride          = 0;
    };

    // The pool for all the pending comands.
    class PendingNotificationMap
    {
    public:
        PendingNotificationEntry * FindLRUEntry();

        PendingNotificationEntry * FindEntry(PeerId peerId);

        CHIP_ERROR AddPendingNotification(PeerId peer, EndpointId endpoint, ClusterId cluster, void * context);

        void RemoveEntry(PendingNotificationEntry * entry) { mPendingNotificationMap.ReleaseObject(entry); }

        template <typename Function>
        Loop ForEachActiveObject(Function && function)
        {
            return mPendingNotificationMap.ForEachActiveObject(std::forward<Function>(function));
        }

    private:
        BitMapObjectPool<PendingNotificationEntry, EMBER_BINDING_TABLE_SIZE> mPendingNotificationMap;
    };

    static void HandleDeviceConnected(void * context, OperationalDeviceProxy * device);
    void HandleDeviceConnected(OperationalDeviceProxy * device);

    static void HandleDeviceConnectionFailure(void * context, PeerId peerId, CHIP_ERROR error);
    void HandleDeviceConnectionFailure(PeerId peerId, CHIP_ERROR error);

    CHIP_ERROR EstablishConnection(FabricIndex fabric, NodeId node);

    // Called when CASE session is established to a peer device. Will send all the pending commands to the peer.
    void SyncPendingNotificationsToPeer(OperationalDeviceProxy * device, PendingNotificationEntry * pendingClusters);

    // Called when CASE session is not established to a peer device. Will enqueue the command and initialize connection.
    CHIP_ERROR EnqueueUnicastNotification(FabricIndex fabric, NodeId node, EndpointId endpoint, ClusterId cluster, void * context);

    PendingNotificationMap mPendingNotificationMap;
    BoundDeviceChangedHandler mBoundDeviceChangedHandler;
    Server * mAppServer = nullptr;

    Callback::Callback<OnDeviceConnected> mOnConnectedCallback;
    Callback::Callback<OnDeviceConnectionFailure> mOnConnectionFailureCallback;
};

} // namespace chip
