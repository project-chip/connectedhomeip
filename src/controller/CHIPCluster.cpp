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

#include <controller/CHIPCluster.h>
#include <protocols/Protocols.h>

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

CHIP_ERROR ClusterBase::SendCommand(chip::System::PacketBufferHandle payload, Callback::Callback<> * responseHandler)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(mDevice != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(!payload.IsNull(), err = CHIP_ERROR_INTERNAL);

    if (mExchangeContext == nullptr)
    {
        mExchangeContext = mDevice->NewExchange();
        mExchangeContext->SetDelegate(this);
        VerifyOrExit(mExchangeContext == nullptr, err = CHIP_ERROR_NO_MEMORY);
    }

    mResponseHandle = responseHandler;

    err = mExchangeContext->SendMessage(Protocols::kProtocol_InteractionModel, 0, std::move(payload), Messaging::SendFlags{});
    SuccessOrExit(err);

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "Failed in sending cluster command. Err %d", err);
    }

    return err;
}

CHIP_ERROR ClusterBase::RequestAttributeReporting(chip::System::PacketBufferHandle payload, Callback::Callback<> * responseHandler,
                                                  Callback::Callback<> * reportHandler)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    if (mExchangeContext == nullptr)
    {
        mExchangeContext = mDevice->NewExchange();
        mExchangeContext->SetDelegate(this);
        VerifyOrExit(mExchangeContext == nullptr, err = CHIP_ERROR_NO_MEMORY);
    }

    mReportHandle = reportHandler;

    err = mExchangeContext->SendMessage(Protocols::kProtocol_InteractionModel, 0, std::move(payload), Messaging::SendFlags{});
    SuccessOrExit(err);

exit:
    return err;
}

} // namespace Controller
} // namespace chip
