/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
 *    This file contains definitions for a base Cluster class. This class will
 *    be derived by various ZCL clusters supported by CHIP. The objects of the
 *    ZCL cluster class will be used by Controller applications to interact with
 *    the CHIP device.
 */

#pragma once

#include "app/ConcreteCommandPath.h"
#include <app/DeviceProxy.h>
#include <app/util/error-mapping.h>
#include <controller/InvokeInteraction.h>
#include <controller/ReadInteraction.h>
#include <controller/WriteInteraction.h>
#include <lib/core/Optional.h>
#include <messaging/ExchangeMgr.h>
#include <system/SystemClock.h>

namespace chip {
namespace Controller {

template <typename T>
using CommandResponseSuccessCallback = void(void * context, const T & responseObject);
using CommandResponseFailureCallback = void(void * context, CHIP_ERROR err);
using CommandResponseDoneCallback    = void();
using WriteResponseSuccessCallback   = void (*)(void * context);
using WriteResponseFailureCallback   = void (*)(void * context, CHIP_ERROR err);
using WriteResponseDoneCallback      = void (*)(void * context);
template <typename T>
using ReadResponseSuccessCallback     = void (*)(void * context, T responseData);
using ReadResponseFailureCallback     = void (*)(void * context, CHIP_ERROR err);
using ReadDoneCallback                = void (*)(void * context);
using SubscriptionEstablishedCallback = void (*)(void * context, SubscriptionId subscriptionId);
using ResubscriptionAttemptCallback   = void (*)(void * context, CHIP_ERROR aError, uint32_t aNextResubscribeIntervalMsec);
using SubscriptionOnDoneCallback      = std::function<void(void)>;

class DLL_EXPORT ClusterBase
{
public:
    ClusterBase(Messaging::ExchangeManager & exchangeManager, const SessionHandle & session, EndpointId endpoint) :
        mExchangeManager(exchangeManager), mSession(session), mEndpoint(endpoint)
    {}

    virtual ~ClusterBase() {}

    // Temporary function to set command timeout before we move over to InvokeCommand
    // TODO: remove when we start using InvokeCommand everywhere
    void SetCommandTimeout(Optional<System::Clock::Timeout> timeout) { mTimeout = timeout; }

    /**
     * Returns the current command timeout set via SetCommandTimeout, or an
     * empty optional if no timeout has been set.
     */
    Optional<System::Clock::Timeout> GetCommandTimeout() { return mTimeout; }

    /*
     * This function permits sending an invoke request using cluster objects that represent the request and response data payloads.
     *
     * Success and Failure callbacks must be passed in through which the decoded response is provided as well as notification of any
     * failure.
     */
    template <typename RequestDataT>
    CHIP_ERROR InvokeCommand(const RequestDataT & requestData, void * context,
                             CommandResponseSuccessCallback<typename RequestDataT::ResponseType> successCb,
                             CommandResponseFailureCallback failureCb, const Optional<uint16_t> & timedInvokeTimeoutMs)
    {
        auto onSuccessCb = [context, successCb](const app::ConcreteCommandPath & aPath, const app::StatusIB & aStatus,
                                                const typename RequestDataT::ResponseType & responseData) {
            successCb(context, responseData);
        };

        auto onFailureCb = [context, failureCb](CHIP_ERROR aError) { failureCb(context, aError); };

        return InvokeCommandRequest(&mExchangeManager, mSession.Get().Value(), mEndpoint, requestData, onSuccessCb, onFailureCb,
                                    timedInvokeTimeoutMs, mTimeout);
    }

    template <typename RequestDataT>
    CHIP_ERROR InvokeCommand(const RequestDataT & requestData, void * context,
                             CommandResponseSuccessCallback<typename RequestDataT::ResponseType> successCb,
                             CommandResponseFailureCallback failureCb, uint16_t timedInvokeTimeoutMs)
    {
        return InvokeCommand(requestData, context, successCb, failureCb, MakeOptional(timedInvokeTimeoutMs));
    }

