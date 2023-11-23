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

#include <app/AppConfig.h>
#include <app/AttributePathParams.h>
#include <app/InteractionModelEngine.h>
#include <app/ReadPrepareParams.h>
#include <controller/TypedReadCallback.h>

#if CHIP_CONFIG_ENABLE_READ_CLIENT
namespace chip {
namespace Controller {
namespace detail {

using SubscriptionOnDoneCallback = std::function<void(void)>;

template <typename DecodableAttributeType>
struct ReportAttributeParams : public app::ReadPrepareParams
{
    ReportAttributeParams(const SessionHandle & sessionHandle) : app::ReadPrepareParams(sessionHandle)
    {
        mKeepSubscriptions = false;
    }
    typename TypedReadAttributeCallback<DecodableAttributeType>::OnSuccessCallbackType mOnReportCb;
    typename TypedReadAttributeCallback<DecodableAttributeType>::OnErrorCallbackType mOnErrorCb;
    typename TypedReadAttributeCallback<DecodableAttributeType>::OnSubscriptionEstablishedCallbackType
        mOnSubscriptionEstablishedCb = nullptr;
    typename TypedReadAttributeCallback<DecodableAttributeType>::OnResubscriptionAttemptCallbackType mOnResubscriptionAttemptCb =
        nullptr;
    SubscriptionOnDoneCallback mOnDoneCb         = nullptr;
    app::ReadClient::InteractionType mReportType = app::ReadClient::InteractionType::Read;
};

template <typename DecodableAttributeType>
CHIP_ERROR ReportAttribute(Messaging::ExchangeManager * exchangeMgr, EndpointId endpointId, ClusterId clusterId,
                           AttributeId attributeId, ReportAttributeParams<DecodableAttributeType> && readParams,
                           const Optional<DataVersion> & aDataVersion = NullOptional)
{
    app::InteractionModelEngine * engine = app::InteractionModelEngine::GetInstance();
    CHIP_ERROR err                       = CHIP_NO_ERROR;

    auto readPaths = Platform::MakeUnique<app::AttributePathParams>(endpointId, clusterId, attributeId);
    VerifyOrReturnError(readPaths != nullptr, CHIP_ERROR_NO_MEMORY);
    readParams.mpAttributePathParamsList    = readPaths.get();
    readParams.mAttributePathParamsListSize = 1;
    chip::Platform::UniquePtr<chip::app::DataVersionFilter> dataVersionFilters;
    if (aDataVersion.HasValue())
    {
        dataVersionFilters = Platform::MakeUnique<app::DataVersionFilter>(endpointId, clusterId, aDataVersion.Value());
        VerifyOrReturnError(dataVersionFilters != nullptr, CHIP_ERROR_NO_MEMORY);
        readParams.mpDataVersionFilterList    = dataVersionFilters.get();
        readParams.mDataVersionFilterListSize = 1;
    }
    auto onDoneCb = readParams.mOnDoneCb;
    auto onDone   = [onDoneCb](TypedReadAttributeCallback<DecodableAttributeType> * callback) {
        if (onDoneCb)
        {
            onDoneCb();
        }
        chip::Platform::Delete(callback);
    };

    auto callback = chip::Platform::MakeUnique<TypedReadAttributeCallback<DecodableAttributeType>>(
        clusterId, attributeId, readParams.mOnReportCb, readParams.mOnErrorCb, onDone, readParams.mOnSubscriptionEstablishedCb,
        readParams.mOnResubscriptionAttemptCb);
    VerifyOrReturnError(callback != nullptr, CHIP_ERROR_NO_MEMORY);

    auto readClient =
        chip::Platform::MakeUnique<app::ReadClient>(engine, exchangeMgr, callback->GetBufferedCallback(), readParams.mReportType);
    VerifyOrReturnError(readClient != nullptr, CHIP_ERROR_NO_MEMORY);

    if (readClient->IsSubscriptionType())
    {
        readPaths.release();
        dataVersionFilters.release();

        err = readClient->SendAutoResubscribeRequest(std::move(readParams));
        ReturnErrorOnFailure(err);
    }
    else
    {
        err = readClient->SendRequest(readParams);
        ReturnErrorOnFailure(err);
    }

    //
    // At this point, we'll get a callback through the OnDone callback above regardless of success or failure
    // of the read operation to permit us to free up the callback object. So, release ownership of the callback
    // object now to prevent it from being reclaimed at the end of this scoped block.
    //
    callback->AdoptReadClient(std::move(readClient));
    callback.release();

    return err;
}

} // namespace detail

/**
 * To avoid instantiating all the complicated read code on a per-attribute
 * basis, we have a helper that's just templated on the type.
 */
template <typename DecodableAttributeType>
CHIP_ERROR ReadAttribute(Messaging::ExchangeManager * exchangeMgr, const SessionHandle & sessionHandle, EndpointId endpointId,
                         ClusterId clusterId, AttributeId attributeId,
                         typename TypedReadAttributeCallback<DecodableAttributeType>::OnSuccessCallbackType onSuccessCb,
                         typename TypedReadAttributeCallback<DecodableAttributeType>::OnErrorCallbackType onErrorCb,
                         bool fabricFiltered = true)
{
    detail::ReportAttributeParams<DecodableAttributeType> params(sessionHandle);
    params.mOnReportCb       = onSuccessCb;
    params.mOnErrorCb        = onErrorCb;
    params.mIsFabricFiltered = fabricFiltered;
    return detail::ReportAttribute(exchangeMgr, endpointId, clusterId, attributeId, std::move(params), NullOptional);
}

/*
 * A typed read attribute function that takes as input a template parameter that encapsulates the type information
 * for a given attribute as well as callbacks for success and failure and either returns a decoded cluster-object representation
 * of the requested attribute through the provided success callback or calls the provided failure callback.
 *
 * The AttributeTypeInfo is generally expected to be a ClusterName::Attributes::AttributeName::TypeInfo struct, but any
 * object that contains type information exposed through a 'DecodableType' type declaration as well as GetClusterId() and
 * GetAttributeId() methods is expected to work.
 */
template <typename AttributeTypeInfo>
CHIP_ERROR
ReadAttribute(Messaging::ExchangeManager * exchangeMgr, const SessionHandle & sessionHandle, EndpointId endpointId,
              typename TypedReadAttributeCallback<typename AttributeTypeInfo::DecodableType>::OnSuccessCallbackType onSuccessCb,
              typename TypedReadAttributeCallback<typename AttributeTypeInfo::DecodableType>::OnErrorCallbackType onErrorCb,
              bool fabricFiltered = true)
{
    return ReadAttribute<typename AttributeTypeInfo::DecodableType>(
        exchangeMgr, sessionHandle, endpointId, AttributeTypeInfo::GetClusterId(), AttributeTypeInfo::GetAttributeId(), onSuccessCb,
        onErrorCb, fabricFiltered);
}

// Helper for SubscribeAttribute to reduce the amount of code generated.
template <typename DecodableAttributeType>
CHIP_ERROR SubscribeAttribute(
    Messaging::ExchangeManager * exchangeMgr, const SessionHandle & sessionHandle, EndpointId endpointId, ClusterId clusterId,
    AttributeId attributeId, typename TypedReadAttributeCallback<DecodableAttributeType>::OnSuccessCallbackType onReportCb,
    typename TypedReadAttributeCallback<DecodableAttributeType>::OnErrorCallbackType onErrorCb, uint16_t minIntervalFloorSeconds,
    uint16_t maxIntervalCeilingSeconds,
    typename TypedReadAttributeCallback<DecodableAttributeType>::OnSubscriptionEstablishedCallbackType onSubscriptionEstablishedCb =
        nullptr,
    typename TypedReadAttributeCallback<DecodableAttributeType>::OnResubscriptionAttemptCallbackType onResubscriptionAttemptCb =
        nullptr,
    bool fabricFiltered = true, bool keepPreviousSubscriptions = false, const Optional<DataVersion> & aDataVersion = NullOptional,
    typename detail::SubscriptionOnDoneCallback onDoneCb = nullptr)
{
    detail::ReportAttributeParams<DecodableAttributeType> params(sessionHandle);
    params.mOnReportCb                  = onReportCb;
    params.mOnErrorCb                   = onErrorCb;
    params.mOnSubscriptionEstablishedCb = onSubscriptionEstablishedCb;
    params.mOnResubscriptionAttemptCb   = onResubscriptionAttemptCb;
    params.mOnDoneCb                    = onDoneCb;
    params.mMinIntervalFloorSeconds     = minIntervalFloorSeconds;
    params.mMaxIntervalCeilingSeconds   = maxIntervalCeilingSeconds;
    params.mKeepSubscriptions           = keepPreviousSubscriptions;
    params.mReportType                  = app::ReadClient::InteractionType::Subscribe;
    params.mIsFabricFiltered            = fabricFiltered;
    return detail::ReportAttribute(exchangeMgr, endpointId, clusterId, attributeId, std::move(params), aDataVersion);
}

/*
 * A typed way to subscribe to the value of a single attribute.  See
 * documentation for ReadAttribute above for details on how AttributeTypeInfo
 * works.
 *
 * A const view-only reference to the underlying ReadClient is passed in through the OnSubscriptionEstablishedCallbackType
 * argument. This reference is valid until the error callback is invoked at which point, this reference is no longer valid
 * and should not be used any more.
 */
template <typename AttributeTypeInfo>
CHIP_ERROR SubscribeAttribute(
    Messaging::ExchangeManager * exchangeMgr, const SessionHandle & sessionHandle, EndpointId endpointId,
    typename TypedReadAttributeCallback<typename AttributeTypeInfo::DecodableType>::OnSuccessCallbackType onReportCb,
    typename TypedReadAttributeCallback<typename AttributeTypeInfo::DecodableType>::OnErrorCallbackType onErrorCb,
    uint16_t aMinIntervalFloorSeconds, uint16_t aMaxIntervalCeilingSeconds,
    typename TypedReadAttributeCallback<typename AttributeTypeInfo::DecodableType>::OnSubscriptionEstablishedCallbackType
        onSubscriptionEstablishedCb = nullptr,
    typename TypedReadAttributeCallback<typename AttributeTypeInfo::DecodableType>::OnResubscriptionAttemptCallbackType
        onResubscriptionAttemptCb = nullptr,
    bool fabricFiltered = true, bool keepPreviousSubscriptions = false, const Optional<DataVersion> & aDataVersion = NullOptional,
    typename detail::SubscriptionOnDoneCallback onDoneCb = nullptr)
{
    return SubscribeAttribute<typename AttributeTypeInfo::DecodableType>(
        exchangeMgr, sessionHandle, endpointId, AttributeTypeInfo::GetClusterId(), AttributeTypeInfo::GetAttributeId(), onReportCb,
        onErrorCb, aMinIntervalFloorSeconds, aMaxIntervalCeilingSeconds, onSubscriptionEstablishedCb, onResubscriptionAttemptCb,
        fabricFiltered, keepPreviousSubscriptions, aDataVersion, onDoneCb);
}

namespace detail {

template <typename DecodableEventType>
struct ReportEventParams : public app::ReadPrepareParams
{
    ReportEventParams(const SessionHandle & sessionHandle) : app::ReadPrepareParams(sessionHandle) {}
    typename TypedReadEventCallback<DecodableEventType>::OnSuccessCallbackType mOnReportCb;
    typename TypedReadEventCallback<DecodableEventType>::OnErrorCallbackType mOnErrorCb;
    typename TypedReadEventCallback<DecodableEventType>::OnDoneCallbackType mOnDoneCb;
    typename TypedReadEventCallback<DecodableEventType>::OnSubscriptionEstablishedCallbackType mOnSubscriptionEstablishedCb =
        nullptr;
    typename TypedReadEventCallback<DecodableEventType>::OnResubscriptionAttemptCallbackType mOnResubscriptionAttemptCb = nullptr;
    app::ReadClient::InteractionType mReportType = app::ReadClient::InteractionType::Read;
};

template <typename DecodableEventType>
CHIP_ERROR ReportEvent(Messaging::ExchangeManager * apExchangeMgr, EndpointId endpointId,
                       ReportEventParams<DecodableEventType> && readParams, bool aIsUrgentEvent)
{
    ClusterId clusterId                  = DecodableEventType::GetClusterId();
    EventId eventId                      = DecodableEventType::GetEventId();
    app::InteractionModelEngine * engine = app::InteractionModelEngine::GetInstance();
    CHIP_ERROR err                       = CHIP_NO_ERROR;

    auto readPaths = Platform::MakeUnique<app::EventPathParams>(endpointId, clusterId, eventId, aIsUrgentEvent);
    VerifyOrReturnError(readPaths != nullptr, CHIP_ERROR_NO_MEMORY);

    readParams.mpEventPathParamsList = readPaths.get();

    readParams.mEventPathParamsListSize = 1;

    auto callback = chip::Platform::MakeUnique<TypedReadEventCallback<DecodableEventType>>(
        readParams.mOnReportCb, readParams.mOnErrorCb, readParams.mOnDoneCb, readParams.mOnSubscriptionEstablishedCb,
        readParams.mOnResubscriptionAttemptCb);

    VerifyOrReturnError(callback != nullptr, CHIP_ERROR_NO_MEMORY);

    auto readClient = chip::Platform::MakeUnique<app::ReadClient>(engine, apExchangeMgr, *callback.get(), readParams.mReportType);
    VerifyOrReturnError(readClient != nullptr, CHIP_ERROR_NO_MEMORY);

    if (readClient->IsSubscriptionType())
    {
        readPaths.release();
        err = readClient->SendAutoResubscribeRequest(std::move(readParams));
        ReturnErrorOnFailure(err);
    }
    else
    {
        err = readClient->SendRequest(readParams);
        ReturnErrorOnFailure(err);
    }

    //
    // At this point, we'll get a callback through the OnDone callback above regardless of success or failure
    // of the read operation to permit us to free up the callback object. So, release ownership of the callback
    // object now to prevent it from being reclaimed at the end of this scoped block.
    //
    callback->AdoptReadClient(std::move(readClient));
    callback.release();

    return err;
}

} // namespace detail

/*
 * A typed read event function that takes as input a template parameter that encapsulates the type information
 * for a given attribute as well as callbacks for success and failure and either returns a decoded cluster-object representation
 * of the requested attribute through the provided success callback or calls the provided failure callback.
 *
 * The DecodableEventType is generally expected to be a ClusterName::Events::EventName::DecodableEventType struct, but any
 * object that contains type information exposed through a 'DecodableType' type declaration as well as GetClusterId() and
 * GetEventId() methods is expected to work.
 *
 * @param[in] onSuccessCb Used to deliver event data received through the Read interactions
 * @param[in] onErrorCb failureCb will be called when an error occurs *after* a successful call to ReadEvent.
 * @param[in] onDoneCb    OnDone will be called when ReadClient has finished all work for event retrieval, it is possible that there
 * is no event.
 */
template <typename DecodableEventType>
CHIP_ERROR ReadEvent(Messaging::ExchangeManager * exchangeMgr, const SessionHandle & sessionHandle, EndpointId endpointId,
                     typename TypedReadEventCallback<DecodableEventType>::OnSuccessCallbackType onSuccessCb,
                     typename TypedReadEventCallback<DecodableEventType>::OnErrorCallbackType onErrorCb,
                     typename TypedReadEventCallback<DecodableEventType>::OnDoneCallbackType onDoneCb)
{
    detail::ReportEventParams<DecodableEventType> params(sessionHandle);
    params.mOnReportCb = onSuccessCb;
    params.mOnErrorCb  = onErrorCb;
    params.mOnDoneCb   = onDoneCb;
    return detail::ReportEvent(exchangeMgr, endpointId, std::move(params), false /*aIsUrgentEvent*/);
}

/**
 * A functon that allows subscribing to one particular event.  This works
 * similarly to ReadEvent but keeps reporting events as they are emitted.
 */
template <typename DecodableEventType>
CHIP_ERROR SubscribeEvent(
    Messaging::ExchangeManager * exchangeMgr, const SessionHandle & sessionHandle, EndpointId endpointId,
    typename TypedReadEventCallback<DecodableEventType>::OnSuccessCallbackType onReportCb,
    typename TypedReadEventCallback<DecodableEventType>::OnErrorCallbackType onErrorCb, uint16_t minIntervalFloorSeconds,
    uint16_t maxIntervalCeilingSeconds,
    typename TypedReadEventCallback<DecodableEventType>::OnSubscriptionEstablishedCallbackType onSubscriptionEstablishedCb =
        nullptr,
    typename TypedReadEventCallback<DecodableEventType>::OnResubscriptionAttemptCallbackType onResubscriptionAttemptCb = nullptr,
    bool keepPreviousSubscriptions = false, bool aIsUrgentEvent = false)
{
    detail::ReportEventParams<DecodableEventType> params(sessionHandle);
    params.mOnReportCb                  = onReportCb;
    params.mOnErrorCb                   = onErrorCb;
    params.mOnDoneCb                    = nullptr;
    params.mOnSubscriptionEstablishedCb = onSubscriptionEstablishedCb;
    params.mOnResubscriptionAttemptCb   = onResubscriptionAttemptCb;
    params.mMinIntervalFloorSeconds     = minIntervalFloorSeconds;
    params.mMaxIntervalCeilingSeconds   = maxIntervalCeilingSeconds;
    params.mKeepSubscriptions           = keepPreviousSubscriptions;
    params.mReportType                  = app::ReadClient::InteractionType::Subscribe;
    return detail::ReportEvent(exchangeMgr, endpointId, std::move(params), aIsUrgentEvent);
}

} // namespace Controller
} // namespace chip
#endif // CHIP_CONFIG_ENABLE_READ_CLIENT
