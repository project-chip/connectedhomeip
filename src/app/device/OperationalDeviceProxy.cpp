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

#include <app/device/OperationalDeviceProxy.h>

namespace chip {
namespace app {
namespace device {

CHIP_ERROR OperationalDeviceProxy::Connect(Callback::Callback<OnOperationalDeviceConnected> * onConnection,
                                           Callback::Callback<OnOperationalDeviceConnectionFailure> * onFailure)
{
    // Create a UnauthenticatedSession for CASE pairing.
    // Don't use mSecureSession here, because mSecureSession is for encrypted communication.
    Optional<SessionHandle> session = mInitParams.sessionManager->CreateUnauthenticatedSession(mAddress);
    VerifyOrReturnError(session.HasValue(), CHIP_ERROR_NO_MEMORY);

    Messaging::ExchangeContext * exchange = mInitParams.exchangeMgr->NewContext(session.Value(), &mCASESession);
    VerifyOrReturnError(exchange != nullptr, CHIP_ERROR_INTERNAL);

    ReturnErrorOnFailure(mCASESession.MessageDispatch().Init(mInitParams.sessionManager));

    uint16_t keyID = 0;
    ReturnErrorOnFailure(mInitParams.idAllocator->Allocate(keyID));

    FabricInfo * fabricInfo = mInitParams.fabricsTable->FindFabricWithIndex(mFabricIndex);
    ReturnErrorCodeIf(fabricInfo == nullptr, CHIP_ERROR_INCORRECT_STATE);

    ReturnErrorOnFailure(mCASESession.EstablishSession(mAddress, fabricInfo, mNodeId, keyID, exchange, this));

    EnqueueConnectionCallbacks(onConnection, onFailure);
    // mState = State::Connecting;

    return CHIP_NO_ERROR;

/*
    // Secure session already established
    if (mDevice.IsSecureConnected())
    {
        onConnection->mCall(onConnection->mContext, this);
        return CHIP_NO_ERROR;
    }

    VerifyOrReturnError(mInitParams.exchangeMgr != nullptr, CHIP_ERROR_INTERNAL);

    EnqueueConnectionCallbacks(onConnection, onFailure);

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
    mDevice.SetActive(true);
    err = mDevice.EstablishConnectivity(&mOnDeviceConnectedCallback, &mOnDeviceConnectionFailureCallback);
    SuccessOrExit(err);

exit:
    if (err != CHIP_NO_ERROR)
    {
        DequeueConnectionSuccessCallbacks(false);
        DequeueConnectionFailureCallbacks(err, true);
    }

    return err;
*/
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
            cb->mCall(cb->mContext, this, error);
        }
    }
}

void OperationalDeviceProxy::OnNewConnection(SessionHandle session)
{
    // If the secure session established is initiated by another device
    // if (!mDevice.IsActive() || mDevice.IsSecureConnected())
    // {
    //     return;
    // }

    // mDevice.OnNewConnection(session);
}

void OperationalDeviceProxy::OnConnectionExpired(SessionHandle session)
{
    // mDevice.OnConnectionExpired(session);
}

void OperationalDeviceProxy::OnSessionEstablishmentError(CHIP_ERROR error)
{
    // VerifyOrReturn(mState != State::Uninitialized,
    //                ChipLogError(Controller, "OnSessionEstablishmentError was called while the device was not initialized"));

    // mState = State::Initialized;
    mInitParams.idAllocator->Free(mCASESession.GetLocalSessionId());

    DequeueConnectionSuccessCallbacks(/* executeCallback */ false);
    DequeueConnectionFailureCallbacks(error, /* executeCallback */ true);
}

void OperationalDeviceProxy::OnSessionEstablished()
{
    // VerifyOrReturn(mState != State::Uninitialized,
    //                ChipLogError(Controller, "OnSessionEstablished was called while the device was not initialized"));

    CHIP_ERROR err = mInitParams.sessionManager->NewPairing(
        Optional<Transport::PeerAddress>::Value(mAddress), mNodeId, &mCASESession,
        CryptoContext::SessionRole::kInitiator, mFabricIndex);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "Failed in setting up CASE secure channel: err %s", ErrorStr(err));
        OnSessionEstablishmentError(err);
        return;
    }

    // mState = State::SecureConnected;

    DequeueConnectionFailureCallbacks(CHIP_NO_ERROR, /* executeCallback */ false);
    DequeueConnectionSuccessCallbacks(/* executeCallback */ true);
}

// void OperationalDeviceProxy::OnDeviceConnectedFn(void * context, Controller::Device * device)
// {
//     VerifyOrReturn(context != nullptr, ChipLogError(OperationalDeviceProxy, "%s: invalid context", __FUNCTION__));
//     OperationalDeviceProxy * operationalDevice = reinterpret_cast<OperationalDeviceProxy *>(context);
//     operationalDevice->DequeueConnectionFailureCallbacks(CHIP_NO_ERROR, false);
//     operationalDevice->DequeueConnectionSuccessCallbacks(true);
// }

// void OperationalDeviceProxy::OnDeviceConnectionFailureFn(void * context, NodeId deviceId, CHIP_ERROR error)
// {
//     VerifyOrReturn(context != nullptr, ChipLogError(OperationalDeviceProxy, "%s: invalid context", __FUNCTION__));
//     OperationalDeviceProxy * operationalDevice = reinterpret_cast<OperationalDeviceProxy *>(context);
//     operationalDevice->DequeueConnectionSuccessCallbacks(false);
//     operationalDevice->DequeueConnectionFailureCallbacks(error, true);
// }

} // namespace device
} // namespace app
} // namespace chip
