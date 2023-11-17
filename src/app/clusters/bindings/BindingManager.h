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
#include <app/clusters/bindings/PendingNotificationMap.h>
#include <app/server/Server.h>
#include <app/util/binding-table.h>
#include <credentials/FabricTable.h>
#include <lib/core/CHIPPersistentStorageDelegate.h>

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
 * The handler is not allowed to hold onto the pointer to the SessionHandler that is passed in.
 */
using BoundDeviceChangedHandler = void (*)(const EmberBindingTableEntry & binding, OperationalDeviceProxy * peer_device,
                                           void * context);

/**
 * Application callback function when a context used in NotifyBoundClusterChanged will not be needed and should be
 * released.
 */
using BoundDeviceContextReleaseHandler = PendingNotificationContextReleaseHandler;

struct BindingManagerInitParams
{
    FabricTable * mFabricTable               = nullptr;
    CASESessionManager * mCASESessionManager = nullptr;
    PersistentStorageDelegate * mStorage     = nullptr;
    bool mEstablishConnectionOnInit          = true;
};

/**
 *
 * The BindingManager class manages the connections for unicast bindings and notifies the application
 * when a binding is ready to be communicated with.
 *
 * A CASE connection will be triggered when:
 *  - During init of the BindingManager, unless the application actively disables this using mEstablishConnectionOnInit
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
    BindingManager() {}

    void RegisterBoundDeviceChangedHandler(BoundDeviceChangedHandler handler) { mBoundDeviceChangedHandler = handler; }

    /*
     * Registers handler that will be called when context used in NotifyBoundClusterChanged will not be needed and could be
     * released.
     *
     */
    void RegisterBoundDeviceContextReleaseHandler(BoundDeviceContextReleaseHandler handler)
    {
        mPendingNotificationMap.RegisterPendingNotificationContextReleaseHandler(handler);
    }

    CHIP_ERROR Init(const BindingManagerInitParams & params);

    /*
     * Notifies the BindingManager that a new unicast binding is created.
     *
     */
    CHIP_ERROR UnicastBindingCreated(uint8_t fabricIndex, NodeId nodeId);

    /*
     * Notifies the BindingManager that a unicast binding is about to be removed from the given index.
     *
     */
    CHIP_ERROR UnicastBindingRemoved(uint8_t bindingEntryId);

    /*
     * Notifies the BindingManager that a fabric is removed from the device
     *
     */
    void FabricRemoved(FabricIndex fabricIndex);

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
    /*
     * Used when providing OnConnection/Failure callbacks to CASESessionManager when establishing session.
     *
     * Since the BindingManager calls EstablishConnection inside of a loop, and it is possible that the
     * callback is called some time after the loop is completed, we need a separate callbacks for each
     * connection we are trying to establish. Failure to provide different instances of the callback
     * to CASESessionManager may result in the callback only be called for that last EstablishConnection
     * that was called when it establishes the connections asynchronously.
     *
     */
    class ConnectionCallback
    {
    public:
        ConnectionCallback(BindingManager & bindingManager) :
            mBindingManager(bindingManager), mOnConnectedCallback(HandleDeviceConnected, this),
            mOnConnectionFailureCallback(HandleDeviceConnectionFailure, this)
        {}

        Callback::Callback<OnDeviceConnected> * GetOnDeviceConnected() { return &mOnConnectedCallback; }
        Callback::Callback<OnDeviceConnectionFailure> * GetOnDeviceConnectionFailure() { return &mOnConnectionFailureCallback; }

    private:
        static void HandleDeviceConnected(void * context, Messaging::ExchangeManager & exchangeMgr,
                                          const SessionHandle & sessionHandle)
        {
            ConnectionCallback * _this = static_cast<ConnectionCallback *>(context);
            _this->mBindingManager.HandleDeviceConnected(exchangeMgr, sessionHandle);
            Platform::Delete(_this);
        }
        static void HandleDeviceConnectionFailure(void * context, const ScopedNodeId & peerId, CHIP_ERROR error)
        {
            ConnectionCallback * _this = static_cast<ConnectionCallback *>(context);
            _this->mBindingManager.HandleDeviceConnectionFailure(peerId, error);
            Platform::Delete(_this);
        }

        BindingManager & mBindingManager;
        Callback::Callback<OnDeviceConnected> mOnConnectedCallback;
        Callback::Callback<OnDeviceConnectionFailure> mOnConnectionFailureCallback;
    };

    static BindingManager sBindingManager;

    CHIP_ERROR EstablishConnection(const ScopedNodeId & nodeId);

    PendingNotificationMap mPendingNotificationMap;
    BoundDeviceChangedHandler mBoundDeviceChangedHandler;
    BindingManagerInitParams mInitParams;

    void HandleDeviceConnected(Messaging::ExchangeManager & exchangeMgr, const SessionHandle & sessionHandle);
    void HandleDeviceConnectionFailure(const ScopedNodeId & peerId, CHIP_ERROR error);

    // Used to keep track of synchronous failures from FindOrEstablishSession.
    CHIP_ERROR mLastSessionEstablishmentError;
};

} // namespace chip