    template <typename RequestDataT, typename std::enable_if_t<!RequestDataT::MustUseTimedInvoke(), int> = 0>
    CHIP_ERROR InvokeCommand(const RequestDataT & requestData, void * context,
                             CommandResponseSuccessCallback<typename RequestDataT::ResponseType> successCb,
                             CommandResponseFailureCallback failureCb)
    {
        return InvokeCommand(requestData, context, successCb, failureCb, NullOptional);
    }

    /**
     * Functions for writing attributes.  We have lots of different
     * AttributeInfo but a fairly small set of types that get written.  So we
     * want to keep the template on AttributeInfo very small, and put all the
     * work in the template with a small number of instantiations (one per
     * type).
     */
    template <typename AttrType>
    CHIP_ERROR WriteAttribute(const AttrType & requestData, void * context, ClusterId clusterId, AttributeId attributeId,
                              WriteResponseSuccessCallback successCb, WriteResponseFailureCallback failureCb,
                              const Optional<uint16_t> & aTimedWriteTimeoutMs, WriteResponseDoneCallback doneCb = nullptr,
                              const Optional<DataVersion> & aDataVersion = NullOptional)
    {
        auto onSuccessCb = [context, successCb](const app::ConcreteAttributePath & aPath) {
            if (successCb != nullptr)
            {
                successCb(context);
            }
        };

        auto onFailureCb = [context, failureCb](const app::ConcreteAttributePath * aPath, CHIP_ERROR aError) {
            if (failureCb != nullptr)
            {
                failureCb(context, aError);
            }
        };

        auto onDoneCb = [context, doneCb](app::WriteClient * pWriteClient) {
            if (doneCb != nullptr)
            {
                doneCb(context);
            }
        };

        return chip::Controller::WriteAttribute<AttrType>(mSession.Get().Value(), mEndpoint, clusterId, attributeId, requestData,
                                                          onSuccessCb, onFailureCb, aTimedWriteTimeoutMs, onDoneCb, aDataVersion);
    }

    template <typename AttrType>
    CHIP_ERROR WriteAttribute(GroupId groupId, FabricIndex fabricIndex, const AttrType & requestData, void * context,
                              ClusterId clusterId, AttributeId attributeId, WriteResponseSuccessCallback successCb,
                              WriteResponseFailureCallback failureCb, const Optional<uint16_t> & aTimedWriteTimeoutMs,
                              WriteResponseDoneCallback doneCb = nullptr, const Optional<DataVersion> & aDataVersion = NullOptional)
    {

        auto onSuccessCb = [context, successCb](const app::ConcreteAttributePath & aPath) {
            if (successCb != nullptr)
            {
                successCb(context);
            }
        };

        auto onFailureCb = [context, failureCb](const app::ConcreteAttributePath * aPath, CHIP_ERROR aError) {
            if (failureCb != nullptr)
            {
                failureCb(context, aError);
            }
        };

        auto onDoneCb = [context, doneCb](app::WriteClient * pWriteClient) {
            if (doneCb != nullptr)
            {
                doneCb(context);
            }
        };

        Transport::OutgoingGroupSession groupSession(groupId, fabricIndex);
        return chip::Controller::WriteAttribute<AttrType>(SessionHandle(groupSession), 0 /*Unused for Group*/, clusterId,
                                                          attributeId, requestData, onSuccessCb, onFailureCb, aTimedWriteTimeoutMs,
                                                          onDoneCb, aDataVersion);
    }

