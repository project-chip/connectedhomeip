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

    /**
     * @brief
     *   Send the command, contained into the associate buffer, to the device. Add a callback
     *   handler, that'll be called when the response is received from the device.
     *
     * @param[in] payload           The payload of the encoded command
     * @param[in] responseHandler   The handler function that's called on receiving command response
     */
    CHIP_ERROR SendCommand(chip::System::PacketBufferHandle payload, Callback::Callback<> * responseHandler);

    /**
     * @brief
     *   Request attribute reports from the device. Add a callback
     *   handler, that'll be called when the reports are received from the device.
     *
     * @param[in] payload           The payload of the encoded command
     * @param[in] responseHandler   The handler function that's called on receiving command response
     * @param[in] reportHandler     The handler function that's called on receiving attribute reports
     *                              The reporting handler continues to be called as long as the callback
     *                              is active. The user can stop the reporting by cancelling the callback.
     *                              Reference: chip::Callback::Cancel()
     */
    CHIP_ERROR RequestAttributeReporting(chip::System::PacketBufferHandle payload, Callback::Callback<> * responseHandler,
                                         Callback::Callback<> * reportHandler);

    const ClusterId mClusterId;
    Device * mDevice;
    EndpointId mEndpoint;
    Messaging::ExchangeContext * mExchangeContext;
    Callback::Callback<> * mResponseHandle;
    Callback::Callback<> * mReportHandle;
};

} // namespace Controller
} // namespace chip
