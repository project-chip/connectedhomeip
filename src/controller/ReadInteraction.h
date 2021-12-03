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

#include <app/AttributePathParams.h>
#include <app/InteractionModelEngine.h>
#include <app/ReadPrepareParams.h>
#include <controller/TypedReadCallback.h>

namespace chip {
namespace Controller {

/*
 * A typed read attribute function that takes as input a template parameter that encapsulates the type information
 * for a given attribute as well as callbacks for success and failure and either returns a decoded cluster-object representation
 * of the requested attribute through the provided success callback or calls the provided failure callback.
 *
 * The AttributeTypeInfo is generally expected to be a ClusterName::Attributes::AttributeName::TypeInfo struct, but any
 * object that contains type information exposed through a 'DecodableType' type declaration as well as GetClusterId() and
 * GetAttributeId() methods is expected to work.
 *
 */

/**
 * To avoid instantiating all the complicated code on a per-attribute basis, we
 * have a helper that's just templated on the type.
 */
template <typename DecodableAttributeType>
CHIP_ERROR ReadAttribute(Messaging::ExchangeManager * aExchangeMgr, const SessionHandle sessionHandle, EndpointId endpointId,
                         ClusterId clusterId, AttributeId attributeId,
                         typename TypedReadAttributeCallback<DecodableAttributeType>::OnSuccessCallbackType onSuccessCb,
                         typename TypedReadAttributeCallback<DecodableAttributeType>::OnErrorCallbackType onErrorCb)
{
    app::AttributePathParams attributePath(endpointId, clusterId, attributeId);
    app::ReadPrepareParams readParams(sessionHandle);
    app::ReadClient * readClient         = nullptr;
    app::InteractionModelEngine * engine = app::InteractionModelEngine::GetInstance();
    CHIP_ERROR err                       = CHIP_NO_ERROR;

    readParams.mpAttributePathParamsList    = &attributePath;
    readParams.mAttributePathParamsListSize = 1;

    auto onDone = [](app::ReadClient * apReadClient, TypedReadAttributeCallback<DecodableAttributeType> * callback) {
        chip::Platform::Delete(callback);
    };

    auto callback = chip::Platform::MakeUnique<TypedReadAttributeCallback<DecodableAttributeType>>(clusterId, attributeId,
                                                                                                   onSuccessCb, onErrorCb, onDone);
    VerifyOrReturnError(callback != nullptr, CHIP_ERROR_NO_MEMORY);

    ReturnErrorOnFailure(engine->NewReadClient(&readClient, app::ReadClient::InteractionType::Read, callback.get()));

    err = readClient->SendReadRequest(readParams);
    if (err != CHIP_NO_ERROR)
    {
        readClient->Shutdown();
        return err;
    }

    //
    // At this point, we'll get a callback through the OnDone callback above regardless of success or failure
    // of the read operation to permit us to free up the callback object. So, release ownership of the callback
    // object now to prevent it from being reclaimed at the end of this scoped block.
    //
    callback.release();
    return err;
}

/*
 * A typed read event function that takes as input a template parameter that encapsulates the type information
 * for a given attribute as well as callbacks for success and failure and either returns a decoded cluster-object representation
 * of the requested attribute through the provided success callback or calls the provided failure callback.
 *
 * The EventTypeInfo is generally expected to be a ClusterName::Events::EventName::TypeInfo struct, but any
 * object that contains type information exposed through a 'DecodableType' type declaration as well as GetClusterId() and
 * GetEventId() methods is expected to work.
 */
template <typename DecodableEventTypeInfo>
CHIP_ERROR ReadEvent(Messaging::ExchangeManager * apExchangeMgr, const SessionHandle sessionHandle, EndpointId endpointId,
                     typename TypedReadEventCallback<DecodableEventTypeInfo>::OnSuccessCallbackType onSuccessCb,
                     typename TypedReadEventCallback<DecodableEventTypeInfo>::OnErrorCallbackType onErrorCb)
{
    ClusterId clusterId = DecodableEventTypeInfo::GetClusterId();
    EventId eventId     = DecodableEventTypeInfo::GetEventId();
    app::EventPathParams eventPath(endpointId, clusterId, eventId);
    app::ReadPrepareParams readParams(sessionHandle);
    app::ReadClient * readClient         = nullptr;
    app::InteractionModelEngine * engine = app::InteractionModelEngine::GetInstance();
    CHIP_ERROR err                       = CHIP_NO_ERROR;

    readParams.mpEventPathParamsList    = &eventPath;
    readParams.mEventPathParamsListSize = 1;

    auto onDone = [](app::ReadClient * apReadClient, TypedReadEventCallback<DecodableEventTypeInfo> * callback) {
        chip::Platform::Delete(callback);
    };

    auto callback = chip::Platform::MakeUnique<TypedReadEventCallback<DecodableEventTypeInfo>>(clusterId, eventId, onSuccessCb,
                                                                                               onErrorCb, onDone);

    VerifyOrReturnError(callback != nullptr, CHIP_ERROR_NO_MEMORY);

    ReturnErrorOnFailure(
        engine->NewReadClient(&readClient, app::ReadClient::InteractionType::Read, &callback.get()->GetBufferedCallback()));

    err = readClient->SendReadRequest(readParams);
    if (err != CHIP_NO_ERROR)
    {
        readClient->Shutdown();
        return err;
    }

    //
    // At this point, we'll get a callback through the OnDone callback above regardless of success or failure
    // of the read operation to permit us to free up the callback object. So, release ownership of the callback
    // object now to prevent it from being reclaimed at the end of this scoped block.
    //
    callback.release();
    return err;
}

template <typename AttributeTypeInfo>
CHIP_ERROR
ReadAttribute(Messaging::ExchangeManager * aExchangeMgr, const SessionHandle sessionHandle, EndpointId endpointId,
              typename TypedReadAttributeCallback<typename AttributeTypeInfo::DecodableType>::OnSuccessCallbackType onSuccessCb,
              typename TypedReadAttributeCallback<typename AttributeTypeInfo::DecodableType>::OnErrorCallbackType onErrorCb)
{
    return ReadAttribute<typename AttributeTypeInfo::DecodableType>(aExchangeMgr, sessionHandle, endpointId,
                                                                    AttributeTypeInfo::GetClusterId(),
                                                                    AttributeTypeInfo::GetAttributeId(), onSuccessCb, onErrorCb);
}

template <typename EventTypeInfo>
CHIP_ERROR ReadEvent(Messaging::ExchangeManager * aExchangeMgr, const SessionHandle sessionHandle, EndpointId endpointId,
                     typename TypedReadEventCallback<typename EventTypeInfo::DecodableType>::OnSuccessCallbackType onSuccessCb,
                     typename TypedReadEventCallback<typename EventTypeInfo::DecodableType>::OnErrorCallbackType onErrorCb)
{
    return ReadEvent<typename EventTypeInfo::DecodableType>(aExchangeMgr, sessionHandle, endpointId, onSuccessCb, onErrorCb);
}
} // namespace Controller
} // namespace chip
