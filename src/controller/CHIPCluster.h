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
using ReadResponseSuccessCallback = void (*)(void * context, T responseData);
using ReadResponseFailureCallback = void (*)(void * context, EmberAfStatus status);

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
                              WriteResponseSuccessCallback successCb, WriteResponseFailureCallback failureCb)
    {
        return WriteAttribute(requestData, context, clusterId, attributeId, successCb, failureCb, nullptr /* doneCb */);
    }

    template <typename AttrType>
    CHIP_ERROR WriteAttribute(const AttrType & requestData, void * context, ClusterId clusterId, AttributeId attributeId,
                              WriteResponseSuccessCallback successCb, WriteResponseFailureCallback failureCb,
                              WriteResponseDoneCallback doneCb)
    {
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

        return chip::Controller::WriteAttribute<AttrType>(
            (mSessionHandle.HasValue() ? mSessionHandle.Value() : mDevice->GetSecureSession().Value()), mEndpoint, clusterId,
            attributeId, requestData, onSuccessCb, onFailureCb, onDoneCb);
    }

    template <typename AttributeInfo>
    CHIP_ERROR WriteAttribute(const typename AttributeInfo::Type & requestData, void * context,
                              WriteResponseSuccessCallback successCb, WriteResponseFailureCallback failureCb)
    {
        return WriteAttribute(requestData, context, AttributeInfo::GetClusterId(), AttributeInfo::GetAttributeId(), successCb,
                              failureCb);
    }

    template <typename AttributeInfo>
    CHIP_ERROR WriteAttribute(const typename AttributeInfo::Type & requestData, void * context,
                              WriteResponseSuccessCallback successCb, WriteResponseFailureCallback failureCb,
                              WriteResponseDoneCallback doneCb)
    {
        return WriteAttribute(requestData, context, AttributeInfo::GetClusterId(), AttributeInfo::GetAttributeId(), successCb,
                              failureCb, doneCb);
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

protected:
    ClusterBase(uint16_t cluster) : mClusterId(cluster) {}

    /**
     * @brief
     *   Request attribute reports from the device. Add a callback
     *   handler, that'll be called when the reports are received from the device.
     *
     * @param[in] attributeId       The report target attribute id
     * @param[in] reportHandler     The handler function that's called on receiving attribute reports
     *                              The reporting handler continues to be called as long as the callback
     *                              is active. The user can stop the reporting by cancelling the callback.
     *                              Reference: chip::Callback::Cancel()
     * @param[in] tlvDataFilter     Filter interface for processing data from TLV
     */
    CHIP_ERROR RequestAttributeReporting(AttributeId attributeId, Callback::Cancelable * reportHandler,
                                         app::TLVDataFilter tlvDataFilter);

    const ClusterId mClusterId;
    DeviceProxy * mDevice;
    EndpointId mEndpoint;
    chip::Optional<SessionHandle> mSessionHandle;
};

} // namespace Controller
} // namespace chip
