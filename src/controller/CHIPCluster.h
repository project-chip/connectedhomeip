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

#include <controller/CHIPDevice.h>
#include <messaging/ExchangeDelegate.h>

namespace chip {
namespace Controller {

class DLL_EXPORT ClusterBase : public Messaging::ExchangeDelegate
{
public:
    virtual ~ClusterBase() {}

    CHIP_ERROR Associate(Device * device, EndpointId endpoint);

    void Dissociate();

    ClusterId GetClusterId() const { return mClusterId; }

protected:
    ClusterBase(uint16_t cluster) : mClusterId(cluster) {}

    typedef uint16_t (*CommandEncoder)(uint8_t * command, uint16_t maxLen, EndpointId endpoint);

    /**
     * @brief
     *   Send the command, constructed using the given encoder, to the device. Add a callback
     *   handler, that'll be called when the response is received from the device.
     *
     * @param[in] commandEncoder    The function that encodes the command
     * @param[in] maxCmdLen         Maximum length expected for the encoded command
     * @param[in] responseHandler   The handler function that's called on receiving command response
     */
    CHIP_ERROR SendCommand(CommandEncoder commandEncoder, uint16_t maxCmdLen, Callback::Callback<> * responseHandler);

    typedef uint16_t (*RequestEncoder)(uint8_t * request, uint16_t maxLen, EndpointId endpoint, uint16_t minInterval,
                                       uint16_t maxInterval);

    /**
     * @brief
     *   Request attribute reports from the device. The request constructed using the given encoder. Add a callback
     *   handler, that'll be called when the reports are received from the device.
     *
     * @param[in] requestEncoder    The function that encodes the report request
     * @param[in] maxCmdLen         Maximum length expected for the encoded command
     * @param[in] minInterval       The minimum time interval between reports
     * @param[in] maxInterval       The maximum time interval between reports
     * @param[in] reportHandler     The handler function that's called on receiving attribute reports
     *                              The reporting handler continues to be called as long as the callback
     *                              is active. The user can stop the reporting by cancelling the callback.
     *                              Reference: chip::Callback::Cancel()
     */
    CHIP_ERROR RequestAttributeReporting(RequestEncoder requestEncoder, uint16_t maxCmdLen, uint16_t minInterval,
                                         uint16_t maxInterval, Callback::Callback<> * reportHandler);

    const ClusterId mClusterId;
    Device * mDevice;
    EndpointId mEndpoint;
    Messaging::ExchangeContext * mExchangeContext;
    Callback::Callback<> * mResponseHandle;
    Callback::Callback<> * mReportHandle;
};

} // namespace Controller
} // namespace chip
