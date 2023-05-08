/*
 *    Copyright (c) 2022-2023 Project CHIP Authors
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
#import <Foundation/Foundation.h>

#import "MTRBaseDevice.h"
#import "MTRCallbackBridge.h"
#import "MTRCluster_Internal.h"

#include <app/CommandSender.h>
#include <app/ReadClient.h>
#include <app/data-model/NullObject.h>
#include <lib/core/DataModelTypes.h>
#include <lib/core/TLV.h>
#include <lib/support/CHIPMem.h>
#include <system/SystemClock.h>

NS_ASSUME_NONNULL_BEGIN

/**
 * Utility functions base clusters use for doing reads and subscribes.
 */
template <typename BridgeType, typename DecodableAttributeType>
class MTRAttributeReportCallback : public chip::app::ReadClient::Callback {
public:
    MTRAttributeReportCallback(BridgeType * _Nonnull bridge, typename BridgeType::SuccessCallbackType _Nonnull onAttributeReport,
        MTRErrorCallback _Nonnull onError, chip::ClusterId clusterID, chip::AttributeId attributeID)
        : mBridge(bridge)
        , mOnAttributeReport(onAttributeReport)
        , mOnError(onError)
        , mClusterID(clusterID)
        , mAttributeID(attributeID)
        , mBufferedReadAdapter(*this)
    {
    }

    ~MTRAttributeReportCallback() {}

    chip::app::BufferedReadCallback & GetBufferedCallback() { return mBufferedReadAdapter; }

    void AdoptReadClient(chip::Platform::UniquePtr<chip::app::ReadClient> readClient) { mReadClient = std::move(readClient); }

protected:
    void OnAttributeData(
        const chip::app::ConcreteDataAttributePath & path, chip::TLV::TLVReader * data, const chip::app::StatusIB & status) override
    {
        if (mCalledCallback && mReadClient->IsReadType()) {
            return;
        }
        mCalledCallback = true;

        CHIP_ERROR err = CHIP_NO_ERROR;
        DecodableAttributeType value;

        //
        // We shouldn't be getting list item operations in the provided path since that should be handled by the buffered read
        // callback. If we do, that's a bug.
        //
        VerifyOrDie(!path.IsListItemOperation());

        VerifyOrExit(status.IsSuccess(), err = status.ToChipError());
        VerifyOrExit(path.mClusterId == mClusterID && path.mAttributeId == mAttributeID, err = CHIP_ERROR_SCHEMA_MISMATCH);
        VerifyOrExit(data != nullptr, err = CHIP_ERROR_INVALID_ARGUMENT);

        SuccessOrExit(err = chip::app::DataModel::Decode(*data, value));

        mOnAttributeReport(mBridge, value);

    exit:
        if (err != CHIP_NO_ERROR) {
            mOnError(mBridge, err);
        }
    }

    void OnError(CHIP_ERROR error) override
    {
        if (mCalledCallback && mReadClient->IsReadType()) {
            return;
        }
        mCalledCallback = true;

        mOnError(mBridge, error);
    }

    void OnDone(chip::app::ReadClient *) override { chip::Platform::Delete(this); }

    BridgeType * _Nonnull mBridge;

    chip::ClusterId mClusterID;
    chip::AttributeId mAttributeID;
    typename BridgeType::SuccessCallbackType mOnAttributeReport;
    MTRErrorCallback mOnError;
    chip::app::BufferedReadCallback mBufferedReadAdapter;
    chip::Platform::UniquePtr<chip::app::ReadClient> mReadClient;
    // For reads, we ensure that we make only one data/error callback to our consumer.
    bool mCalledCallback = false;
};

template <typename SubscriptionBridgeType, typename DecodableAttributeType>
class MTRAttributeSubscriptionCallback : public MTRAttributeReportCallback<SubscriptionBridgeType, DecodableAttributeType> {
public:
    MTRAttributeSubscriptionCallback(SubscriptionBridgeType * _Nonnull bridge,
        typename SubscriptionBridgeType::SuccessCallbackType onAttributeReport, MTRErrorCallback onError, chip::ClusterId clusterID,
        chip::AttributeId attributeID)
        : MTRAttributeReportCallback<SubscriptionBridgeType, DecodableAttributeType>(
            bridge, onAttributeReport, onError, clusterID, attributeID)
    {
    }

    ~MTRAttributeSubscriptionCallback()
    {
        // Ensure we release the ReadClient before we tear down anything else,
        // so it can call our OnDeallocatePaths properly.
        this->mReadClient = nullptr;
    }

private:
    // The superclass OnResubscriptionNeeded is fine for our purposes.

