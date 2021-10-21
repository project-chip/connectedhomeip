/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <controller/CHIPDevice.h>

namespace chip {
namespace app {
namespace device {

/**
 * This struct contains device specific parameters that are needed to establish a secure session. The
 * pointers passed in are not owned by this object and should have a lifetime beyond this object.
 */
struct OperationalDeviceProxyInitParams
{
    SessionManager * sessionManager          = nullptr;
    Messaging::ExchangeManager * exchangeMgr = nullptr;
    SessionIDAllocator * idAllocator         = nullptr;
    FabricTable * fabricsTable               = nullptr;
};

class OperationalDeviceProxy;

// TODO: https://github.com/project-chip/connectedhomeip/issues/10423 will provide a refactor of the `Device`
// class. When that happens, the type of the last param for this callback may change as the registrar of this
// callback would need to be able to associate the peer device with the cluster command being setn.
typedef void (*OnOperationalDeviceConnected)(void * context, OperationalDeviceProxy * operationalDeviceProxy);
typedef void (*OnOperationalDeviceConnectionFailure)(void * context, OperationalDeviceProxy * operationalDeviceProxy,
                                                     CHIP_ERROR error);

/**
 * @class OperationalDeviceProxy
 *
 * @brief This is a device proxy class for any two devices on the operational network to establish a
 * secure session with each other via CASE. To establish a secure session, the caller of this class
 * must supply the node ID, fabric index, as well as other device specific parameters to the peer node
 * it wants to communicate with.
 */
class DLL_EXPORT OperationalDeviceProxy
{
public:
    virtual ~OperationalDeviceProxy() {}
    OperationalDeviceProxy() :
        mOnDeviceConnectedCallback(OnDeviceConnectedFn, this), mOnDeviceConnectionFailureCallback(OnDeviceConnectionFailureFn, this)
    {}

    /**
     * @brief
     *   Initialize an operational device object with node ID, fabric index and other
     *   device specific parameters used to establish a secure session.
     *
     * @param[in] nodeId       Node ID of the device in which the secure session is established for
     * @param[in] fabricIndex  Fabric index of the device in which the secure session is established for
     * @param[in] initParams   Device specific parameters used in establishing the secure session
     */
    void Init(NodeId nodeId, FabricIndex fabricIndex, OperationalDeviceProxyInitParams initParams)
    {
        VerifyOrReturn(initParams.sessionManager != nullptr);
        VerifyOrReturn(initParams.exchangeMgr != nullptr);
        VerifyOrReturn(initParams.idAllocator != nullptr);
        VerifyOrReturn(initParams.fabricsTable != nullptr);

        mNodeId      = nodeId;
        mFabricIndex = fabricIndex;
        mInitParams  = initParams;
    }

    /**
     * @brief
     *   Establish a secure session with the device via CASE.
     *
     *   On establishing the session, the callback function `onConnection` will be called. If the
     *   session setup fails, `onFailure` will be called.
     *
     *   If the session already exists, `onConnection` will be called immediately.
     *
     * @param[in] onConnection  Callback to call when secure session successfully established
     * @param[in] onFailure     Callback to call when secure session fails to be established
     */
    CHIP_ERROR Connect(Callback::Callback<OnOperationalDeviceConnected> * onConnection,
                       Callback::Callback<OnOperationalDeviceConnectionFailure> * onFailure);

    /**
     * @brief
     *   Update address of the device. The address is used as part of secure session establishment
     *   and therefore, must be updated before a secure session is established.
     *
     * @param[in] address  Address of the device in which the secure session is established for
     */
    // TODO: After a requested CHIP node ID has been successfully resolved, call this to update
    CHIP_ERROR UpdateAddress(const Transport::PeerAddress & address);

    /**
     * @brief
     *   Called when a secure session is being established
     *
     * @param[in] session  The handle to the secure session
     */
    void OnNewConnection(SessionHandle session);

    /**
     * @brief
     *   Called when a secure session is closing
     *
     * @param[in] session  The handle to the secure session
     */
    void OnConnectionExpired(SessionHandle session);

    chip::Controller::Device & GetDevice() { return mDevice; }

private:
    enum class State
    {
        Uninitialized,
        Initialized,
        Connecting,
        SecureConnected,
    };

    /* Node ID assigned to the device */
    NodeId mNodeId = kUndefinedNodeId;

    /* Fabric index of the device */
    FabricIndex mFabricIndex = kUndefinedFabricIndex;

    /* Device specific parameters needed to establish a secure session */
    OperationalDeviceProxyInitParams mInitParams;

    /* Address used to communicate with the device */
    Transport::PeerAddress mAddress = Transport::PeerAddress::UDP(Inet::IPAddress::Any);

    /* Current state of the proxy */
    State mState = State::Uninitialized;

    /* Tracker of callbacks for the device */
    Callback::CallbackDeque mConnectionSuccess;
    Callback::CallbackDeque mConnectionFailure;

    // TODO: https://github.com/project-chip/connectedhomeip/issues/10423 will provide a refactor of the `Device`
    // class. When that happens, this class will no longer act as a wrapper to the `Device` class. This class
    // should not need to hold a Device class object.
    Controller::Device mDevice;

    /**
     * ----- Member functions -----
     */
    void EnqueueConnectionCallbacks(Callback::Callback<OnOperationalDeviceConnected> * onConnection,
                                    Callback::Callback<OnOperationalDeviceConnectionFailure> * onFailure);

    void DequeueConnectionSuccessCallbacks(bool executeCallback);
    void DequeueConnectionFailureCallbacks(CHIP_ERROR error, bool executeCallback);

    /**
     * ----- Wrapper callbacks for Device class -----
     */
    // TODO: https://github.com/project-chip/connectedhomeip/issues/10423 will provide a refactor of the `Device`
    // class. When that happens, these callbacks are no longer needed. They are currently being used to forward
    // callbacks from the `Device` class to the users of the `OperationalDeviceProxy` class. Once the
    // `OperationalDeviceProxy` class is no longer a wrapper to the `Device` class, the former will no longer
    // need to register for the following callbacks to the `Device` class.
    static void OnDeviceConnectedFn(void * context, chip::Controller::Device * device);
    static void OnDeviceConnectionFailureFn(void * context, NodeId deviceId, CHIP_ERROR error);

    chip::Callback::Callback<chip::Controller::OnDeviceConnected> mOnDeviceConnectedCallback;
    chip::Callback::Callback<chip::Controller::OnDeviceConnectionFailure> mOnDeviceConnectionFailureCallback;
};

} // namespace device
} // namespace app
} // namespace chip
