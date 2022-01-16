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

namespace chip {
namespace Controller {

template <typename T>
using CommandResponseSuccessCallback = void(void * context, const T & responseObject);
using CommandResponseFailureCallback = void(void * context, EmberAfStatus status);
using CommandResponseDoneCallback    = void();
using WriteResponseSuccessCallback   = void (*)(void * context);
using WriteResponseFailureCallback   = void (*)(void * context, EmberAfStatus status);
using WriteResponseDoneCallback      = void (*)(void * context);
template <typename T>
using ReadResponseSuccessCallback     = void (*)(void * context, T responseData);
using ReadResponseFailureCallback     = void (*)(void * context, EmberAfStatus status);
using SubscriptionEstablishedCallback = void (*)(void * context);

class DLL_EXPORT ClusterBase
{
public:
    virtual ~ClusterBase() {}

    CHIP_ERROR Associate(DeviceProxy * device, EndpointId endpoint);
    CHIP_ERROR AssociateWithGroup(DeviceProxy * device, GroupId groupId);

    void Dissociate();

    ClusterId GetClusterId() const { return mClusterId; }

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
        VerifyOrReturnError(mDevice != nullptr, CHIP_ERROR_INCORRECT_STATE);

        auto onSuccessCb = [context, successCb](const app::ConcreteCommandPath & commandPath, const app::StatusIB & aStatus,
                                                const typename RequestDataT::ResponseType & responseData) {
            successCb(context, responseData);
        };

        auto onFailureCb = [context, failureCb](const app::StatusIB & aStatus, CHIP_ERROR aError) {
            failureCb(context, app::ToEmberAfStatus(aStatus.mStatus));
        };

        return InvokeCommandRequest(mDevice->GetExchangeManager(), mDevice->GetSecureSession().Value(), mEndpoint, requestData,
                                    onSuccessCb, onFailureCb, timedInvokeTimeoutMs);
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
                              const Optional<uint16_t> & aTimedWriteTimeoutMs, WriteResponseDoneCallback doneCb = nullptr)
    {
        CHIP_ERROR err = CHIP_NO_ERROR;
        VerifyOrReturnError(mDevice != nullptr, CHIP_ERROR_INCORRECT_STATE);

        auto onSuccessCb = [context, successCb](const app::ConcreteAttributePath & commandPath) {
            if (successCb != nullptr)
            {
                successCb(context);
            }
        };

        auto onFailureCb = [context, failureCb](const app::ConcreteAttributePath * commandPath, app::StatusIB status,
                                                CHIP_ERROR aError) {
            if (failureCb != nullptr)
            {
                failureCb(context, app::ToEmberAfStatus(status.mStatus));
            }
        };

        auto onDoneCb = [context, doneCb](app::WriteClient * pWriteClient) {
            if (doneCb != nullptr)
            {
                doneCb(context);
            }
        };

        if (mGroupSession)
        {
            err = chip::Controller::WriteAttribute<AttrType>(mGroupSession.Get(), mEndpoint, clusterId, attributeId, requestData,
                                                             onSuccessCb, onFailureCb, aTimedWriteTimeoutMs, onDoneCb);
            mDevice->GetExchangeManager()->GetSessionManager()->RemoveGroupSession(mGroupSession->AsGroupSession());
        }
        else
        {
            err = chip::Controller::WriteAttribute<AttrType>(mDevice->GetSecureSession().Value(), mEndpoint, clusterId, attributeId,
                                                             requestData, onSuccessCb, onFailureCb, aTimedWriteTimeoutMs, onDoneCb);
        }

        return err;
    }

    template <typename AttributeInfo>
    CHIP_ERROR WriteAttribute(const typename AttributeInfo::Type & requestData, void * context,
                              WriteResponseSuccessCallback successCb, WriteResponseFailureCallback failureCb,
                              const Optional<uint16_t> & aTimedWriteTimeoutMs, WriteResponseDoneCallback doneCb = nullptr)
    {
        return WriteAttribute(requestData, context, AttributeInfo::GetClusterId(), AttributeInfo::GetAttributeId(), successCb,
                              failureCb, aTimedWriteTimeoutMs, doneCb);
    }