    void OnDeallocatePaths(chip::app::ReadPrepareParams && readPrepareParams) override
    {
        VerifyOrDie(readPrepareParams.mAttributePathParamsListSize == 1 && readPrepareParams.mpAttributePathParamsList != nullptr);
        chip::Platform::Delete<chip::app::AttributePathParams>(readPrepareParams.mpAttributePathParamsList);

        if (readPrepareParams.mDataVersionFilterListSize == 1 && readPrepareParams.mpDataVersionFilterList != nullptr) {
            chip::Platform::Delete<chip::app::DataVersionFilter>(readPrepareParams.mpDataVersionFilterList);
        }
    }

    void OnSubscriptionEstablished(chip::SubscriptionId subscriptionId) override { this->mBridge->OnSubscriptionEstablished(); }

    void OnDone(chip::app::ReadClient * readClient) override
    {
        this->mBridge->OnDone();
        MTRAttributeReportCallback<SubscriptionBridgeType, DecodableAttributeType>::OnDone(readClient);
    }
};

template <typename DecodableAttributeType, typename BridgeType>
CHIP_ERROR MTRStartReadInteraction(BridgeType * _Nonnull bridge, MTRReadParams * params,
    chip::Messaging::ExchangeManager & exchangeManager, const chip::SessionHandle & session,
    typename BridgeType::SuccessCallbackType successCb, MTRErrorCallback failureCb, chip::EndpointId endpoint,
    chip::ClusterId clusterID, chip::AttributeId attributeID)
{
    auto readPaths = chip::Platform::MakeUnique<chip::app::AttributePathParams>(endpoint, clusterID, attributeID);
    VerifyOrReturnError(readPaths != nullptr, CHIP_ERROR_NO_MEMORY);

    chip::app::ReadPrepareParams readPrepareParams(session);
    [params toReadPrepareParams:readPrepareParams];
    readPrepareParams.mpAttributePathParamsList = readPaths.get();
    readPrepareParams.mAttributePathParamsListSize = 1;

    auto callback = chip::Platform::MakeUnique<MTRAttributeReportCallback<BridgeType, DecodableAttributeType>>(
        bridge, successCb, failureCb, clusterID, attributeID);
    VerifyOrReturnError(callback != nullptr, CHIP_ERROR_NO_MEMORY);

    auto readClient = chip::Platform::MakeUnique<chip::app::ReadClient>(chip::app::InteractionModelEngine::GetInstance(),
        &exchangeManager, callback->GetBufferedCallback(), chip::app::ReadClient::InteractionType::Read);
    VerifyOrReturnError(readClient != nullptr, CHIP_ERROR_NO_MEMORY);

    CHIP_ERROR err = readClient->SendRequest(readPrepareParams);
    ReturnErrorOnFailure(err);

    callback->AdoptReadClient(std::move(readClient));
    callback.release();

    return CHIP_NO_ERROR;
}

template <typename DecodableAttributeType, typename BridgeType>
CHIP_ERROR MTRStartSubscribeInteraction(BridgeType * _Nonnull bridge, MTRSubscribeParams * params,
    chip::Messaging::ExchangeManager & exchangeManager, const chip::SessionHandle & session,
    typename BridgeType::SuccessCallbackType successCb, MTRErrorCallback failureCb, chip::EndpointId endpoint,
    chip::ClusterId clusterID, chip::AttributeId attributeID)
{
    auto readPaths = chip::Platform::MakeUnique<chip::app::AttributePathParams>(endpoint, clusterID, attributeID);
    VerifyOrReturnError(readPaths != nullptr, CHIP_ERROR_NO_MEMORY);

    chip::app::ReadPrepareParams readPrepareParams(session);
    [params toReadPrepareParams:readPrepareParams];
    readPrepareParams.mpAttributePathParamsList = readPaths.get();
    readPrepareParams.mAttributePathParamsListSize = 1;

    auto callback = chip::Platform::MakeUnique<MTRAttributeSubscriptionCallback<BridgeType, DecodableAttributeType>>(
        bridge, successCb, failureCb, clusterID, attributeID);
    VerifyOrReturnError(callback != nullptr, CHIP_ERROR_NO_MEMORY);

    auto readClient = chip::Platform::MakeUnique<chip::app::ReadClient>(chip::app::InteractionModelEngine::GetInstance(),
        &exchangeManager, callback->GetBufferedCallback(), chip::app::ReadClient::InteractionType::Subscribe);
    VerifyOrReturnError(readClient != nullptr, CHIP_ERROR_NO_MEMORY);

    CHIP_ERROR err;
    if (params.resubscribeAutomatically) {
        readPaths.release();

        err = readClient->SendAutoResubscribeRequest(std::move(readPrepareParams));
    } else {
        err = readClient->SendRequest(readPrepareParams);
    }
    ReturnErrorOnFailure(err);

    bridge->KeepAliveOnCallback();

    callback->AdoptReadClient(std::move(readClient));
    callback.release();

    return CHIP_NO_ERROR;
}

