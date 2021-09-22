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

#include <app/device/OperationalDeviceProxy.h>

namespace chip {
namespace app {
namespace device {

CHIP_ERROR OperationalDeviceProxy::Connect(Callback::Callback<OnOperationalDeviceConnected> * onConnection,
                                           Callback::Callback<OnOperationalDeviceConnectionFailure> * onFailure)
{
    EnqueueConnectionCallbacks(onConnection, onFailure);

    // Secure session already established
    if (mDevice.IsSecureConnected())
    {
        DequeueConnectionFailureCallbacks(CHIP_NO_ERROR, false);
        DequeueConnectionSuccessCallbacks(true);
        return CHIP_NO_ERROR;
    }

    VerifyOrReturnError(mInitParams.exchangeMgr != nullptr, CHIP_ERROR_INTERNAL);
    mInitParams.exchangeMgr->SetDelegate(this);

    Controller::ControllerDeviceInitParams initParams = {
        .sessionManager  = mInitParams.sessionManager,
        .exchangeMgr     = mInitParams.exchangeMgr,
        .storageDelegate = nullptr,
        .idAllocator     = mInitParams.idAllocator,
        .fabricsTable    = mInitParams.fabricsTable,
    };

    CHIP_ERROR err = CHIP_NO_ERROR;
    mDevice.Init(initParams, mNodeId, mAddress, mFabricIndex);
    mDevice.OperationalCertProvisioned();
    err = mDevice.EstablishConnectivity(&mOnDeviceConnectedCallback, &mOnDeviceConnectionFailureCallback);
    SuccessOrExit(err);

exit:
    if (err != CHIP_NO_ERROR)
    {
        DequeueConnectionSuccessCallbacks(false);
        DequeueConnectionFailureCallbacks(err, true);
    }

    return err;
}

CHIP_ERROR OperationalDeviceProxy::UpdateAddress(const Transport::PeerAddress & address)
{
    mAddress = address;
    return CHIP_NO_ERROR;
}

void OperationalDeviceProxy::EnqueueConnectionCallbacks(Callback::Callback<OnOperationalDeviceConnected> * onConnection,
                                                        Callback::Callback<OnOperationalDeviceConnectionFailure> * onFailure)
{
    if (onConnection != nullptr)
    {
        mConnectionSuccess.Enqueue(onConnection->Cancel());
    }

    if (onFailure != nullptr)
    {
        mConnectionFailure.Enqueue(onFailure->Cancel());
    }
}

void OperationalDeviceProxy::DequeueConnectionSuccessCallbacks(bool executeCallback)
{
    Callback::Cancelable ready;
    mConnectionSuccess.DequeueAll(ready);
    while (ready.mNext != &ready)
    {
        Callback::Callback<OnOperationalDeviceConnected> * cb =
            Callback::Callback<OnOperationalDeviceConnected>::FromCancelable(ready.mNext);

        cb->Cancel();
        if (executeCallback)
        {
            cb->mCall(cb->mContext, this);
        }
    }
}

void OperationalDeviceProxy::DequeueConnectionFailureCallbacks(CHIP_ERROR error, bool executeCallback)
{
    Callback::Cancelable ready;
    mConnectionFailure.DequeueAll(ready);
    while (ready.mNext != &ready)
    {
        Callback::Callback<OnOperationalDeviceConnectionFailure> * cb =
            Callback::Callback<OnOperationalDeviceConnectionFailure>::FromCancelable(ready.mNext);

        cb->Cancel();
        if (executeCallback)
        {
            cb->mCall(cb->mContext, mNodeId, error);
        }
    }
}

void OperationalDeviceProxy::OnNewConnection(SessionHandle session, Messaging::ExchangeManager * mgr)
{
    mDevice.OnNewConnection(session);
    mDevice.SetActive(true);
}

void OperationalDeviceProxy::OnConnectionExpired(SessionHandle session, Messaging::ExchangeManager * mgr)
{
    mDevice.OnConnectionExpired(session);
}

void OperationalDeviceProxy::OnDeviceConnectedFn(void * context, Controller::Device * device)
{
    VerifyOrReturn(context != nullptr);
    OperationalDeviceProxy * operationalDevice = reinterpret_cast<OperationalDeviceProxy *>(context);
    operationalDevice->DequeueConnectionFailureCallbacks(CHIP_NO_ERROR, false);
    operationalDevice->DequeueConnectionSuccessCallbacks(true);
}

void OperationalDeviceProxy::OnDeviceConnectionFailureFn(void * context, NodeId deviceId, CHIP_ERROR error)
{
    VerifyOrReturn(context != nullptr);
    OperationalDeviceProxy * operationalDevice = reinterpret_cast<OperationalDeviceProxy *>(context);
    operationalDevice->DequeueConnectionSuccessCallbacks(false);
    operationalDevice->DequeueConnectionFailureCallbacks(error, true);
}

} // namespace device
} // namespace app
} // namespace chip