    template <typename AttributeInfo>
    CHIP_ERROR WriteAttribute(const typename AttributeInfo::Type & requestData, void * context,
                              WriteResponseSuccessCallback successCb, WriteResponseFailureCallback failureCb,
                              uint16_t aTimedWriteTimeoutMs, WriteResponseDoneCallback doneCb = nullptr)
    {
        return WriteAttribute<AttributeInfo>(requestData, context, successCb, failureCb, MakeOptional(aTimedWriteTimeoutMs),
                                             doneCb);
    }

    template <typename AttributeInfo, typename std::enable_if_t<!AttributeInfo::MustUseTimedWrite(), int> = 0>
    CHIP_ERROR WriteAttribute(const typename AttributeInfo::Type & requestData, void * context,
                              WriteResponseSuccessCallback successCb, WriteResponseFailureCallback failureCb,
                              WriteResponseDoneCallback doneCb = nullptr)
    {
        return WriteAttribute<AttributeInfo>(requestData, context, successCb, failureCb, NullOptional, doneCb);
    }

    /**
     * Read an attribute and get a type-safe callback with the attribute value.
     */
    template <typename AttributeInfo>
    CHIP_ERROR ReadAttribute(void * context, ReadResponseSuccessCallback<typename AttributeInfo::DecodableArgType> successCb,
                             ReadResponseFailureCallback failureCb)
    {
        return ReadAttribute<typename AttributeInfo::DecodableType, typename AttributeInfo::DecodableArgType>(
            context, AttributeInfo::GetClusterId(), AttributeInfo::GetAttributeId(), successCb, failureCb);
    }

    template <typename DecodableType, typename DecodableArgType>
    CHIP_ERROR ReadAttribute(void * context, ClusterId clusterId, AttributeId attributeId,
                             ReadResponseSuccessCallback<DecodableArgType> successCb, ReadResponseFailureCallback failureCb)
    {
        VerifyOrReturnError(mDevice != nullptr, CHIP_ERROR_INCORRECT_STATE);

        auto onSuccessCb = [context, successCb](const app::ConcreteAttributePath & commandPath, const DecodableType & aData) {
            if (successCb != nullptr)
            {
                successCb(context, aData);
            }
        };

        auto onFailureCb = [context, failureCb](const app::ConcreteAttributePath * commandPath, app::StatusIB status,
                                                CHIP_ERROR aError) {
            if (failureCb != nullptr)
            {
                failureCb(context, app::ToEmberAfStatus(status.mStatus));
            }
        };

        return Controller::ReadAttribute<DecodableType>(mDevice->GetExchangeManager(), mDevice->GetSecureSession().Value(),
                                                        mEndpoint, clusterId, attributeId, onSuccessCb, onFailureCb);
    }

    /**
     * Subscribe to attribute and get a type-safe callback with the attribute
     * value when it changes.
     */
    template <typename AttributeInfo>
    CHIP_ERROR SubscribeAttribute(void * context, ReadResponseSuccessCallback<typename AttributeInfo::DecodableArgType> reportCb,
                                  ReadResponseFailureCallback failureCb, uint16_t minIntervalFloorSeconds,
                                  uint16_t maxIntervalCeilingSeconds,
                                  SubscriptionEstablishedCallback subscriptionEstablishedCb = nullptr)
    {
        return SubscribeAttribute<typename AttributeInfo::DecodableType, typename AttributeInfo::DecodableArgType>(
            context, AttributeInfo::GetClusterId(), AttributeInfo::GetAttributeId(), reportCb, failureCb, minIntervalFloorSeconds,
            maxIntervalCeilingSeconds, subscriptionEstablishedCb);
    }

