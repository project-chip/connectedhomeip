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

/**
 *  @file
 *    This file contains implementation of OperationalDeviceProxy class.
 *    The objects of this class is for any two devices on the operational
 *    network to establish a secure session with each other. The class
 *    provides mechanism to construct, send and receive messages to and
 *    from the corresponding device.
 */

#pragma once

#include <controller/CHIPDevice.h>

namespace chip {
namespace app {
namespace device {

struct OperationalDeviceProxyInitParams
{
    SessionManager * sessionManager          = nullptr;
    Messaging::ExchangeManager * exchangeMgr = nullptr;
    SessionIDAllocator * idAllocator         = nullptr;
    FabricTable * fabricsTable               = nullptr;
};

class OperationalDeviceProxy;

// TODO: CHIPDevice needs to be refactored and when that happens, the type for this callback may change
typedef void (*OnOperationalDeviceConnected)(void * context, OperationalDeviceProxy * operationalDevice);
typedef void (*OnOperationalDeviceConnectionFailure)(void * context, NodeId nodeId, CHIP_ERROR error);

class DLL_EXPORT OperationalDeviceProxy : public Messaging::ExchangeMgrDelegate
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
     * @param[in] nodeId       Node ID of the device
     * @param[in] fabricIndex  Fabric index of the device
     * @param[in] params       Device specific parameters
     */
    void Init(NodeId nodeId, FabricIndex fabricIndex, OperationalDeviceProxyInitParams params)
    {
        VerifyOrReturn(params.sessionManager != nullptr);
        VerifyOrReturn(params.exchangeMgr != nullptr);
        VerifyOrReturn(params.idAllocator != nullptr);
        VerifyOrReturn(params.fabricsTable != nullptr);

        mNodeId      = nodeId;
        mFabricIndex = fabricIndex;
        mInitParams  = params;
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
     *   Update address of the device
     *
     * @param[in] address  Address of the device
     */
    // TODO: After a requested CHIP node ID has been successfully resolved, call this to update
    CHIP_ERROR UpdateAddress(const Transport::PeerAddress & address);

    chip::Controller::Device & GetDevice() { return mDevice; }

private:
    /**
     * ----- Data members -----
     */

    /* Node ID assigned to the device */
    NodeId mNodeId;

    /* Fabric index of the device */
    FabricIndex mFabricIndex = kUndefinedFabricIndex;

    /* Device specific parameters needed to establish a secure session */
    OperationalDeviceProxyInitParams mInitParams;

    /* Address used to communicate with the device */
    Transport::PeerAddress mAddress = Transport::PeerAddress::UDP(Inet::IPAddress::Any);

    /* Tracker of callbacks for the device */
    Callback::CallbackDeque mConnectionSuccess;
    Callback::CallbackDeque mConnectionFailure;

    // TODO: CHIPDevice needs to be refactored and when that happens, this class will no longer act
    // as a wrapper to Device class. This class should not need to hold a Device class object.
    Controller::Device mDevice;

    /**
     * ----- Member functions -----
     */
    void EnqueueConnectionCallbacks(Callback::Callback<OnOperationalDeviceConnected> * onConnection,
                                    Callback::Callback<OnOperationalDeviceConnectionFailure> * onFailure);

    void DequeueConnectionSuccessCallbacks(bool executeCallback);
    void DequeueConnectionFailureCallbacks(CHIP_ERROR error, bool executeCallback);

    /**
     * ----- ExchangeMgrDelegate Implementation -----
     */
    void OnNewConnection(SessionHandle session, Messaging::ExchangeManager * mgr) override;
    void OnConnectionExpired(SessionHandle session, Messaging::ExchangeManager * mgr) override;

    /**
     * ----- Wrapper callbacks for Device class -----
     */
    // TODO: CHIPDevice needs to be refactored and when that happens, these callbacks are no longer
    // needed. They are currently being used to forward callbacks from the Device class to the users
    // of the OperationalDeviceProxy class.
    static void OnDeviceConnectedFn(void * context, chip::Controller::Device * device);
    static void OnDeviceConnectionFailureFn(void * context, NodeId deviceId, CHIP_ERROR error);

    chip::Callback::Callback<chip::Controller::OnDeviceConnected> mOnDeviceConnectedCallback;
    chip::Callback::Callback<chip::Controller::OnDeviceConnectionFailure> mOnDeviceConnectionFailureCallback;
};

} // namespace device
} // namespace app
} // namespace chip
