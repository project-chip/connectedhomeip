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
    using FailureCallback = void (*)(void * context, EmberAfStatus status);

    virtual ~ResponseReceiver() {}

protected:
    ResponseReceiver(void * aContext, SuccessCallback aOnSuccess, FailureCallback aOnError) :
        mContext(aContext), mOnSuccess(aOnSuccess), mOnError(aOnError)
    {}

    inline void OnResponse(app::CommandSender * aCommandSender, const app::ConcreteCommandPath & aPath,
                           const app::StatusIB & aStatus, TLV::TLVReader * aData) override;

    void OnError(const app::CommandSender * aCommandSender, const app::StatusIB & aStatus, CHIP_ERROR aError) override
    {
        mOnError(mContext, app::ToEmberAfStatus(aStatus.mStatus));
    }

    void OnDone(app::CommandSender * aCommandSender) override
    {
        Platform::Delete(aCommandSender);
        Platform::Delete(this);
    }

private:
    void * mContext;
    SuccessCallback mOnSuccess;
    FailureCallback mOnError;
};

template <typename RequestType>
class CommandInvoker final : public ResponseReceiver<typename RequestType::ResponseType>
{
    using Super = ResponseReceiver<typename RequestType::ResponseType>;

public:
    CommandInvoker(void * aContext, typename Super::SuccessCallback aOnSuccess, typename Super::FailureCallback aOnError) :
        Super(aContext, aOnSuccess, aOnError)
    {}

    /**
     * Use of CommandInvoker looks as follows:
     *
     *   auto invoker = CommandInvoker<type>::Alloc(args);
     *   VerifyOrReturnError(invoker != nullptr, CHIP_ERROR_NO_MEMORY);
     *   ReturnErrorOnFailure(invoker->InvokeCommand(args));
     *   invoker.release(); // The invoker will deallocate itself now.
     */
    static auto Alloc(void * aContext, typename Super::SuccessCallback aOnSuccess, typename Super::FailureCallback aOnError)
    {
        return Platform::MakeUnique<CommandInvoker>(aContext, aOnSuccess, aOnError);
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

    CHIP_ERROR InvokeGroupCommand(DeviceProxy * aDevice, GroupId groupId, const RequestType & aRequestData)
    {
        app::CommandPathParams commandPath = { 0 /* endpoint */, groupId, RequestType::GetClusterId(), RequestType::GetCommandId(),
                                               (app::CommandPathFlags::kGroupIdValid) };

        auto commandSender = Platform::MakeUnique<app::CommandSender>(this, aDevice->GetExchangeManager());
        VerifyOrReturnError(commandSender != nullptr, CHIP_ERROR_NO_MEMORY);

        ReturnErrorOnFailure(commandSender->AddRequestData(commandPath, aRequestData));

        if (aDevice->GetSecureSession().HasValue())
        {
            SessionHandle session = aDevice->GetSecureSession().Value();
            session.SetGroupId(groupId);

            if (!session.IsGroupSession())
            {
                return CHIP_ERROR_INCORRECT_STATE;
            }

            ReturnErrorOnFailure(commandSender->SendCommandRequest(session));
        }
        else
        {
            // something fishy is going on
            return CHIP_ERROR_INCORRECT_STATE;
        }

        commandSender.release();
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
        mOnError(mContext, app::ToEmberAfStatus(Protocols::InteractionModel::Status::Failure));
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
        mOnError(mContext, app::ToEmberAfStatus(Protocols::InteractionModel::Status::Failure));
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
    auto invoker = detail::CommandInvoker<RequestType>::Alloc(aContext, aSuccessCallback, aFailureCallback);
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
                              typename detail::CommandInvoker<RequestType>::FailureCallback aFailureCallback, GroupId groupId,
                              const RequestType & aRequestData)
{
    auto invoker = detail::CommandInvoker<RequestType>::Alloc(aContext, aSuccessCallback, aFailureCallback);
    VerifyOrReturnError(invoker != nullptr, CHIP_ERROR_NO_MEMORY);
    ReturnErrorOnFailure(invoker->InvokeGroupCommand(aDevice, groupId, aRequestData));
    invoker.release();
    return CHIP_NO_ERROR;
}

} // namespace Controller
} // namespace chip