    template <typename DecodableType, typename DecodableArgType>
    CHIP_ERROR SubscribeAttribute(void * context, ClusterId clusterId, AttributeId attributeId,
                                  ReadResponseSuccessCallback<DecodableArgType> reportCb, ReadResponseFailureCallback failureCb,
                                  uint16_t minIntervalFloorSeconds, uint16_t maxIntervalCeilingSeconds,
                                  SubscriptionEstablishedCallback subscriptionEstablishedCb = nullptr)
    {
        VerifyOrReturnError(mDevice != nullptr, CHIP_ERROR_INCORRECT_STATE);

        auto onReportCb = [context, reportCb](const app::ConcreteAttributePath & commandPath, const DecodableType & aData) {
            if (reportCb != nullptr)
            {
                reportCb(context, aData);
            }
        };

        auto onFailureCb = [context, failureCb](const app::ConcreteAttributePath * commandPath, app::StatusIB status,
                                                CHIP_ERROR aError) {
            if (failureCb != nullptr)
            {
                failureCb(context, app::ToEmberAfStatus(status.mStatus));
            }
        };

        auto onSubscriptionEstablishedCb = [context, subscriptionEstablishedCb]() {
            if (subscriptionEstablishedCb != nullptr)
            {
                subscriptionEstablishedCb(context);
            }
        };

        return Controller::SubscribeAttribute<DecodableType>(
            mDevice->GetExchangeManager(), mDevice->GetSecureSession().Value(), mEndpoint, clusterId, attributeId, onReportCb,
            onFailureCb, minIntervalFloorSeconds, maxIntervalCeilingSeconds, onSubscriptionEstablishedCb);
    }

    /**
     * Read an event and get a type-safe callback with the event data.
     */
    template <typename DecodableType>
    CHIP_ERROR ReadEvent(void * context, ReadResponseSuccessCallback<DecodableType> successCb,
                         ReadResponseFailureCallback failureCb)
    {
        VerifyOrReturnError(mDevice != nullptr, CHIP_ERROR_INCORRECT_STATE);

        auto onSuccessCb = [context, successCb](const app::EventHeader & eventHeader, const DecodableType & aData) {
            if (successCb != nullptr)
            {
                successCb(context, aData);
            }
        };

        auto onFailureCb = [context, failureCb](const app::EventHeader * eventHeader, Protocols::InteractionModel::Status status,
                                                CHIP_ERROR error) {
            if (failureCb != nullptr)
            {
                failureCb(context, app::ToEmberAfStatus(status));
            }
        };

        return Controller::ReadEvent<DecodableType>(mDevice->GetExchangeManager(), mDevice->GetSecureSession().Value(), mEndpoint,
                                                    onSuccessCb, onFailureCb);
    }

    template <typename DecodableType>
    CHIP_ERROR SubscribeEvent(void * context, ReadResponseSuccessCallback<DecodableType> reportCb,
                              ReadResponseFailureCallback failureCb, uint16_t minIntervalFloorSeconds,
                              uint16_t maxIntervalCeilingSeconds,
                              SubscriptionEstablishedCallback subscriptionEstablishedCb = nullptr)
    {
        VerifyOrReturnError(mDevice != nullptr, CHIP_ERROR_INCORRECT_STATE);

        auto onReportCb = [context, reportCb](const app::EventHeader & eventHeader, const DecodableType & aData) {
            if (reportCb != nullptr)
            {
                reportCb(context, aData);
            }
        };

        auto onFailureCb = [context, failureCb](const app::EventHeader * eventHeader, Protocols::InteractionModel::Status status,
                                                CHIP_ERROR aError) {
            if (failureCb != nullptr)
            {
                failureCb(context, app::ToEmberAfStatus(status));
            }
        };

        auto onSubscriptionEstablishedCb = [context, subscriptionEstablishedCb]() {
            if (subscriptionEstablishedCb != nullptr)
            {
                subscriptionEstablishedCb(context);
            }
        };

        return Controller::SubscribeEvent<DecodableType>(mDevice->GetExchangeManager(), mDevice->GetSecureSession().Value(),
                                                         mEndpoint, onReportCb, onFailureCb, minIntervalFloorSeconds,
                                                         maxIntervalCeilingSeconds, onSubscriptionEstablishedCb);
    }

protected:
    ClusterBase(uint16_t cluster) : mClusterId(cluster) {}

    const ClusterId mClusterId;
    DeviceProxy * mDevice;
    EndpointId mEndpoint;
    SessionHolder mGroupSession;
};

} // namespace Controller
} // namespace chip
