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

#include <controller/InteractionPeer.h>

namespace chip {
namespace Controller {

CHIP_ERROR InteractionPeer::SendReadAttributeRequest(app::AttributePathParams aPath, Callback::Cancelable * onSuccessCallback,
                                                     Callback::Cancelable * onFailureCallback, app::TLVDataFilter aTlvDataFilter)
{

    uint8_t seqNum = GetNextSequenceNumber();
    aPath.mNodeId  = mDelegate->GetDeviceId();

    ReturnErrorOnFailure(mDelegate->PreparePeer());

    if (onSuccessCallback != nullptr || onFailureCallback != nullptr)
    {
        AddResponseHandler(seqNum, onSuccessCallback, onFailureCallback, aTlvDataFilter);
    }
    // The application context is used to identify different requests from client application the type of it is intptr_t, here we
    // use the seqNum.
    CHIP_ERROR err = chip::app::InteractionModelEngine::GetInstance()->SendReadRequest(
        mDelegate->GetDeviceId(), 0, mDelegate->GetSessionHandle(), nullptr /*event path params list*/, 0, &aPath, 1,
        0 /* event number */, seqNum /* application context */);
    if (err != CHIP_NO_ERROR)
    {
        CancelResponseHandler(seqNum);
    }
    return err;
}

CHIP_ERROR InteractionPeer::SendWriteAttributeRequest(app::WriteClientHandle aHandle, Callback::Cancelable * onSuccessCallback,
                                                      Callback::Cancelable * onFailureCallback)
{
    uint8_t seqNum = GetNextSequenceNumber();
    CHIP_ERROR err = CHIP_NO_ERROR;

    aHandle->SetAppIdentifier(seqNum);
    ReturnErrorOnFailure(mDelegate->PreparePeer());

    AddResponseHandler(seqNum, onSuccessCallback, onFailureCallback);
    if ((err = aHandle.SendWriteRequest(mDelegate->GetDeviceId(), 0, mDelegate->GetSessionHandle())) != CHIP_NO_ERROR)
    {
        CancelResponseHandler(seqNum);
    }
    return err;
}

void InteractionPeer::AddResponseHandler(uint8_t seqNum, Callback::Cancelable * onSuccessCallback,
                                         Callback::Cancelable * onFailureCallback, app::TLVDataFilter tlvDataFilter)
{
    app::CHIPDeviceCallbacksMgr::GetInstance().AddResponseCallback(mDelegate->GetDeviceId(), seqNum, onSuccessCallback,
                                                                   onFailureCallback, tlvDataFilter);
}

void InteractionPeer::CancelResponseHandler(uint8_t seqNum)
{
    app::CHIPDeviceCallbacksMgr::GetInstance().CancelResponseCallback(mDelegate->GetDeviceId(), seqNum);
}

void InteractionPeer::AddIMResponseHandler(app::CommandSender * commandObj, Callback::Cancelable * onSuccessCallback,
                                           Callback::Cancelable * onFailureCallback)
{
    // We are using the pointer to command sender object as the identifier of command transactions. This makes sense as long as
    // there are only one active command transaction on one command sender object. This is a bit tricky, we try to assume that
    // chip::NodeId is uint64_t so the pointer can be used as a NodeId for CallbackMgr.
    static_assert(std::is_same<chip::NodeId, uint64_t>::value, "chip::NodeId is not uint64_t");
    chip::NodeId transactionId = reinterpret_cast<chip::NodeId>(commandObj);
    app::CHIPDeviceCallbacksMgr::GetInstance().AddResponseCallback(transactionId, 0 /* seqNum, always 0 for IM before #6559 */,
                                                                   onSuccessCallback, onFailureCallback);
}

void InteractionPeer::CancelIMResponseHandler(app::CommandSender * commandObj)
{
    // We are using the pointer to command sender object as the identifier of command transactions. This makes sense as long as
    // there are only one active command transaction on one command sender object. This is a bit tricky, we try to assume that
    // chip::NodeId is uint64_t so the pointer can be used as a NodeId for CallbackMgr.
    static_assert(std::is_same<chip::NodeId, uint64_t>::value, "chip::NodeId is not uint64_t");
    chip::NodeId transactionId = reinterpret_cast<chip::NodeId>(commandObj);
    app::CHIPDeviceCallbacksMgr::GetInstance().CancelResponseCallback(transactionId, 0 /* seqNum, always 0 for IM before #6559 */);
}

void InteractionPeer::AddReportHandler(EndpointId endpoint, ClusterId cluster, AttributeId attribute,
                                       Callback::Cancelable * onReportCallback)
{
    app::CHIPDeviceCallbacksMgr::GetInstance().AddReportCallback(mDelegate->GetDeviceId(), endpoint, cluster, attribute,
                                                                 onReportCallback);
}

CHIP_ERROR InteractionPeer::SendCommands(app::CommandSender * commandObj)
{
    ReturnErrorOnFailure(mDelegate->PreparePeer());
    VerifyOrReturnError(commandObj != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    return commandObj->SendCommandRequest(mDelegate->GetDeviceId(), mDelegate->GetFabricIndex(), mDelegate->GetSessionHandle());
}

} // namespace Controller
} // namespace chip