    template <typename AttributeInfo>
    CHIP_ERROR WriteAttribute(GroupId groupId, FabricIndex fabricIndex, const typename AttributeInfo::Type & requestData,
                              void * context, WriteResponseSuccessCallback successCb, WriteResponseFailureCallback failureCb,
                              WriteResponseDoneCallback doneCb = nullptr, const Optional<DataVersion> & aDataVersion = NullOptional,
                              const Optional<uint16_t> & aTimedWriteTimeoutMs = NullOptional)
    {
        return WriteAttribute(groupId, fabricIndex, requestData, context, AttributeInfo::GetClusterId(),
                              AttributeInfo::GetAttributeId(), successCb, failureCb, aTimedWriteTimeoutMs, doneCb, aDataVersion);
    }

    template <typename AttributeInfo>
    CHIP_ERROR WriteAttribute(const typename AttributeInfo::Type & requestData, void * context,
                              WriteResponseSuccessCallback successCb, WriteResponseFailureCallback failureCb,
                              const Optional<uint16_t> & aTimedWriteTimeoutMs, WriteResponseDoneCallback doneCb = nullptr,
                              const Optional<DataVersion> & aDataVersion = NullOptional)
    {
        return WriteAttribute(requestData, context, AttributeInfo::GetClusterId(), AttributeInfo::GetAttributeId(), successCb,
                              failureCb, aTimedWriteTimeoutMs, doneCb, aDataVersion);
    }

    template <typename AttributeInfo>
    CHIP_ERROR WriteAttribute(const typename AttributeInfo::Type & requestData, void * context,
                              WriteResponseSuccessCallback successCb, WriteResponseFailureCallback failureCb,
                              uint16_t aTimedWriteTimeoutMs, WriteResponseDoneCallback doneCb = nullptr,
                              const Optional<DataVersion> & aDataVersion = NullOptional)
    {
        return WriteAttribute<AttributeInfo>(requestData, context, successCb, failureCb, MakeOptional(aTimedWriteTimeoutMs), doneCb,
                                             aDataVersion);
    }

    template <typename AttributeInfo, typename std::enable_if_t<!AttributeInfo::MustUseTimedWrite(), int> = 0>
    CHIP_ERROR WriteAttribute(const typename AttributeInfo::Type & requestData, void * context,
                              WriteResponseSuccessCallback successCb, WriteResponseFailureCallback failureCb,
                              WriteResponseDoneCallback doneCb = nullptr, const Optional<DataVersion> & aDataVersion = NullOptional)
    {
        return WriteAttribute<AttributeInfo>(requestData, context, successCb, failureCb, NullOptional, doneCb, aDataVersion);
    }

    /**
     * Read an attribute and get a type-safe callback with the attribute value.
     */
    template <typename AttributeInfo>
    CHIP_ERROR ReadAttribute(void * context, ReadResponseSuccessCallback<typename AttributeInfo::DecodableArgType> successCb,
                             ReadResponseFailureCallback failureCb, bool aIsFabricFiltered = true)
    {
        return ReadAttribute<typename AttributeInfo::DecodableType, typename AttributeInfo::DecodableArgType>(
            context, AttributeInfo::GetClusterId(), AttributeInfo::GetAttributeId(), successCb, failureCb, aIsFabricFiltered);
    }

    template <typename DecodableType, typename DecodableArgType>
    CHIP_ERROR ReadAttribute(void * context, ClusterId clusterId, AttributeId attributeId,
                             ReadResponseSuccessCallback<DecodableArgType> successCb, ReadResponseFailureCallback failureCb,
                             bool aIsFabricFiltered = true)
    {
        auto onSuccessCb = [context, successCb](const app::ConcreteAttributePath & aPath, const DecodableType & aData) {
            if (successCb != nullptr)
            {
                successCb(context, aData);
            }
        };

        auto onFailureCb = [context, failureCb](const app::ConcreteAttributePath * aPath, CHIP_ERROR aError) {
            if (failureCb != nullptr)
            {
                failureCb(context, aError);
            }
        };

        return Controller::ReadAttribute<DecodableType>(&mExchangeManager, mSession.Get().Value(), mEndpoint, clusterId,
                                                        attributeId, onSuccessCb, onFailureCb, aIsFabricFiltered);
    }

