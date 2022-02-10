/*
 *   Copyright (c) 2021 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#pragma once

#include <app-common/zap-generated/cluster-objects.h>
#include <app/CommandSender.h>
#include <app/DeviceProxy.h>
#include <app/InteractionModelEngine.h>
#include <app/util/error-mapping.h>
#include <lib/core/Optional.h>

namespace chip {
namespace Controller {
namespace detail {
template <typename ResponseType>
class ResponseReceiver : public app::CommandSender::Callback
{
public:
    using SuccessCallback = void (*)(void * context, const ResponseType & data);
    using FailureCallback = void (*)(void * context, CHIP_ERROR err);
    using DoneCallback    = void (*)(void * context);

    virtual ~ResponseReceiver() {}

protected:
    ResponseReceiver(void * aContext, SuccessCallback aOnSuccess, FailureCallback aOnError, DoneCallback aOnDone) :
        mContext(aContext), mOnSuccess(aOnSuccess), mOnError(aOnError), mOnDone(aOnDone)
    {}

    inline void OnResponse(app::CommandSender * aCommandSender, const app::ConcreteCommandPath & aPath,
                           const app::StatusIB & aStatus, TLV::TLVReader * aData) override;

    void OnError(const app::CommandSender * aCommandSender, CHIP_ERROR aError) override { mOnError(mContext, aError); }

    void OnDone(app::CommandSender * aCommandSender) override
    {
        if (mOnDone != nullptr)
        {
            mOnDone(mContext);
        }

        Platform::Delete(aCommandSender);
        Platform::Delete(this);
    }

private:
    void * mContext;
    SuccessCallback mOnSuccess;
    FailureCallback mOnError;
    DoneCallback mOnDone = nullptr;
};

template <typename RequestType>
class CommandInvoker final : public ResponseReceiver<typename RequestType::ResponseType>
{
    using Super = ResponseReceiver<typename RequestType::ResponseType>;

public:
    CommandInvoker(void * aContext, typename Super::SuccessCallback aOnSuccess, typename Super::FailureCallback aOnError,
                   typename Super::DoneCallback aOnDone) :
        Super(aContext, aOnSuccess, aOnError, aOnDone)
    {}

    /**
     * Use of CommandInvoker looks as follows:
     *
     *   auto invoker = CommandInvoker<type>::Alloc(args);
     *   VerifyOrReturnError(invoker != nullptr, CHIP_ERROR_NO_MEMORY);
     *   ReturnErrorOnFailure(invoker->InvokeCommand(args));
     *   invoker.release(); // The invoker will deallocate itself now.
     */
    static auto Alloc(void * aContext, typename Super::SuccessCallback aOnSuccess, typename Super::FailureCallback aOnError,
                      typename Super::DoneCallback aOnDone)
    {
        return Platform::MakeUnique<CommandInvoker>(aContext, aOnSuccess, aOnError, aOnDone);
    }

    CHIP_ERROR InvokeCommand(DeviceProxy * aDevice, EndpointId aEndpoint, const RequestType & aRequestData,
                             const Optional<uint16_t> & aTimedInvokeTimeoutMs)
    {
        app::CommandPathParams commandPath = { aEndpoint, 0 /* groupId */, RequestType::GetClusterId(), RequestType::GetCommandId(),
                                               (app::CommandPathFlags::kEndpointIdValid) };
        auto commandSender =
            Platform::MakeUnique<app::CommandSender>(this, aDevice->GetExchangeManager(), aTimedInvokeTimeoutMs.HasValue());
        VerifyOrReturnError(commandSender != nullptr, CHIP_ERROR_NO_MEMORY);

        ReturnErrorOnFailure(commandSender->AddRequestDataNoTimedCheck(commandPath, aRequestData, aTimedInvokeTimeoutMs));
        ReturnErrorOnFailure(commandSender->SendCommandRequest(aDevice->GetSecureSession().Value()));
        commandSender.release();
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR InvokeGroupCommand(Messaging::ExchangeManager * exchangeManager, FabricIndex fabric, GroupId groupId,
                                  NodeId sourceNodeId, const RequestType & aRequestData)
    {
        app::CommandPathParams commandPath = { 0 /* endpoint */, groupId, RequestType::GetClusterId(), RequestType::GetCommandId(),
                                               (app::CommandPathFlags::kGroupIdValid) };

        auto commandSender = Platform::MakeUnique<app::CommandSender>(this, exchangeManager);
        VerifyOrReturnError(commandSender != nullptr, CHIP_ERROR_NO_MEMORY);

        ReturnErrorOnFailure(commandSender->AddRequestData(commandPath, aRequestData));

        Optional<SessionHandle> session = exchangeManager->GetSessionManager()->CreateGroupSession(groupId, fabric, sourceNodeId);
        if (!session.HasValue())
        {
            return CHIP_ERROR_NO_MEMORY;
        }

        // this (invoker) and commandSender will be deleted by the onDone call before the return of SendGroupCommandRequest
        // this (invoker) should not be used after the SendGroupCommandRequest call
        ReturnErrorOnFailure(commandSender->SendGroupCommandRequest(session.Value()));

        // this (invoker) and commandSender are already deleted and are not to be used
        commandSender.release();
        exchangeManager->GetSessionManager()->RemoveGroupSession(session.Value()->AsGroupSession());

        return CHIP_NO_ERROR;
    }
};

