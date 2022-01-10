/*
 *    Copyright (c) 2020-2021 Project CHIP Authors
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

#include <controller/CHIPDeviceControllerSystemState.h>
#include <controller/InvokeInteraction.h>
#include <controller/ReadInteraction.h>

namespace chip {
namespace Commissioner {

using namespace Controller;

struct Commissionee
{
    DeviceControllerSystemState * mSystemState = nullptr;
    chip::SessionHolder mPaseSession;
    chip::SessionHolder mCaseSession;
    chip::Optional<chip::Transport::PeerAddress> mCommissionableNodeAddress;
    chip::Optional<chip::Transport::PeerAddress> mOperationalAddress;
    chip::Optional<chip::ReliableMessageProtocolConfig> mMrpConfig;
    chip::Optional<chip::PeerId> mOperationalId;

    void Init(DeviceControllerSystemState * systemState)
    {
        if (mSystemState)
        {
            mSystemState->Release();
        }
        mSystemState = systemState;
        if (mSystemState)
        {
            mSystemState->Retain();
        }
    }

#if CONFIG_NETWORK_LAYER_BLE
    CHIP_ERROR OpenBle()
    {
        VerifyOrReturnError(mSystemState && mBleConnection.HasValue(), CHIP_ERROR_INCORRECT_STATE);
        ReturnErrorOnFailure(mSystemState->BleLayer()->NewBleConnectionByObject(mBleConnection.Value()));
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR CloseBle()
    {
        VerifyOrReturnError(mSystemState && mBleConnection.HasValue(), CHIP_ERROR_INCORRECT_STATE);
        ReturnErrorOnFailure(mSystemState->BleLayer()->CloseBleConnection(mBleConnection.Value()));
        mBleConnection.ClearValue();
        return CHIP_NO_ERROR;
    }

    chip::Optional<BLE_CONNECTION_OBJECT> mBleConnection;
#endif // CONFIG_NETWORK_LAYER_BLE

    CHIP_ERROR ClosePase()
    {
        ReturnErrorOnFailure(ValidatePaseState());
        this->mSystemState->SessionMgr()->ExpirePairing(mPaseSession.Get());
        mPaseSession.Release();
#if CONFIG_NETWORK_LAYER_BLE
        ReturnErrorOnFailure(CloseBle());
#endif
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR CloseCase()
    {
        ReturnErrorOnFailure(ValidateCaseState());
        this->mSystemState->SessionMgr()->ExpirePairing(mCaseSession.Get());
        mCaseSession.Release();
        return CHIP_NO_ERROR;
    }

    void Shutdown()
    {
        ClosePase();
        CloseCase();
        if (mSystemState != nullptr)
        {
            mSystemState->Release();
            mSystemState = nullptr;
        }
    }

    template <typename DecodableAttributeType>
    CHIP_ERROR PaseRead(EndpointId endpointId, ClusterId clusterId, AttributeId attributeId,
                        typename TypedReadAttributeCallback<DecodableAttributeType>::OnSuccessCallbackType onSuccessCb,
                        typename TypedReadAttributeCallback<DecodableAttributeType>::OnErrorCallbackType onErrorCb)
    {
        ReturnErrorOnFailure(ValidatePaseState());
        return ReadAttribute<DecodableAttributeType>(mSystemState->ExchangeMgr(), mPaseSession.Get(), endpointId, clusterId,
                                                     attributeId, onSuccessCb, onErrorCb);
    }

    template <typename RequestObjectT>
    CHIP_ERROR PaseInvoke(EndpointId endpointId, const RequestObjectT & requestCommandData,
                          typename TypedCommandCallback<typename RequestObjectT::ResponseType>::OnSuccessCallbackType onSuccessCb,
                          typename TypedCommandCallback<typename RequestObjectT::ResponseType>::OnErrorCallbackType onErrorCb)
    {
        ReturnErrorOnFailure(ValidatePaseState());
        return InvokeCommandRequest(mSystemState->ExchangeMgr(), mPaseSession.Get(), endpointId, requestCommandData, onSuccessCb,
                                    onErrorCb);
    }

    template <typename DecodableAttributeType>
    CHIP_ERROR CaseRead(EndpointId endpointId, ClusterId clusterId, AttributeId attributeId,
                        typename TypedReadAttributeCallback<DecodableAttributeType>::OnSuccessCallbackType onSuccessCb,
                        typename TypedReadAttributeCallback<DecodableAttributeType>::OnErrorCallbackType onErrorCb)
    {
        ReturnErrorOnFailure(ValidateCaseState());
        return ReadAttribute(mSystemState->ExchangeMgr(), mCaseSession.Get(), clusterId, attributeId, onSuccessCb, onErrorCb);
    }

    template <typename RequestObjectT>
    CHIP_ERROR CaseInvoke(EndpointId endpointId, const RequestObjectT & requestCommandData,
                          typename TypedCommandCallback<typename RequestObjectT::ResponseType>::OnSuccessCallbackType onSuccessCb,
                          typename TypedCommandCallback<typename RequestObjectT::ResponseType>::OnErrorCallbackType onErrorCb)
    {
        ReturnErrorOnFailure(ValidateCaseState());
        return InvokeCommandRequest(mSystemState->ExchangeMgr(), mCaseSession.Get(), endpointId, requestCommandData, onSuccessCb,
                                    onErrorCb);
    }

private:
    CHIP_ERROR ValidatePaseState()
    {
        if (!mPaseSession || mSystemState == nullptr)
        {
            return CHIP_ERROR_INCORRECT_STATE;
        }
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR ValidateCaseState()
    {
        if (!mCaseSession || mSystemState == nullptr)
        {
            return CHIP_ERROR_INCORRECT_STATE;
        }
        return CHIP_NO_ERROR;
    }
};

} // namespace Commissioner
} // namespace chip
