/*
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
#import <Foundation/Foundation.h>

#import "MTRBaseDevice.h"
#import "MTRCluster_internal.h"
#import "zap-generated/MTRCallbackBridge_internal.h"

#include <app/ReadClient.h>
#include <lib/core/CHIPTLV.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/CHIPMem.h>

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
    if (params.resubscribeIfLost) {
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

NS_ASSUME_NONNULL_END