    /**
     * Subscribe to attribute and get a type-safe callback with the attribute
     * value when it changes.
     */
    template <typename AttributeInfo>
    CHIP_ERROR
    SubscribeAttribute(void * context, ReadResponseSuccessCallback<typename AttributeInfo::DecodableArgType> reportCb,
                       ReadResponseFailureCallback failureCb, uint16_t minIntervalFloorSeconds, uint16_t maxIntervalCeilingSeconds,
                       SubscriptionEstablishedCallback subscriptionEstablishedCb = nullptr,
                       ResubscriptionAttemptCallback resubscriptionAttemptCb = nullptr, bool aIsFabricFiltered = true,
                       bool aKeepPreviousSubscriptions = false, const Optional<DataVersion> & aDataVersion = NullOptional,
                       SubscriptionOnDoneCallback subscriptionDoneCb = nullptr)
    {
        return SubscribeAttribute<typename AttributeInfo::DecodableType, typename AttributeInfo::DecodableArgType>(
            context, AttributeInfo::GetClusterId(), AttributeInfo::GetAttributeId(), reportCb, failureCb, minIntervalFloorSeconds,
            maxIntervalCeilingSeconds, subscriptionEstablishedCb, resubscriptionAttemptCb, aIsFabricFiltered,
            aKeepPreviousSubscriptions, aDataVersion, subscriptionDoneCb);
    }

    template <typename DecodableType, typename DecodableArgType>
    CHIP_ERROR SubscribeAttribute(void * context, ClusterId clusterId, AttributeId attributeId,
                                  ReadResponseSuccessCallback<DecodableArgType> reportCb, ReadResponseFailureCallback failureCb,
                                  uint16_t minIntervalFloorSeconds, uint16_t maxIntervalCeilingSeconds,
                                  SubscriptionEstablishedCallback subscriptionEstablishedCb = nullptr,
                                  ResubscriptionAttemptCallback resubscriptionAttemptCb = nullptr, bool aIsFabricFiltered = true,
                                  bool aKeepPreviousSubscriptions               = false,
                                  const Optional<DataVersion> & aDataVersion    = NullOptional,
                                  SubscriptionOnDoneCallback subscriptionDoneCb = nullptr)
    {
        auto onReportCb = [context, reportCb](const app::ConcreteAttributePath & aPath, const DecodableType & aData) {
            if (reportCb != nullptr)
            {
                reportCb(context, aData);
            }
        };

        auto onFailureCb = [context, failureCb](const app::ConcreteAttributePath * aPath, CHIP_ERROR aError) {
            if (failureCb != nullptr)
            {
                failureCb(context, aError);
            }
        };

        auto onSubscriptionEstablishedCb = [context, subscriptionEstablishedCb](const app::ReadClient & readClient,
                                                                                SubscriptionId subscriptionId) {
            if (subscriptionEstablishedCb != nullptr)
            {
                subscriptionEstablishedCb(context, subscriptionId);
            }
        };

        auto onResubscriptionAttemptCb = [context, resubscriptionAttemptCb](const app::ReadClient & readClient, CHIP_ERROR aError,
                                                                            uint32_t aNextResubscribeIntervalMsec) {
            if (resubscriptionAttemptCb != nullptr)
            {
                resubscriptionAttemptCb(context, aError, aNextResubscribeIntervalMsec);
            }
        };

        return Controller::SubscribeAttribute<DecodableType>(
            &mExchangeManager, mSession.Get().Value(), mEndpoint, clusterId, attributeId, onReportCb, onFailureCb,
            minIntervalFloorSeconds, maxIntervalCeilingSeconds, onSubscriptionEstablishedCb, onResubscriptionAttemptCb,
            aIsFabricFiltered, aKeepPreviousSubscriptions, aDataVersion, subscriptionDoneCb);
    }

