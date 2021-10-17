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
#include <app/util/error-mapping.h>
#include <controller/CHIPDevice.h>
#include <controller/InvokeInteraction.h>

namespace chip {
namespace Controller {

template <typename T>
using CommandResponseSuccessCallback = void(void * context, const T & responseObject);
using CommandResponseFailureCallback = void(void * context, EmberAfStatus status);

class DLL_EXPORT ClusterBase
{
public:
    virtual ~ClusterBase() {}

    CHIP_ERROR Associate(Device * device, EndpointId endpoint);

    void Dissociate();

    ClusterId GetClusterId() const { return mClusterId; }

    /*
     * This function permits sending an invoke request using cluster objects that represent the request and response data payloads.
     *
     * Success and Failure callbacks must be passed in through which the decoded response is provided as well as notification of any
     * failure.
     */
    template <typename RequestDataT, typename ResponseDataT>
    CHIP_ERROR InvokeCommand(const RequestDataT & requestData, void * context,
                             CommandResponseSuccessCallback<ResponseDataT> successCb, CommandResponseFailureCallback failureCb)
    {
        VerifyOrReturnError(mDevice != nullptr, CHIP_ERROR_INCORRECT_STATE);
        ReturnErrorOnFailure(mDevice->LoadSecureSessionParametersIfNeeded());

        auto onSuccessCb = [context, successCb](const app::ConcreteCommandPath & commandPath, const ResponseDataT & responseData) {
            successCb(context, responseData);
        };

        auto onFailureCb = [context, failureCb](Protocols::InteractionModel::Status aIMStatus, CHIP_ERROR aError) {
            failureCb(context, app::ToEmberAfStatus(aIMStatus));
        };

        return InvokeCommandRequest<ResponseDataT>(mDevice->GetExchangeManager(), mDevice->GetSecureSession().Value(), mEndpoint,
                                                   requestData, onSuccessCb, onFailureCb);
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
    Device * mDevice;
    EndpointId mEndpoint;
};

} // namespace Controller
} // namespace chip
