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

#include <app/InteractionModelEngine.h>
#include <controller/CHIPCluster.h>
#include <protocols/temp_zcl/TempZCL.h>
#include <support/CodeUtils.h>

namespace chip {
namespace Controller {

CHIP_ERROR ClusterBase::Associate(Device * device, EndpointId endpoint)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    // TODO: Check if the device supports mCluster at the requested endpoint

    mDevice   = device;
    mEndpoint = endpoint;

    return err;
}

void ClusterBase::Dissociate()
{
    mDevice = nullptr;
}

CHIP_ERROR ClusterBase::SendCommand(uint8_t seqNum, chip::System::PacketBufferHandle && payload,
                                    Callback::Cancelable * onSuccessCallback, Callback::Cancelable * onFailureCallback)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(mDevice != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(!payload.IsNull(), err = CHIP_ERROR_INTERNAL);

    if (onSuccessCallback != nullptr || onFailureCallback != nullptr)
    {
        mDevice->AddResponseHandler(seqNum, onSuccessCallback, onFailureCallback);
    }

    err = mDevice->SendMessage(Protocols::TempZCL::MsgType::TempZCLRequest, std::move(payload));
    SuccessOrExit(err);

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "Failed in sending cluster command. Err %" CHIP_ERROR_FORMAT, ChipError::FormatError(err));
        mDevice->CancelResponseHandler(seqNum);
    }

    return err;
}

CHIP_ERROR ClusterBase::RequestAttributeReporting(AttributeId attributeId, Callback::Cancelable * onReportCallback)
{
    VerifyOrReturnError(onReportCallback != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    mDevice->AddReportHandler(mEndpoint, mClusterId, attributeId, onReportCallback);

    return CHIP_NO_ERROR;
}

} // namespace Controller
} // namespace chip
