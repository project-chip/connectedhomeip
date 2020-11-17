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

namespace chip {
namespace Controller {

#define SEND_MESSAGE_TO_CLUSTER(err, device, maxLength, message, encoder)                                                          \
    do                                                                                                                             \
    {                                                                                                                              \
        uint16_t encodedLength = 0;                                                                                                \
        VerifyOrExit(device != nullptr, err = CHIP_ERROR_INCORRECT_STATE);                                                         \
        message = System::PacketBuffer::NewWithAvailableSize(maxLength);                                                           \
        VerifyOrExit(message != nullptr, err = CHIP_ERROR_NO_MEMORY);                                                              \
        encodedLength = encoder;                                                                                                   \
        VerifyOrExit(encodedLength != 0, err = CHIP_ERROR_INTERNAL);                                                               \
        message->SetDataLength(encodedLength);                                                                                     \
        err     = mDevice->SendMessage(message);                                                                                   \
        message = nullptr;                                                                                                         \
        SuccessOrExit(err);                                                                                                        \
    exit:                                                                                                                          \
        if (err != CHIP_NO_ERROR)                                                                                                  \
        {                                                                                                                          \
            if (message != nullptr)                                                                                                \
            {                                                                                                                      \
                System::PacketBuffer::Free(message);                                                                               \
            }                                                                                                                      \
            ChipLogError(Controller, "Cluster command encoding failed. Err %d", err);                                              \
        }                                                                                                                          \
    } while (0)

#define SEND_CLUSTER_COMMAND(err, device, maxLength, message, encoder, onCompletion)                                               \
    do                                                                                                                             \
    {                                                                                                                              \
        SEND_MESSAGE_TO_CLUSTER(err, device, maxLength, message, encoder);                                                         \
        if (err == CHIP_NO_ERROR)                                                                                                  \
        {                                                                                                                          \
            mDevice->OnResponse(mEndpoint, mClusterId, onCompletion);                                                              \
        }                                                                                                                          \
    } while (0)

#define REQUEST_CLUSTER_REPORT(err, device, maxLength, message, encoder, onReport)                                                 \
    do                                                                                                                             \
    {                                                                                                                              \
        SEND_MESSAGE_TO_CLUSTER(err, device, maxLength, message, encoder);                                                         \
        if (err == CHIP_NO_ERROR)                                                                                                  \
        {                                                                                                                          \
            mDevice->OnReport(mEndpoint, mClusterId, onReport);                                                                    \
        }                                                                                                                          \
    } while (0)

class DLL_EXPORT ClusterBase
{
public:
    virtual ~ClusterBase() {}

    CHIP_ERROR Associate(Device * device, uint8_t endpoint);

    void Dissociate();

    uint16_t GetClusterId() const { return mClusterId; }

protected:
    ClusterBase(uint16_t cluster) : mClusterId(cluster) {}
    const uint16_t mClusterId;
    Device * mDevice;
    uint8_t mEndpoint;
};

} // namespace Controller
} // namespace chip