template <typename SubscriptionBridgeType, typename AttributeObjCType, typename DecodableAttributeType>
void MTRSubscribeAttribute(MTRSubscribeParams * _Nonnull params,
    MTRSubscriptionEstablishedHandler _Nullable subscriptionEstablished,
    void (^reportHandler)(AttributeObjCType * _Nullable value, NSError * _Nullable error), dispatch_queue_t callbackQueue,
    MTRBaseDevice * device, chip::EndpointId endpoint, chip::ClusterId clusterID, chip::AttributeId attributeID)
{
    // Make a copy of params before we go async.
    params = [params copy];
    auto * callbackBridge = new SubscriptionBridgeType(
        callbackQueue,
        // This treats reportHandler as taking an id for the data.  This is
        // not great from a type-safety perspective, of course.
        reportHandler,
        ^(chip::Messaging::ExchangeManager & exchangeManager, const chip::SessionHandle & session,
            typename SubscriptionBridgeType::SuccessCallbackType successCb, MTRErrorCallback failureCb,
            MTRCallbackBridgeBase * bridge) {
            auto * subscriptionBridge = static_cast<SubscriptionBridgeType *>(bridge);
            return MTRStartSubscribeInteraction<DecodableAttributeType>(
                subscriptionBridge, params, exchangeManager, session, successCb, failureCb, endpoint, clusterID, attributeID);
        },
        subscriptionEstablished);
    std::move(*callbackBridge).DispatchAction(device);
}

template <typename ReadBridgeType, typename AttributeObjCType, typename DecodableAttributeType>
void MTRReadAttribute(MTRReadParams * _Nonnull params,
    void (^reportHandler)(AttributeObjCType * _Nullable value, NSError * _Nullable error), dispatch_queue_t callbackQueue,
    MTRBaseDevice * device, chip::EndpointId endpoint, chip::ClusterId clusterID, chip::AttributeId attributeID)
{
    // Make a copy of params before we go async.
    params = [params copy];
    auto * callbackBridge = new ReadBridgeType(callbackQueue,
        // This treats reportHandler as taking an id for the data.  This is
        // not great from a type-safety perspective, of course.
        reportHandler,
        ^(chip::Messaging::ExchangeManager & exchangeManager, const chip::SessionHandle & session,
            typename ReadBridgeType::SuccessCallbackType successCb, MTRErrorCallback failureCb, MTRCallbackBridgeBase * bridge) {
            auto * readBridge = static_cast<ReadBridgeType *>(bridge);
            return MTRStartReadInteraction<DecodableAttributeType>(
                readBridge, params, exchangeManager, session, successCb, failureCb, endpoint, clusterID, attributeID);
        });
    std::move(*callbackBridge).DispatchAction(device);
}

/**
 * Utility functions base clusters use for doing commands.
 */