template <typename ResponseType>
void ResponseReceiver<ResponseType>::OnResponse(app::CommandSender * aCommandSender, const app::ConcreteCommandPath & aPath,
                                                const app::StatusIB & aStatus, TLV::TLVReader * aData)
{
    ResponseType response;
    CHIP_ERROR err = CHIP_NO_ERROR;

    //
    // We're expecting response data in this variant of OnResponse. Consequently, aReader should always be
    // non-null. If it is, it means we received a success status code instead, which is not what was expected.
    //
    VerifyOrExit(aData != nullptr, err = CHIP_ERROR_SCHEMA_MISMATCH);

    //
    // Validate that the data response we received matches what we expect in terms of its cluster and command IDs.
    //
    VerifyOrExit(aPath.mClusterId == ResponseType::GetClusterId() && aPath.mCommandId == ResponseType::GetCommandId(),
                 err = CHIP_ERROR_SCHEMA_MISMATCH);

    err = app::DataModel::Decode(*aData, response);
    SuccessOrExit(err);

    mOnSuccess(mContext, response);

exit:
    if (err != CHIP_NO_ERROR)
    {
        mOnError(mContext, err);
    }
}

template <>
inline void ResponseReceiver<app::DataModel::NullObjectType>::OnResponse(app::CommandSender * aCommandSender,
                                                                         const app::ConcreteCommandPath & aPath,
                                                                         const app::StatusIB & aStatus, TLV::TLVReader * aData)
{
    //
    // If we got a valid reader, it means we received response data that we were not expecting to receive.
    //
    if (aData != nullptr)
    {
        mOnError(mContext, CHIP_ERROR_SCHEMA_MISMATCH);
        return;
    }

    app::DataModel::NullObjectType nullResp;
    mOnSuccess(mContext, nullResp);
}

} // namespace detail

template <typename RequestType>
CHIP_ERROR InvokeCommand(DeviceProxy * aDevice, void * aContext,
                         typename detail::CommandInvoker<RequestType>::SuccessCallback aSuccessCallback,
                         typename detail::CommandInvoker<RequestType>::FailureCallback aFailureCallback, EndpointId aEndpoint,
                         const RequestType & aRequestData, const Optional<uint16_t> & aTimedInvokeTimeoutMs)
{
    auto invoker =
        detail::CommandInvoker<RequestType>::Alloc(aContext, aSuccessCallback, aFailureCallback, nullptr /* aDoneCallback */);
    VerifyOrReturnError(invoker != nullptr, CHIP_ERROR_NO_MEMORY);
    ReturnErrorOnFailure(invoker->InvokeCommand(aDevice, aEndpoint, aRequestData, aTimedInvokeTimeoutMs));
    invoker.release();
    return CHIP_NO_ERROR;
}

template <typename RequestType>
CHIP_ERROR InvokeCommand(DeviceProxy * aDevice, void * aContext,
                         typename detail::CommandInvoker<RequestType>::SuccessCallback aSuccessCallback,
                         typename detail::CommandInvoker<RequestType>::FailureCallback aFailureCallback, EndpointId aEndpoint,
                         const RequestType & aRequestData, uint16_t aTimedInvokeTimeoutMs)
{
    return InvokeCommand(aDevice, aContext, aSuccessCallback, aFailureCallback, aEndpoint, aRequestData,
                         MakeOptional(aTimedInvokeTimeoutMs));
}

template <typename RequestType, typename std::enable_if_t<!RequestType::MustUseTimedInvoke(), int> = 0>
CHIP_ERROR InvokeCommand(DeviceProxy * aDevice, void * aContext,
                         typename detail::CommandInvoker<RequestType>::SuccessCallback aSuccessCallback,
                         typename detail::CommandInvoker<RequestType>::FailureCallback aFailureCallback, EndpointId aEndpoint,
                         const RequestType & aRequestData)
{
    return InvokeCommand(aDevice, aContext, aSuccessCallback, aFailureCallback, aEndpoint, aRequestData, NullOptional);
}

// Group commands can't do timed invoke in a meaningful way.
template <typename RequestType, typename std::enable_if_t<!RequestType::MustUseTimedInvoke(), int> = 0>
CHIP_ERROR InvokeGroupCommand(DeviceProxy * aDevice, void * aContext,
                              typename detail::CommandInvoker<RequestType>::SuccessCallback aSuccessCallback,
                              typename detail::CommandInvoker<RequestType>::FailureCallback aFailureCallback,
                              typename detail::CommandInvoker<RequestType>::DoneCallback aDoneCallback, GroupId groupId,
                              const RequestType & aRequestData)
{
    auto invoker = detail::CommandInvoker<RequestType>::Alloc(aContext, aSuccessCallback, aFailureCallback, aDoneCallback);
    VerifyOrReturnError(invoker != nullptr, CHIP_ERROR_NO_MEMORY);

    // invoker will be deleted by the onDone call before the return of InvokeGroupCommand
    // invoker should not be used after the InvokeGroupCommand call
    //
    //  We assume the aDevice already has a Case session which is way we can use he established Secure Session
    ReturnErrorOnFailure(invoker->InvokeGroupCommand(aDevice->GetExchangeManager(),
                                                     aDevice->GetSecureSession().Value()->GetFabricIndex(), groupId,
                                                     aDevice->GetDeviceId(), aRequestData));

    //  invoker is already deleted and is not to be used
    invoker.release();
    return CHIP_NO_ERROR;
}

} // namespace Controller
} // namespace chip