    /**
     * Read an event and get a type-safe callback with the event data.
     *
     * @param[in] successCb Used to deliver event data received through the Read interactions
     * @param[in] failureCb failureCb will be called when an error occurs *after* a successful call to ReadEvent.
     * @param[in] doneCb    OnDone will be called when ReadClient has finished all work for event retrieval, it is possible that
     * there is no event.
     */
    template <typename DecodableType>
    CHIP_ERROR ReadEvent(void * context, ReadResponseSuccessCallback<DecodableType> successCb,
                         ReadResponseFailureCallback failureCb, ReadDoneCallback doneCb)
    {
        auto onSuccessCb = [context, successCb](const app::EventHeader & aEventHeader, const DecodableType & aData) {
            if (successCb != nullptr)
            {
                successCb(context, aData);
            }
        };

        auto onFailureCb = [context, failureCb](const app::EventHeader * aEventHeader, CHIP_ERROR aError) {
            if (failureCb != nullptr)
            {
                failureCb(context, aError);
            }
        };

        auto onDoneCb = [context, doneCb](app::ReadClient * apReadClient) {
            if (doneCb != nullptr)
            {
                doneCb(context);
            }
        };
        return Controller::ReadEvent<DecodableType>(&mExchangeManager, mSession.Get().Value(), mEndpoint, onSuccessCb, onFailureCb,
                                                    onDoneCb);
    }

    template <typename DecodableType>
    CHIP_ERROR SubscribeEvent(void * context, ReadResponseSuccessCallback<DecodableType> reportCb,
                              ReadResponseFailureCallback failureCb, uint16_t minIntervalFloorSeconds,
                              uint16_t maxIntervalCeilingSeconds,
                              SubscriptionEstablishedCallback subscriptionEstablishedCb = nullptr,
                              ResubscriptionAttemptCallback resubscriptionAttemptCb     = nullptr,
                              bool aKeepPreviousSubscriptions = false, bool aIsUrgentEvent = false)
    {
        auto onReportCb = [context, reportCb](const app::EventHeader & aEventHeader, const DecodableType & aData) {
            if (reportCb != nullptr)
            {
                reportCb(context, aData);
            }
        };

        auto onFailureCb = [context, failureCb](const app::EventHeader * aEventHeader, CHIP_ERROR aError) {
            if (failureCb != nullptr)
            {
                failureCb(context, aError);
            }
        };

        auto onSubscriptionEstablishedCb = [context, subscriptionEstablishedCb](const app::ReadClient & readClient,
                                                                                SubscriptionId subscriptionId) {
            if (subscriptionEstablishedCb != nullptr)
            {
                subscriptionEstablishedCb(context, subscriptionId);
            }
        };

        auto onResubscriptionAttemptCb = [context, resubscriptionAttemptCb](const app::ReadClient & readClient, CHIP_ERROR aError,
                                                                            uint32_t aNextResubscribeIntervalMsec) {
            if (resubscriptionAttemptCb != nullptr)
            {
                resubscriptionAttemptCb(context, aError, aNextResubscribeIntervalMsec);
            }
        };

        return Controller::SubscribeEvent<DecodableType>(&mExchangeManager, mSession.Get().Value(), mEndpoint, onReportCb,
                                                         onFailureCb, minIntervalFloorSeconds, maxIntervalCeilingSeconds,
                                                         onSubscriptionEstablishedCb, onResubscriptionAttemptCb,
                                                         aKeepPreviousSubscriptions, aIsUrgentEvent);
    }

protected:
    Messaging::ExchangeManager & mExchangeManager;

    // Since cluster object is ephemeral, the session shall be valid during the entire lifespan, so we do not need to check the
    // session existence when using it. For java and objective-c binding, the cluster object is allocated in the heap, such that we
    // can't use SessionHandle here, in such case, the cluster object must be freed when the session is released.
    SessionHolder mSession;

    EndpointId mEndpoint;
    Optional<System::Clock::Timeout> mTimeout;
};

} // namespace Controller
} // namespace chip