template <typename InvokeBridgeType, typename ResponseType> class MTRInvokeCallback : public chip::app::CommandSender::Callback {
public:
    MTRInvokeCallback(InvokeBridgeType * _Nonnull bridge, typename InvokeBridgeType::SuccessCallbackType _Nonnull onResponse,
        MTRErrorCallback _Nonnull onError)
        : mBridge(bridge)
        , mOnResponse(onResponse)
        , mOnError(onError)
    {
    }

    ~MTRInvokeCallback() {}

    void AdoptCommandSender(chip::Platform::UniquePtr<chip::app::CommandSender> commandSender)
    {
        mCommandSender = std::move(commandSender);
    }

protected:
    // We need to have different OnResponse implementations depending on whether
    // ResponseType is DataModel::NullObjectType or not.  Since template class methods
    // can't be partially specialized (either you have to partially specialize
    // the class template, or you have to fully specialize the method), use
    // enable_if to deal with this.
    void OnResponse(chip::app::CommandSender * commandSender, const chip::app::ConcreteCommandPath & commandPath,
        const chip::app::StatusIB & status, chip::TLV::TLVReader * reader) override
    {
        HandleResponse(commandSender, commandPath, status, reader);
    }

    /**
     * Response handler for data responses.
     */
    template <typename T = ResponseType, std::enable_if_t<!std::is_same<T, chip::app::DataModel::NullObjectType>::value, int> = 0>
    void HandleResponse(chip::app::CommandSender * commandSender, const chip::app::ConcreteCommandPath & commandPath,
        const chip::app::StatusIB & status, chip::TLV::TLVReader * reader)
    {
        if (mCalledCallback) {
            return;
        }
        mCalledCallback = true;

        ResponseType response;
        CHIP_ERROR err = CHIP_NO_ERROR;

        //
        // We're expecting response data in this variant of OnResponse. Consequently, reader should always be
        // non-null. If it is, it means we received a success status code instead, which is not what was expected.
        //
        VerifyOrExit(reader != nullptr, err = CHIP_ERROR_SCHEMA_MISMATCH);

        //
        // Validate that the data response we received matches what we expect in terms of its cluster and command IDs.
        //
        VerifyOrExit(
            commandPath.mClusterId == ResponseType::GetClusterId() && commandPath.mCommandId == ResponseType::GetCommandId(),
            err = CHIP_ERROR_SCHEMA_MISMATCH);

        err = chip::app::DataModel::Decode(*reader, response);
        SuccessOrExit(err);

        mOnResponse(mBridge, response);

    exit:
        if (err != CHIP_NO_ERROR) {
            mOnError(mBridge, err);
        }
    }

    /**
     * Response handler for status responses.
     */
    template <typename T = ResponseType, std::enable_if_t<std::is_same<T, chip::app::DataModel::NullObjectType>::value, int> = 0>
    void HandleResponse(chip::app::CommandSender * commandSender, const chip::app::ConcreteCommandPath & commandPath,
        const chip::app::StatusIB & status, chip::TLV::TLVReader * reader)
    {
        if (mCalledCallback) {
            return;
        }
        mCalledCallback = true;

        //
        // If we got a valid reader, it means we received response data that we were not expecting to receive.
        //
        if (reader != nullptr) {
            mOnError(mBridge, CHIP_ERROR_SCHEMA_MISMATCH);
            return;
        }

        chip::app::DataModel::NullObjectType nullResp;
        mOnResponse(mBridge, nullResp);
    }

    void OnError(const chip::app::CommandSender * commandSender, CHIP_ERROR error) override
    {
        if (mCalledCallback) {
            return;
        }
        mCalledCallback = true;

        mOnError(mBridge, error);
    }

    void OnDone(chip::app::CommandSender * commandSender) override
    {
        if (!mCalledCallback) {
            // This can happen if the server sends a response with an empty
            // InvokeResponses list.  Since we are not sending wildcard command
            // paths, that's not a valid response and we should treat it as an
            // error.  Use the error we would have gotten if we in fact expected
            // a nonempty list.
            OnError(commandSender, CHIP_END_OF_TLV);
        }

        chip::Platform::Delete(this);
    }

    InvokeBridgeType * _Nonnull mBridge;

    typename InvokeBridgeType::SuccessCallbackType mOnResponse;
    MTRErrorCallback mOnError;
    chip::Platform::UniquePtr<chip::app::CommandSender> mCommandSender;
    // For reads, we ensure that we make only one data/error callback to our consumer.
    bool mCalledCallback = false;
};

/**
 * timedInvokeTimeoutMs, if provided, is how long the server will wait for us to
 * send the invoke after we sent the Timed Request message.
 *
 * invokeTimeout, if provided, will have possible MRP latency added to it and
 * the result is how long we will wait for the server to respond.
 */
template <typename BridgeType, typename RequestDataType>
CHIP_ERROR MTRStartInvokeInteraction(BridgeType * _Nonnull bridge, const RequestDataType & requestData,
    chip::Messaging::ExchangeManager & exchangeManager, const chip::SessionHandle & session,
    typename BridgeType::SuccessCallbackType successCb, MTRErrorCallback failureCb, chip::EndpointId endpoint,
    chip::Optional<uint16_t> timedInvokeTimeoutMs, chip::Optional<chip::System::Clock::Timeout> invokeTimeout)
{
    auto callback = chip::Platform::MakeUnique<MTRInvokeCallback<BridgeType, typename RequestDataType::ResponseType>>(
        bridge, successCb, failureCb);
    VerifyOrReturnError(callback != nullptr, CHIP_ERROR_NO_MEMORY);

    auto commandSender
        = chip::Platform::MakeUnique<chip::app::CommandSender>(callback.get(), &exchangeManager, timedInvokeTimeoutMs.HasValue());
    VerifyOrReturnError(commandSender != nullptr, CHIP_ERROR_NO_MEMORY);

    chip::app::CommandPathParams commandPath(endpoint, 0, RequestDataType::GetClusterId(), RequestDataType::GetCommandId(),
        chip::app::CommandPathFlags::kEndpointIdValid);
    ReturnErrorOnFailure(commandSender->AddRequestData(commandPath, requestData, timedInvokeTimeoutMs));

    if (invokeTimeout.HasValue()) {
        invokeTimeout.SetValue(session->ComputeRoundTripTimeout(invokeTimeout.Value()));
    }
    ReturnErrorOnFailure(commandSender->SendCommandRequest(session, invokeTimeout));

    callback->AdoptCommandSender(std::move(commandSender));
    callback.release();

    return CHIP_NO_ERROR;
};

NS_ASSUME_NONNULL_END
